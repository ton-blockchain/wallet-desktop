// This file is part of TON Desktop Wallet,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet/blob/master/LEGAL
//
#include "core/launcher.h"

#include "ui/main_queue_processor.h"
#include "core/sandbox.h"
#include "base/platform/base_platform_info.h"
#include "base/concurrent_timer.h"

#include <QtWidgets/QApplication>
#include <QtCore/QJsonObject>
#include <QtCore/QStandardPaths>

namespace Core {
namespace {

class FilteredCommandLineArguments {
public:
	FilteredCommandLineArguments(int argc, char **argv);

	int &count();
	char **values();

private:
	static constexpr auto kForwardArgumentCount = 1;

	int _count = 0;
	char *_arguments[kForwardArgumentCount + 1] = { nullptr };

};

FilteredCommandLineArguments::FilteredCommandLineArguments(
	int argc,
	char **argv)
: _count(std::clamp(argc, 0, kForwardArgumentCount)) {
	// For now just pass only the first argument, the executable path.
	for (auto i = 0; i != _count; ++i) {
		_arguments[i] = argv[i];
	}
}

int &FilteredCommandLineArguments::count() {
	return _count;
}

char **FilteredCommandLineArguments::values() {
	return _arguments;
}

} // namespace

std::unique_ptr<Launcher> Launcher::Create(int argc, char *argv[]) {
	return std::make_unique<Launcher>(argc, argv);
}

Launcher::Launcher(int argc, char *argv[])
: _argc(argc)
, _argv(argv) {
}

void Launcher::init() {
	_arguments = readArguments(_argc, _argv);

	prepareSettings();

	QApplication::setApplicationName("TonDesktopWallet");

#ifdef Q_OS_MAC
	// macOS Retina display support is working fine, others are not.
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling, false);
#else // Q_OS_MAC
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling, true);
#endif // Q_OS_MAC
}

int Launcher::exec() {
	init();

	auto options = QJsonObject();
	const auto tempFontConfigPath = QStandardPaths::writableLocation(
		QStandardPaths::TempLocation
	) + "/fc-custom-1.conf";
	options.insert("custom_font_config_src", QString(":/fc/fc-custom.conf"));
	options.insert("custom_font_config_dst", tempFontConfigPath);
	Platform::Start(options);

	auto result = executeApplication();

	Platform::Finish();

	return result;
}

QStringList Launcher::readArguments(int argc, char *argv[]) const {
	Expects(argc >= 0);

	auto result = QStringList();
	result.reserve(argc);
	for (auto i = 0; i != argc; ++i) {
		result.push_back(base::FromUtf8Safe(argv[i]));
	}
	return result;
}

QString Launcher::argumentsString() const {
	return _arguments.join(' ');
}

void Launcher::prepareSettings() {
	processArguments();
}

void Launcher::processArguments() {
}

int Launcher::executeApplication() {
	FilteredCommandLineArguments arguments(_argc, _argv);
	Sandbox sandbox(this, arguments.count(), arguments.values());
	Ui::MainQueueProcessor processor;
	base::ConcurrentTimerEnvironment environment;
	return sandbox.exec();
}

} // namespace Core

namespace base {
namespace assertion {

inline void log(const char *message, const char *file, int line) {
	const auto info = QStringLiteral("%1 %2:%3"
	).arg(message
	).arg(file
	).arg(line
	);
	const auto entry = QStringLiteral("Assertion Failed! ") + info;
#ifdef LOG
	LOG((entry));
#endif // LOG
}

} // namespace assertion
} // namespace base
