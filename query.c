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

// Returns 1 if the query was handled

int handle_query(const char *qs) {

	if (!strcmp(qs, "downloads")) { // Send just the downloads via ajax

		htmlpage();

		getStats();

		if (stats.total > 0 && !offline) {
			downloads = xcalloc(stats.total, sizeof(struct download));

			getDownloads();
		}

	} else if (!strcmp(qs, "stats")) { // Send just the stats via ajax

		htmlpage();

		getStats();

		if (!offline)
			printStats();

	} else if (!strncmp(qs, "add=", 4)) { // Add this url to download

		plainpage();

		addDownload(qs + 4);

	} else if (!strcmp(qs, "cleanup")) { // Add this url to download

		plainpage();

		cleandl();

	} else {

		return 0;
	}

	return 1;
}