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

#include <math.h>
#include "a2web.h"

#define bufsize 80

static char eta[bufsize]; // We use this static space for the eta


void printStats() {

	printf(
		"\tTotal download speed: <span class=number>%llu</span> kB/s<p>\n"
		"\tTotal upload speed: <span class=number>%llu</span> kB/s<p>\n"

		"\tTotal %u downloads, active/waiting/stopped: "
		"<span class=number>%u/%u/%u</span>\n",

		stats.down, stats.up, stats.total,
		stats.active, stats.waiting, stats.stopped);
}


static void calcEta(unsigned long long speed, unsigned long long size) {

	if (!speed) {
		snprintf(eta, bufsize, "unknown");
		return;
	}

	unsigned secs = size/speed;

	if (secs < 120)
		snprintf(eta, bufsize, "%u second%s", secs, secs==1 ? "" : "s");
	else if (secs < 60*60)
		snprintf(eta, bufsize, "%u minutes, %u second%s", secs/60, secs%60,
			secs==1 ? "" : "s");
	else
		snprintf(eta, bufsize, "%.2f hours", (float) secs / (60.0f*60));

	eta[bufsize-1] = '\0';
}


void printDownloads() {

	printf("<table border=1>\n"
		"\t<thead><tr>"
		"<th>Status</th>"
		"<th>ETA</th>"
		"<th>Progress</th>"
		"<th>Uploaded</th>"
		"<th>Speed down/up kB/s</th>"
		"<th>Seeds/peers</th>"
		"<th>URL</th>"
		"</tr></thead>\n\t<tbody>\n");

	unsigned i;
	for (i = 0; i < stats.total; i++) {

		struct download *cur = &downloads[i];

		printf("\t<tr id='gid-%s'>", cur->gid);

		char upped[bufsize] = "", seeded[bufsize] = "";
		char progress[2*bufsize] = "";

		if (cur->uploaded)
			snprintf(upped, bufsize, "%llu", cur->uploaded);

		if (cur->seeders || cur->connections)
			snprintf(seeded, bufsize, "%u/%u", cur->seeders,
				 cur->connections);

		upped[bufsize - 1] = '\0';
		seeded[bufsize - 1] = '\0';

		float percent = (float) cur->completed / cur->length;
		percent *= 100;

		if (isnan(percent))
			percent = 0;

		int last = percent*10;
		last %= 10;
		int len;

		// Pretty printing.
		if (percent < 5)
			len = 2;
		else if (last)
			len = 1;
		else
			len = 0;

		if (strcmp(cur->status, "complete"))
			calcEta(cur->down, cur->length - cur->completed);
		else {
			eta[0] = '-';
			eta[1] = '\0';
		}

		// Pretty units for the download size
		char progr_unit[] = "kB";
		if (cur->length > 1024) {
			cur->length /= 1024;
			cur->completed /= 1024;
			progr_unit[0] = 'M';
		}
		if (cur->length > 1024) {
			cur->length /= 1024;
			cur->completed /= 1024;
			progr_unit[0] = 'G';
		}

		snprintf(progress, 2*bufsize,
			"<div class=meter-outer>"
			"<div class=meter-value style=\"width: %.0f%%;\">"
			"<div class=meter-text>"
			"%.*f%% - %llu/%llu %s"
			"</div>"
			"</div>"
			"</div>",
			percent, len, percent, cur->completed, cur->length,
			progr_unit);

		progress[2*bufsize - 1] = '\0';

		printf("\t<td>%s</td>"
			"<td>%s</td>"
			"<td>%s</td>"
			"<td>%s</td>"
			"<td>%llu/%llu</td>"
			"<td>%s</td>"
			"<td>%s</td>",

			cur->status, eta, progress,
			upped,
			cur->down, cur->up,
			seeded, cur->uris ? cur->uris : "");

		printf("</tr>\n");
	}

	printf("\t</tbody>\n</table>\n");
}


void printAdditions() {

	printf("<div id=additions>\n"

		"\tAdd new download:<br>\n"
		"\t<input type=text id=add1 size=80 "
			"onkeypress='return doenter(event)'> \n"
		"\t<input type=button value=Start onclick=\"startdl()\">\n"

		"</div>\n\n");
}


void printOptions() {

	printf("<div id=options>\n");

	printf("\t<button onclick='pausedl()' title=Pause>"
		"<img src='%s/pause.png' alt=Pause width=32 height=32>"
		"</button>\n", themedir);
	printf("\t<button onclick='stopdl()' title=Stop>"
		"<img src='%s/stop.png' alt=Stop width=32 height=32>"
		"</button>\n", themedir);
	printf("\t<button onclick='removedl()' title='Remove from list'>"
		"<img src='%s/remove.png' alt=Remove width=32 height=32>"
		"</button>\n", themedir);
	printf("\t<button onclick='prioupdl()' title='Priority up'>"
		"<img src='%s/up.png' alt=MorePrio width=32 height=32>"
		"</button>\n", themedir);
	printf("\t<button onclick='priodowndl()' title='Priority down'>"
		"<img src='%s/down.png' alt=LessPrio width=32 height=32>"
		"</button>\n", themedir);
	printf("\t<button onclick='cleandl()' "
		"title='Clean up completed/error/removed downloads'>"
		"<img src='%s/reload.png' alt=Cleanup width=32 height=32>"
		"</button>\n", themedir);

	printf("</div>\n\n");
}
