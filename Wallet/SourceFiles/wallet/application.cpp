// This file is part of TON Desktop Wallet,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet/blob/master/LEGAL
//
#include "wallet/application.h"

#include "wallet/phrases.h"
#include "ui/widgets/window.h"
#include "ui/text/text_utilities.h"
#include "ui/rp_widget.h"
#include "core/sandbox.h"
#include "base/platform/base_platform_info.h"
#include "base/call_delayed.h"
#include "base/openssl_help.h"
#include "styles/style_wallet.h"
#include "styles/style_widgets.h"
#include "styles/palette.h"

#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtGui/QtEvents>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QFileDialog>

namespace Wallet {

Application::Application()
: _window(std::make_unique<Ui::Window>()) {
	QApplication::setWindowIcon(QIcon(QPixmap(":/gui/art/logo.png", "PNG")));
	initWindow();
	crl::async([] {
		// Init random, because it is slow.
		static_cast<void>(openssl::RandomValue<uint8>());
	});
}

Application::~Application() {
}

void Application::run() {
	_window->show();
	_window->setFocus();
}

void Application::initWindow() {
	_window->setTitle(tr::lng_window_title(tr::now));
	_window->setSizeMin(st::windowSizeMin);
	_window->setGeometry(style::centerrect(
		QApplication::desktop()->geometry(),
		QRect(QPoint(), st::windowSize)));

	updateWindowPalette();
	style::PaletteChanged(
	) | rpl::start_with_next([=] {
		updateWindowPalette();
	}, _window->lifetime());

	_window->events(
	) | rpl::start_with_next([=](not_null<QEvent*> e) {
		handleWindowEvent(e);
	}, _window->lifetime());
}

void Application::updateWindowPalette() {
	auto palette = _window->palette();
	palette.setColor(QPalette::Window, st::windowBg->c);
	_window->setPalette(palette);
	Ui::ForceFullRepaint(_window.get());
}

void Application::handleWindowEvent(not_null<QEvent*> e) {
	if (e->type() == QEvent::KeyPress) {
		handleWindowKeyPress(static_cast<QKeyEvent*>(e.get()));
	}
}

void Application::handleWindowKeyPress(not_null<QKeyEvent*> e) {
	const auto key = e->key();
	const auto modifiers = e->modifiers();
	switch (key) {
	case Qt::Key_Q:
		if (modifiers & Qt::ControlModifier) {
			QApplication::quit();
		}
		break;
	}
}

} // namespace Wallet
