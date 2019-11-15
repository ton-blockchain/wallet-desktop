// This file is part of Gram Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#pragma once

#include "base/weak_ptr.h"

class QEvent;
class QKeyEvent;
class QWidget;

namespace Ui {
class Window;
class LayerManager;
} // namespace Ui

namespace Ton {
class Wallet;
class AccountViewer;
} // namespace Ton

namespace Wallet {

class Window;
class Intro;
class Info;
class UpdateInfo;
class ConfigUpgradeChecker;

class Application final : public base::has_weak_ptr {
public:
	explicit Application(const QString &path);
	Application(const Application &other) = delete;
	Application &operator=(const Application &other) = delete;
	~Application();

	void run();
	QWidget *handleCommandGetActivated(const QByteArray &command);

private:
	void installGlobalShortcutFilter();
	void openWallet();
	void criticalError(const QString &text);
	void handleLaunchCommand();
	bool handleCommand(const QByteArray &command);
	UpdateInfo *walletUpdateInfo();

	const QString _path;
	const std::unique_ptr<Ton::Wallet> _wallet;
	std::unique_ptr<Window> _window;
	std::unique_ptr<UpdateInfo> _updateInfo;
	std::unique_ptr<ConfigUpgradeChecker> _upgradeChecker;
	QByteArray _launchCommand;

	rpl::lifetime _lifetime;

};

} // namespace Wallet
