#!/usr/bin/env python
#
#   mtr  --  a network diagnostic tool
#   Copyright (C) 2016  Matt Kimball
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2 as
#   published by the Free Software Foundation.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

'''Test sending probes and receiving respones.'''

import socket
import sys
import time
import unittest

import mtrpacket


def resolve_ipv6_address(hostname):  # type: (str) -> str
    'Resolve a hostname to an IP version 6 address'

    for addrinfo in socket.getaddrinfo(hostname, 0):
        # pylint: disable=locally-disabled, unused-variable
        (family, socktype, proto, name, sockaddr) = addrinfo

        if family == socket.AF_INET6:
            sockaddr6 = sockaddr  # type: tuple

            (address, port, flow, scope) = sockaddr6
            return address

    raise LookupError(hostname)


def check_feature(test, feature):
    'Check for support for a particular feature with mtr-packet'

    check_cmd = '70 check-support feature ' + feature
    test.write_command(check_cmd)

    reply = test.parse_reply()
    test.assertEqual(reply.command_name, 'feature-support')
    test.assertIn('support', reply.argument)

    if reply.argument['support'] != 'ok':
        return False

    return True


def test_basic_remote_probe(test, ip_version, protocol):
    'Test a probe to a remote host with a TTL of 1'

    protocol_str = 'protocol ' + protocol
    if ip_version == 6:
        address_str = 'ip-6 ' + resolve_ipv6_address(mtrpacket.IPV6_TEST_HOST)
    elif ip_version == 4:
        address_str = 'ip-4 8.8.8.8'
    else:
        raise ValueError(ip_version)

    cmd = '60 send-probe ' + \
        protocol_str + ' ' + address_str + ' port 164 ttl 1'
    test.write_command(cmd)

    reply = test.parse_reply()
    test.assertEqual(reply.command_name, 'ttl-expired')


def test_basic_local_probe(test, ip_version, protocol):
    'Test a probe to a closed port on localhost'

    protocol_str = 'protocol ' + protocol
    if ip_version == 6:
        address_str = 'ip-6 ::1'
    elif ip_version == 4:
        address_str = 'ip-4 127.0.0.1'

    cmd = '61 send-probe ' + \
        protocol_str + ' ' + address_str + ' port 164'
    test.write_command(cmd)

    reply = test.parse_reply()
    test.assertEqual(reply.command_name, 'reply')

    if ip_version == 6:
        test.assertIn('ip-6', reply.argument)
        test.assertEqual(reply.argument['ip-6'], '::1')
    elif ip_version == 4:
        test.assertIn('ip-4', reply.argument)
        test.assertEqual(reply.argument['ip-4'], '127.0.0.1')


def test_basic_probe(test, ip_version, protocol):
    # type: (mtrpacket.MtrPacketTest, int, unicode) -> None

    '''Test a probe with TTL expiration and a probe which reaches its
    destination with a particular protocol.'''

    if not check_feature(test, protocol):
        err_str = 'Skipping ' + protocol + ' test due to no support\n'
        sys.stderr.write(err_str.encode('utf-8'))
        return

    test_basic_remote_probe(test, ip_version, protocol)
    test_basic_local_probe(test, ip_version, protocol)


