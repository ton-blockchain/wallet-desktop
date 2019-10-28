// This file is part of TON Desktop Wallet,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet/blob/master/LEGAL
//
#include "core/launcher.h"

#include "ui/main_queue_processor.h"
#include "ui/ui_utility.h"
#include "core/sandbox.h"
#include "base/platform/base_platform_info.h"
#include "base/concurrent_timer.h"

#include <QtWidgets/QApplication>
#include <QtCore/QJsonObject>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>

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
, _argv(argv)
, _baseIntegration(argc, argv) {
	base::Integration::Set(&_baseIntegration);
}

void Launcher::init() {
	_arguments = readArguments(_argc, _argv);
	QApplication::setApplicationName("Gram Wallet");
	prepareSettings();
	initWorkingPath();
}

void Launcher::initWorkingPath() {
	_workingPath = computeWorkingPathBase() + "data/";
}

QString Launcher::computeWorkingPathBase() {
	if (const auto path = checkPortablePath(); !path.isEmpty()) {
		return path;
	}
#if defined Q_OS_MAC || defined Q_OS_LINUX
#if defined _DEBUG && !defined OS_MAC_STORE
	return _baseIntegration.executableDir();
#else // _DEBUG
	return _appDataPath;
#endif // !_DEBUG
#elif defined OS_WIN_STORE // Q_OS_MAC || Q_OS_LINUX
#ifdef _DEBUG
	return _baseIntegration.executableDir();
#else // _DEBUG
	return _appDataPath;
#endif // !_DEBUG
#elif defined Q_OS_WIN
	if (canWorkInExecutablePath()) {
		return _baseIntegration.executableDir();
	} else {
		return _appDataPath;
	}
#endif // Q_OS_MAC || Q_OS_LINUX || Q_OS_WINRT || OS_WIN_STORE
}

bool Launcher::canWorkInExecutablePath() const {
	const auto dataPath = _baseIntegration.executableDir() + "data";
	if (!QDir(dataPath).exists() && !QDir().mkpath(dataPath)) {
		return false;
	} else if (QFileInfo(dataPath + "/salt").exists()) {
		return true;
	}
	auto index = 0;
	while (true) {
		const auto temp = dataPath + "/temp" + QString::number(++index);
		auto file = QFile(temp);
		if (file.open(QIODevice::WriteOnly)) {
			file.close();
			file.remove();
			return true;
		} else if (!file.exists()) {
			return false;
		} else if (index == std::numeric_limits<int>::max()) {
			return false;
		}
	}
}

QString Launcher::checkPortablePath() {
	const auto portable = _baseIntegration.executableDir()
		+ "WalletForcePortable";
	return QDir(portable).exists() ? (portable + '/') : QString();
}

int Launcher::exec() {
	init();

	if (_action == Action::Cleanup) {
		cleanupInstallation();
		return 0;
	}
	setupScale();

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

void Launcher::cleanupInstallation() {
}

void Launcher::setupScale() {
#ifdef Q_OS_MAC
	// macOS Retina display support is working fine, others are not.
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling, false);
#else // Q_OS_MAC
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling, true);
#endif // Q_OS_MAC
	Ui::DisableCustomScaling();
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

QString Launcher::workingPath() const {
	return _workingPath;
}

QString Launcher::openedUrl() const {
	return _openedUrl;
}

void Launcher::initAppDataPath() {
	const auto path = QStandardPaths::writableLocation(
		QStandardPaths::AppDataLocation);
	const auto absolute = QDir(path).absolutePath();
	_appDataPath = absolute.endsWith('/') ? absolute : (absolute + '/');
}

void Launcher::prepareSettings() {
	initAppDataPath();
	processArguments();
}

void Launcher::processArguments() {
	auto nextUrl = false;
	for (const auto &argument : _arguments) {
		if (argument == "cleanup") {
			_action = Action::Cleanup;
		} else if (nextUrl) {
			_openedUrl = argument;
		} else if (argument == "--") {
			nextUrl = true;
		}
	}
}

int Launcher::executeApplication() {
	FilteredCommandLineArguments arguments(_argc, _argv);
	Sandbox sandbox(this, arguments.count(), arguments.values());
	Ui::MainQueueProcessor processor;
	base::ConcurrentTimerEnvironment environment;
	return sandbox.exec();
}

} // namespace Core
