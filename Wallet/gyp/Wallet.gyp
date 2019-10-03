# This file is part of TON Desktop Wallet,
# a desktop application for the TON Blockchain project.
#
# For license and copyright information please follow this link:
# https://github.com/ton-blockchain/wallet/blob/master/LEGAL

{
  'includes': [
    'helpers/common/common.gypi',
  ],
  'targets': [{
    'target_name': 'Wallet',
    'variables': {
      'src_loc': '../SourceFiles',
      'res_loc': '../Resources',
      'style_files': [
        '<(src_loc)/wallet/wallet.style',
      ],
      'dependent_style_files': [
        '<(submodules_loc)/lib_ui/ui/colors.palette',
        '<(submodules_loc)/lib_ui/ui/basic.style',
        '<(submodules_loc)/lib_ui/ui/layers/layers.style',
        '<(submodules_loc)/lib_ui/ui/widgets/widgets.style',
      ],
      'style_timestamp': '<(SHARED_INTERMEDIATE_DIR)/update_dependent_styles.timestamp',
      'qrc_timestamp': '<(SHARED_INTERMEDIATE_DIR)/update_dependent_qrc.timestamp',
      'build_defines%': '',
      'list_sources_command': 'python <(submodules_loc)/lib_base/gyp/list_sources.py --input <(DEPTH)/wallet/sources.txt --replace src_loc=<(src_loc)',
      'pch_source': '<(src_loc)/core/pch.cpp',
      'pch_header': '<(src_loc)/core/pch.h',
    },
    'includes': [
      'helpers/common/executable.gypi',
      'helpers/modules/openssl.gypi',
      'helpers/modules/qt.gypi',
      'helpers/modules/pch.gypi',
      'wallet/qrc.gypi',
      'wallet/win.gypi',
      'wallet/mac.gypi',
      'wallet/linux.gypi',
      '../lib_ui/gyp/styles_rule.gypi',
      '../lib_ui/gyp/qrc_rule.gypi',
    ],

    'dependencies': [
      '<(submodules_loc)/codegen/codegen.gyp:codegen_style',
      '<(submodules_loc)/lib_base/lib_base.gyp:lib_base',
      '<(submodules_loc)/lib_ui/lib_ui.gyp:lib_ui',
      '<(submodules_loc)/lib_ton/lib_ton.gyp:lib_ton',
      '<(submodules_loc)/lib_lottie/lib_lottie.gyp:lib_lottie',
    ],

    'defines': [
      '<!@(python -c "for s in \'<(build_defines)\'.split(\',\'): print(s)")',
    ],
    'conditions': [[ '"<(official_build_target)" != ""', {
      'defines': [
        'WALLET_OFFICIAL_BUILD',
      ],
    }]],
    'include_dirs': [
      '<(src_loc)',
      '<(SHARED_INTERMEDIATE_DIR)',
      '<(libs_loc)/range-v3/include',
      '<(submodules_loc)/GSL/include',
      '<(submodules_loc)/variant/include',
      '<(submodules_loc)/crl/src',
    ],
    'sources': [
      '<@(qrc_files)',
      '<@(style_files)',
      '<(DEPTH)/wallet/sources.txt',
      '<!@(<(list_sources_command) <(qt_moc_list_sources_arg))',
    ],
    'sources!': [
      '<!@(<(list_sources_command) --exclude_for <(build_os))',
    ],
  }],
}
