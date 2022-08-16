# This file is part of TON Wallet Desktop,
# a desktop application for the TON Blockchain project.
#
# For license and copyright information please follow this link:
# https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL

if (NOT disable_autoupdate)
    target_compile_definitions(Wallet PRIVATE WALLET_AUTOUPDATING_BUILD)
endif()
