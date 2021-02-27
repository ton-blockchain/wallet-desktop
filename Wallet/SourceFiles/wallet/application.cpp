// This file is part of TON Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#include "wallet/application.h"

#include "wallet/config_upgrade_checker.h"
#include "wallet/wallet_window.h"
#include "wallet/wallet_update_info.h"
#include "wallet/wallet_log.h"
#include "wallet/ton_default_settings.h"
#include "wallet/update_info_provider.h"
#include "ton/ton_settings.h"
#include "ton/ton_state.h"
#include "ton/ton_wallet.h"
#include "ton/ton_account_viewer.h"
#include "ui/widgets/window.h"
#include "ui/layers/generic_box.h"
#include "ui/layers/layer_manager.h"
#include "ui/text/text_utilities.h"
#include "ui/rp_widget.h"
#include "core/sandbox.h"
#include "core/launcher.h"
#include "base/platform/base_platform_info.h"
#include "base/event_filter.h"
#include "base/call_delayed.h"
#include "styles/style_wrapper.h"
#include "styles/style_widgets.h"
#include "styles/style_layers.h"
#include "styles/palette.h"

#include <QtGui/QIcon>
#include <QtGui/QtEvents>

namespace Wallet {

Application::Application(const QString &path)
: _path(path)
, _wallet(std::make_unique<Ton::Wallet>(_path))
, _launchCommand("SHOW") {
	QApplication::setWindowIcon(QIcon(QPixmap(":/gui/art/logo.png", "PNG")));
}

Application::~Application() {
}

void Application::run() {
	installGlobalShortcutFilter();
	openWallet();
}

void Application::installGlobalShortcutFilter() {
	const auto handleGlobalShortcut = [=](not_null<QEvent*> e) {
		if (e->type() == QEvent::KeyPress) {
			const auto ke = static_cast<QKeyEvent*>(e.get());
			if (ke->modifiers() == Qt::ControlModifier) {
				if (ke->key() == Qt::Key_W || ke->key() == Qt::Key_Q) {
					Core::Sandbox::Instance().quit();
				}
			}
		}
		return base::EventFilterResult::Continue;
	};
	base::install_event_filter(
		&Core::Sandbox::Instance(),
		handleGlobalShortcut);
}

QWidget *Application::handleCommandGetActivated(const QByteArray &command) {
	_launchCommand = command;
	handleLaunchCommand();
	return _window ? _window->widget().get() : nullptr;
}

void Application::handleLaunchCommand() {
	if (!_window || _launchCommand.isEmpty()) {
		return;
	}
	_window->showAndActivate();
	if (handleCommand(_launchCommand)) {
		_launchCommand = QByteArray();
	}
}

bool Application::handleCommand(const QByteArray &command) {
	if (_launchCommand == "SHOW") {
		return true;
	} else if (command.startsWith("OPEN:")) {
		return _window->handleLinkOpen(QString::fromUtf8(command.mid(5)));
	}
	return false;
}

UpdateInfo *Application::walletUpdateInfo() {
#ifdef WALLET_AUTOUPDATING_BUILD
	const auto launcher = Core::Sandbox::Instance().launcher();
	_updateInfo = std::make_unique<UpdateInfoProvider>(
		launcher->updateChecker(),
		[=] { return launcher->updateCheckerEnabled(); },
		[=](bool enabled) { launcher->setUpdateCheckerEnabled(enabled); },
		[=] { launcher->restartForUpdater(); });
	return _updateInfo.get();
#else // WALLET_AUTOUPDATING_BUILD
	return nullptr;
#endif // WALLET_AUTOUPDATING_BUILD
}

void Application::openWallet() {
	WALLET_LOG(("Opening the wallet."));

	const auto showError = [=](Ton::Error error) {
		const auto text = (error.type == Ton::Error::Type::IO)
			? ("IO error at path: " + error.details)
			: (error.type == Ton::Error::Type::TonLib)
			? ("Library error, details: " + error.details)
			: ("Global Password didn't work.");
		const auto solution = "\n\nTry deleting all at " + _path;
		criticalError(text + solution);
	};
	auto started = [=](Ton::Result<> result) {
		if (!result) {
			showError(result.error());
			return;
		}
		WALLET_LOG(("Creating the wallet."));
		_window = std::make_unique<Wallet::Window>(
			_wallet.get(),
			walletUpdateInfo());

		const auto upgrades = base::take(_upgradeChecker)->takeUpgrades();
		for (const auto upgrade : upgrades) {
			_window->showConfigUpgrade(upgrade);
		}
		handleLaunchCommand();
	};
	auto opened = [=](Ton::Result<> result) {
		if (!result) {
			showError(result.error());
			return;
		}
		if (_wallet->settings().useNetworkCallbacks) {
			auto copy = _wallet->settings();
			copy.useNetworkCallbacks = false;
			_wallet->updateSettings(copy, nullptr);
		}
		_wallet->start(started);
	};

	_upgradeChecker = std::make_unique<ConfigUpgradeChecker>(_wallet.get());

	_wallet->open(QByteArray(), GetDefaultSettings(), std::move(opened));
}

void Application::criticalError(const QString &text) {
	const auto window = _lifetime.make_state<Ui::Window>();
	const auto layers = _lifetime.make_state<Ui::LayerManager>(window->body());
	layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
		box->setCloseByEscape(false);
		box->setCloseByOutsideClick(false);
		box->setTitle(rpl::single(QString("Error")));
		box->addRow(object_ptr<Ui::FlatLabel>(box, text, st::boxLabel));
		box->addButton(rpl::single(QString("Quit")), [] {
			QCoreApplication::quit();
		});
	}));

	window->resize(2 * st::boxWidth, 3 * st::boxWidth / 2);
	window->show();
}

} // namespace Wallet
