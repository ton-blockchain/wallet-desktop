// This file is part of Gram Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#include "wallet/update_info_provider.h"

#include "core/version.h"

#ifdef WALLET_AUTOUPDATING_BUILD

#include "updater/updater_instance.h"

namespace Wallet {

UpdateInfoProvider::UpdateInfoProvider(
	not_null<Updater::Instance*> updater,
	Fn<bool()> toggled,
	Fn<void(bool)> toggle,
	Fn<void()> install)
: _updater(updater)
, _toggled(toggled)
, _toggle(toggle)
, _install(install) {
}

rpl::producer<> UpdateInfoProvider::checking() {
	return _updater->checking();
}

rpl::producer<> UpdateInfoProvider::isLatest() {
	return _updater->isLatest();
}

rpl::producer<UpdateProgress> UpdateInfoProvider::progress() {
	return _updater->progress(
	) | rpl::map([](Updater::Progress progress) -> UpdateProgress {
		return { progress.already, progress.size };
	});
}

rpl::producer<> UpdateInfoProvider::failed() {
	return _updater->failed();
}

rpl::producer<> UpdateInfoProvider::ready() {
	return _updater->ready();
}

UpdateState UpdateInfoProvider::state() {
	switch (_updater->state()) {
	case Updater::State::None: return UpdateState::None;
	case Updater::State::Download: return UpdateState::Download;
	case Updater::State::Ready: return UpdateState::Ready;
	}
	Unexpected("State in UpdateInfoProvider::state.");
}

int64 UpdateInfoProvider::already() {
	return _updater->already();
}

int64 UpdateInfoProvider::size() {
	return _updater->size();
}

void UpdateInfoProvider::toggle(bool enabled) {
	if (enabled != toggled()) {
		_toggle(enabled);
	}
}

bool UpdateInfoProvider::toggled() {
	return _toggled();
}

void UpdateInfoProvider::test() {
	_updater->test();
}

void UpdateInfoProvider::install() {
	_install();
}

int UpdateInfoProvider::currentVersion() {
	return AppVersion;
}

} // namespace Wallet

#else // WALLET_AUTOUPDATING_BUILD

namespace Wallet {

UpdateInfoProvider::UpdateInfoProvider(
	not_null<Updater::Instance*> updater,
	Fn<bool()> toggled,
	Fn<void(bool)> toggle,
	Fn<void()> install) {
}

rpl::producer<> UpdateInfoProvider::checking() {
	return rpl::never<>();
}

rpl::producer<> UpdateInfoProvider::isLatest() {
	return rpl::never<>();
}

rpl::producer<UpdateProgress> UpdateInfoProvider::progress() {
	return rpl::never<UpdateProgress>();
}

rpl::producer<> UpdateInfoProvider::failed() {
	return rpl::never<>();
}

rpl::producer<> UpdateInfoProvider::ready() {
	return rpl::never<>();
}

UpdateState UpdateInfoProvider::state() {
	return UpdateState::None;
}

int64 UpdateInfoProvider::already() {
	return 0;
}

int64 UpdateInfoProvider::size() {
	return 0;
}

void UpdateInfoProvider::toggle(bool enabled) {
}

bool UpdateInfoProvider::toggled() {
	return false;
}

void UpdateInfoProvider::test() {
}

void UpdateInfoProvider::install() {
}

int UpdateInfoProvider::currentVersion() {
	return AppVersion;
}

} // namespace Wallet

#endif // WALLET_AUTOUPDATING_BUILD
