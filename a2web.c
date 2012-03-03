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
char *server = NULL, *user = NULL, *pw = NULL, *themedir = NULL;

char offline = 0;

struct stats stats;
struct download *downloads = NULL;

static void handle() {

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

	puts("Content-type: text/html\n");

	printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
		"<html><head>\n"
		"\t<title>%s v%s</title>\n"
		"\t<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">"
		"</head>\n"
		"<body>\n\n", NAME, VERSION);

	printf("moi %s<p>\n", qs);

	initxml();

	getVersion();

	if (!offline) {
		getStats();

		if (stats.total > 0) {
			downloads = xcalloc(stats.total, sizeof(struct download));

			getDownloads();
		}

		printOptions();
	}

	printf("</body></html>\n");

	deinitxml();

	free((char *) version);
	free(downloads);
}

int main() {

	server = strdup("http://localhost:6800/rpc");
	themedir = strdup("a2web");

	readConfig();

	while (FCGI_Accept() >= 0)
		handle();

	free(server);
	free(user);
	free(pw);
	free(themedir);

	return 0;
}
