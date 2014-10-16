ChangeLog
=========

Version 1.99.11: Oct 15 2014
----------------------------
- Fix building on FreeBSD by converting to use GNU Configure & Build system
- Fixes to add support for TLS 1.x with SNI, thanks to Thomas Waldmann
- Add support for https://nsupdate.info, thanks to Thomas Waldmann
- Add support for https://www.loopia.com DynDNS service extension
- Add support for https://duckdns.org, thanks to Andy Padavan
- Updated man pages, both inadyn(8) and inadyn.conf(5) with examples
- SSL mitigation fixes for POODLE

Version 1.99.10: Sep 13 2014
----------------------------
- Fix issue #57: snprintf() causes loss of \= from password string
- Fix issue #58: Add support for GnuTLS as the default SSL library
- Fix bugs found by Coverity Scan
- Fix memory leaks found by Glibc on PowerPC
- Refactor string functions strcat()/strcpy() to use secure OpenBSD
  variants strlcat()/strlcpy()
- Fix include order problem with error.h

Version 1.99.9: May 21 2014
---------------------------
- Fix memory leak in new HTTPS support, found by Valgrind
- Support for Zerigo DDNS provider
- Support for DHIS DDNS provider
- Other misc. Valgrind and Cppcheck fixes

Version 1.99.8: May 20 2014
---------------------------
- Support for HTTPS to secure login credentials at DNS update, issue #36
- Support for persistent cache files with new --cache-dir PATH
- Support for twoDNS.de in generic DDNS plugin, see README for details
- Man page updates

Version 1.99.7: May 14 2014
---------------------------
- Support for multiple cache files, one per DDNS provider, issue #35
- Refactor DDNS providers as plugins, issue #30

Version 1.99.6: Dec 25 2013
---------------------------
Fix nasty socket leak. Update documentation for custom servers and
add missing compatibility entry for custom servers.

Version 1.99.5: Nov 27 2013
---------------------------
- Support for --fake-address on new SIGUSR1 (forced update)
- Support for SIGUSR2 (check now),
- Support for --startup-delay SEC, for embedded systems
- Many minor bug fixes
- Man page updates

Version 1.99.4: Aug 8 2013
--------------------------
This release fixes a base64 password encoding regression in 1.99.3

Version 1.99.3: Jul 15 2013
---------------------------
This release adds the ability to specify the cache file and the
ability to check the IP of the interface (UNIX only).  If no
interface is specified, no external IP check is performed.  The old
--iface option has been renamed --bind. changeip.com support has
been added.  Minor bugfixes and code optimizations have been made.

- Add ability to specify cache file
- Add ability to check IP of interface (UNIX only)
  If interface is specified, no external IP check performed
  Old --iface option renamed to --bind
- Specify IP address in freedns.afraid.org update request (only
  autodetect was used)
- Add changeip.com support
- Minor bugfixes and code optimization

Version 1.99.2: Sep 7 2012
--------------------------
- Fix inability to change update period (broken in 1.99.0)
- Get http status description
- Fix debug output description

Version 1.99.1: Sep 1 2012
--------------------------
- Make HTTP status code check server-specific
- Change e-mail address

Version 1.99.0: Aug 17 2012
---------------------------
- Merge wl500g patches from http://code.google.com/p/wl500g:
  - 120-heipv6tb.patch adds support for tunnelbroker
  - 121-hedyndns.patch adds support for HE dyndns
  - 210-wildcard.patch makes wildcard option account specific
- For ddns services that have their own checkip service, use it instead of
  dyndns.org checkip service
- Add ability to handle non-fatal temporary errors ("update too often",
  system error etc.)
- Fix malformed HTTP request
- Warn if initial DNS request failed
- Add dnsexit.com support
- Modify http client to parse response for http status code and response
  body
- Remove DynDNS ignored and deprecated parameters (wildcard, mx, backmx,
  system). Wildcard kept for easydns.com
- Report detected IP to sitelutions and dynsip servers (only autodetect
  was used)
- Update TZO support
- Check HTTP status code before validating response
- Remake zoneedit response validation
- Little code cleanup

