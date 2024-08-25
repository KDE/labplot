/*
	File                 : TeXRenderer.cc
	Project              : LabPlot
	Description          : TeX renderer class
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2021 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2021 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "TeXRenderer.h"
#include "backend/core/Settings.h"
#include "backend/lib/hostprocess.h"
#include "backend/lib/macros.h"
#include "kdefrontend/GuiTools.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <QColor>
#include <QDir>
#include <QImage>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>

#ifdef HAVE_POPPLER
#include <poppler-qt5.h>
#endif

/*!
	\class TeXRenderer
	\brief Implements rendering of latex code to a PNG image.

	Uses latex engine specified by the user (default xelatex) to render LaTeX text

	\ingroup tools
*/
QByteArray TeXRenderer::renderImageLaTeX(const QString& teXString, Result* res, const TeXRenderer::Formatting& format) {
	const QColor& fontColor = format.fontColor;
	const QColor& backgroundColor = format.backgroundColor;
	const int fontSize = format.fontSize;
	const QString& fontFamily = format.fontFamily;
	const int dpi = format.dpi;

	// determine the temp directory where the produced files are going to be created
	QString tempPath;
#ifdef Q_OS_LINUX
	// on linux try to use shared memory device first if available
	static bool useShm = QDir(QStringLiteral("/dev/shm/")).exists();
	if (useShm)
		tempPath = QStringLiteral("/dev/shm/");
	else
		tempPath = QDir::tempPath();
#else
	tempPath = QDir::tempPath();
#endif

	// make sure we have preview.sty available
	if (!tempPath.contains(QStringLiteral("preview.sty"))) {
		QString file = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("latex/preview.sty"));
		if (file.isEmpty()) {
			QString err = i18n("Couldn't find preview.sty.");
			WARN(err.toStdString());
			res->successful = false;
			res->errorMessage = std::move(err);
			return {};
		} else
			QFile::copy(file, tempPath + QLatin1Char('/') + QStringLiteral("preview.sty"));
	}

	// create a temporary file
	QTemporaryFile file(tempPath + QLatin1Char('/') + QStringLiteral("labplot_XXXXXX.tex"));
	// FOR DEBUG: file.setAutoRemove(false);
	// DEBUG("temp file path = " << file.fileName().toUtf8().constData());
	if (file.open()) {
		QDir::setCurrent(tempPath);
	} else {
		QString err = i18n("Couldn't open the file") + QStringLiteral(" ") + file.fileName();
		WARN(err.toStdString());
		res->successful = false;
		res->errorMessage = std::move(err);
		return {};
	}

	// determine latex engine to be used
	const auto& group = Settings::group(QStringLiteral("Settings_Worksheet"));
	const auto& engine = group.readEntry(QStringLiteral("LaTeXEngine"), QStringLiteral("pdflatex"));

	// create latex code
	QTextStream out(&file);
	const int headerIndex = teXString.indexOf(QStringLiteral("\\begin{document}"));
	QString body;
	if (headerIndex != -1) {
		// user provided a complete latex document -> extract the document header and body
		QString header = teXString.left(headerIndex);
		const int footerIndex = teXString.indexOf(QStringLiteral("\\end{document}"));
		body = teXString.mid(headerIndex + 16, footerIndex - headerIndex - 16);
		out << header;
	} else {
		// user simply provided a document body (assume it's a math. expression) -> add a minimal header
		out << "\\documentclass{minimal}";
		if (teXString.indexOf(QLatin1Char('$')) == -1)
			body = QLatin1Char('$') + teXString + QLatin1Char('$');
		else
			body = teXString;

		// replace line breaks with tex command for a line break '\\'
		body = body.replace(QLatin1String("\n"), QLatin1String("\\\\"));
	}

	if (engine == QStringLiteral("xelatex") || engine == QStringLiteral("lualatex")) {
		out << QStringLiteral("\\usepackage{fontspec}");
		out << QStringLiteral("\\defaultfontfeatures{Ligatures=TeX}");
		if (!fontFamily.isEmpty())
			out << QStringLiteral("\\setmainfont[Mapping=tex-text]{") << fontFamily << QLatin1Char('}');
	}

	out << QStringLiteral("\\usepackage{color}");
	out << QStringLiteral("\\usepackage[active,displaymath,textmath,tightpage]{preview}");
	out << "\\setlength\\PreviewBorder{0pt}";
	// TODO: this fails with pdflatex
	// out << "\\usepackage{mathtools}";
	out << QStringLiteral("\\begin{document}");
	out << QStringLiteral("\\begin{preview}");
	out << QStringLiteral("\\setlength{\\fboxsep}{1.0pt}");
	if (backgroundColor.alpha() != 0)
		out << QStringLiteral("\\colorbox[rgb]{") << backgroundColor.redF() << QLatin1Char(',') << backgroundColor.greenF() << QLatin1Char(',')
			<< backgroundColor.blueF() << QLatin1Char('}');
	out << QLatin1Char('{');
	out << QStringLiteral("\\fontsize{") << QString::number(fontSize) << QStringLiteral("}{") << QString::number(fontSize) << QStringLiteral("}\\selectfont");
	out << QStringLiteral("\\color[rgb]{") << fontColor.redF() << QLatin1Char(',') << fontColor.greenF() << QLatin1Char(',') << fontColor.blueF()
		<< QLatin1Char('}');
	out << body;
	out << QLatin1Char('}');
	out << QStringLiteral("\\end{preview}");
	out << QStringLiteral("\\end{document}");
	out.flush();

	if (engine == QStringLiteral("latex"))
		return imageFromDVI(file, dpi, res);
	else
		return imageFromPDF(file, engine, res);
}

