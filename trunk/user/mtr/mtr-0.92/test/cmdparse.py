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

'''Test mtr-packet's command parsing.'''


import time
import unittest

import mtrpacket


class TestCommandParse(mtrpacket.MtrPacketTest):
    '''Test cases with malformed commands and version checks'''

    def test_unknown_command(self):
        'Test sending a command unknown to mtr-packet'

        self.write_command('13 argle-bargle')
        self.assertEqual(self.read_reply(), '13 unknown-command')

    def test_malformed_command(self):
        'Test sending a malformed command request to mtr-packet'

        self.write_command('malformed')
        self.assertEqual(self.read_reply(), '0 command-parse-error')

    def test_exit_on_stdin_closed(self):
        '''Test that the packet process terminates after stdin is closed

        Test that, when outstanding requests are complete, the process
        terminates following stdin being closed.'''

        self.write_command('15 send-probe ip-4 8.8.254.254 timeout 1')
        self.packet_process.stdin.close()
        time.sleep(2)
        self.read_reply()
        exit_code = self.packet_process.poll()
        self.assertIsNotNone(exit_code)

    def test_invalid_argument(self):
        'Test sending invalid arguments with probe requests'

        bad_commands = [
            '22 send-probe',
            '23 send-probe ip-4 str-value',
            '24 send-probe ip-4 8.8.8.8 timeout str-value',
            '25 send-probe ip-4 8.8.8.8 ttl str-value',
        ]

        for cmd in bad_commands:
            self.write_command(cmd)
            reply = self.parse_reply()
            self.assertEqual(reply.command_name, 'invalid-argument')

    def test_versioning(self):
        'Test version checks and feature support checks'

        feature_tests = [
            ('31 check-support feature ip-4', 'ok'),
            ('32 check-support feature send-probe', 'ok'),
            ('33 check-support feature bogus-feature', 'no')
        ]

        self.write_command('30 check-support feature version')
        reply = self.parse_reply()
        self.assertEqual(reply.token, 30)
        self.assertEqual(reply.command_name, 'feature-support')
        self.assertIn('support', reply.argument)

        for (request, expected) in feature_tests:
            self.write_command(request)
            reply = self.parse_reply()
            self.assertEqual(reply.command_name, 'feature-support')
            self.assertIn('support', reply.argument)
            self.assertEqual(reply.argument['support'], expected)

    def test_command_overflow(self):
        'Test overflowing the incoming command buffer'

        big_buffer = 'x' * (64 * 1024)
        self.write_command(big_buffer)

        reply = self.read_reply()
        self.assertEqual(reply, '0 command-buffer-overflow')


if __name__ == '__main__':
    mtrpacket.check_running_as_root()
    unittest.main()