class TestProbeICMPv4(mtrpacket.MtrPacketTest):
    '''Test sending probes using IP version 4'''

    def test_probe(self):
        'Test sending regular ICMP probes to known addresses'

        #  Probe Google's well-known DNS server and expect a reply
        self.write_command('14 send-probe ip-4 8.8.8.8')
        reply = self.parse_reply()
        self.assertEqual(reply.token, 14)
        self.assertEqual(reply.command_name, 'reply')
        self.assertIn('ip-4', reply.argument)
        self.assertEqual(reply.argument['ip-4'], '8.8.8.8')
        self.assertIn('round-trip-time', reply.argument)

    def test_timeout(self):
        'Test timeouts when sending to a non-existant address'

        #
        #  Probe a non-existant address, and expect no reply
        #
        #  I'm not sure what the best way to find an address that doesn't
        #  exist, but is still route-able.  If we use a reserved IP
        #  address range, Windows will tell us it is non-routeable,
        #  rather than timing out when transmitting to that address.
        #
        #  We're just using a currently unused address in Google's
        #  range instead.  This is probably not the best solution.
        #

        # pylint: disable=locally-disabled, unused-variable
        for i in range(16):
            self.write_command('15 send-probe ip-4 8.8.254.254 timeout 1')
            reply = self.parse_reply()
            self.assertEqual(reply.token, 15)
            self.assertEqual(reply.command_name, 'no-reply')

    def test_exhaust_probes(self):
        'Test exhausting all available probes'

        probe_count = 4 * 1024
        token = 1024

        # pylint: disable=locally-disabled, unused-variable
        for i in range(probe_count):
            command = str(token) + ' send-probe ip-4 8.8.254.254 timeout 60'
            token += 1
            self.write_command(command)

            reply = None
            try:
                reply = self.parse_reply(0)
            except mtrpacket.ReadReplyTimeout:
                pass

            if reply:
                if reply.command_name == 'probes-exhausted':
                    break

        self.assertIsNotNone(reply)
        self.assertEqual(reply.command_name, 'probes-exhausted')

    def test_timeout_values(self):
        '''Test that timeout values wait the right amount of time

        Give each probe a half-second grace period to probe a timeout
        reply after the expected timeout time.'''

        begin = time.time()
        self.write_command('19 send-probe ip-4 8.8.254.254 timeout 0')
        self.parse_reply()
        elapsed = time.time() - begin
        self.assertLess(elapsed, 0.5)

        begin = time.time()
        self.write_command('20 send-probe ip-4 8.8.254.254 timeout 1')
        self.parse_reply()
        elapsed = time.time() - begin
        self.assertGreaterEqual(elapsed, 0.9)
        self.assertLess(elapsed, 1.5)

        begin = time.time()
        self.write_command('21 send-probe ip-4 8.8.254.254 timeout 3')
        self.parse_reply()
        elapsed = time.time() - begin
        self.assertGreaterEqual(elapsed, 2.9)
        self.assertLess(elapsed, 3.5)

    def test_ttl_expired(self):
        'Test sending a probe which will have its time-to-live expire'

        #  Probe Goolge's DNS server, but give the probe only one hop
        #  to live.
        self.write_command('16 send-probe ip-4 8.8.8.8 ttl 1')
        reply = self.parse_reply()
        self.assertEqual(reply.command_name, 'ttl-expired')
        self.assertIn('ip-4', reply.argument)
        self.assertIn('round-trip-time', reply.argument)

    def test_parallel_probes(self):
        '''Test sending multiple probes in parallel

        We will expect the probes to complete out-of-order by sending
        a probe to a distant host immeidately followed by a probe to
        the local host.'''

        success_count = 0
        loop_count = 32

        # pylint: disable=locally-disabled, unused-variable
        for i in range(loop_count):
            #  Probe the distant host before the local host.
            self.write_command('17 send-probe ip-4 8.8.8.8 timeout 1')
            self.write_command('18 send-probe ip-4 127.0.0.1 timeout 1')

            reply = self.parse_reply()
            if reply.command_name == 'no-reply':
                continue

            self.assertEqual(reply.command_name, 'reply')
            self.assertIn('ip-4', reply.argument)
            self.assertEqual(reply.argument['ip-4'], '127.0.0.1')
            self.assertIn('round-trip-time', reply.argument)
            first_time = int(reply.argument['round-trip-time'])

            reply = self.parse_reply()
            if reply.command_name == 'no-reply':
                continue

            self.assertEqual(reply.command_name, 'reply')
            self.assertIn('ip-4', reply.argument)
            self.assertEqual(reply.argument['ip-4'], '8.8.8.8')
            self.assertIn('round-trip-time', reply.argument)
            second_time = int(reply.argument['round-trip-time'])

            #  Ensure we got a reply from the host with the lowest latency
            #  first.
            self.assertLess(first_time, second_time)

            success_count += 1

        #  We need 90% success to pass.  This allows a few probes to be
        #  occasionally dropped by the network without failing the test.
        required_success = int(loop_count * 0.90)
        self.assertGreaterEqual(success_count, required_success)


