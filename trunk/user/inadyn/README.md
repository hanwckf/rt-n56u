README
======
[![Build Status](https://travis-ci.org/troglobit/inadyn.png?branch=master)](https://travis-ci.org/troglobit/inadyn)[![Coverity Scan Status](https://scan.coverity.com/projects/2981/badge.svg)](https://scan.coverity.com/projects/2981)

Inadyn is a small and simple
[DDNS](http://en.wikipedia.org/wiki/Dynamic_DNS) client with HTTPS
support.  It is commonly available in many GNU/Linux distributions, used
in off-the-shelf routers and Internet gateways to automate the task of
keeping your DNS record up to date with any IP address changes from your
[ISP](http://en.wikipedia.org/wiki/ISP).  It can also be used in
installations with redundant (backup) connections to the Internet.

If your ISP provides you with a DHCP or PPPoE/PPPoA connection you risk
losing your IP address every time you reconnect, or in DHCP even when
the lease is renegotiated.

By using a DDNS client such as Inadyn you can register an Internet name
at certain providers that the DDNS client updates, periodically and/or
on demand when your IP changes.

Inadyn can maintain multiple host names with the same IP address, and
has a web based IP detection which runs well behind a NAT router.

The following DDNS providers are supported natively, other providers,
like http://twoDNS.de for instance, can be supported using the generic
DDNS plugin.  See below for configuration examples.

* http://www.dyndns.org
* http://freedns.afraid.org
* http://www.zoneedit.com
* http://www.no-ip.com
* http://www.easydns.com
* http://www.tzo.com
* http://www.3322.org
* http://www.dnsomatic.com
* http://www.tunnelbroker.net
* http://dns.he.net/
* http://www.dynsip.org
* http://www.sitelutions.com
* http://www.dnsexit.com
* http://www.changeip.com
* http://www.zerigo.com
* http://www.dhis.org
* https://nsupdate.info
* http://duckdns.org
* https://www.loopia.com

Some of these services are free of charge for non-commercial use, others
take a small fee, but also provide more domains to choose from.

Inadyn v1.99.8 and later support HTTPS (v1.99.11 and later also support
SNI), for DDNS providers that support this (you must check this
yourself).  Tested are DynDNS, FreeDNS, nsupdate.info, and Loopia.

Using HTTPS is recommended since it protects your credentials from being
snooped and further reduces the risk of someone hijacking your account.

Note: No HTTPS certificate validation is currently done, patches welcome!


Example Configuration
=====================

Inadyn supports updating several DDNS servers, several accounts even on
different DDNS providers.  The following example config file illustrates
how it can be used.

Feature is courtesy of [Christian Eyrich](http://eyrich-net.org/programmiertes.html)

Example /etc/inadyn.conf:

    background
    verbose        1
    period         300
    cache-dir      /mnt/ddns
    startup-delay  60
    #logfile /var/log/ddns.log
    #pidfile /var/run/ddns.pid
    
    system default@dyndns.org
      ssl
      username yxxx
      password xyxx
      alias yyy
      alias zzz
    
    system default@no-ip.com
      username xxyx
      password xxxy
      alias yyy

In a multi-user setup, make sure to chmod your .conf to 600 (read-write
only by you/root) to prevent other users from accessing your DDNS server
credentials.

Note, here only the DynDNS account uses SSL, the No-IP account will
still use regular HTTP.  See below for SSL build instructions.

The example has two commented out lines: logfile is disabled, causing
Inadyn to default to use syslog, the pidfile is also commented out, so
Inadyn defaults to create `/var/run/inadyn/inadyn.pid` instead.

The example also has a cache directory specified, which in this case is
a persistent store for the three cache files
`/mnt/ddns/yyy.dyndns.org.cache`, `/mnt/ddns/zzz.dyndns.org.cache` and
`/mnt/ddns/yyy.no-ip.com.cache`


Generic DDNS Plugin
===================

Aside from the dedicated DDNS provider support Inadyn also has a generic
DDNS provider plugin.  A DDNS provider like twoDNS.de can be setup like
this:

    period         300
    startup-delay  60
    cache-dir      /etc/inadyn

    system custom@http_srv_basic_auth
        ssl
        checkip-url checkip.two-dns.de /
        server-name update.twodns.de
        server-url /update?hostname=
        username myuser
        password mypass
        alias myalias.dd-dns.de

When using the generic plugin you should first inspect the response from
the DDNS provider.  Inadyn currently looks for a 200 HTTP response OK
code and the string "good" or "OK" in the HTTP response body.  You may
have to modify Inadyn manually, any patches for this are most welcome!


Building with/witouth HTTPS Support
===================================

By default Inadyn tries to build with GnuTLS for HTTPS support.  GnuTLS
is the recommended SSL library to use on UNIX distributions which do not
provide OpenSSL as a system library.  However, when OpenSSL is available
as a system library, for example in many embedded systems:

    USE_OPENSSL=1 make distclean all

To completely disable the HTTPS support in Inadyn:

    ENABLE_SSL=0 make distclean all

For more details on the OpenSSL and GNU GPL license issue, see:

* https://lists.debian.org/debian-legal/2004/05/msg00595.html
* https://people.gnome.org/~markmc/openssl-and-the-gpl


Contact
=======

This is the continuation of the
[original INADYN](http://www.inatech.eu/inadyn/) by Narcis Ilisei.  It
is currently being developed and maintained by
[Joachim Nilsson](http://troglobit.com) at
[GitHub](http://github.com/troglobit/inadyn).  Please file bug reports,
clone it, or send pull requests for bug fixes and proposed extensions.

