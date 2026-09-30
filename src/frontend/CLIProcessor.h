/*
	File                 : CLIProcessor.h
	Project              : LabPlot
	Description          : Process command-line operations
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLIPROCESSOR_H
#define CLIPROCESSOR_H

#include <QCommandLineParser>
#include <QString>

class Project;
class Worksheet;

// Exit codes for scripting/CI-CD
namespace CLIExitCode {
constexpr int Success = 0;
constexpr int GeneralError = 1;
constexpr int InvalidArgument = 2;
constexpr int FileNotFound = 3;
constexpr int ExportFailed = 4;
constexpr int ProjectLoadFailed = 5;
constexpr int TemplateFailed = 6;
constexpr int ScriptFailed = 7;
constexpr int WorksheetNotFound = 8;
constexpr int DependencyMissing = 9;
}

// Option names
static const QString CLI_INPUT = QStringLiteral("input");
static const QString CLI_OUTPUT = QStringLiteral("output");
static const QString CLI_FORMAT = QStringLiteral("format");
static const QString CLI_DPI = QStringLiteral("dpi");
static const QString CLI_NO_GUI = QStringLiteral("no-gui");
static const QString CLI_EXPORT_WORKSHEET = QStringLiteral("export-worksheet");
static const QString CLI_EXPORT_ALL = QStringLiteral("export-all");
static const QString CLI_TEMPLATE = QStringLiteral("template");
static const QString CLI_OUTPUT_DIR = QStringLiteral("output-dir");
static const QString CLI_TITLE = QStringLiteral("title");
static const QString CLI_XLABEL = QStringLiteral("xlabel");
static const QString CLI_YLABEL = QStringLiteral("ylabel");
static const QString CLI_NO_SPLASH = QStringLiteral("no-splash");
static const QString CLI_PRESENTER = QStringLiteral("presenter");

void configureCLI(QCommandLineParser&);
bool isHeadlessMode(const QCommandLineParser&);
QString getInputFile(const QCommandLineParser&);
int processCLI(const QCommandLineParser&);

#endif // CLIPROCESSOR_H
