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
struct TransactionsSlice;
} // namespace Ton

namespace Wallet {

class Intro;
class Info;

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
	void showInfo();

	const QString _path;
	const std::unique_ptr<Ton::Wallet> _wallet;
	const std::unique_ptr<Ui::Window> _window;
	const std::unique_ptr<Ui::LayerManager> _layers;

	std::unique_ptr<Intro> _intro;
	std::unique_ptr<Info> _info;
	rpl::event_stream<int64> _balance;
	rpl::event_stream<Ton::TransactionsSlice> _lastTransactions;

	rpl::lifetime _lifetime;

};

} // namespace Wallet
