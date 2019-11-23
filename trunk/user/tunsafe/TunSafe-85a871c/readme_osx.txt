WARNING: ALPHA SOFTWARE - USE AT YOUR OWN RISK

License: https://tunsafe.com/downloads/LICENSE.TXT

This is the experimental OSX version of TunSafe.

It is single threaded, has no UI, does not support IPv6,
and does not support switching DNS.

Still - it's roughly 2x as fast as OpenVPN. 260mbit vs 140mbit.

It uses the built-in utun network adapter so you need a
reasonably new OSX version.

Usage (from a Terminal):
sudo ./tunsafe Config.conf

Press Ctrl-C to exit.