Version 1.98.1: Jul 18 2011
---------------------------
- Preserve time since last update (forced update counter) and num interations
  from being reset by SIGHUP restart command
- Bug fix --drop-privs uid/gid was swapped and a possible buffer overflow
- Extend --drop-privs to support hyphens
- Bug fix segfault at initial DNS lookup
- Cleanup of inadyn log messages, reformat & clarification
- Typo fixes and polish to man pages inadyn(8) and inadyn.conf(5)

Version 1.98.0: Feb 28 2011
---------------------------
- New config file, command line, syntax (still backwards compatible!)
- New option --drop-privs USER[:GROUP] to support privilege separation
- Drop privileges before creating any files
- Documentation updates

Version 1.97.4: Nov 2 2010
--------------------------
- Support for dynsip.org by milkfish, from DD-WRT
- Add support for sitelutions.com, from inadyn-mt (untested)
- Clear DNS cache before calling getaddrinfo(), should fix GitHub issue #3

Version 1.97.3: Nov 2 2010
--------------------------
- Merge wl500g patches from http://code.google.com/p/wl500g:
  - 101-http-request.patch. This cleans up the DDNS server defintions
	and callbacks, evidently originating from ideas implemented by
	DD-WRT.
  - 102-zoneedit.patch. This fixes issues in what appears to be both
	request and response formats in ZoneEdit DDNS. Originating from
	DD-WRT.
  - 103-tzo.patch. This patch adds support for tzo.com DDNS serivices.
  - 110-dnsomatic.patch.  This patch adds support for DNS-O-Matic
	<http://dnsomatic.com/>, an OpenDNS service which can act as a
	"meta" update to several other DDNS service providers.
  - 120-heipv6tb.patch. This patch adds support for Hurricane Electric's
	http://tunnelbroker.net/ DDNS services <https://dns.he.net/>.
- When starting: always use cache file, if it exists, or seed with DNS
  lookup
- Fix Debian bug #575549: freedns.afraid.org example in inadyn(8) is
  incorrect.

Version 1.97.2: Oct 30 2010
---------------------------
- Fix missing man pages from install/uninstall targets
- Fix GitHub issue #2: setsocktopt() takes pointer to struct timeval,
  not int as argument
- Replace gethostbyname() with getaddrinfo() and improve logging at
  connect()

Version 1.97.1: Oct 19 2010
---------------------------
- Add support for properly restarting inadyn on SIGHUP
- Remove INADYN: prefix in MODULE_TAG entirely - messes up syslog output

Version 1.97.0: Oct 18 2010
---------------------------
- Apply patches by Neufbox4 from http://dev.efixo.net:
  - 100-inadyn-1.96.2.patch, cache file support
  - 100-inadyn-1.96.2.patch, bind interface support
  - 200-inadyn-1.96.2-64bits-fix.patch
  - 300-inadyn-1.96.2-pidfile-and-improve.patch
