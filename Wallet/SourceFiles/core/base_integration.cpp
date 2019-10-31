// This file is part of Gram Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#include "core/base_integration.h"

#include "core/sandbox.h"

namespace Core {

BaseIntegration::BaseIntegration(int argc, char *argv[])
: Integration(argc, argv) {
}

void BaseIntegration::enterFromEventLoop(FnMut<void()> &&method) {
	Core::Sandbox::Instance().customEnterFromEventLoop(
		std::move(method));
}

void BaseIntegration::logAssertionViolation(const QString &info) {
#ifdef LOG
	LOG(("Assertion Failed! ") + info);
#endif // LOG
	if (QCoreApplication::instance()) {
		Core::Sandbox::Instance().reportAssertionViolation(info);
	}
}

} // namespace Core
