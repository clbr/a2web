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
char *mypath = NULL;

char offline = 0;

struct stats stats;
struct download *downloads = NULL;

static void handle() {

	offline = 0;

	char *gw = getenv("GATEWAY_INTERFACE");
//	char *ct = getenv("CONTENT_TYPE");
//	char *cl = getenv("CONTENT_LENGTH");
//	char *ua = getenv("HTTP_USER_AGENT");
//	char *ck = getenv("HTTP_COOKIE");
	char *qs = getenv("QUERY_STRING");
	mypath = getenv("SCRIPT_NAME");

	if (!gw || strcmp(gw, "CGI/1.1") != 0) {
		error(503);
		return;
	}

	if (qs) {
		if (!strcmp(qs, "downloads")) { // Send just the downloads via ajax

			puts("Content-type: text/html\n");

			getStats();

			if (stats.total > 0 && !offline) {
				downloads = xcalloc(stats.total, sizeof(struct download));

				getDownloads();
			}

			goto outdownloads;

		} else if (!strcmp(qs, "stats")) { // Send just the stats via ajax

			puts("Content-type: text/html\n");

			getStats();

			if (!offline)
				printStats();

			goto out;

		} else if (!strncmp(qs, "add=", 4)) { // Add this url to download

			addDownload(qs + 4);
			puts("Content-type: text/plain\n");

			goto out;
		}
	}

	puts("Content-type: text/html\n");

	printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
		"<html><head>\n"
		"\t<title>%s v%s</title>\n"
		"\t<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n"
		"\t<link rel=stylesheet href=\"%s/style.css\" type=\"text/css\">\n"
		"</head>\n"
		"<body>\n\n", NAME, VERSION, themedir);

	printf("<div id=logo>Running %s v%s</div>\n\n", NAME, VERSION);

	getVersion();

	if (!offline) {

		getStats();


		printf("<br><br><div id=stats>\n");
		printStats();
		printf("</div><p>\n\n");

		printf("<div id=downloads>\n");
		if (stats.total > 0) {
			downloads = xcalloc(stats.total, sizeof(struct download));

			getDownloads();
		}
		printf("</div>\n\n");

		printOptions();

		printAdditions();

		printJS();
	}

	printf("</body></html>\n");


	unsigned i;

	outdownloads:
	for (i = 0; i < stats.total; i++) {
		free((char *) downloads[i].gid);
		free((char *) downloads[i].parent);
		free((char *) downloads[i].status);
		free((char *) downloads[i].uris);
	}

	out:

	free((char *) version);
	version = NULL;

	free(downloads);
	downloads = NULL;
}

int main() {

	server = strdup("http://localhost:6800/rpc");
	themedir = strdup("a2web/crystal");

	readConfig();

	initxml();

	while (FCGI_Accept() >= 0)
		handle();

	deinitxml();

	free(server);
	free(user);
	free(pw);
	free(themedir);

	return 0;
}
