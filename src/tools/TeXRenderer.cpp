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
#include "backend/lib/macros.h"
#include "kdefrontend/GuiTools.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QColor>
#include <QDir>
#include <QImage>
#include <QProcess>
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
QByteArray TeXRenderer::renderImageLaTeX(const QString& teXString, bool* success, const TeXRenderer::Formatting& format) {
	const QColor& fontColor = format.fontColor;
	const QColor& backgroundColor = format.backgroundColor;
	const int fontSize = format.fontSize;
	const QString& fontFamily = format.fontFamily;
	const int dpi = format.dpi;

	//determine the temp directory where the produced files are going to be created
	QString tempPath;
#ifdef Q_OS_LINUX
	//on linux try to use shared memory device first if available
	static bool useShm = QDir("/dev/shm/").exists();
	if (useShm)
		tempPath = "/dev/shm/";
	else
		tempPath = QDir::tempPath();
#else
	tempPath = QDir::tempPath();
#endif

	// make sure we have preview.sty available
	if (!tempPath.contains(QLatin1String("preview.sty"))) {
		QString file = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("latex/preview.sty"));
		if (file.isEmpty()) {
			WARN("Couldn't find preview.sty.");
			*success = false;
			return QByteArray();
		}
		else
			QFile::copy(file, tempPath + QLatin1String("/") + QLatin1String("preview.sty"));
	}

	//create a temporary file
	QTemporaryFile file(tempPath + QLatin1String("/") + "labplot_XXXXXX.tex");
	// FOR DEBUG: file.setAutoRemove(false);
	// DEBUG("temp file path = " << file.fileName().toUtf8().constData());
	if (file.open()) {
		QDir::setCurrent(tempPath);
	} else {
		WARN("Couldn't open the file " << STDSTRING(file.fileName()));
		*success = false;
		return QByteArray();
	}

	//determine latex engine to be used
	const auto& group = KSharedConfig::openConfig()->group("Settings_Worksheet");
	const auto& engine = group.readEntry("LaTeXEngine", "pdflatex");

	// create latex code
	QTextStream out(&file);
	int headerIndex = teXString.indexOf("\\begin{document}");
	QString body;
	if (headerIndex != -1) {
		//user provided a complete latex document -> extract the document header and body
		QString header = teXString.left(headerIndex);
		int footerIndex = teXString.indexOf("\\end{document}");
		body = teXString.mid(headerIndex + 16, footerIndex - headerIndex - 16);
		out << header;
	} else {
		//user simply provided a document body (assume it's a math. expression) -> add a minimal header
		out << "\\documentclass{minimal}";
		if (teXString.indexOf('$') == -1)
			body = '$' + teXString + '$';
		else
			body = teXString;

		//replace line breaks with tex command for a line break '\\'
		body = body.replace(QLatin1String("\n"), QLatin1String("\\\\"));
	}

	if (engine == "xelatex" || engine == "lualatex") {
		out << "\\usepackage{xltxtra}";
		out << "\\defaultfontfeatures{Ligatures=TeX}";
		if (!fontFamily.isEmpty())
			out << "\\setmainfont[Mapping=tex-text]{" << fontFamily << "}";
	}

	out << "\\usepackage{color}";
	out << "\\usepackage[active,displaymath,textmath,tightpage]{preview}";
	out << "\\setlength\\PreviewBorder{0pt}";
	// TODO: this fails with pdflatex
	//out << "\\usepackage{mathtools}";
	out << "\\begin{document}";
	out << "\\begin{preview}";
	out << "\\setlength{\\fboxsep}{0.2pt}";
	out << "\\colorbox[rgb]{" << backgroundColor.redF() << ',' << backgroundColor.greenF() << ',' << backgroundColor.blueF() << "}{";
	out << "\\fontsize{" << QString::number(fontSize) << "}{" << QString::number(fontSize) << "}\\selectfont";
	out << "\\color[rgb]{" << fontColor.redF() << ',' << fontColor.greenF() << ',' << fontColor.blueF() << "}";
	out << body;
	out << "}";
	out << "\\end{preview}";
	out << "\\end{document}";
	out.flush();

	if (engine == "latex")
		return imageFromDVI(file, dpi, success);
	else
		return imageFromPDF(file, dpi, engine, success);
}

// TEX -> PDF -> QImage
QByteArray TeXRenderer::imageFromPDF(const QTemporaryFile& file, const int dpi, const QString& engine, bool* success) {
	Q_UNUSED(dpi)
	//DEBUG(Q_FUNC_INFO << ", tmp file = " << STDSTRING(file.fileName()) << ", engine = " << STDSTRING(engine) << ", dpi = " << dpi)
	QFileInfo fi(file.fileName());
	const QString& baseName = fi.completeBaseName();

	// produce the PDF file with 'engine'
	const QString engineFullPath = QStandardPaths::findExecutable(engine);
	if (engineFullPath.isEmpty()) {
		WARN("engine " << STDSTRING(engine) << " not found");
		return QByteArray();
	}

	QProcess latexProcess;
#if defined(HAVE_WINDOWS)
	latexProcess.setNativeArguments("-interaction=batchmode " + file.fileName());
	latexProcess.start(engineFullPath, QStringList() << QString());
#else
	latexProcess.start(engineFullPath, QStringList() << "-interaction=batchmode" << file.fileName());
#endif

	if (!latexProcess.waitForFinished() || latexProcess.exitCode() != 0) {
		WARN("LaTeX process failed, exit code = " << latexProcess.exitCode());
		*success = false;
		QFile::remove(baseName + ".aux");
		QFile::remove(baseName + ".log");
		return QByteArray();
	}

	QFile::remove(baseName + ".aux");
	QFile::remove(baseName + ".log");

	//read PDF file
	QFile pdfFile(baseName + QLatin1String(".pdf"));
	if (!pdfFile.open(QIODevice::ReadOnly)) {
		QFile::remove(baseName + ".pdf");
        return QByteArray();
	}

	QByteArray ba = pdfFile.readAll();
 	QFile::remove(baseName + ".pdf");
	*success = true;

	return ba;
}

