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


// Handle one call from the client
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
		if (handle_query(qs))
			goto out;
	}

	htmlpage();

	printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
		"<html><head>\n"
		"\t<title>%s v%s</title>\n"
		"\t<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n"
		"\t<link rel=stylesheet href=\"%s/style.css\" type=\"text/css\">\n"
		"</head>\n"
		"<body>\n\n", NAME, VERSION, themedir);

	printf("<div id=logo>"
		"<img src=\"%s/logo_small.png\" alt=logo width=250 height=218><br>"
		"Running %s v%s."
		"</div>\n\n",
		themedir, NAME, VERSION);

	getVersion();

	if (!offline) {

		getStats();


		printf("<div id=stats>\n");
		printStats();
		printf("</div>\n\n");

		printf("<div id=downloads>\n");
		if (stats.total > 0) {
			downloads = xcalloc(stats.total, sizeof(struct download));

			getDownloads();
		}
		printf("</div>\n\n");

		printOptions();

		printAdditions();

		printSettings();

		printJS();
	}

	printf("</body></html>\n");


	unsigned i;

	out:

	if (downloads) for (i = 0; i < stats.total; i++) {
		free((char *) downloads[i].gid);
		free((char *) downloads[i].parent);
		free((char *) downloads[i].status);
		free((char *) downloads[i].uris);
	}

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
