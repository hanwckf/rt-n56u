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

'''Test probe customization parameters'''

import sys
import unittest

import mtrpacket


@unittest.skipIf(sys.platform == 'cygwin', 'No Cygwin test')
class TestParameters(mtrpacket.MtrPacketTest):
    'Use parameter arguments to mtr-packet and examine the resulting packet'

    def test_size(self):
        'Test probes sent with an explicit packet size'

        with mtrpacket.PacketListen('-4') as listen:
            cmd = '20 send-probe ip-4 127.0.0.1 size 512'

            self.write_command(cmd)

        self.assertEqual(listen.attrib['size'], '512')

    def test_pattern(self):
        'Test probes are filled with the requested bit pattern'

        with mtrpacket.PacketListen('-4') as listen:
            cmd = '20 send-probe ip-4 127.0.0.1 bit-pattern 44'

            self.write_command(cmd)

        self.assertEqual(listen.attrib['bitpattern'], '44')

    def test_tos(self):
        'Test setting the TOS field'

        with mtrpacket.PacketListen('-4') as listen:
            cmd = '20 send-probe ip-4 127.0.0.1 tos 62'

            self.write_command(cmd)

        self.assertEqual(listen.attrib['tos'], '62')


@unittest.skipIf(sys.platform == 'cygwin', 'No Cygwin test')
class TestIPv6Parameters(mtrpacket.MtrPacketTest):
    'Test packet paramter customization for IPv6'

    @unittest.skipUnless(mtrpacket.HAVE_IPV6, 'No IPv6')
    def test_param(self):
        'Test a variety of packet parameters'

        with mtrpacket.PacketListen('-6') as listen:
            param = 'size 256 bit-pattern 51 tos 77'
            cmd = '20 send-probe ip-6 ::1 ' + param

            self.write_command(cmd)

        self.assertEqual(listen.attrib['size'], '256')
        self.assertEqual(listen.attrib['bitpattern'], '51')


if __name__ == '__main__':
    mtrpacket.check_running_as_root()
    unittest.main()
