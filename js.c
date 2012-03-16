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
		"<!--\n"

		"setInterval('upd_downloads()', 1800);\n"
		"setInterval('upd_stats()', 5000);\n"
		"setDownloadChoices();\n"

		"function upd_downloads() {\n"
		"	var downloads = document.getElementById('downloads');\n"
		"	if (!downloads) return;\n"

		"	var x = new XMLHttpRequest();\n"

		"	x.open('GET', '%1$s?downloads', true);\n"
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
		"	if (!stats) return;\n"

		"	var x = new XMLHttpRequest();\n"

		"	x.open('GET', '%1$s?stats', true);\n"
		"	x.onreadystatechange = function() {\n"
		"		if (x.readyState == 4) {\n"
		"			stats.innerHTML = x.responseText;\n"
		"		}\n"
		"	}\n"
		"	x.send(null);\n"
		"}\n"

// Fire and forget
		"function fireajax(where) {\n"
		"	var x = new XMLHttpRequest();\n"

		"	x.open('GET', where, true);\n"
		"	x.send(null);\n"
		"}\n"

		"function startdl() {\n"
		"	var urlbox = document.getElementById('add1');\n"
		"	var url = urlbox.value;\n"

		"	fireajax('%1$s?add=' + url);\n"
		"	urlbox.value = '';\n"
		"}\n"

// Was the enter key pressed in the "start download" field?
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

// Keep the "picked download" status through updates
		"function setDownloadChoices() {\n"
		"	var dl = document.getElementById('downloads');\n"
		"	var tbody = dl.getElementsByTagName('tbody')[0];\n"
		"	if (!tbody) return;\n"

		"	var tr = tbody.getElementsByTagName('tr');\n"
		"	if (!tr) return;\n"

		"	var len = tr.length;\n"
		"	for (var i = 0; i < len; i++) {\n"
		"		tr[i].onclick = pickDownload;\n"

		"		if (window.picked == tr[i].id) {\n"
		"			tr[i].className = 'picked';\n"
		"			window.pickedstatus = tr[i].children[0].innerHTML;\n"
		"		}\n"
		"	}\n"
		"}\n"

		"function clearDownloadChoices() {\n"
		"	var dl = document.getElementById('downloads');\n"
		"	var tbody = dl.getElementsByTagName('tbody')[0];\n"
		"	if (!tbody) return;\n"

		"	var tr = tbody.getElementsByTagName('tr');\n"
		"	if (!tr) return;\n"

		"	var len = tr.length;\n"
		"	for (var i = 0; i < len; i++) {\n"
		"		tr[i].className = null;\n"
		"	}\n"
		"}\n"

		"function pickDownload() {\n"
		"	clearDownloadChoices();\n"

		"	if (window.picked == this.id) {\n"
		"		this.className = null;\n"
		"		window.picked = null;\n"
		"		window.pickedstatus = null;\n"
		"	} else {\n"
		"		this.className = 'picked';\n"
		"		window.picked = this.id;\n"
		"		window.pickedstatus = this.children[0].innerHTML;\n"
		"	}\n"
		"}\n"

		"function checkPicked() {\n"

		"	if (!window.picked || window.picked.indexOf('gid') == -1) {\n"
		"		alert('No download selected?');\n"
		"		return false;\n"
		"	}\n"

		"	return true;\n"
		"}\n"

		"function pausedl() {\n"
		"	if (!checkPicked())\n"
		"		return;\n"

		"	var gid = window.picked.substr(4);\n"
		"	var x = new XMLHttpRequest();\n"

		"	if (window.pickedstatus == 'paused')\n"
		"		x.open('GET', '%1$s?unpause=' + gid, true);\n"
		"	else\n"
		"		x.open('GET', '%1$s?pause=' + gid, true);\n"

		"	x.send(null);\n"

		"}\n"

		"function removedl() {\n"
		"	if (!checkPicked())\n"
		"		return;\n"

		"	var gid = window.picked.substr(4);\n"

		"	fireajax('%1$s?remove=' + gid);\n"

		"}\n"

		"function prioupdl() {\n"
		"	if (!checkPicked())\n"
		"		return;\n"

		"	var gid = window.picked.substr(4);\n"

		"	fireajax('%1$s?prioup=' + gid);\n"

		"}\n"

		"function priodowndl() {\n"
		"	if (!checkPicked())\n"
		"		return;\n"

		"	var gid = window.picked.substr(4);\n"

		"	fireajax('%1$s?priodown=' + gid);\n"

		"}\n"

		"function cleandl() {\n"

		"	fireajax('%1$s?cleanup');\n"

		"}\n"

		"function showsettings() {\n"

		"	var settings = document.getElementById('settings');\n"
		"	settings.className = 'settingsShown';\n"

		"	var x = new XMLHttpRequest();\n"

		"	x.open('GET', '%1$s?settings', true);\n"
		"	x.onreadystatechange = function() {\n"
		"		if (x.readyState == 4) {\n"
		"			settings.innerHTML = x.responseText;\n"
		"		}\n"
		"	}\n"
		"	x.send(null);\n"
		"}\n"

		"function hidesettings() {\n"

		"	var settings = document.getElementById('settings');\n"
		"	settings.innerHTML = '';\n"
		"	settings.className = 'null';\n"
		"}\n"

// The settings functions:
		"function updatedir() {\n"

		"	var dir = document.getElementById('dir').value;\n"

		"	fireajax('%1$s?set=dir=' + dir);\n"
		"}\n"

		"function updateipv6() {\n"

		"	var tick = document.getElementById('tickv6').checked;\n"

		"	fireajax('%1$s?set=disable-ipv6=' + tick);\n"
		"}\n"

		"function updateflwent() {\n"

		"	var ent = document.getElementById('flwtorrent').value;\n"

		"	fireajax('%1$s?set=follow-torrent=' + ent);\n"
		"}\n"

		"function updateentports() {\n"

		"	var ports = document.getElementById('entports').value;\n"

		"	fireajax('%1$s?set=listen-port=' + ports);\n"
		"}\n"

		"function updatedllimit() {\n"

		"	var dl = document.getElementById('dllimit').value;\n"

		"	fireajax('%1$s?set=max-download-limit=' + dl);\n"
		"}\n"

		"function updatealldllimit() {\n"

		"	var dl = document.getElementById('alldl').value;\n"

		"	fireajax('%1$s?set=max-overall-download-limit=' + dl);\n"
		"}\n"

		"function updateuplimit() {\n"

		"	var up = document.getElementById('uplimit').value;\n"

		"	fireajax('%1$s?set=max-upload-limit=' + up);\n"
		"}\n"

		"function updatealluplimit() {\n"

		"	var up = document.getElementById('allup').value;\n"

		"	fireajax('%1$s?set=max-overall-upload-limit=' + up);\n"
		"}\n"

		"function updateseedrt() {\n"

		"	var ratio = document.getElementById('seedrt').value;\n"
		"	var r = /^[\\d.]*$/;\n"
		"	if (r.test(ratio) == false) {\n"
		"		alert('Invalid seed ratio');\n"
		"		return;\n"
		"	}\n"


		"	fireajax('%1$s?set=seed-ratio=' + ratio);\n"
		"}\n"

		"function updateseedtm() {\n"

		"	var tm = document.getElementById('seedtm').value;\n"

		"	fireajax('%1$s?set=seed-time=' + tm);\n"
		"}\n"
// End settings functions

		"-->\n"
		"\n</script>\n",
		mypath);
}