bool TeXRenderer::executeLatexProcess(const QString engine,
									  const QString& baseName,
									  const QTemporaryFile& file,
									  const QString& resultFileExtension,
									  Result* res) {
	// latex: produce the DVI file
	const QString engineFullPath = safeExecutableName(engine);
	if (engineFullPath.isEmpty()) {
		res->successful = false;
		res->errorMessage = i18n("%1 not found").arg(engine);
		WARN(QStringLiteral("%1 not found").arg(engine).toStdString());
		return {};
	}

	WARN(QStringLiteral("Engine fullpath: %1").arg(engineFullPath).toStdString());

	QProcess latexProcess;
	startHostProcess(latexProcess, engineFullPath, QStringList() << QStringLiteral("-interaction=batchmode") << file.fileName());

	WARN(QStringLiteral("Workdir: %1").arg(QDir::currentPath()).toStdString());

	bool finished = latexProcess.waitForFinished();
	if (!finished || latexProcess.exitCode() != 0) {
		QFile logFile(baseName + QStringLiteral(".log"));
		QString errorLogs;
		WARN(QStringLiteral("executeLatexProcess: logfile: %1").arg(QFileInfo(logFile).absoluteFilePath()).toStdString());
		if (logFile.open(QIODevice::ReadOnly)) {
			// really slow, but texrenderer is running asynchronous so it is not a problem
			while (!logFile.atEnd()) {
				const auto line = logFile.readLine();
				if (line.count() > 0 && line.at(0) == '!') {
					errorLogs += QLatin1String(line);
					break; // only first error message is enough
				}
			}
			logFile.close();
		} else
			WARN(QStringLiteral("Unable to open logfile").toStdString());

		WARN(latexProcess.readAllStandardOutput().toStdString());
		WARN(latexProcess.readAllStandardError().toStdString());

		QString err;
		if (errorLogs.isEmpty()) {
			if (!finished) {
				err = i18n("Timeout: Unable to generate latex file");
				WARN(QStringLiteral("Timeout: Unable to generate latex file").toStdString());
			} else {
				err = QStringLiteral("latex ") + i18n("process failed, exit code =") + QStringLiteral(" ") + QString::number(latexProcess.exitCode())
					+ QStringLiteral("\n");
				WARN(QStringLiteral("latex process failed, exit code = %1").arg(latexProcess.exitCode()).toStdString());
			}
		} else {
			err = errorLogs;
			WARN(err.toStdString());
		}

		res->successful = false;
		res->errorMessage = std::move(err);
		QFile::remove(baseName + QStringLiteral(".aux"));
		QFile::remove(logFile.fileName());
		QFile::remove(baseName + QStringLiteral(".%1").arg(resultFileExtension)); // in some cases the file was also created
		return false;
	}
	res->successful = true;
	res->errorMessage = QStringLiteral("");
	return true;
}

