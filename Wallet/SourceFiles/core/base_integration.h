// This file is part of Gram Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#pragma once

#include "base/integration.h"

namespace Core {

class BaseIntegration : public base::Integration {
public:
	BaseIntegration(int argc, char *argv[]);

	void enterFromEventLoop(FnMut<void()> &&method) override;
	void logAssertionViolation(const QString &info) override;

};

} // namespace Core
