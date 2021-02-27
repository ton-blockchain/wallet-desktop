// This file is part of TON Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#pragma once

namespace Ton {
struct Settings;
} // namespace Ton

namespace Wallet {

[[nodiscard]] Ton::Settings GetDefaultSettings();

} // namespace Wallet
