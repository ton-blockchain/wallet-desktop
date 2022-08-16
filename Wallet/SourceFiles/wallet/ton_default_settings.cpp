// This file is part of TON Wallet Desktop,
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

	auto test = QFile(":/config/test-default.json");
	test.open(QIODevice::ReadOnly);
	result.test.config = test.readAll();
	result.test.blockchainName = "mainnet";
	result.test.configUrl = "https://ton.org/global-config-wallet.json";

	//auto main = QFile(":/config/default.json"); // #TODO postponed
	//main.open(QIODevice::ReadOnly);
	//result.main.config = main.readAll();
	//result.main.blockchainName = "mainnet";
	//result.main.configUrl = "https://ton.org/config.json";

	result.useNetworkCallbacks = false;
	result.useTestNetwork = true; // #TODO postponed
	result.version = 2;
	return result;
}

} // namespace Wallet