// TEX -> PDF -> QImage
QByteArray TeXRenderer::imageFromPDF(const QTemporaryFile& file, const QString& engine, Result* res) {
	// DEBUG(Q_FUNC_INFO << ", tmp file = " << STDSTRING(file.fileName()) << ", engine = " << STDSTRING(engine))
	QFileInfo fi(file.fileName());
	const QString& baseName = fi.completeBaseName();

	if (!executeLatexProcess(engine, baseName, file, QStringLiteral("pdf"), res))
		return {};

	// Can we move this into executeLatexProcess?
	QFile::remove(baseName + QStringLiteral(".aux"));
	QFile::remove(baseName + QStringLiteral(".log"));

	// read PDF file
	QFile pdfFile(baseName + QStringLiteral(".pdf"));
	if (!pdfFile.open(QIODevice::ReadOnly)) {
		QFile::remove(baseName + QStringLiteral(".pdf"));
		return {};
	}

	QByteArray ba = pdfFile.readAll();
	pdfFile.close();
	QFile::remove(baseName + QStringLiteral(".pdf"));
	res->successful = true;
	res->errorMessage = QString();

	return ba;
}

// TEX -> DVI -> PS -> PNG
QByteArray TeXRenderer::imageFromDVI(const QTemporaryFile& file, const int dpi, Result* res) {
	QFileInfo fi(file.fileName());
	const QString& baseName = fi.completeBaseName();

	if (!executeLatexProcess(QLatin1String("latex"), baseName, file, QStringLiteral("dvi"), res))
		return {};

	// dvips: DVI -> PS
	const QString dvipsFullPath = safeExecutableName(QStringLiteral("dvips"));
	if (dvipsFullPath.isEmpty()) {
		res->successful = false;
		res->errorMessage = i18n("dvips not found");
		WARN("dvips not found");
		return {};
	}
	QProcess dvipsProcess;
	startHostProcess(dvipsProcess, dvipsFullPath, QStringList() << QStringLiteral("-E") << baseName);
	if (!dvipsProcess.waitForFinished() || dvipsProcess.exitCode() != 0) {
		QString err = i18n("dvips process failed, exit code =") + QStringLiteral(" ") + QString::number(dvipsProcess.exitCode());
		WARN(err.toStdString());
		res->successful = false;
		res->errorMessage = std::move(err);
		QFile::remove(baseName + QStringLiteral(".aux"));
		QFile::remove(baseName + QStringLiteral(".log"));
		QFile::remove(baseName + QStringLiteral(".dvi"));
		return {};
	}

	// convert: PS -> PNG
	QProcess convertProcess;
#if defined(HAVE_WINDOWS)
	// need to set path to magick coder modules (which are in the labplot directory)
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	env.insert(QStringLiteral("MAGICK_CODER_MODULE_PATH"), QString::fromLocal8Bit(qgetenv("PROGRAMFILES")) + QStringLiteral("\\labplot"));
	convertProcess.setProcessEnvironment(env);
#endif
	const QString convertFullPath = safeExecutableName(QStringLiteral("convert"));
	if (convertFullPath.isEmpty()) {
		WARN("convert not found");
		res->successful = false;
		res->errorMessage = i18n("convert not found");
		return {};
	}

	const QStringList params{QStringLiteral("-density"), QString::number(dpi), baseName + QStringLiteral(".ps"), baseName + QStringLiteral(".pdf")};
	startHostProcess(convertProcess, convertFullPath, params);

	if (!convertProcess.waitForFinished() || convertProcess.exitCode() != 0) {
		QString err = i18n("convert process failed, exit code =") + QStringLiteral(" ") + QString::number(convertProcess.exitCode());
		WARN(err.toStdString());
		res->successful = false;
		res->errorMessage = std::move(err);
		QFile::remove(baseName + QStringLiteral(".aux"));
		QFile::remove(baseName + QStringLiteral(".log"));
		QFile::remove(baseName + QStringLiteral(".dvi"));
		QFile::remove(baseName + QStringLiteral(".ps"));
		return {};
	}

	// final clean up
	QFile::remove(baseName + QStringLiteral(".aux"));
	QFile::remove(baseName + QStringLiteral(".log"));
	QFile::remove(baseName + QStringLiteral(".dvi"));
	QFile::remove(baseName + QStringLiteral(".ps"));

	// read PDF file
	QFile pdfFile(baseName + QLatin1String(".pdf"));
	if (!pdfFile.open(QIODevice::ReadOnly)) {
		QFile::remove(baseName + QStringLiteral(".pdf"));
		res->successful = false;
		res->errorMessage = i18n("Unable to open file:") + pdfFile.fileName();
		return {};
	}

	QByteArray ba = pdfFile.readAll();
	QFile::remove(baseName + QStringLiteral(".pdf"));
	res->successful = true;
	res->errorMessage = QString();

	return ba;
}

