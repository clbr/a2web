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

#include <xmlrpc.h>
#include <xmlrpc_client.h>

static xmlrpc_env xenv;
static xmlrpc_env *x = &xenv;

void getVersion() {

	xmlrpc_value *res, *tmp = NULL;
	xmlrpc_value *a = xmlrpc_array_new(x); // empty array, because xmlrpc-c sucks

	res = xmlrpc_client_call_params(x, server, "aria2.getVersion",
					a);

	xmlrpc_DECREF(a);

	if (xenv.fault_occurred) {

		printf("<div id=error>Aria2 doesn't seem to be running: %s</div>\n",
			xenv.fault_string);

		offline = 1;
		return;
	}

	xmlrpc_struct_find_value(x, res, "version", &tmp);
	if (tmp) {

		xmlrpc_read_string(x, tmp, &version);
		checkxml();

		printf("\n<div id=version>Connected to aria2 %s.</div>\n", version);

		xmlrpc_DECREF(tmp);
	}

	xmlrpc_DECREF(res);
}

static unsigned long long xmltoull(xmlrpc_value *tmp) {

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

static unsigned long xmltoul(xmlrpc_value *tmp) {

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

void getStats() {

	xmlrpc_value *res, *tmp;
	xmlrpc_value *a = xmlrpc_array_new(x);

	res = xmlrpc_client_call_params(x, server, "aria2.getGlobalStat",
					a);
	xmlrpc_DECREF(a);

	if (xenv.fault_occurred) {

		printf("<div id=error>Aria2 doesn't seem to be running: %s</div>\n",
			xenv.fault_string);

		offline = 1;
		return;
	}

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

	xmlrpc_DECREF(res);
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
	if (val) {
		xmlrpc_read_string(x, val, &cur->gid);
		xmlrpc_DECREF(val);
	} else
		cur->gid = strdup("unknown");

	xmlrpc_struct_find_value(x, in, "belongsTo", &val);
	if (val) {
		xmlrpc_read_string(x, val, &cur->parent);
		xmlrpc_DECREF(val);
	}

	xmlrpc_struct_find_value(x, in, "status", &val);
	if (val) {
		xmlrpc_read_string(x, val, &cur->status);
		xmlrpc_DECREF(val);
	} else
		cur->status = strdup("unknown");

	xmlrpc_struct_find_value(x, in, "numSeeders", &val);
	cur->seeders = xmltoul(val);

	xmlrpc_struct_find_value(x, in, "connections", &val);
	cur->connections = xmltoul(val);

	// I shiver in horror when I look at this.
	xmlrpc_struct_find_value(x, in, "files", &val);
	if (val) {
		xmlrpc_value *arr;
		xmlrpc_array_read_item(x, val, 0, &arr);

		if (!arr)
			return;

		xmlrpc_value *xpath;
		xmlrpc_struct_find_value(x, arr, "path", &xpath);

		if (!xpath)
			return;

		const char *path;
		xmlrpc_read_string(x, xpath, &path);
		xmlrpc_DECREF(xpath);

		xmlrpc_value *uris;
		xmlrpc_struct_find_value(x, arr, "uris", &uris);

		if (!uris)
			return;

		unsigned size = 0, i;
		size = xmlrpc_array_size(x, uris);

		for (i = 0; i < size; i++) {

			xmlrpc_value *uri, *string;
			xmlrpc_array_read_item(x, uris, i, &uri);

			if (!uri)
				continue;

			xmlrpc_struct_find_value(x, uri, "uri", &string);

			if (!string)
				continue;

			const char *thisuri;
			xmlrpc_read_string(x, string, &thisuri);

			if (!cur->uris)
				cur->uris = thisuri;
			else {
				if (strstr(cur->uris, thisuri)) {
					free((char *) thisuri);
					goto out;
				}

				const size_t len = strlen(cur->uris) + 5 + strlen(thisuri);

				cur->uris = xrealloc((char *) cur->uris, len);
				strcat((char *) cur->uris, ", ");
				strcat((char *) cur->uris, thisuri);
				free((char *) thisuri);
			}

			out:
			xmlrpc_DECREF(uri);
			xmlrpc_DECREF(string);
		}

		if (!cur->uris)
			asprintf((char **) &cur->uris, "Torrent: %s...", path);

		free((char *) path);

		xmlrpc_DECREF(uris);
		xmlrpc_DECREF(arr);
		xmlrpc_DECREF(val);
	}
}

void getDownloads() {

	xmlrpc_value *res, *tmp;
	xmlrpc_value *a = xmlrpc_array_new(x);

	unsigned size = 0, i;

	// Active

	res = xmlrpc_client_call_params(x, server, "aria2.tellActive", a);

	if (xenv.fault_occurred) {

		printf("<div id=error>Aria2 doesn't seem to be running: %s</div>\n",
			xenv.fault_string);

		offline = 1;
		return;
	}

	if (res) {
		size = xmlrpc_array_size(x, res);

		for (i = 0; i < size; i++) {

			xmlrpc_array_read_item(x, res, i, &tmp);

			if (tmp)
				parseDownload(tmp);

			xmlrpc_DECREF(tmp);
		}

		xmlrpc_DECREF(res);
	}


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

			xmlrpc_DECREF(tmp);
		}

		xmlrpc_DECREF(res);
	}

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

			xmlrpc_DECREF(tmp);
		}

		xmlrpc_DECREF(res);
	}

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

