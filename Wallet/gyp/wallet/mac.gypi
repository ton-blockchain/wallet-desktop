# This file is part of TON Desktop Wallet,
# a desktop application for the TON Blockchain project.
#
# For license and copyright information please follow this link:
# https://github.com/ton-blockchain/wallet/blob/master/LEGAL

{
  'conditions': [[ 'build_mac', {
    'mac_hardened_runtime': 1,
    'mac_bundle': '1',
    'mac_bundle_resources': [
      '<(res_loc)/art/Images.xcassets',
    ],
    'xcode_settings': {
      'ENABLE_HARDENED_RUNTIME': 'YES',
    },
    'sources': [
      '<(res_loc)/mac/Wallet.entitlements',
    ],
    'xcode_settings': {
      'INFOPLIST_FILE': '<(res_loc)/mac/Wallet.plist',
      'CURRENT_PROJECT_VERSION': '<!(helpers/common/print_version.sh <(DEPTH)/../build/version)',
      'ASSETCATALOG_COMPILER_APPICON_NAME': 'AppIcon',
      'OTHER_LDFLAGS': [
        '-lbsm',
        '-lm',
        '-lbase',
        '-lcrashpad_client',
        '-lcrashpad_util',
      ],
    },
    'include_dirs': [
      '/usr/local/include',
    ],
    'library_dirs': [
      '/usr/local/lib',
    ],
    'configurations': {
      'Debug': {
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '0',
          'PRODUCT_BUNDLE_IDENTIFIER': 'org.ton.wallet.desktop.debug',
        },
        'library_dirs': [
          '<(libs_loc)/crashpad/out/Debug',
        ],
      },
      'Release': {
        'xcode_settings': {
          'DEBUG_INFORMATION_FORMAT': 'dwarf-with-dsym',
          'LLVM_LTO': 'YES',
          'GCC_OPTIMIZATION_LEVEL': 'fast',
          'PRODUCT_BUNDLE_IDENTIFIER': 'org.ton.wallet.desktop',
        },
        'library_dirs': [
          '<(libs_loc)/crashpad/out/Release',
        ],
      },
    },
    'postbuilds': [{
      'postbuild_name': 'Force Helpers path',
      'action': [
        'mkdir', '-p', '${BUILT_PRODUCTS_DIR}/Wallet.app/Contents/Helpers/'
      ],
    }, {
      'postbuild_name': 'Copy crashpad_client to Helpers',
      'action': [
        'cp',
        '<(libs_loc)/crashpad/out/${CONFIGURATION}/crashpad_handler',
        '${BUILT_PRODUCTS_DIR}/Wallet.app/Contents/Helpers/',
      ],
    }],
  }]],
}
