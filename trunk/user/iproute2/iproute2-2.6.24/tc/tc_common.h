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

#define TCA_BUF_MAX	(64*1024)

extern struct rtnl_handle rth;
extern int do_qdisc(int argc, char **argv);
extern int do_class(int argc, char **argv);
extern int do_filter(int argc, char **argv);
extern int do_action(int argc, char **argv);
extern int do_tcmonitor(int argc, char **argv);
extern int print_action(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg);
extern int print_filter(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg);
extern int print_qdisc(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg);
 extern int print_class(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg);

struct tc_estimator;
extern int parse_estimator(int *p_argc, char ***p_argv, struct tc_estimator *est);
