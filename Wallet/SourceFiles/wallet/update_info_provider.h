// This file is part of Gram Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#pragma once

#ifdef WALLET_AUTOUPDATING_BUILD

#include "wallet/wallet_update_info.h"

namespace Updater {
class Instance;
} // namespace Updater

namespace Wallet {

class UpdateInfoProvider : public UpdateInfo {
public:
	UpdateInfoProvider(
		not_null<Updater::Instance*> updater,
		Fn<bool()> toggled,
		Fn<void(bool)> toggle,
		Fn<void()> install);

	rpl::producer<> checking() override;
	rpl::producer<> isLatest() override;
	rpl::producer<UpdateProgress> progress() override;
	rpl::producer<> failed() override;
	rpl::producer<> ready() override;

	UpdateState state() override;
	int64 already() override;
	int64 size() override;

	void toggle(bool enabled) override;
	bool toggled() override;
	void check() override;
	void test() override;
	void install() override;

	int currentVersion() override;

private:
	const not_null<Updater::Instance*> _updater;
	Fn<bool()> _toggled;
	Fn<void(bool)> _toggle;
	Fn<void()> _install;

};

} // namespace Wallet

#endif // WALLET_AUTOUPDATING_BUILD
