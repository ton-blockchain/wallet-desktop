// This file is part of Gram Wallet Desktop,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet-desktop/blob/master/LEGAL
//
#pragma once

#include "base/timer.h"
#include "core/ui_integration.h"

#include <QtWidgets/QApplication>
#include <QtCore/QAbstractNativeEventFilter>

namespace base {
class SingleInstance;
class CrashReportWriter;
} // namespace base

namespace Ui {
namespace Animations {
class Manager;
} // namespace Animations
} // namespace Ui

namespace Wallet {
class Application;
} // namespace Wallet

namespace Core {

class Launcher;

class Sandbox final
	: public QApplication
	, private QAbstractNativeEventFilter {
	auto createEventNestingLevelWrap() {
		incrementEventNestingLevel();
		return gsl::finally([=] { decrementEventNestingLevel(); });
	}

public:
	Sandbox(not_null<Launcher*> launcher, int &argc, char **argv);
	Sandbox(const Sandbox &other) = delete;
	Sandbox &operator=(const Sandbox &other) = delete;
	~Sandbox();

	not_null<Launcher*> launcher() const {
		return _launcher;
	}

	void postponeCall(FnMut<void()> &&callable);
	bool notify(QObject *receiver, QEvent *e) override;

	void reportAssertionViolation(const QString &info);

	template <typename Callable>
	auto customEnterFromEventLoop(Callable &&callable) {
		registerEnterFromEventLoop();
		const auto wrap = createEventNestingLevelWrap();
		return callable();
	}
	template <typename Callable>
	auto runNestedEventLoop(not_null<QObject*> guard, Callable &&callable) {
		const auto state = createNestedEventLoopState(guard);
		const auto finish = gsl::finally([&] { state->finished = true; });
		return callable();
	}

	rpl::producer<> widgetUpdateRequests() const;

	static Sandbox &Instance() {
		Expects(QCoreApplication::instance() != nullptr);

		return *static_cast<Sandbox*>(QCoreApplication::instance());
	}

	void run();

	Ui::Animations::Manager &animationManager() const {
		return *_animationsManager;
	}

	void registerLeaveSubscription(not_null<QWidget*> widget);
	void unregisterLeaveSubscription(not_null<QWidget*> widget);

	void handleAppActivated();
	void handleAppDeactivated();

protected:
	bool event(QEvent *e) override;

private:
	static constexpr auto kDefaultSaveDelay = crl::time(1000);

	struct PostponedCall {
		int loopNestingLevel = 0;
		FnMut<void()> callable;
	};
	struct NestedEventLoopState {
		bool finished = false;
		Fn<void()> checkEntered;

		~NestedEventLoopState() {
			Expects(finished);
		}
	};

	void checkSingleInstance();
	bool notifyOrInvoke(QObject *receiver, QEvent *e);
	void registerEnterFromEventLoop();
	void incrementEventNestingLevel();
	void decrementEventNestingLevel();
	bool nativeEventFilter(
		const QByteArray &eventType,
		void *message,
		long *result) override;
	void processPostponedCalls(int level);
	void launchApplication();
	void setupScreenScale();
	std::shared_ptr<NestedEventLoopState> createNestedEventLoopState(
		not_null<QObject*> guard);

	[[nodiscard]] QByteArray computeLaunchCommand() const;
	QWidget *handleLaunchCommand();

	void setScale(int scale);
	void stateChanged(Qt::ApplicationState state);

	const not_null<Launcher*> _launcher;
	UiIntegration uiIntegration;

	std::unique_ptr<base::CrashReportWriter> _crashReportWriter;
	std::unique_ptr<base::SingleInstance> _singleInstance;

	QByteArray _launchCommand;

	const Qt::HANDLE _mainThreadId = nullptr;
	int _eventNestingLevel = 0;
	int _loopNestingLevel = 0;
	std::vector<int> _previousLoopNestingLevels;
	std::vector<PostponedCall> _postponedCalls;

	rpl::event_stream<> _widgetUpdateRequests;

	const std::unique_ptr<Ui::Animations::Manager> _animationsManager;
	int _scale = 0;

	std::unique_ptr<Wallet::Application> _application;

	struct LeaveSubscription {
		LeaveSubscription(
			QPointer<QWidget> pointer,
			rpl::lifetime &&subscription)
		: pointer(pointer), subscription(std::move(subscription)) {
		}

		QPointer<QWidget> pointer;
		rpl::lifetime subscription;
	};
	std::vector<LeaveSubscription> _leaveSubscriptions;

	rpl::lifetime _lifetime;

};

} // namespace Core
