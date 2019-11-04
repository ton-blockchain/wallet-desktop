// This file is part of Gram Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#include "core/base_integration.h"

#include "core/launcher.h"
#include "core/sandbox.h"

namespace Core {

BaseIntegration::BaseIntegration(
	int argc,
	char *argv[],
	not_null<Launcher*> launcher)
: Integration(argc, argv)
, _launcher(launcher) {
}

void BaseIntegration::enterFromEventLoop(FnMut<void()> &&method) {
	Core::Sandbox::Instance().customEnterFromEventLoop(
		std::move(method));
}

void BaseIntegration::logMessage(const QString &message) {
	_launcher->logMessage(message);
}

void BaseIntegration::logAssertionViolation(const QString &info) {
	if (QCoreApplication::instance()) {
		_launcher->logMessage("Assertion Failed! " + info);
		Core::Sandbox::Instance().reportAssertionViolation(info);
	}
}

} // namespace Core
