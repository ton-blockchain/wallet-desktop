# This file is part of Gram Wallet Desktop,
# a desktop application for the TON Blockchain project.
#
# For license and copyright information please follow this link:
# https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL

{
  'conditions': [[ 'build_linux', {
    'variables': {
      'variables': {
        'build_defines%': '',
      },
      'pkgconfig_libs': [
# In order to work libxkbcommon must be linked statically,
# PKGCONFIG links it like "-L/usr/local/lib -lxkbcommon"
# which makes a dynamic link which leads to segfault in
# QApplication() -> createPlatformIntegration -> QXcbIntegrationPlugin::create
        #'xkbcommon',
      ],
    },
    'libraries': [
      '-Wl,-Bstatic',
      '-lbreakpad_client',
      '-lz',
      '-lXi',
      '-lXext',
      '-lXfixes',
      '-lXrender',
#      '<!(pkg-config 2> /dev/null --libs <@(pkgconfig_libs))',
    ],
    'cflags_cc': [
      '-Wno-strict-overflow',
      '-Wno-maybe-uninitialized',
    ],
    'ldflags': [
      '-Wl,-wrap,aligned_alloc',
      '-Wl,-wrap,secure_getenv',
      '-Wl,-wrap,clock_gettime',
      '-Wl,--no-as-needed,-lrt',
      '-Wl,-Bstatic',
    ],
    'configurations': {
      'Release': {
        'cflags_c': [
          '-Ofast',
          '-fno-strict-aliasing',
        ],
        'cflags_cc': [
          '-Ofast',
          '-fno-strict-aliasing',
        ],
        'ldflags': [
          '-Ofast',
        ],
      },
    },
    'conditions': [
      [ '"<!(uname -p)" != "x86_64"', {
        'ldflags': [
          '-Wl,-wrap,__divmoddi4',
        ],
      }]
    ],
  }]],
}
