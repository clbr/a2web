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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <regex.h>
#include <string.h>

#define bufsize 256

extern char *server, *user, *pw, *themedir;

static void settings(const char *arg, const char *param) {

	if (!strcmp(arg, "server")) {
		free(server);
		server = strdup(param);
	}

	if (!strcmp(arg, "user")) {
		free(user);
		user = strdup(param);
	}

	if (!strcmp(arg, "password")) {
		free(pw);
		pw = strdup(param);
	}

	if (!strcmp(arg, "themedir")) {
		free(themedir);
		themedir = strdup(param);
	}

}

static void readLine(char *in, const regex_t *reg) {

	if (regexec(reg, in, 0, NULL, 0)) {

		printf("Error on line: %s\n", in);
		return;
	}

	char *p = in;
	int i;

	while (*p != '=') p++;
	*p = '\0';
	p++;
	while (isspace(*p)) p++;

	char *arg = strdup(in);
	char *param = strdup(p);
	for (i = 0; arg[i]; i++) {
		if (isspace(arg[i])) {
			arg[i] = '\0';
			break;
		}
	}

	settings(arg, param);

	free(arg);
	free(param);
}

void readConfig() {

	const char name[] = "a2web.conf";

	if (access(name, R_OK))
		return;

	char buf[bufsize];

	FILE *f = fopen(name, "r");

	if (!f)
		return;

	regex_t reg;
	int i = regcomp(&reg, "^[[:alpha:][:space:]]*=[[:print:]]*$", REG_NOSUB);

	if (i)
		printf("Failed to compile regex, %d\n", i);

	while (fgets(buf, bufsize, f)) {

		char *ptr = buf;

		while (isspace(*ptr)) ptr++;

		if (*ptr == '#')
			continue;

		for (i = 0; ptr[i]; i++)
			if (ptr[i] == '\n') ptr[i] = '\0';

		readLine(ptr, &reg);
	}

	regfree(&reg);
	fclose(f);
}