bool TeXRenderer::enabled() {
	KConfigGroup group = Settings::group(QStringLiteral("Settings_Worksheet"));
	QString engine = group.readEntry("LaTeXEngine", "");
	if (engine.isEmpty()) {
		// empty string was found in the settings (either the settings never saved or no tex engine was available during the last save)
		//->check whether the latex environment was installed in the meantime
		engine = QLatin1String("xelatex");
		if (!executableExists(engine)) {
			engine = QLatin1String("lualatex");
			if (!executableExists(engine)) {
				engine = QLatin1String("pdflatex");
				if (!executableExists(engine))
					engine = QLatin1String("latex");
			}
		}

		if (!engine.isEmpty()) {
			// one of the tex engines was found -> automatically save it in the settings without any user action
			group.writeEntry(QLatin1String("LaTeXEngine"), engine);
			group.sync();
		}
	} else if (!executableExists(engine)) {
		WARN("LaTeX engine does not exist");
		return false;
	}

	// Tools needed to convert generated  DVI files to PS and PDF
	if (engine == QLatin1String("latex")) {
		if (!executableExists(QLatin1String("convert"))) {
			WARN("program \"convert\" does not exist");
			return false;
		}
		if (!executableExists(QLatin1String("dvips"))) {
			WARN("program \"dvips\" does not exist");
			return false;
		}

#if defined(_WIN64)
		if (!executableExists(QLatin1String("gswin64c")) && !QDir(QString::fromLocal8Bit(qgetenv("PROGRAMFILES")) + QStringLiteral("/gs")).exists()
			&& !QDir(QString::fromLocal8Bit(qgetenv("PROGRAMFILES(X86)")) + QStringLiteral("/gs")).exists()) {
			WARN("ghostscript (64bit) does not exist");
			return false;
		}
#elif defined(HAVE_WINDOWS)
		if (!executableExists(QLatin1String("gswin32c")) && !QDir(QString::fromLocal8Bit(qgetenv("PROGRAMFILES")) + QStringLiteral("/gs")).exists()) {
			WARN("ghostscript (32bit) does not exist");
			return false;
		}
#endif
	}

	return true;
}

bool TeXRenderer::executableExists(const QString& exe) {
	return !safeExecutableName(exe).isEmpty();
}
