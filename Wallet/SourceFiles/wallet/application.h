// This file is part of TON Desktop Wallet,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet/blob/master/LEGAL
//
#pragma once

#include "base/weak_ptr.h"

class QEvent;
class QKeyEvent;

namespace Ui {
class Window;
class LayerManager;
} // namespace Ui

namespace Ton {
class Wallet;
} // namespace Ton

namespace Wallet {

class Intro;

class Application final : public base::has_weak_ptr {
public:
	explicit Application(const QString &path);
	Application(const Application &other) = delete;
	Application &operator=(const Application &other) = delete;
	~Application();

	void run();

private:
	void initWindow();
	void updateWindowPalette();
	void handleWindowEvent(not_null<QEvent*> e);
	void handleWindowKeyPress(not_null<QKeyEvent*> e);
	void openWallet();
	void showIntro();
	void showWallet();

	const QString _path;
	const std::unique_ptr<Ton::Wallet> _wallet;
	const std::unique_ptr<Ui::Window> _window;
	const std::unique_ptr<Ui::LayerManager> _layers;

	std::unique_ptr<Intro> _intro;

	rpl::lifetime _lifetime;

};

} // namespace Wallet
