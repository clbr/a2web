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

void getVersion() {

	xmlrpc_value *res, *tmp = NULL;

	res = xmlrpc_client_call_params(x, server, "aria2.getVersion",
					xmlrpc_array_new(x));
	if (xenv.fault_occurred) {
		printf("Aria2 doesn't seem to be running: %s", xenv.fault_string);
		offline = 1;
		return;
	}

	xmlrpc_struct_find_value(x, res, "version", &tmp);
	if (tmp) {
		xmlrpc_read_string(x, tmp, &version);
		checkxml();
		printf("Version %s<p>", version);
	}

}

void getStats() {

}

void getDownloads() {

}
