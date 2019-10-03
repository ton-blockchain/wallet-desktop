# This file is part of TON Desktop Wallet,
# a desktop application for the TON Blockchain project.
#
# For license and copyright information please follow this link:
# https://github.com/ton-blockchain/wallet/blob/master/LEGAL

import sys, os, re, subprocess

def finish(code):
    global executePath
    os.chdir(executePath)
    sys.exit(code)

if sys.platform == 'win32' and not 'COMSPEC' in os.environ:
    print('[ERROR] COMSPEC environment variable is not set.')
    finish(1)

executePath = os.getcwd()
scriptPath = os.path.dirname(os.path.realpath(__file__))

inputVersion = ''
versionMajor = ''
versionMinor = ''
versionPatch = ''
for arg in sys.argv:
  match = re.match(r'^\s*(\d+)\.(\d+)(\.(\d+))?\s*$', arg)
  if match:
    inputVersion = arg
    versionMajor = match.group(1)
    versionMinor = match.group(2)
    versionPatch = match.group(4) if match.group(4) else '0'

if not len(versionMajor):
  print("Wrong version parameter")
  finish(1)

def checkVersionPart(part):
  cleared = int(part) % 1000 if len(part) > 0 else 0
  if str(cleared) != part:
    print("Bad version part: " + part)
    finish(1)

checkVersionPart(versionMajor)
checkVersionPart(versionMinor)
checkVersionPart(versionPatch)

versionFull = str(int(versionMajor) * 1000000 + int(versionMinor) * 1000 + int(versionPatch))
versionStr = versionMajor + '.' + versionMinor + '.' + versionPatch
versionStrSmall = versionStr if versionPatch != '0' else versionMajor + '.' + versionMinor

print('Setting version: ' + versionStr + ' stable')

def replaceInFile(path, replacements):
  content = ''
  foundReplacements = {}
  updated = False
  with open(path, 'r') as f:
    for line in f:
      for replacement in replacements:
        if re.search(replacement[0], line):
          changed = re.sub(replacement[0], replacement[1], line)
          if changed != line:
            line = changed
            updated = True
          foundReplacements[replacement[0]] = True
      content = content + line
  for replacement in replacements:
    if not replacement[0] in foundReplacements:
      print('Could not find "' + replacement[0] + '" in "' + path + '".')
      finish(1)
  if updated:
    with open(path, 'w') as f:
      f.write(content)

print('Patching build/version...')
replaceInFile(scriptPath + '/version', [
  [ r'(AppVersion\s+)\d+', r'\g<1>' + versionFull ],
  [ r'(AppVersionStrMajor\s+)\d[\d\.]*', r'\g<1>' + versionMajor + '.' + versionMinor ],
  [ r'(AppVersionStrSmall\s+)\d[\d\.]*', r'\g<1>' + versionStrSmall ],
  [ r'(AppVersionStr\s+)\d[\d\.]*', r'\g<1>' + versionStr ],
])

parts = [versionMajor, versionMinor, versionPatch, '0']
withcomma = ','.join(parts)
withdot = '.'.join(parts)
rcReplaces = [
  [ r'(FILEVERSION\s+)\d+,\d+,\d+,\d+', r'\g<1>' + withcomma ],
  [ r'(PRODUCTVERSION\s+)\d+,\d+,\d+,\d+', r'\g<1>' + withcomma ],
  [ r'("FileVersion",\s+)"\d+\.\d+\.\d+\.\d+"', r'\g<1>"' + withdot + '"' ],
  [ r'("ProductVersion",\s+)"\d+\.\d+\.\d+\.\d+"', r'\g<1>"' + withdot + '"' ],
]

print('Patching Wallet.rc...')
replaceInFile(scriptPath + '/../Resources/win/Wallet.rc', rcReplaces)
