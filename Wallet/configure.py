# This file is part of Gram Wallet Desktop,
# a desktop application for the TON Blockchain project.
#
# For license and copyright information please follow this link:
# https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL

import sys, os, re

sys.dont_write_bytecode = True
scriptPath = os.path.dirname(os.path.realpath(__file__))
sys.path.append(scriptPath + '/../cmake')
import run_cmake

executePath = os.getcwd()
def finish(code):
    global executePath
    os.chdir(executePath)
    sys.exit(code)

def error(message):
    print('[ERROR] ' + message)
    finish(1)

if sys.platform == 'win32' and not 'COMSPEC' in os.environ:
    error('COMSPEC environment variable is not set.')

executePath = os.getcwd()
scriptPath = os.path.dirname(os.path.realpath(__file__))
scriptName = os.path.basename(scriptPath)

arguments = sys.argv[1:]

officialTarget = ''
officialTargetFile = scriptPath + '/build/target'
if os.path.isfile(officialTargetFile):
    with open(officialTargetFile, 'r') as f:
        for line in f:
            officialTarget = line.strip()

if officialTarget != '':
    arguments.append('-DDESKTOP_APP_UPDATER_PUBLIC_KEY_LOC=SourceFiles/core')
    arguments.append('-DDESKTOP_APP_UPDATER_PRIVATE_NAME=wallet')
    finish(run_cmake.run(scriptName, arguments))
elif 'linux' in sys.platform:
    debugCode = run_cmake.run(scriptName, arguments, "Debug")
    finish(debugCode if debugCode else run_cmake.run(scriptName, arguments, "Release"))
else:
    finish(run_cmake.run(scriptName, arguments))
