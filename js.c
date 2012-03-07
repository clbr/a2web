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

void printJS() {

	printf("<script type=\"text/javascript\">\n"

		"setInterval('upd_downloads()', 1800);\n"
		"setInterval('upd_stats()', 5000);\n"

		"function upd_downloads() {\n"
		"	var downloads = document.getElementById('downloads');\n"
		"	if (!downloads) return;"

		"	var x = new XMLHttpRequest();\n"

		"	x.open('GET', '%s?downloads', true);\n"
		"	x.onreadystatechange = function() {\n"
		"		if (x.readyState == 4) {\n"
		"			downloads.innerHTML = x.responseText;\n"
		"		}\n"
		"	}\n"
		"	x.send(null);\n"
		"}\n"

		"function upd_stats() {\n"
		"	var stats = document.getElementById('stats');\n"
		"	if (!stats) return;"

		"	var x = new XMLHttpRequest();\n"

		"	x.open('GET', '%s?stats', true);\n"
		"	x.onreadystatechange = function() {\n"
		"		if (x.readyState == 4) {\n"
		"			stats.innerHTML = x.responseText;\n"
		"		}\n"
		"	}\n"
		"	x.send(null);\n"
		"}\n"

		"function startdl() {\n"
		"	var urlbox = document.getElementById('add1');\n"
		"	var url = urlbox.value;\n"

		"	var x = new XMLHttpRequest();\n"

		"	x.open('GET', '%s?add=' + url, true);\n"
		"	x.send(null);\n"
		"	urlbox.value = '';\n"
		"}\n"

		"function doenter(e) {\n"
		"	var key;\n"

		"	if (window.event)\n"
		"		key = window.event.keyCode;\n"
		"	else\n"
		"		key = e.which;\n"

		"	if (key == 13)\n"
		"		startdl();\n"

		"	return true;\n"
		"}\n"

		"\n</script>\n",
		mypath, mypath, mypath);
}