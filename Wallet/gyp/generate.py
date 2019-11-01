# This file is part of Gram Wallet Desktop,
# a desktop application for the TON Blockchain project.
#
# For license and copyright information please follow this link:
# https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL

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

specialTarget = ''
specialTargetFile = scriptPath + '/../build/target'
if os.path.isfile(specialTargetFile):
    with open(specialTargetFile, 'r') as f:
        for line in f:
            specialTarget = line.strip()

gypScript = 'gyp'
gypFormats = []
gypArguments = []
cmakeConfigurations = []
gypArguments.append('--depth=.')
gypArguments.append('--generator-output=..')
gypArguments.append('-Goutput_dir=../out')
gypArguments.append('-Dspecial_build_target=' + specialTarget)
gypArguments.append('-Dprivate_name=wallet')
gypArguments.append('-Dpublic_key_loc=../SourceFiles/core')

if sys.platform == 'win32':
    gypFormats.append('ninja')
    gypFormats.append('msvs-ninja')
elif sys.platform == 'darwin':
    # use patched gyp with Xcode project generator
    gypScript = '../../../Libraries/gyp/gyp'
    gypArguments.append('-Gxcode_upgrade_check_project_version=1030')
    gypFormats.append('xcode')
else:
    gypScript = '../../../Libraries/gyp/gyp'
    gypFormats.append('cmake')
    cmakeConfigurations.append('Debug')
    cmakeConfigurations.append('Release')

os.chdir(scriptPath)
if sys.platform == 'darwin':
    subprocess.call('mkdir -p ../../out', shell=True)
for format in gypFormats:
    command = gypArguments[:]
    command.insert(0, gypScript)
    command.append('--format=' + format)
    command.append('Wallet.gyp')
    result = subprocess.call(' '.join(command), shell=True)
    if result != 0:
        print('[ERROR] Failed generating for format: ' + format)
        finish(result)

os.chdir(scriptPath + '/../../out')
for configuration in cmakeConfigurations:
    os.chdir(configuration)
    result = subprocess.call('cmake "-GCodeBlocks - Unix Makefiles" .', shell=True)
    if result != 0:
        print('[ERROR] Failed calling cmake for ' + configuration)
        finish(result)
    os.chdir('..')

finish(0)
