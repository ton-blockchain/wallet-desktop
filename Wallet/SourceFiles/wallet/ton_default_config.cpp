// This file is part of Gram Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#include "wallet/ton_default_config.h"

#include "ton/ton_config.h"

#include <QtCore/QFile>

namespace Wallet {

Ton::Config GetDefaultConfig() {
	auto result = Ton::Config();
	auto file = QFile(":/config/default.json");
	file.open(QIODevice::ReadOnly);
	result.json = file.readAll();
	result.ignoreCache = result.useNetworkCallbacks = false;
	result.blockchainName = QString();
	return result;
}

} // namespace Wallet
