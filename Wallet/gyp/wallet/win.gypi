# This file is part of TON Desktop Wallet,
# a desktop application for the TON Blockchain project.
#
# For license and copyright information please follow this link:
# https://github.com/ton-blockchain/wallet/blob/master/LEGAL

{
  'conditions': [[ 'build_win', {
    'libraries': [
      '-lzlibstat',
      '-lUxTheme',
      '-lDwmapi',
      '-lDbgHelp',
    ],
    'msvs_settings': {
      'VCManifestTool': {
        'AdditionalManifestFiles': '<(res_loc)/win/Wallet.manifest',
      }
    },
    'configurations': {
      'Debug': {
        'library_dirs': [
          '<(libs_loc)/zlib/contrib/vstudio/vc14/x86/ZlibStatDebug',
        ],
      },
      'Release': {
        'library_dirs': [
          '<(libs_loc)/zlib/contrib/vstudio/vc14/x86/ZlibStatReleaseWithoutAsm',
        ],
      },
    },
  }]],
}
