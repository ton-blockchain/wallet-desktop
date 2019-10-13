// This file is part of TON Desktop Wallet,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet/blob/master/LEGAL
//
#include "wallet/application.h"

#include "wallet/wallet_window.h"
#include "wallet/ton_default_config.h"
#include "ton/ton_config.h"
#include "ton/ton_state.h"
#include "ton/ton_wallet.h"
#include "ton/ton_account_viewer.h"
#include "ui/widgets/window.h"
#include "ui/text/text_utilities.h"
#include "ui/rp_widget.h"
#include "core/sandbox.h"
#include "base/platform/base_platform_info.h"
#include "base/call_delayed.h"
#include "styles/style_wrapper.h"
#include "styles/style_widgets.h"
#include "styles/palette.h"

#include <QtGui/QIcon>

namespace Wallet {

Application::Application(const QString &path)
: _path(path)
, _wallet(std::make_unique<Ton::Wallet>(_path)) {
	QApplication::setWindowIcon(QIcon(QPixmap(":/gui/art/logo.png", "PNG")));
}

Application::~Application() {
}

void Application::run() {
	openWallet();
}

void Application::openWallet() {
	auto opened = [=](Ton::Result<> result) {
		if (!result) {
			const auto text = (result.error().type == Ton::Error::Type::IO)
				? "IO error at path: " + result.error().details
				: ("Global Password didn't work.\n\nTry deleting all at "
					+ _path);
			criticalError(text);
		} else {
			_window = std::make_unique<Wallet::Window>(_wallet.get());
			_window->show();
			_window->setFocus();
		}
	};
	_wallet->open(QByteArray(), GetDefaultConfig(), std::move(opened));
}

void Application::criticalError(const QString &text) {
	//_layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
	//	box->setCloseByEscape(false);
	//	box->setCloseByOutsideClick(false);
	//	box->setTitle(rpl::single(QString("Error")));
	//	box->addRow(object_ptr<Ui::FlatLabel>(box, error, st::boxLabel));
	//	box->addButton(rpl::single(QString("Quit")), [] {
	//		QCoreApplication::quit();
	//	});
	//}));
}

} // namespace Wallet
