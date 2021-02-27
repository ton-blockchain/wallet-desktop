// This file is part of TON Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#pragma once

#include "ui/integration.h"

namespace Core {

class UiIntegration : public Ui::Integration {
public:
	void postponeCall(FnMut<void()> &&callable) override;
	void registerLeaveSubscription(not_null<QWidget*> widget) override;
	void unregisterLeaveSubscription(not_null<QWidget*> widget) override;

	void writeLogEntry(const QString &entry) override;
	QString emojiCacheFolder() override;

	void textActionsUpdated() override;
	void activationFromTopPanel() override;

};

} // namespace Core
