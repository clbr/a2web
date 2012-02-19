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

const char *cwd;
const char *version;
const char *server = "http://localhost:6800/rpc";

char offline = 0;
xmlrpc_env xenv;
xmlrpc_env *x = &xenv;

struct stats stats;
struct download *downloads = NULL;

void handle() {

	char *gw = getenv("GATEWAY_INTERFACE");
//	char *ct = getenv("CONTENT_TYPE");
//	char *cl = getenv("CONTENT_LENGTH");
//	char *ua = getenv("HTTP_USER_AGENT");
//	char *ck = getenv("HTTP_COOKIE");
	char *qs = getenv("QUERY_STRING");

	if (!gw || strcmp(gw, "CGI/1.1") != 0) {
		error(503);
		return;
	}

	xmlrpc_env_init(x);

	puts("Content-type: text/html\n");

	printf("<html><body>moi %s<p>", qs);


	xmlrpc_client_init2(x, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0);
	checkxml();

	getVersion();

	if (!offline) {
		getStats();

		if (stats.total > 0) {
			downloads = calloc(stats.total, sizeof(struct download));

			getDownloads();
		}
	}

	printf("</body></html>\n");

	xmlrpc_env_clean(x);
	xmlrpc_client_cleanup();

	free((char *) version);
	free(downloads);
}

int main() {

	while (FCGI_Accept() >= 0)
		handle();

	return 0;
}
