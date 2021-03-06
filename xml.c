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

#include <regex.h>

static xmlrpc_env xenv;
static xmlrpc_env *x = &xenv;
static xmlrpc_server_info *srv;

static int checkxml() {

	if (x->fault_occurred) {
		printf("%s\n", x->fault_string);

		return 1;
	}

	return 0;
}

void getVersion() {

	xmlrpc_value *res, *tmp = NULL;
	xmlrpc_value *a = xmlrpc_array_new(x); // empty array, because xmlrpc-c sucks

	res = xmlrpc_client_call_server_params(x, srv, "aria2.getVersion",
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

	res = xmlrpc_client_call_server_params(x, srv, "aria2.getGlobalStat",
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

	res = xmlrpc_client_call_server_params(x, srv, "aria2.tellActive", a);

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

	res = xmlrpc_client_call_server_params(x, srv, "aria2.tellWaiting", a);
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

	res = xmlrpc_client_call_server_params(x, srv, "aria2.tellStopped", a);
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

	srv = xmlrpc_server_info_new(x, server);
	checkxml();

	if (user && pw) {
		xmlrpc_server_info_set_user(x, srv, user, pw);
		xmlrpc_server_info_allow_auth_digest(x, srv);
		xmlrpc_server_info_allow_auth_basic(x, srv);
	}
}

void deinitxml() {

	xmlrpc_server_info_free(srv);

	xmlrpc_env_clean(x);
	xmlrpc_client_cleanup();

}

void addDownload(const char *url) {

	xmlrpc_value *xml_url, *res;
	xmlrpc_value *a = xmlrpc_array_new(x);
	xmlrpc_value *senda = xmlrpc_array_new(x);

	xml_url = xmlrpc_string_new(x, url);
	xmlrpc_array_append_item(x, a, xml_url);
	xmlrpc_array_append_item(x, senda, a);

	res = xmlrpc_client_call_server_params(x, srv, "aria2.addUri", senda);
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

	res = xmlrpc_client_call_server_params(x, srv, "aria2.purgeDownloadResult", a);
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
		res = xmlrpc_client_call_server_params(x, srv, "aria2.unpause", a);
	else
		res = xmlrpc_client_call_server_params(x, srv, "aria2.pause", a);
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

	res = xmlrpc_client_call_server_params(x, srv, "aria2.remove", a);
	checkxml();

	res2 = xmlrpc_client_call_server_params(x, srv, "aria2.removeDownloadResult", a);
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

	res = xmlrpc_client_call_server_params(x, srv, "aria2.changePosition", a);
	checkxml();

	xmlrpc_DECREF(xml_gid);
	xmlrpc_DECREF(xml_how);
	xmlrpc_DECREF(xml_change);
	xmlrpc_DECREF(a);

	if (x->fault_occurred)
		return;

	xmlrpc_DECREF(res);
}

void getSettings() {

	printf("<button onclick='hidesettings()' style='float: right;'>\n"
		"<img src=\"%s/cancel.png\" width=32 height=32>\n</button>\n",
		themedir);

	xmlrpc_value *res, *tmp;
	xmlrpc_value *a = xmlrpc_array_new(x);
	const char *str;

	res = xmlrpc_client_call_server_params(x, srv, "aria2.getGlobalOption",
					a);
	xmlrpc_DECREF(a);

	if (xenv.fault_occurred) {

		printf("<div id=error>Aria2 doesn't seem to be running: %s</div>\n",
			xenv.fault_string);

		offline = 1;
		return;
	}

	printf("<h2>Preferences</h2>\n<hr width='50%%'><br>");

	printf("<table border=0 id=settingstable>\n");

	// Start the options

	xmlrpc_struct_find_value(x, res, "dir", &tmp);
	if (tmp) {
		xmlrpc_read_string(x, tmp, &str);
		printf("<tr><td>Download directory:</td>"
			"<td><input type=text size=30 value='%s' id=dir onblur='updatedir()'></td></tr>",
			str);

		free((char *) str);
		xmlrpc_DECREF(tmp);
	}

	xmlrpc_struct_find_value(x, res, "disable-ipv6", &tmp);
	if (tmp) {
		xmlrpc_read_string(x, tmp, &str);

		int checked = 0;
		if (!strcmp(str, "true")) checked = 1;

		printf("<tr><td>Disable ipv6:</td>"
			"<td><input type=checkbox %s id=tickv6 disabled></td></tr>",
			checked ? "checked" : "");

		free((char *) str);
		xmlrpc_DECREF(tmp);
	}

	xmlrpc_struct_find_value(x, res, "follow-torrent", &tmp);
	if (tmp) {
		xmlrpc_read_string(x, tmp, &str);

		int fol = 0; // 0 false, 1 true, 2 mem
		if (!strcmp(str, "true")) fol = 1;
		else if (!strcmp(str, "mem")) fol = 2;

		printf("<tr><td>Follow torrents:</td>"
			"<td><select id=flwtorrent onchange='updateflwent()'>"
			"<option value=false %s>Only download the .torrent</option>"
			"<option value=true %s>Download the files the .torrent points to</option>"
			"<option value=mem %s>Download what is pointed to, and don't save the .torrent</option>"
			"</select></td></tr>",
			!fol ? "selected" : "",
			fol == 1 ? "selected" : "",
			fol == 2 ? "selected" : "");

		free((char *) str);
		xmlrpc_DECREF(tmp);
	}

	xmlrpc_struct_find_value(x, res, "listen-port", &tmp);
	if (tmp) {
		xmlrpc_read_string(x, tmp, &str);
		printf("<tr><td>Bittorrent ports:</td>"
			"<td><input type=text size=30 value='%s' onblur='updateentports()' id=entports></td></tr>",
			str);

		free((char *) str);
		xmlrpc_DECREF(tmp);
	}

	xmlrpc_struct_find_value(x, res, "max-download-limit", &tmp);
	if (tmp) {
		xmlrpc_read_string(x, tmp, &str);
		printf("<tr><td>Per-download limit:</td>"
			"<td><input type=text size=30 value='%s' onblur='updatedllimit()' id=dllimit></td></tr>",
			str);

		free((char *) str);
		xmlrpc_DECREF(tmp);
	}

	xmlrpc_struct_find_value(x, res, "max-overall-download-limit", &tmp);
	if (tmp) {
		xmlrpc_read_string(x, tmp, &str);
		printf("<tr><td>Overall download limit:</td>"
			"<td><input type=text size=30 value='%s' onblur='updatealldllimit()' id=alldl></td></tr>",
			str);

		free((char *) str);
		xmlrpc_DECREF(tmp);
	}

	xmlrpc_struct_find_value(x, res, "max-upload-limit", &tmp);
	if (tmp) {
		xmlrpc_read_string(x, tmp, &str);
		printf("<tr><td>Per-upload limit:</td>"
			"<td><input type=text size=30 value='%s' onblur='updateuplimit()' id=uplimit></td></tr>",
			str);

		free((char *) str);
		xmlrpc_DECREF(tmp);
	}

	xmlrpc_struct_find_value(x, res, "max-overall-upload-limit", &tmp);
	if (tmp) {
		xmlrpc_read_string(x, tmp, &str);
		printf("<tr><td>Overall upload limit:</td>"
			"<td><input type=text size=30 value='%s' onblur='updatealluplimit()' id=allup></td></tr>",
			str);

		free((char *) str);
		xmlrpc_DECREF(tmp);
	}

	xmlrpc_struct_find_value(x, res, "seed-ratio", &tmp);
	if (tmp) {
		xmlrpc_read_string(x, tmp, &str);
		printf("<tr><td>Seed ratio:</td>"
			"<td><input type=text size=30 value='%s' onblur='updateseedrt()' id=seedrt></td></tr>",
			str);

		free((char *) str);
		xmlrpc_DECREF(tmp);
	}

	xmlrpc_struct_find_value(x, res, "seed-time", &tmp);
	if (tmp) {
		xmlrpc_read_string(x, tmp, &str);
		printf("<tr><td>Seed time (min):</td>"
			"<td><input type=text size=30 value='%s' onblur='updateseedtm()' id=seedtm></td></tr>",
			str);

		free((char *) str);
		xmlrpc_DECREF(tmp);
	} else {
		printf("<tr><td>Seed time (min):</td>"
			"<td><input type=text size=30 value='unset' onblur='updateseedtm()' id=seedtm>"
			"</td></tr>");
	}

	// End the options

	printf("</table>\n");

	puts("<br><br><br>");
	printf("<h2>All options except overall speed limits only affect new downloads.</h2>\n");


#if 0
	unsigned len = xmlrpc_struct_size(x, res);
	printf("It has %d members\n", len);

	/*
		dir
		disable-ipv6
		follow-torrent
		listen-port
		max-download-limit
		max-overall-download-limit
		max-upload-limit
		max-overall-upload-limit
		seed-ratio
		seed-time?
	*/

	unsigned i;
	for (i = 0; i < len; i++) {
		xmlrpc_value *key, *val;

		xmlrpc_struct_read_member(x, res, i, &key, &val);
		checkxml();

		const char *ckey, *cval;

		xmlrpc_read_string(x, key, &ckey);
		checkxml();
		xmlrpc_read_string(x, val, &cval);
		checkxml();

		printf("%s = %s<br>\n", ckey, cval);
	}
#endif
	xmlrpc_DECREF(res);
}


void setOption(const char *in) {

	// First check it's well-formed
	regex_t reg;
	int i = regcomp(&reg, "^[[:lower:][:digit:]_-]*=[[:print:]]*$", REG_NOSUB);

	if (i)
		printf("Failed to compile regex, %d\n", i);

	if (regexec(&reg, in, 0, NULL, 0)) {

		printf("Ill-formed input: %s\n", in);
		regfree(&reg);
		return;
	}
	regfree(&reg);

	// It's ok, handle it

	char *opt, *val;
	opt = strdup(in);

	char *tmp = strchr(opt, '=');
	*tmp = '\0';
	val = tmp + 1;

	// Separated. XML time
	xmlrpc_value *xml_opt, *res, *xml_val;
	xmlrpc_value *a = xmlrpc_array_new(x);
	xmlrpc_value *s = xmlrpc_struct_new(x);

	xml_opt = xmlrpc_string_new(x, opt);
	xml_val = xmlrpc_string_new(x, val);

	xmlrpc_struct_set_value_v(x, s, xml_opt, xml_val);

	xmlrpc_array_append_item(x, a, s);

	res = xmlrpc_client_call_server_params(x, srv, "aria2.changeGlobalOption", a);
	checkxml();

	xmlrpc_DECREF(xml_val);
	xmlrpc_DECREF(xml_opt);
	xmlrpc_DECREF(a);
	xmlrpc_DECREF(s);

	if (x->fault_occurred)
		return;

	xmlrpc_DECREF(res);

	free(opt);
}