void addDownload(const char *url) {

	xmlrpc_value *xml_url, *res;
	xmlrpc_value *a = xmlrpc_array_new(x);
	xmlrpc_value *senda = xmlrpc_array_new(x);

	xml_url = xmlrpc_string_new(x, url);
	xmlrpc_array_append_item(x, a, xml_url);
	xmlrpc_array_append_item(x, senda, a);

	res = xmlrpc_client_call_params(x, server, "aria2.addUri", senda);
	checkxml();

	xmlrpc_DECREF(xml_url);
	xmlrpc_DECREF(a);
	xmlrpc_DECREF(senda);

	if (x->fault_occurred)
		return;

	xmlrpc_DECREF(res);
}

// Remove stopped/error/completed downloads from the list
void cleanDownload() {

	xmlrpc_value *a = xmlrpc_array_new(x), *res;

	res = xmlrpc_client_call_params(x, server, "aria2.purgeDownloadResult", a);
	checkxml();

	xmlrpc_DECREF(a);

	if (x->fault_occurred)
		return;

	xmlrpc_DECREF(res);
}

void pauseDownload(const char *gid, const int unpause) {

	xmlrpc_value *xml_gid, *res;
	xmlrpc_value *a = xmlrpc_array_new(x);

	xml_gid = xmlrpc_string_new(x, gid);
	xmlrpc_array_append_item(x, a, xml_gid);

	if (unpause)
		res = xmlrpc_client_call_params(x, server, "aria2.unpause", a);
	else
		res = xmlrpc_client_call_params(x, server, "aria2.pause", a);
	checkxml();

	xmlrpc_DECREF(xml_gid);
	xmlrpc_DECREF(a);

	if (x->fault_occurred)
		return;

	xmlrpc_DECREF(res);
}

void removeDownload(const char *gid) {

	xmlrpc_value *xml_gid, *res, *res2;
	xmlrpc_value *a = xmlrpc_array_new(x);

	xml_gid = xmlrpc_string_new(x, gid);
	xmlrpc_array_append_item(x, a, xml_gid);

	res = xmlrpc_client_call_params(x, server, "aria2.remove", a);
	checkxml();

	res2 = xmlrpc_client_call_params(x, server, "aria2.removeDownloadResult", a);
	checkxml();

	xmlrpc_DECREF(xml_gid);
	xmlrpc_DECREF(a);

	if (x->fault_occurred)
		return;

	xmlrpc_DECREF(res);
	xmlrpc_DECREF(res2);
}

// Change the priority of this waiting download
void prioDownload(const char *gid, const int change) {

	xmlrpc_value *xml_gid, *res, *xml_change, *xml_how;
	xmlrpc_value *a = xmlrpc_array_new(x);

	xml_gid = xmlrpc_string_new(x, gid);
	xml_how = xmlrpc_string_new(x, "POS_CUR");
	xml_change = xmlrpc_int_new(x, change);

	xmlrpc_array_append_item(x, a, xml_gid);
	xmlrpc_array_append_item(x, a, xml_change);
	xmlrpc_array_append_item(x, a, xml_how);

	res = xmlrpc_client_call_params(x, server, "aria2.changePosition", a);
	checkxml();

	xmlrpc_DECREF(xml_gid);
	xmlrpc_DECREF(xml_how);
	xmlrpc_DECREF(xml_change);
	xmlrpc_DECREF(a);

	if (x->fault_occurred)
		return;

	xmlrpc_DECREF(res);
}