class TestProbeICMPv6(mtrpacket.MtrPacketTest):
    '''Test sending probes using IP version 6'''

    def __init__(self, *args):
        google_addr = resolve_ipv6_address(mtrpacket.IPV6_TEST_HOST)

        self.google_addr = google_addr  # type: str

        super(TestProbeICMPv6, self).__init__(*args)

    @unittest.skipUnless(mtrpacket.HAVE_IPV6, 'No IPv6')
    def test_probe(self):
        "Test a probe to Google's public DNS server"

        #  Probe Google's well-known DNS server and expect a reply
        self.write_command('51 send-probe ip-6 ' + self.google_addr)
        reply = self.parse_reply()
        self.assertEqual(reply.command_name, 'reply')
        self.assertIn('ip-6', reply.argument)
        self.assertIn('round-trip-time', reply.argument)

        #  Probe the loopback, and check the address we get a reply from is
        #  also the loopback.  While implementing IPv6, I had a bug where
        #  the low bits of the received address got zeroed.  This checks for
        #  that bug.
        self.write_command('52 send-probe ip-6 ::1')
        reply = self.parse_reply()
        self.assertEqual(reply.command_name, 'reply')
        self.assertIn('ip-6', reply.argument)
        self.assertIn('round-trip-time', reply.argument)
        self.assertEqual(reply.argument['ip-6'], '::1')

    @unittest.skipUnless(mtrpacket.HAVE_IPV6, 'No IPv6')
    def test_ttl_expired(self):
        'Test sending a probe which will have its time-to-live expire'

        #  Probe Goolge's DNS server, but give the probe only one hop
        #  to live.
        cmd = '53 send-probe ip-6 ' + self.google_addr + ' ttl 1'
        self.write_command(cmd)
        reply = self.parse_reply()
        self.assertEqual('ttl-expired', reply.command_name)
        self.assertIn('ip-6', reply.argument)
        self.assertIn('round-trip-time', reply.argument)


class TestProbeUDP(mtrpacket.MtrPacketTest):
    'Test transmitting probes using UDP'

    def udp_port_test(self, address):  # type: (unicode) -> None
        'Test UDP probes with variations on source port and dest port'

        if not check_feature(self, 'udp'):
            return

        cmd = '80 send-probe protocol udp ' + address
        self.write_command(cmd)
        reply = self.parse_reply()
        self.assertEqual('reply', reply.command_name)

        cmd = '81 send-probe protocol udp port 990 ' + address
        self.write_command(cmd)
        reply = self.parse_reply()
        self.assertEqual('reply', reply.command_name)

        cmd = '82 send-probe protocol udp local-port 1991 ' + address
        self.write_command(cmd)
        reply = self.parse_reply()
        self.assertEqual('reply', reply.command_name)

    def test_udp_v4(self):
        'Test IPv4 UDP probes'

        test_basic_probe(self, 4, 'udp')

        self.udp_port_test('ip-4 127.0.0.1')

    @unittest.skipUnless(mtrpacket.HAVE_IPV6, 'No IPv6')
    def test_udp_v6(self):
        'Test IPv6 UDP probes'

        test_basic_probe(self, 6, 'udp')

        self.udp_port_test('ip-6 ::1')


class TestProbeTCP(mtrpacket.MtrPacketTest):
    'Test TCP probe support'

    def test_tcp_v4(self):
        '''Test IPv4 TCP probes, with TTL expiration, to a refused port
        and to an open port'''

        test_basic_probe(self, 4, 'tcp')

        if not check_feature(self, 'tcp'):
            return

        #  Probe a local port assumed to be open  (ssh)
        cmd = '80 send-probe ip-4 127.0.0.1 protocol tcp port 22'
        self.write_command(cmd)

        reply = self.parse_reply()
        self.assertEqual(reply.command_name, 'reply')

    @unittest.skipUnless(mtrpacket.HAVE_IPV6, 'No IPv6')
    def test_tcp_v6(self):
        'Test IPv6 TCP probes'

        test_basic_probe(self, 6, 'tcp')

        if not check_feature(self, 'tcp'):
            return

        #  Probe a local port assumed to be open  (ssh)
        cmd = '80 send-probe ip-6 ::1 protocol tcp port 22'
        self.write_command(cmd)

        reply = self.parse_reply()
        self.assertEqual(reply.command_name, 'reply')


class TestProbeSCTP(mtrpacket.MtrPacketTest):
    'Test SCTP probes'

    def test_sctp_v4(self):
        'Test basic SCTP probes over IPv4'

        test_basic_probe(self, 4, 'sctp')

    @unittest.skipUnless(mtrpacket.HAVE_IPV6, 'No IPv6')
    def test_sctp_v6(self):
        'Test basic SCTP probes over IPv6'

        test_basic_probe(self, 6, 'sctp')


if __name__ == '__main__':
    mtrpacket.check_running_as_root()
    unittest.main()
