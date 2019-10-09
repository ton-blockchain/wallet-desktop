// This file is part of TON Desktop Wallet,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet/blob/master/LEGAL
//
#include "wallet/application.h"

#include "wallet/phrases.h"
#include "wallet/ton_default_config.h"
#include "ton/ton_config.h"
#include "ton/ton_state.h"
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
#include "ui/widgets/input_fields.h"
#include "wallet/wallet_intro.h"
#include "wallet/wallet_phrases.h"
#include "wallet/wallet_info.h"
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
	auto opened = [=](Ton::Result<> result) {
		if (result) {
			if (_wallet->publicKeys().empty()) {
				showIntro();
			} else {
				showInfo();
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
	};
	_wallet->open(QByteArray(), GetDefaultConfig(), std::move(opened));
}

void Application::showIntro() {
	_info = nullptr;
	_intro = std::make_unique<Intro>(_window->body());

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
			_wallet->createKey([=](
					Ton::Result<std::vector<QString>> result) {
				if (result) {
					saveKey(*result);
				}
			});
		} break;
		}
	}, _intro->lifetime());
}

void Application::saveKey(const std::vector<QString> &words) {
	_layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(rpl::single(QString("Words")));
		for (const auto &word : words) {
			box->addRow(object_ptr<Ui::FlatLabel>(
				box,
				rpl::single(word),
				st::boxLabel));
		}
		const auto passwordWrap = box->addRow(object_ptr<Ui::RpWidget>(box));
		const auto password = Ui::CreateChild<Ui::PasswordInput>(
			passwordWrap,
			st::defaultInputField,
			rpl::single(QString("Password")));
		passwordWrap->widthValue(
		) | rpl::start_with_next([=](int width) {
			password->resize(width, password->height());
		}, password->lifetime());
		password->heightValue(
		) | rpl::start_with_next([=](int height) {
			passwordWrap->resize(passwordWrap->width(), height);
		}, password->lifetime());
		password->move(0, 0);
		const auto saving = box->lifetime().make_state<bool>(false);
		box->setCloseByEscape(false);
		box->setCloseByOutsideClick(false);
		box->addButton(rpl::single(QString("Save")), [=] {
			if (*saving) {
				return;
			}
			auto pwd = password->getLastText().trimmed();
			if (pwd.isEmpty()) {
				password->showError();
				return;
			}
			*saving = true;
			_wallet->saveKey(pwd.toUtf8(), [=](Ton::Result<QByteArray> r) {
				*saving = false;
				if (r) {
					box->closeBox();
					showInfo();
				} else {
					password->showError();
					return;
				}
			});
		});
	}));
}

void Application::showInfo() {
	Expects(!_wallet->publicKeys().empty());

	const auto key = _wallet->publicKeys().front();
	const auto address = Ton::Wallet::GetAddress(key);

	_intro = nullptr;

	auto data = Info::Data();
	data.address = address;
	data.balance = _balance.events();
	data.lastTransactions = _lastTransactions.events();
	_info = std::make_unique<Info>(_window->body(), data);

	_layers->raise();
	_layers->hideAll();

	_window->body()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_info->setGeometry({ QPoint(), size });
	}, _info->lifetime());

	const auto refresh = [=] {
		_wallet->requestState(address, [=](
				Ton::Result<Ton::AccountState> result) {
			if (result) {
				_balance.fire_copy(std::max(result->balance, 0LL));
				_wallet->requestTransactions(
					address,
					result->lastTransactionId,
					[=](Ton::Result<Ton::TransactionsSlice> result) {
						if (result) {
							_lastTransactions.fire(std::move(*result));
						}
					});
			}
		});
	};

	_info->actionRequests(
	) | rpl::start_with_next([=](Info::Action action) {
		switch (action) {
		case Info::Action::Refresh: refresh(); break;
		case Info::Action::Send: sendGrams(); break;
		case Info::Action::ChangePassword: changePassword(); break;
		case Info::Action::LogOut: logout(); break;
		}
	}, _info->lifetime());

	refresh();
}

