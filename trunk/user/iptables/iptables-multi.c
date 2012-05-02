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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

int iptables_main(int argc, char **argv);
int iptables_save_main(int argc, char **argv);
int iptables_restore_main(int argc, char **argv);
/*int iptables_xml_main(int argc, char **argv);*/

int main(int argc, char **argv) {
  char *progname;

  if (argc == 0) {
    fprintf(stderr, "no argv[0]?");
    exit(1);
  } else {
    progname = basename(argv[0]);

    if (!strcmp(progname, "iptables"))
      return iptables_main(argc, argv);
    
    if (!strcmp(progname, "iptables-save"))
      return iptables_save_main(argc, argv);
    
    if (!strcmp(progname, "iptables-restore"))
      return iptables_restore_main(argc, argv);
/*
    if (!strcmp(progname, "iptables-xml"))
      return iptables_xml_main(argc, argv);
*/
    fprintf(stderr, "iptables multi-purpose version: unknown applet name %s\n", progname);
    exit(1);
  }
}
