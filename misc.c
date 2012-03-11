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

void error(const int num) {

	// This bypasses busybox httpd, so do CRLF

	printf("Status: %d\r\n", num);
	printf("Content-type: text/html\r\n\r\n");

	printf("<h1>Error %d</h1>\n", num);
}

void *xcalloc(size_t nmemb, size_t size) {

	void *out = calloc(nmemb, size);
	if (!out)
		exit(2);

	return out;
}

void *xrealloc(void *ptr, size_t size) {

	void *out = realloc(ptr, size);
	if (!out)
		exit(2);

	return out;
}

void plainpage() {
	puts("Content-type: text/plain\n");
}

void htmlpage() {
	puts("Content-type: text/html\n");
}