- New README, COPYING and LICENSE file, remove readme.html
- Refactor and cleanup Makefile (renamed from makefile)
- Add support for SIGTERM signal
- Relocate include files to include directory
- Apply patch for multiple accounts from Christian Eyrich
- Remove unused uint typedef causing trouble on ARM GCC 4.4.x
- Fix missing strdup() of input config file and free any preexisting
- Make sure TYPE_TCP enum relly is 0 on all compilers
- Improve error messages using strerror() and use -1 as stale socket, not 0
- Fix nasty socket leak
- Merge with inadyn-advanced (http://sf.net/projects/inadyn-advanced):
  - Add support for 3322.org and easydns.org
  - Add support for domain wildcarding, --wildcard option
    NOTE: Domain wildcarding is now *disabled* by default
  - Add support for running an external command hook on IP update,
	new --exec option
  - Add support for datetime in debug messages
- Refactor DBG_PRINTF((..)) --> logit(..)
- Update man page inadyn(8) with info on --bind_interface, --wildcard,
  --exec options and support for easydns.org and 3322.org services
- Misc fixes and cleanups

Version 1.96.2: 12 Mar 2007
---------------------------
Fixes:
- If the Dynamic DNS server responds with an error Inadyn will abort.
  This will prevent further retries with wrong dyndns credentials.
- Default port number included in the request, to support the requests
  via Proxy, to ports different than 80.
- Simplified main inadyn update loop function. (there was no bug there)

Version 1.96: 9 Sep 2005
------------------------
New features:
- zoneedit.com supported.
- no-ip.com supported.
- support for generic DNS services that support HTTP updates

Fixes:
- Immediate exit in case of --iterations=1 (not after a sleep period)
- Add missing option for specifying the path in the DNS server

Version 1.95: 20 Jul 2005
-------------------------
New features:
- UNIX signals supported - inadyn will stop gracefully in case
  of  ALRM, HUP, INT, ...
- New dynamic DNS service supported - www.zoneedit.com - Not tested!
- Makefile adjusted for Solaris - compilable under Solaris.
- support for generic DYNDNS service that supports update via HTTP with
  basic authentication not yet fully complete. Not known where might be
  applicable.

Version 1.90: 24 Feb 2005
-------------------------
New features/enhancements:
- new option '--change_persona uid:gid' - inadyn can switch the user
  after launch. Typical feature for daemons
- addition to '--ip_server_name' feature, now it has another parameter:
  the URL to read from the given server name
- reduced some error messages
- manual pages updated. (thanks to Shaul Karl)

Bugfixes:
- Typo fixed (--ip_server_name option)

Version 1.86: 30 Jan 2005
-------------------------
Enhancements:
- Updated UNIX man pages for inadyn. Even a page for inadyn.conf!  Thanks to Shaul Karl
- Inadyn doesn't print anything (e.g. ver. number) anymore when goes to background
- New config file parser. Accepts ESCAPE character: '\'

Bugfix:
- Corrected check of the return code from socket() call

Version 1.85: 10 Jan 2005
-------------------------
Config file related enhancements:
- A default location for the config file in case of no arguments for inadyn
- A more *NIX like format for the config file.  Thanks to Jerome Benoit

Small bugfix:
- When 'iterations' option is specified as being '1', inadyn exits
  immediately after first update, not after the sleep period as before

Version 1.81: 23 Nov 2004
-------------------------
No new features, just a better integration with Linux.  Reviewed usage
of syslog and fork to background in daemon mode, thanks to Shaul Karl.

Version 1.80: 16 Oct 2004
-------------------------
New feature:
- Optional output to syslog for Linux, should work for all *nix systems
- Run in background for Linux, should work for all *nix systems

Bug fixes:
- Minor compile warnings removed

Version 1.70: 5 Jul 2004
------------------------
New feature:
- New --iterations cmd line option.  Now one can run inadyn with only one iteration.
  Untested.  It was a debug option now made accessible via cmd line. It should work

Bug fixes:
- Custom DNS from dyndns option was not accepted by the cmd line parser.
  "Copy-paste" error :-(

Version 1.60: 5 Jun 2004
------------------------
On users' request the inadyn can read the options from file.

Version 1.51: 3 May 2004
------------------------
Support for more aliases for DNS service offered by freedns.afraid.org

Version 1.5: 1 May 2004
-----------------------
- Support for dynamic DNS service offered by freedns.afraid.org
- Support for http proxy
- GPL copyright notice added.

Version 1.4: 1 Mar 2004
-----------------------
- Support for custom DNS and static DNS services offered by dyndns.org
- Support for forced IP update, so the account will not expire even
  though the IP never changes

Version 1.35: Feb 2004
----------------------
Bug fixes:
- Multiple aliases are AGAIN supported
- In case of error in IP update the OS signal handler is not installed again.

Ver. 1.34:
----------
Various bugfixes.

Version 1.3: 6 Nov 2003 -- first port to *NIX - (Linux)
-------------------------------------------------------
Linux version running OK as console app

Future plans:
- Run as a background daemon in UNIX
- Better interface

Version 1.2: Jun 2003
---------------------
Port to pSOS

**Note:** no DNS support under pSOS -> hard coded IP addresses of the server

Version 1.0: 20 May 2003, first stable version
----------------------------------------------
Main features ready:
- DYNDNS client
- free
- works fine behind a NAT router
- runs fine as a service
- has a nice log file

Future plans:
- port to *NIX
- port to pSOS
