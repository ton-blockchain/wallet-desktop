// This file is part of Gram Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#include "wallet/ton_default_settings.h"

#include "ton/ton_settings.h"

#include <QtCore/QFile>

namespace Wallet {

Ton::Settings GetDefaultSettings() {
	auto result = Ton::Settings();
	auto file = QFile(":/config/test-default.json");
	file.open(QIODevice::ReadOnly);
	result.config = file.readAll();
	result.useNetworkCallbacks = false;
	result.blockchainName = "testnet";
	result.configUrl = "https://test.ton.org/config.json";
	return result;
}

} // namespace Wallet
