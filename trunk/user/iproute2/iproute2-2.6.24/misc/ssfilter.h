/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#define SSF_DCOND 0
#define SSF_SCOND 1
#define SSF_OR	  2
#define SSF_AND	  3
#define SSF_NOT	  4
#define SSF_D_GE  5
#define SSF_D_LE  6
#define SSF_S_GE  7
#define SSF_S_LE  8
#define SSF_S_AUTO  9

struct ssfilter
{
	int type;
	struct ssfilter *post;
	struct ssfilter *pred;
};

int ssfilter_parse(struct ssfilter **f, int argc, char **argv, FILE *fp);
void *parse_hostcond(char*);

