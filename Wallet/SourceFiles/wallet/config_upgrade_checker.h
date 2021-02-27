// This file is part of TON Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#pragma once

namespace Ton {
class Wallet;
enum class ConfigUpgrade;
} // namespace Ton

namespace Wallet {

class ConfigUpgradeChecker final {
public:
	ConfigUpgradeChecker(not_null<Ton::Wallet*> wallet);

	std::vector<Ton::ConfigUpgrade> takeUpgrades();

private:
	std::vector<Ton::ConfigUpgrade> _upgrades;
	rpl::lifetime _lifetime;

};

} // namespace Wallet