// TEX -> DVI -> PS -> PNG
QByteArray TeXRenderer::imageFromDVI(const QTemporaryFile& file, const int dpi, bool* success) {
	QFileInfo fi(file.fileName());
	const QString& baseName = fi.completeBaseName();

	//latex: produce the DVI file
	const QString latexFullPath = QStandardPaths::findExecutable(QLatin1String("latex"));
	if (latexFullPath.isEmpty()) {
		WARN("latex not found");
		return QByteArray();
	}
	QProcess latexProcess;
	latexProcess.start(latexFullPath, QStringList() << "-interaction=batchmode" << file.fileName());
	if (!latexProcess.waitForFinished() || latexProcess.exitCode() != 0) {
		WARN("latex process failed, exit code = " << latexProcess.exitCode());
		*success = false;
		QFile::remove(baseName + ".aux");
		QFile::remove(baseName + ".log");
		return QByteArray();
	}

	// dvips: DVI -> PS
	const QString dvipsFullPath = QStandardPaths::findExecutable(QLatin1String("dvips"));
	if (dvipsFullPath.isEmpty()) {
		WARN("dvips not found");
		return QByteArray();
	}
	QProcess dvipsProcess;
	dvipsProcess.start(dvipsFullPath, QStringList() << "-E" << baseName);
	if (!dvipsProcess.waitForFinished() || dvipsProcess.exitCode() != 0) {
		WARN("dvips process failed, exit code = " << dvipsProcess.exitCode());
		*success = false;
		QFile::remove(baseName + ".aux");
		QFile::remove(baseName + ".log");
		QFile::remove(baseName + ".dvi");
		return QByteArray();
	}

	// convert: PS -> PNG
	QProcess convertProcess;
#if defined(HAVE_WINDOWS)
	// need to set path to magick coder modules (which are in the labplot2 directory)
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	env.insert("MAGICK_CODER_MODULE_PATH", qPrintable(qgetenv("PROGRAMFILES") + QString("\\labplot2")));
	convertProcess.setProcessEnvironment(env);
#endif
	const QString convertFullPath = QStandardPaths::findExecutable(QLatin1String("convert"));
	if (convertFullPath.isEmpty()) {
		WARN("convert not found");
		return QByteArray();
	}

	const QStringList params{"-density", QString::number(dpi), baseName + ".ps", baseName + ".pdf"};
	convertProcess.start(convertFullPath, params);

	if (!convertProcess.waitForFinished() || convertProcess.exitCode() != 0) {
		WARN("convert process failed, exit code = " << convertProcess.exitCode());
		*success = false;
		QFile::remove(baseName + ".aux");
		QFile::remove(baseName + ".log");
		QFile::remove(baseName + ".dvi");
		QFile::remove(baseName + ".ps");
		return QByteArray();
	}

	// final clean up
	QFile::remove(baseName + ".aux");
	QFile::remove(baseName + ".log");
	QFile::remove(baseName + ".dvi");
	QFile::remove(baseName + ".ps");

	//read PDF file
	QFile pdfFile(baseName + QLatin1String(".pdf"));
	if (!pdfFile.open(QIODevice::ReadOnly)) {
		QFile::remove(baseName + ".pdf");
        return QByteArray();
	}

	QByteArray ba = pdfFile.readAll();
 	QFile::remove(baseName + ".pdf");
	*success = true;

	return ba;
}

bool TeXRenderer::enabled() {
	KConfigGroup group = KSharedConfig::openConfig()->group("Settings_Worksheet");
	QString engine = group.readEntry("LaTeXEngine", "");
	if (engine.isEmpty()) {
		//empty string was found in the settings (either the settings never saved or no tex engine was available during the last save)
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
			//one of the tex engines was found -> automatically save it in the settings without any user action
			group.writeEntry(QLatin1String("LaTeXEngine"), engine);
			group.sync();
		}
	} else if (!executableExists(engine)) {
		WARN("LaTeX engine does not exist");
		return false;
	}


	//Tools needed to convert generated  DVI files to PS and PDF
	if (engine == "latex") {
		if (!executableExists(QLatin1String("convert"))) {
			WARN("program \"convert\" does not exist");
			return false;
		}
		if (!executableExists(QLatin1String("dvips"))) {
			WARN("program \"dvips\" does not exist");
			return false;
		}

#if defined(_WIN64)
		if (!executableExists(QLatin1String("gswin64c")) && !QDir(qgetenv("PROGRAMFILES") + QString("/gs")).exists()
			&& !QDir(qgetenv("PROGRAMFILES(X86)") + QString("/gs")).exists()) {
			WARN("ghostscript (64bit) does not exist");
			return false;
		}
#elif defined(HAVE_WINDOWS)
		if (!executableExists(QLatin1String("gswin32c")) && !QDir(qgetenv("PROGRAMFILES") + QString("/gs")).exists()) {
			WARN("ghostscript (32bit) does not exist");
			return false;
		}
#endif
	}

	return true;
}

bool TeXRenderer::executableExists(const QString& exe) {
	return !QStandardPaths::findExecutable(exe).isEmpty();
}
