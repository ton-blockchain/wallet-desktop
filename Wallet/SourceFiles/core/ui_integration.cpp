// This file is part of TON Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#include "core/ui_integration.h"

#include "core/sandbox.h"

namespace Core {

void UiIntegration::postponeCall(FnMut<void()> &&callable) {
	Sandbox::Instance().postponeCall(std::move(callable));
}

void UiIntegration::registerLeaveSubscription(not_null<QWidget*> widget) {
	Sandbox::Instance().registerLeaveSubscription(widget);
}

void UiIntegration::unregisterLeaveSubscription(not_null<QWidget*> widget) {
	Sandbox::Instance().unregisterLeaveSubscription(widget);
}

void UiIntegration::writeLogEntry(const QString &entry) {
}

QString UiIntegration::emojiCacheFolder() {
	return QString();
}

void UiIntegration::textActionsUpdated() {
	//if (const auto window = App::wnd()) {
	//	window->updateGlobalMenu();
	//}
}

void UiIntegration::activationFromTopPanel() {
	//Platform::IgnoreApplicationActivationRightNow();
}

} // namespace Core
