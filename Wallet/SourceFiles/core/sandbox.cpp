// This file is part of TON Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#include "core/sandbox.h"

#include "core/launcher.h"
#include "wallet/application.h"
#include "wallet/wallet_log.h"
#include "ui/widgets/tooltip.h"
#include "ui/emoji_config.h"
#include "ui/effects/animations.h"
#include "ton/ton_wallet.h"
#include "base/crash_report_writer.h"
#include "base/integration.h"
#include "base/concurrent_timer.h"
#include "base/single_instance.h"
#include "base/unixtime.h"
#include "base/timer.h"

#ifdef WALLET_AUTOUPDATING_BUILD
#include "updater/updater_instance.h"
#endif // WALLET_AUTOUPDATING_BUILD

#include <QtWidgets/QDesktopWidget>
#include <QtGui/QScreen>
#include <QtGui/QGuiApplication>
#include <QtGui/QDesktopServices>
#include <QtGui/QtEvents>

namespace Core {

Sandbox::Sandbox(
	not_null<Launcher*> launcher,
	int &argc,
	char **argv)
: QApplication(argc, argv)
, _launcher(launcher)
, _mainThreadId(QThread::currentThreadId())
, _animationsManager(std::make_unique<Ui::Animations::Manager>()) {
	Ui::Integration::Set(&uiIntegration);
	InvokeQueued(this, [=] { checkSingleInstance(); });
}

Sandbox::~Sandbox() {
	Ui::Emoji::Clear();
	style::stopManager();
}

void Sandbox::checkSingleInstance() {
	_singleInstance = std::make_unique<base::SingleInstance>();

	_singleInstance->commands(
	) | rpl::start_with_next([=](const base::SingleInstance::Message &cmd) {
		_launchCommand = cmd.data;
		_singleInstance->reply(cmd.id, handleLaunchCommand());
	}, _lifetime);

	WALLET_LOG(("Checking single instance."));
	_launchCommand = computeLaunchCommand();
	_singleInstance->start(
		QApplication::applicationName(),
		_launcher->workingPath(),
		[=] { crl::on_main(this, [=] { run(); }); },
		[=] { _singleInstance->send(_launchCommand, [=] { quit(); }); },
		[=] { crl::on_main(this, [=] { run(); }); }); // Try running anyway.
}

QByteArray Sandbox::computeLaunchCommand() const {
	if (const auto url = _launcher->openedUrl(); !url.isEmpty()) {
		return "OPEN:" + url.toUtf8();
	}
	return "SHOW";
}

QWidget *Sandbox::handleLaunchCommand() {
	return _application
		? _application->handleCommandGetActivated(base::take(_launchCommand))
		: nullptr;
}

void Sandbox::run() {
	WALLET_LOG(("Single instance checked."));
	Ton::Wallet::EnableLogging(
		_launcher->verbose(),
		_launcher->workingPath());

	style::internal::StartFonts();
	setupScreenScale();
	installNativeEventFilter(this);

#ifdef WALLET_AUTOUPDATING_BUILD
	_launcher->startUpdateChecker();
	if (_launcher->restartingForUpdater()) {
		return;
	}
#endif // WALLET_AUTOUPDATING_BUILD

	WALLET_LOG(("Starting crash reporter."));
	_crashReportWriter = std::make_unique<base::CrashReportWriter>(
		_launcher->workingPath() + "crashes/");
	//if (_crashReportWriter->lastLaunchFailed()) {
	//	// show and check for updates..
	//}
	_crashReportWriter->start();

	WALLET_LOG(("Starting style manager."));
	style::startManager(_scale);
	Ui::Emoji::Init();

	connect(
		this,
		&Sandbox::applicationStateChanged,
		this,
		&Sandbox::stateChanged);

	launchApplication();
}

void Sandbox::launchApplication() {
	WALLET_LOG(("Creating the application."));
	_application = std::make_unique<Wallet::Application>(
		_launcher->workingPath());
	connect(this, &Sandbox::aboutToQuit, [=] {
		customEnterFromEventLoop([&] {
			WALLET_LOG(("Qutting the application."));
			_singleInstance = nullptr;
			_application = nullptr;
			_crashReportWriter = nullptr;
		});
	});

	WALLET_LOG(("Running the application."));
	_application->run();
	handleLaunchCommand();

	_launcher->registerUrlScheme();
}

auto Sandbox::createNestedEventLoopState(not_null<QObject*> guard)
-> std::shared_ptr<NestedEventLoopState> {
	auto state = std::make_shared<NestedEventLoopState>();
	const auto waslevel = _loopNestingLevel;
	state->checkEntered = [=] {
		if (state->finished) {
			return;
		}
		if (_loopNestingLevel > waslevel) {
			state->checkEntered = nullptr;
			processPostponedCalls(waslevel);
		} else {
			InvokeQueued(guard, state->checkEntered);
		}
	};
	InvokeQueued(guard, state->checkEntered);
	return state;
}

bool Sandbox::event(QEvent *event) {
	if (event->type() == QEvent::Close) {
		quit();
	} else if (event->type() == QEvent::FileOpen) {
		const auto e = static_cast<QFileOpenEvent*>(event);
		const auto url = e->url().toEncoded().trimmed();
		const auto string = QString::fromUtf8(url);
		if (string.startsWith("ton://", Qt::CaseInsensitive)) {
			_launchCommand = "OPEN:" + url.mid(0, 8192);
			handleLaunchCommand();
		}
	}
	return QApplication::event(event);
}

void Sandbox::setupScreenScale() {
	const auto dpi = Sandbox::primaryScreen()->logicalDotsPerInch();
	if (dpi <= 108) {
		setScale(100); // 100%:  96 DPI (0-108)
	} else if (dpi <= 132) {
		setScale(125); // 125%: 120 DPI (108-132)
	} else if (dpi <= 168) {
		setScale(150); // 150%: 144 DPI (132-168)
	} else if (dpi <= 216) {
		setScale(200); // 200%: 192 DPI (168-216)
	} else if (dpi <= 264) {
		setScale(250); // 250%: 240 DPI (216-264)
	} else {
		setScale(300); // 300%: 288 DPI (264-inf)
	}
	const auto ratio = devicePixelRatio();
	if (ratio > 1.) {
		style::SetDevicePixelRatio(int(ratio));
		setScale(style::kScaleDefault);
	}
}

void Sandbox::setScale(int scale) {
	WALLET_LOG(("Scale: %1").arg(scale));
	_scale = scale;
	style::SetScale(scale);
}

void Sandbox::stateChanged(Qt::ApplicationState state) {
	if (state == Qt::ApplicationActive) {
		handleAppActivated();
	} else {
		handleAppDeactivated();
	}
}

void Sandbox::handleAppActivated() {
	base::CheckLocalTime();
}

void Sandbox::handleAppDeactivated() {
	Ui::Tooltip::Hide();
}

// macOS Qt bug workaround, sometimes no leaveEvent() gets to the nested widgets.
void Sandbox::registerLeaveSubscription(not_null<QWidget*> widget) {
#ifdef Q_OS_MAC
//	if (const auto topLevel = widget->window()) {
//		if (topLevel == _window->widget()) {
//			auto weak = Ui::MakeWeak(widget);
//			auto subscription = _window->widget()->leaveEvents(
//			) | rpl::start_with_next([weak] {
//				if (const auto window = weak.data()) {
//					QEvent ev(QEvent::Leave);
//					QGuiSandbox::sendEvent(window, &ev);
//				}
//			});
//			_leaveSubscriptions.emplace_back(weak, std::move(subscription));
//		}
//	}
#endif // Q_OS_MAC
}

void Sandbox::unregisterLeaveSubscription(not_null<QWidget*> widget) {
#ifdef Q_OS_MAC
//	_leaveSubscriptions = std::move(
//		_leaveSubscriptions
//	) | ranges::action::remove_if([&](const LeaveSubscription &subscription) {
//		auto pointer = subscription.pointer.data();
//		return !pointer || (pointer == widget);
//	});
#endif // Q_OS_MAC
}

void Sandbox::postponeCall(FnMut<void()> &&callable) {
	Expects(callable != nullptr);
	Expects(_eventNestingLevel >= _loopNestingLevel);

	// _loopNestingLevel == _eventNestingLevel means that we had a
	// native event in a nesting loop that didn't get a notify() call
	// after. That means we already have exited the nesting loop and
	// there must not be any postponed calls with that nesting level.
	if (_loopNestingLevel == _eventNestingLevel) {
		Assert(_postponedCalls.empty()
			|| _postponedCalls.back().loopNestingLevel < _loopNestingLevel);
		Assert(!_previousLoopNestingLevels.empty());

		_loopNestingLevel = _previousLoopNestingLevels.back();
		_previousLoopNestingLevels.pop_back();
	}

	_postponedCalls.push_back({
		_loopNestingLevel,
		std::move(callable)
	});
}

void Sandbox::incrementEventNestingLevel() {
	++_eventNestingLevel;
}

void Sandbox::decrementEventNestingLevel() {
	if (_eventNestingLevel == _loopNestingLevel) {
		_loopNestingLevel = _previousLoopNestingLevels.back();
		_previousLoopNestingLevels.pop_back();
	}
	const auto processTillLevel = _eventNestingLevel - 1;
	processPostponedCalls(processTillLevel);
	_eventNestingLevel = processTillLevel;
}

void Sandbox::registerEnterFromEventLoop() {
	if (_eventNestingLevel > _loopNestingLevel) {
		_previousLoopNestingLevels.push_back(_loopNestingLevel);
		_loopNestingLevel = _eventNestingLevel;
	}
}

void Sandbox::reportAssertionViolation(const QString &info) {
	if (_crashReportWriter) {
		_crashReportWriter->addAnnotation("Assertion", info.toStdString());
	}
}

bool Sandbox::notifyOrInvoke(QObject *receiver, QEvent *e) {
	if (e->type() == base::InvokeQueuedEvent::kType) {
		static_cast<base::InvokeQueuedEvent*>(e)->invoke();
		return true;
	}
	return QApplication::notify(receiver, e);
}

bool Sandbox::notify(QObject *receiver, QEvent *e) {
	if (QThread::currentThreadId() != _mainThreadId) {
		return notifyOrInvoke(receiver, e);
	}
	const auto wrap = createEventNestingLevelWrap();
	if (e->type() == QEvent::UpdateRequest) {
		const auto weak = QPointer<QObject>(receiver);
		_widgetUpdateRequests.fire({});
		if (!weak) {
			return true;
		}
	}
	return notifyOrInvoke(receiver, e);
}

void Sandbox::processPostponedCalls(int level) {
	while (!_postponedCalls.empty()) {
		auto &last = _postponedCalls.back();
		if (last.loopNestingLevel != level) {
			break;
		}
		auto taken = std::move(last);
		_postponedCalls.pop_back();
		taken.callable();
	}
}

bool Sandbox::nativeEventFilter(
		const QByteArray &eventType,
		void *message,
		long *result) {
	registerEnterFromEventLoop();
	return false;
}

rpl::producer<> Sandbox::widgetUpdateRequests() const {
	return _widgetUpdateRequests.events();
}

} // namespace Core

namespace crl {

rpl::producer<> on_main_update_requests() {
	return Core::Sandbox::Instance().widgetUpdateRequests();
}

} // namespace crl
