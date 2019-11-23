# SPDX-License-Identifier: AGPL-1.0-only
# Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
import os
import shutil
import win32crypt
import base64
import sys
import zipfile
import re
import json

CONFIG = json.loads(open('../misc/config/build_py_conf.json', 'r').read())
SIGNTOOL_PASS = str(win32crypt.CryptUnprotectData(base64.b64decode(CONFIG["SIGNTOOL_ENC_PASS"]), None, None, None, 0)[1].decode('utf-16-le'))

def RmTree(path):
  try:
    print ('Deleting %s' % path)
    shutil.rmtree(path)
  except FileNotFoundError:
    pass
  
def Run(s):
  print ('Running %s' % s)
  x = os.system(s)
  if x:
    raise Exception('Command failed (%d) : %s' % (x, s))

def CopyFile(src, dst):
  shutil.copyfile(src, dst)

def SignExe(src):
  print ('Signing %s' % src)
  cmd = r'""%s" sign /f "%s" /p %s /t http://timestamp.verisign.com/scripts/timstamp.dll "%s"' % (CONFIG["SIGNTOOL_PATH"], CONFIG["SIGNTOOL_KEY_PATH"], SIGNTOOL_PASS, src)
  #cmd = r'""c:\Program Files (x86)\Windows Kits\10\bin\10.0.15063.0\x86\signtool.exe" sign %s ' % (SIGNTOOL_KEY_PATH, )
  x = os.system(cmd)
  if x:
    raise Exception('Signing failed (%d) : %s' % (x, cmd))

def GetVersion():
  for line in open(BASE + '/tunsafe_config.h', 'r'):
    m = re.match('^#define TUNSAFE_VERSION_STRING "TunSafe (.*)"$', line)
    if m:
      return m.group(1)
  raise Exception('Version not found')

#

#os.system(r'""')

command = sys.argv[1]

BASE = os.getcwd()

if command == 'build_tap':
  Run(r'%s /V4 installer\tap\tap-windows6.nsi'  % CONFIG["NSIS_PATH"])
  SignExe(r'installer\tap\TunSafe-TAP-9.21.2.exe')
  sys.exit(0)

if command != 'skip_build':
  RmTree(BASE + r'\build')
  Run('%s TunSafe.sln /t:Clean;Rebuild /p:Configuration=Release /m /p:Platform=x64' % CONFIG["MSBUILD_PATH"])
  Run('%s TunSafe.sln /t:Clean;Rebuild /p:Configuration=Release /m /p:Platform=Win32' % CONFIG["MSBUILD_PATH"])

if 1:
  try:
    os.mkdir(BASE + r'\installer\x86')
  except OSError:
    pass
  CopyFile(BASE + r'\build\Win32_Release\TunSafe.exe', BASE + r'\installer\x86\TunSafe.exe')
  CopyFile(BASE + r'\build\Win32_Release\ts.exe', BASE + r'\installer\x86\TunSafe.com')
  SignExe(BASE + r'\installer\x86\TunSafe.exe')
  SignExe(BASE + r'\installer\x86\TunSafe.com')

  try:
    os.mkdir(BASE + r'\installer\x64')
  except OSError:
    pass
  CopyFile(BASE + r'\build\x64_Release\TunSafe.exe',   BASE + r'\installer\x64\TunSafe.exe')
  CopyFile(BASE + r'\build\x64_Release\ts.exe',   BASE + r'\installer\x64\TunSafe.com')
  SignExe(BASE + r'\installer\x64\TunSafe.exe')
  SignExe(BASE + r'\installer\x64\TunSafe.com')

VERSION = GetVersion()

Run(r'%s /V4 -DPRODUCT_VERSION=%s installer\tunsafe.nsi ' % (CONFIG["NSIS_PATH"], VERSION))
SignExe(BASE + r'\installer\TunSafe-%s.exe' % VERSION)

zipf = zipfile.ZipFile(BASE + '\installer\TunSafe-%s-x86.zip' % VERSION, 'w', zipfile.ZIP_DEFLATED)
zipf.write(BASE + r'\installer\x86\TunSafe.exe', 'TunSafe.exe')
zipf.write(BASE + r'\installer\x86\TunSafe.com', 'TunSafe.com')
zipf.write(BASE + r'\installer\License.txt', 'License.txt')
zipf.write(BASE + r'\installer\ChangeLog.txt', 'ChangeLog.txt')
zipf.write(BASE + r'\installer\TunSafe.conf', 'Config\\TunSafe.conf')
zipf.close()

zipf = zipfile.ZipFile(BASE + '\installer\TunSafe-%s-x64.zip' % VERSION, 'w', zipfile.ZIP_DEFLATED)
zipf.write(BASE + r'\installer\x64\TunSafe.exe', 'TunSafe.exe')
zipf.write(BASE + r'\installer\x64\TunSafe.com', 'TunSafe.com')
zipf.write(BASE + r'\installer\License.txt', 'License.txt')
zipf.write(BASE + r'\installer\ChangeLog.txt', 'ChangeLog.txt')
zipf.write(BASE + r'\installer\TunSafe.conf', 'Config\\TunSafe.conf')
zipf.close()
