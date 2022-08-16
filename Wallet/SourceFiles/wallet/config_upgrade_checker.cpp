// This file is part of TON Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#include "wallet/config_upgrade_checker.h"

#include "ton/ton_wallet.h"
#include "ton/ton_state.h"

namespace Wallet {

ConfigUpgradeChecker::ConfigUpgradeChecker(not_null<Ton::Wallet*> wallet) {
	wallet->updates(
	) | rpl::filter([](const Ton::Update &update) {
		return update.data.is<Ton::ConfigUpgrade>();
	}) | rpl::start_with_next([=](const Ton::Update &update) {
		_upgrades.push_back(update.data.get<Ton::ConfigUpgrade>());
	}, _lifetime);
}

std::vector<Ton::ConfigUpgrade> ConfigUpgradeChecker::takeUpgrades() {
	return std::move(_upgrades);
}

} // namespace Wallet
