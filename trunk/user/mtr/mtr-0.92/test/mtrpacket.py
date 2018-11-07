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

'''Infrastructure for running tests which invoke mtr-packet.'''

import fcntl
import os
import select
import socket
import subprocess
import sys
import time
import unittest

#
#  typing is used for mypy type checking, but isn't required to run,
#  so it's okay if we can't import it.
#
try:
    # pylint: disable=locally-disabled, unused-import
    from typing import Dict, List
except ImportError:
    pass


IPV6_TEST_HOST = 'google-public-dns-a.google.com'


class MtrPacketExecuteError(Exception):
    "Exception raised when MtrPacketTest can't execute mtr-packet"
    pass


class ReadReplyTimeout(Exception):
    'Exception raised by TestProbe.read_reply upon timeout'

    pass


class WriteCommandTimeout(Exception):
    'Exception raised by TestProbe.write_command upon timeout'

    pass


class MtrPacketReplyParseError(Exception):
    "Exception raised when MtrPacketReply can't parse the reply string"

    pass


class PacketListenError(Exception):
    'Exception raised when we have unexpected results from mtr-packet-listen'

    pass


def set_nonblocking(file_descriptor):  # type: (int) -> None
    'Put a file descriptor into non-blocking mode'

    flags = fcntl.fcntl(file_descriptor, fcntl.F_GETFL)

    # pylint: disable=locally-disabled, no-member
    fcntl.fcntl(file_descriptor, fcntl.F_SETFL, flags | os.O_NONBLOCK)


