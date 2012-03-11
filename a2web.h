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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcgi_stdio.h>

#define NAME "a2web.cgi"
#define VERSION "0.0"

void error(const int num);
int checkxml();
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);

void getVersion();
void getStats();
void getDownloads();
void printOptions();
void printAdditions();
void addDownload(const char *);

void initxml();
void deinitxml();

void printDownloads();
void printStats();

void printJS();

void readConfig();

void handle_query(const char *);

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
	const char *uris;
	const char *parent;

	unsigned seeders;
	unsigned connections;
};

extern const char *cwd;
extern const char *version;
extern char *server, *user, *pw, *themedir, *mypath;
extern char offline;

extern struct stats stats;
extern struct download *downloads;

#endif
