// This file is part of Gram Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#pragma once

#include "base/integration.h"

namespace Core {

class Launcher;

class BaseIntegration : public base::Integration {
public:
	BaseIntegration(int argc, char *argv[], not_null<Launcher*> launcher);

	void enterFromEventLoop(FnMut<void()> &&method) override;
	void logMessage(const QString &message) override;
	void logAssertionViolation(const QString &info) override;

private:
	const not_null<Launcher*> _launcher;

};

} // namespace Core
