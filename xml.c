/*
a2web.cgi
Copyright (C) 2012 Lauri Kasanen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 3
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "a2web.h"

#define bufsize 80

static xmlrpc_env xenv;
static xmlrpc_env *x = &xenv;

void getVersion() {

	xmlrpc_value *res, *tmp = NULL;
	xmlrpc_value *a = xmlrpc_array_new(x); // empty array, because xmlrpc-c sucks

	res = xmlrpc_client_call_params(x, server, "aria2.getVersion",
					a);

	xmlrpc_DECREF(a);

	if (xenv.fault_occurred) {
		printf("<div id=error>Aria2 doesn't seem to be running: %s</div>",
			xenv.fault_string);
		offline = 1;
		return;
	}

	xmlrpc_struct_find_value(x, res, "version", &tmp);
	if (tmp) {
		xmlrpc_read_string(x, tmp, &version);
		checkxml();
		printf("\n<h3 id=version>Version %s</h3><p>", version);
		xmlrpc_DECREF(tmp);
	}

	xmlrpc_DECREF(res);
}

unsigned long long xmltoull(xmlrpc_value *tmp) {

	if (!tmp)
		return 0;

	const char *str;
	unsigned long long val;

	xmlrpc_read_string(x, tmp, &str);
	val = strtoull(str, NULL, 10);

	free((char *) str);
	xmlrpc_DECREF(tmp);

	return val;
}

unsigned long xmltoul(xmlrpc_value *tmp) {

	if (!tmp)
		return 0;

	const char *str;
	unsigned long val;

	xmlrpc_read_string(x, tmp, &str);
	val = strtoul(str, NULL, 10);

	free((char *) str);
	xmlrpc_DECREF(tmp);

	return val;
}

static void printStats() {

	printf(
		"\n\n<div id=stats>\n"
		"\tTotal download speed: <span class=number>%llu</span> kB/s<p>\n"
		"\tTotal upload speed: <span class=number>%llu</span> kB/s<p>\n"

		"\tTotal %u downloads, active/waiting/stopped: "
		"<span class=number>%u/%u/%u</span>\n"
		"</div>\n\n",

		stats.down, stats.up, stats.total,
		stats.active, stats.waiting, stats.stopped);
}

void getStats() {

	xmlrpc_value *res, *tmp;
	xmlrpc_value *a = xmlrpc_array_new(x);

	res = xmlrpc_client_call_params(x, server, "aria2.getGlobalStat",
					a);
	checkxml();

	xmlrpc_struct_find_value(x, res, "downloadSpeed", &tmp);
	stats.down = xmltoull(tmp) / 1024;

	xmlrpc_struct_find_value(x, res, "uploadSpeed", &tmp);
	stats.up = xmltoull(tmp) / 1024;

	xmlrpc_struct_find_value(x, res, "numActive", &tmp);
	stats.active = xmltoul(tmp);

	xmlrpc_struct_find_value(x, res, "numWaiting", &tmp);
	stats.waiting = xmltoul(tmp);

	xmlrpc_struct_find_value(x, res, "numStopped", &tmp);
	stats.stopped = xmltoul(tmp);

	stats.total = stats.stopped + stats.waiting + stats.active;

	xmlrpc_DECREF(a);
	xmlrpc_DECREF(res);

	printStats();
}

static void printDownloads() {

	printf("<div id=downloads>\n");

	unsigned i;
	for (i = 0; i < stats.total; i++) {

		struct download *cur = &downloads[i];

		char upped[bufsize] = "", seeded[bufsize] = "";

		if (cur->uploaded)
			snprintf(upped, bufsize, "uploaded %llu kB, ", cur->uploaded);

		if (cur->seeders)
			snprintf(seeded, bufsize, "seeds/peers %u/%u", cur->seeders,
				 cur->connections);

		upped[bufsize - 1] = '\0';
		seeded[bufsize - 1] = '\0';


		printf("\t ..progress bar... ");
		printf("%llu/%llu kB, "
			"%s"
			"speed %llu/%llu, "
			"%s"
			"<br>\n",

			cur->completed, cur->length,
			upped,
			cur->down, cur->up,
			seeded);
	}

	printf("</div>\n\n");
}

static void parseDownload(xmlrpc_value *in) {

	// Find the first empty slot
	struct download *cur = downloads;
	while (cur->gid)
		cur++;

	// Start parsing
	xmlrpc_value *val;

	xmlrpc_struct_find_value(x, in, "totalLength", &val);
	cur->length = xmltoull(val) / 1024;

	xmlrpc_struct_find_value(x, in, "completedLength", &val);
	cur->completed = xmltoull(val) / 1024;

	xmlrpc_struct_find_value(x, in, "uploadedLength", &val);
	cur->uploaded = xmltoull(val) / 1024;

	xmlrpc_struct_find_value(x, in, "uploadSpeed", &val);
	cur->up = xmltoull(val) / 1024;

	xmlrpc_struct_find_value(x, in, "downloadSpeed", &val);
	cur->down = xmltoull(val) / 1024;

	xmlrpc_struct_find_value(x, in, "gid", &val);
	if (val)
		xmlrpc_read_string(x, val, &cur->gid);
	else
		cur->gid = strdup("unknown");

	xmlrpc_struct_find_value(x, in, "status", &val);
	if (val)
		xmlrpc_read_string(x, val, &cur->status);
	else
		cur->status = strdup("unknown");

	xmlrpc_struct_find_value(x, in, "numSeeders", &val);
	cur->seeders = xmltoul(val);

	xmlrpc_struct_find_value(x, in, "connections", &val);
	cur->connections = xmltoul(val);
}

void getDownloads() {

	xmlrpc_value *res, *tmp;
	xmlrpc_value *a = xmlrpc_array_new(x);

	unsigned size = 0, i;

	// Active

	res = xmlrpc_client_call_params(x, server, "aria2.tellActive", a);
	checkxml();

	if (res) {
		size = xmlrpc_array_size(x, res);

		for (i = 0; i < size; i++) {

			xmlrpc_array_read_item(x, res, i, &tmp);

			if (tmp)
				parseDownload(tmp);
		}
	}

	xmlrpc_DECREF(res);


	// Waiting

	xmlrpc_value *offset, *num;
	offset = xmlrpc_int_new(x, 0);
	num = xmlrpc_int_new(x, stats.waiting);
	xmlrpc_array_append_item(x, a, offset);
	xmlrpc_array_append_item(x, a, num);

	res = xmlrpc_client_call_params(x, server, "aria2.tellWaiting", a);
	checkxml();

	if (res) {
		size = xmlrpc_array_size(x, res);

		for (i = 0; i < size; i++) {

			xmlrpc_array_read_item(x, res, i, &tmp);

			if (tmp)
				parseDownload(tmp);
		}
	}

	xmlrpc_DECREF(res);
	xmlrpc_DECREF(a);
	xmlrpc_DECREF(num);


	// Stopped

	num = xmlrpc_int_new(x, stats.stopped);
	a = xmlrpc_array_new(x);
	xmlrpc_array_append_item(x, a, offset);
	xmlrpc_array_append_item(x, a, num);

	res = xmlrpc_client_call_params(x, server, "aria2.tellStopped", a);
	checkxml();

	if (res) {
		size = xmlrpc_array_size(x, res);

		for (i = 0; i < size; i++) {

			xmlrpc_array_read_item(x, res, i, &tmp);

			if (tmp)
				parseDownload(tmp);
		}
	}

	xmlrpc_DECREF(res);
	xmlrpc_DECREF(a);
	xmlrpc_DECREF(num);
	xmlrpc_DECREF(offset);

	printDownloads();
}

void initxml() {

	xmlrpc_env_init(x);

	xmlrpc_client_init2(x, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0);
	checkxml();

}

void deinitxml() {

	xmlrpc_env_clean(x);
	xmlrpc_client_cleanup();

}

int checkxml() {

	if (x->fault_occurred) {
		printf("%s\n", x->fault_string);

		return 1;
	}

	return 0;
}
