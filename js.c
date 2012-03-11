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
		"setDownloadChoices();\n"

		"function upd_downloads() {\n"
		"	var downloads = document.getElementById('downloads');\n"
		"	if (!downloads) return;"

		"	var x = new XMLHttpRequest();\n"

		"	x.open('GET', '%s?downloads', true);\n"
		"	x.onreadystatechange = function() {\n"
		"		if (x.readyState == 4) {\n"
		"			downloads.innerHTML = x.responseText;\n"
		"			setDownloadChoices();\n"
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

		"function setDownloadChoices() {\n"
		"	var dl = document.getElementById('downloads');\n"
		"	var tbody = dl.getElementsByTagName('tbody')[0];\n"

		"	var tr = tbody.getElementsByTagName('tr');\n"
		"	if (!tr) return;\n"

		"	var len = tr.length;\n"
		"	for (var i = 0; i < len; i++) {\n"
		"		tr[i].onclick = pickDownload;\n"

		"		if (window.picked == tr[i].id)\n"
		"			tr[i].className = 'picked';"
		"	}\n"
		"}\n"

		"function clearDownloadChoices() {\n"
		"	var dl = document.getElementById('downloads');\n"
		"	var tbody = dl.getElementsByTagName('tbody')[0];\n"

		"	var tr = tbody.getElementsByTagName('tr');\n"
		"	if (!tr) return;\n"

		"	var len = tr.length;\n"
		"	for (var i = 0; i < len; i++) {\n"
		"		tr[i].className = null;"
		"	}\n"
		"}\n"

		"function pickDownload() {\n"
		"	clearDownloadChoices();\n"

		"	if (window.picked == this.id) {\n"
		"		this.className = null;\n"
		"		window.picked = null;\n"
		"	} else {\n"
		"		this.className = 'picked';\n"
		"		window.picked = this.id;\n"
		"	}\n"
		"}\n"

		"function checkPicked() {\n"

		"	if (!window.picked || window.picked.indexOf('gid') == -1) {\n"
		"		alert('No download selected?');\n"
		"		return false;\n"
		"	}\n"

		"	return true;\n"
		"}\n"

		"\n</script>\n",
		mypath, mypath, mypath);
}
