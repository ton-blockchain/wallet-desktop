// This file is part of TON Desktop Wallet,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet/blob/master/LEGAL
//
#include "wallet/application.h"

#include "wallet/phrases.h"
#include "ton/ton_wallet.h"
#include "ui/widgets/window.h"
#include "ui/text/text_utilities.h"
#include "ui/rp_widget.h"
#include "core/sandbox.h"
#include "base/platform/base_platform_info.h"
#include "base/call_delayed.h"
#include "styles/style_wrapper.h"
#include "styles/style_widgets.h"
#include "styles/palette.h"

#include "ui/layers/generic_box.h"
#include "ui/layers/layer_manager.h"
#include "ui/widgets/labels.h"
#include "wallet/wallet_intro.h"
#include "wallet/wallet_phrases.h"
#include "styles/style_layers.h"

#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtGui/QtEvents>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QFileDialog>

namespace Wallet {

Application::Application(const QString &path)
: _path(path)
, _wallet(std::make_unique<Ton::Wallet>(_path))
, _window(std::make_unique<Ui::Window>())
, _layers(std::make_unique<Ui::LayerManager>(_window->body())) {
	QApplication::setWindowIcon(QIcon(QPixmap(":/gui/art/logo.png", "PNG")));
	initWindow();
}

Application::~Application() {
}

void Application::run() {
	_window->show();
	_window->setFocus();
	openWallet();
}

void Application::openWallet() {
	_wallet->open(QByteArray(), crl::guard(this, [=](Ton::Result<> result) {
		if (result) {
			if (_wallet->publicKeys().empty()) {
				showIntro();
			} else {
				showWallet();
			}
			return;
		}
		const auto text = (result.error().type == Ton::Error::Type::IO)
			? "IO error at path: " + result.error().details
			: ("Global Password didn't work.\n\nTry deleting all data at "
				+ _path);
		_layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
			box->setCloseByEscape(false);
			box->setCloseByOutsideClick(false);
			box->setTitle(rpl::single(QString("Error")));
			box->addRow(object_ptr<Ui::FlatLabel>(box, text, st::boxLabel));
			box->addButton(rpl::single(QString("Quit")), [] {
				QApplication::quit();
			});
		}));
	}));
}

void Application::showIntro() {
	_intro = std::make_unique<Intro>(
		_window->body(),
		Intro::Mode::Standalone);

	_layers->raise();
	_layers->hideAll();

	_window->body()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_intro->setGeometry({ QPoint(), size });
	}, _intro->lifetime());

	_intro->actionRequests(
	) | rpl::start_with_next([=](Intro::Action action) {
		switch (action) {
		case Intro::Action::CreateWallet: {
			_wallet->createKey(crl::guard(this, [=](Ton::Result<std::vector<QString>>) {
				_wallet->saveKey("testingpass", crl::guard(this, [=](Ton::Result<QByteArray>) {
					showWallet();
				}));
			}));
		} break;
		}
	}, _intro->lifetime());
}

void Application::showWallet() {
	Expects(!_wallet->publicKeys().empty());

	_intro = nullptr;

	const auto text = "YOUR KEY: "
		+ QString::fromLatin1(_wallet->publicKeys().front());
	_layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
		box->setCloseByEscape(false);
		box->setCloseByOutsideClick(false);
		box->setTitle(rpl::single(QString("Wallet")));
		box->addRow(object_ptr<Ui::FlatLabel>(box, text, st::boxLabel));
		box->addButton(rpl::single(QString("Delete")), [=] {
			_wallet->deleteKey(_wallet->publicKeys().front(), [=](auto) {
				showIntro();
			});
		});
		box->addButton(rpl::single(QString("Quit")), [] {
			QApplication::quit();
		});
	}));
}

void Application::initWindow() {
	_window->setTitle(::tr::lng_window_title(::tr::now));
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
