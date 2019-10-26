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
      'VCLinkerTool': {
        'AdditionalOptions': [
          'windows/common.lib',
          'windows/handler/exception_handler.lib',
          'windows/crash_generation/crash_generation_client.lib',
        ],
      },
      'VCManifestTool': {
        'AdditionalManifestFiles': '<(res_loc)/win/Wallet.manifest',
      }
    },
    'configurations': {
      'Debug': {
        'library_dirs': [
          '<(libs_loc)/zlib/contrib/vstudio/vc14/x86/ZlibStatDebug',
          '<(libs_loc)/breakpad/src/out/Debug/obj/client',
        ],
      },
      'Release': {
        'library_dirs': [
          '<(libs_loc)/zlib/contrib/vstudio/vc14/x86/ZlibStatReleaseWithoutAsm',
          '<(libs_loc)/breakpad/src/out/Release/obj/client',
        ],
      },
    },
  }]],
}