def check_for_local_ipv6():
    '''Check for IPv6 support on the test host, to see if we should skip
    the IPv6 tests'''

    addrinfo = socket.getaddrinfo(IPV6_TEST_HOST, 1, socket.AF_INET6)
    if len(addrinfo):
        addr = addrinfo[0][4]

    #  Create a UDP socket and check to see it can be connected to
    #  IPV6_TEST_HOST.  (Connecting UDP requires no packets sent, just
    #  a route present.)
    sock = socket.socket(
        socket.AF_INET6, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

    connect_success = False
    try:
        sock.connect(addr)
        connect_success = True
    except socket.error:
        pass

    sock.close()

    if not connect_success:
        sys.stderr.write(
            'This host has no IPv6.  Skipping IPv6 tests.\n')

    return connect_success


HAVE_IPV6 = check_for_local_ipv6()


# pylint: disable=locally-disabled, too-few-public-methods
class MtrPacketReply(object):
    'A parsed reply from mtr-packet'

    def __init__(self, reply):  # type: (unicode) -> None
        self.token = 0  # type: int
        self.command_name = None  # type: unicode
        self.argument = {}  # type: Dict[unicode, unicode]

        self.parse_reply(reply)

    def parse_reply(self, reply):  # type (unicode) -> None
        'Parses a reply string into members for the instance of this class'

        tokens = reply.split()  # type List[unicode]

        try:
            self.token = int(tokens[0])
            self.command_name = tokens[1]
        except IndexError:
            raise MtrPacketReplyParseError(reply)

        i = 2
        while i < len(tokens):
            try:
                name = tokens[i]
                value = tokens[i + 1]
            except IndexError:
                raise MtrPacketReplyParseError(reply)

            self.argument[name] = value
            i += 2


class PacketListen(object):
    'A test process which listens for a single packet'

    def __init__(self, *args):
        self.process_args = list(args)  # type: List[unicode]
        self.listen_process = None  # type: subprocess.Popen
        self.attrib = None  # type: Dict[unicode, unicode]

    def __enter__(self):
        try:
            self.listen_process = subprocess.Popen(
                ['./mtr-packet-listen'] + self.process_args,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE)
        except OSError:
            raise PacketListenError('unable to launch mtr-packet-listen')

        status = self.listen_process.stdout.readline().decode('utf-8')
        if status != 'status listening\n':
            raise PacketListenError('unexpected status')

        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.wait_for_exit()

        self.attrib = {}
        for line in self.listen_process.stdout.readlines():
            tokens = line.decode('utf-8').split()

            if len(tokens) >= 2:
                name = tokens[0]
                value = tokens[1]

                self.attrib[name] = value

        self.listen_process.stdin.close()
        self.listen_process.stdout.close()

    def wait_for_exit(self):
        '''Poll the subprocess for up to ten seconds, until it exits.

        We need to wait for its exit to ensure we are able to read its
        output.'''

        wait_time = 10
        wait_step = 0.1

        steps = int(wait_time / wait_step)

        exit_value = None

        # pylint: disable=locally-disabled, unused-variable
        for i in range(steps):
            exit_value = self.listen_process.poll()
            if exit_value is not None:
                break

            time.sleep(wait_step)

        if exit_value is None:
            raise PacketListenError('mtr-packet-listen timeout')

        if exit_value != 0:
            raise PacketListenError('mtr-packet-listen unexpected error')


class MtrPacketTest(unittest.TestCase):
    '''Base class for tests invoking mtr-packet.

    Start a new mtr-packet subprocess for each test, and kill it
    at the conclusion of the test.

    Provide methods for writing commands and reading replies.
    '''

    def __init__(self, *args):
        self.reply_buffer = None  # type: unicode
        self.packet_process = None  # type: subprocess.Popen
        self.stdout_fd = None  # type: int

        super(MtrPacketTest, self).__init__(*args)

    def setUp(self):
        'Set up a test case by spawning a mtr-packet process'

        packet_path = os.environ.get('MTR_PACKET', './mtr-packet')

        self.reply_buffer = ''
        try:
            self.packet_process = subprocess.Popen(
                [packet_path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE)
        except OSError:
            raise MtrPacketExecuteError(packet_path)

        #  Put the mtr-packet process's stdout in non-blocking mode
        #  so that we can read from it without a timeout when
        #  no reply is available.
        self.stdout_fd = self.packet_process.stdout.fileno()
        set_nonblocking(self.stdout_fd)

        self.stdin_fd = self.packet_process.stdin.fileno()
        set_nonblocking(self.stdin_fd)

    def tearDown(self):
        'After a test, kill the running mtr-packet instance'

        self.packet_process.stdin.close()
        self.packet_process.stdout.close()

        try:
            self.packet_process.kill()
        except OSError:
            return

    def parse_reply(self, timeout=10.0):  # type: (float) -> MtrPacketReply
        '''Read the next reply from mtr-packet and parse it into
        an MtrPacketReply object.'''

        reply_str = self.read_reply(timeout)

        return MtrPacketReply(reply_str)

    def read_reply(self, timeout=10.0):  # type: (float) -> unicode
        '''Read the next reply from mtr-packet.

        Attempt to read the next command reply from mtr-packet.  If no reply
        is available withing the timeout time, raise ReadReplyTimeout
        instead.'''

        start_time = time.time()

        #  Read from mtr-packet until either the timeout time has elapsed
        #  or we read a newline character, which indicates a finished
        #  reply.
        while True:
            now = time.time()
            elapsed = now - start_time

            select_time = timeout - elapsed
            if select_time < 0:
                select_time = 0

            select.select([self.stdout_fd], [], [], select_time)

            reply_bytes = None

            try:
                reply_bytes = os.read(self.stdout_fd, 1024)
            except OSError:
                pass

            if reply_bytes:
                self.reply_buffer += reply_bytes.decode('utf-8')

            #  If we have read a newline character, we can stop waiting
            #  for more input.
            newline_ix = self.reply_buffer.find('\n')
            if newline_ix != -1:
                break

            if elapsed >= timeout:
                raise ReadReplyTimeout()

        reply = self.reply_buffer[:newline_ix]
        self.reply_buffer = self.reply_buffer[newline_ix + 1:]
        return reply

    def write_command(self, cmd, timeout=10.0):
        # type: (unicode, float) -> None

        '''Send a command string to the mtr-packet instance, timing out
        if we are unable to write for an extended period of time.  The
        timeout is to avoid deadlocks with the child process where both
        the parent and the child are writing to their end of the pipe
        and expecting the other end to be reading.'''

        command_str = cmd + '\n'
        command_bytes = command_str.encode('utf-8')

        start_time = time.time()

        while True:
            now = time.time()
            elapsed = now - start_time

            select_time = timeout - elapsed
            if select_time < 0:
                select_time = 0

            select.select([], [self.stdin_fd], [], select_time)

            bytes_written = 0
            try:
                bytes_written = os.write(self.stdin_fd, command_bytes)
            except OSError:
                pass

            command_bytes = command_bytes[bytes_written:]
            if not len(command_bytes):
                break

            if elapsed >= timeout:
                raise WriteCommandTimeout()


def check_running_as_root():
    'Print a warning to stderr if we are not running as root.'

    # pylint: disable=locally-disabled, no-member
    if sys.platform != 'cygwin' and os.getuid() > 0:
        sys.stderr.write(
            'Warning: many tests require running as root\n')