void Application::sendGrams() {
	_layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(rpl::single(QString("Send grams")));
		const auto recipient = box->addRow(object_ptr<Ui::InputField>(
			box,
			st::defaultInputField,
			rpl::single(QString("Recipient"))));
		const auto amount = box->addRow(object_ptr<Ui::InputField>(
			box,
			st::defaultInputField,
			rpl::single(QString("Amount"))));
		const auto comment = box->addRow(object_ptr<Ui::InputField>(
			box,
			st::defaultInputField,
			rpl::single(QString("Comment"))));
		const auto passwordWrap = box->addRow(object_ptr<Ui::RpWidget>(box));
		const auto password = Ui::CreateChild<Ui::PasswordInput>(
			passwordWrap,
			st::defaultInputField,
			rpl::single(QString("Password")));
		passwordWrap->widthValue(
		) | rpl::start_with_next([=](int width) {
			password->resize(width, password->height());
		}, password->lifetime());
		password->heightValue(
		) | rpl::start_with_next([=](int height) {
			passwordWrap->resize(passwordWrap->width(), height);
		}, password->lifetime());
		password->move(0, 0);
		const auto sending = box->lifetime().make_state<bool>(false);
		box->addButton(rpl::single(QString("Send")), [=] {
			if (*sending) {
				return;
			}
			auto data = Ton::TransactionToSend();
			data.recipient = recipient->getLastText().trimmed();
			if (data.recipient.isEmpty()) {
				recipient->showError();
				return;
			}
			data.amount = int64(amount->getLastText().trimmed().toDouble()
				* 1'000'000'000);
			if (data.amount <= 0) {
				amount->showError();
				return;
			}
			data.comment = comment->getLastText().trimmed();
			data.allowSendToUninited = true;
			auto pwd = password->getLastText().trimmed();
			if (pwd.isEmpty()) {
				password->showError();
				return;
			}
			*sending = true;
			auto done = [=](Ton::Result<Ton::SentTransaction> result) {
				*sending = false;
				if (result) {
					box->closeBox();
				} else {
					recipient->showError();
					return;
				}
			};
			_wallet->sendGrams(
				_wallet->publicKeys().front(),
				pwd.toUtf8(),
				data,
				done);
		});
		box->addButton(rpl::single(QString("Cancel")), [=] {
			box->closeBox();
		});
	}));
}

void Application::changePassword() {
	_layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(rpl::single(QString("Change password")));
		const auto oldWrap = box->addRow(object_ptr<Ui::RpWidget>(box));
		const auto old = Ui::CreateChild<Ui::PasswordInput>(
			oldWrap,
			st::defaultInputField,
			rpl::single(QString("Old password")));
		oldWrap->widthValue(
		) | rpl::start_with_next([=](int width) {
			old->resize(width, old->height());
		}, old->lifetime());
		old->heightValue(
		) | rpl::start_with_next([=](int height) {
			oldWrap->resize(oldWrap->width(), height);
		}, old->lifetime());
		old->move(0, 0);
		const auto passwordWrap = box->addRow(object_ptr<Ui::RpWidget>(box));
		const auto password = Ui::CreateChild<Ui::PasswordInput>(
			passwordWrap,
			st::defaultInputField,
			rpl::single(QString("New password")));
		passwordWrap->widthValue(
		) | rpl::start_with_next([=](int width) {
			password->resize(width, password->height());
		}, password->lifetime());
		password->heightValue(
		) | rpl::start_with_next([=](int height) {
			passwordWrap->resize(passwordWrap->width(), height);
		}, password->lifetime());
		password->move(0, 0);
		const auto saving = box->lifetime().make_state<bool>(false);
		box->addButton(rpl::single(QString("Change")), [=] {
			if (*saving) {
				return;
			}
			auto from = old->getLastText().trimmed();
			if (from.isEmpty()) {
				old->showError();
				return;
			}
			auto pwd = password->getLastText().trimmed();
			if (pwd.isEmpty()) {
				password->showError();
				return;
			}
			*saving = true;
			auto done = [=](Ton::Result<> result) {
				*saving = false;
				if (result) {
					box->closeBox();
				} else {
					old->showError();
					return;
				}
			};
			_wallet->changePassword(from.toUtf8(), pwd.toUtf8(), done);
		});
		box->addButton(rpl::single(QString("Cancel")), [=] {
			box->closeBox();
		});
	}));
}

void Application::logout() {
	_wallet->deleteAllKeys([=](Ton::Result<> result) {
		if (result) {
			showIntro();
		}
	});
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
