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

#ifndef A2WEB_H
#define A2WEB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcgi_stdio.h>

#include <xmlrpc.h>
#include <xmlrpc_client.h>

#define NAME "a2web.cgi"
#define VERSION "0.0"

void error(int num);
int checkxml();

void handle();

void getVersion();
void getStats();
void getDownloads();

unsigned long long xmltoull(xmlrpc_value *tmp);
unsigned long xmltoul(xmlrpc_value *tmp);

struct stats {
	unsigned long long down;
	unsigned long long up;
	unsigned active;
	unsigned waiting;
	unsigned stopped;

	unsigned total;
};

struct download {
	unsigned long long length;
	unsigned long long completed;
	unsigned long long uploaded;

	unsigned long long down;
	unsigned long long up;

	const char *gid;
	const char *status;

	unsigned seeders;
	unsigned connections;
};

extern const char *cwd;
extern const char *version;
extern const char *server;
extern char offline;
extern xmlrpc_env xenv;
extern xmlrpc_env *x;

extern struct stats stats;
extern struct download *downloads;

#endif
