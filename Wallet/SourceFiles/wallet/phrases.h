// This file is part of TON Desktop Wallet,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet/blob/master/LEGAL
//
#pragma once

#include "ui/ph.h"

namespace tr {

using now_t = ph::now_t;
inline constexpr auto now = ph::now;

using phrase = ph::phrase;

extern phrase lng_window_title;

} // namespace tr
