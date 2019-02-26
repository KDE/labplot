/***************************************************************************
    File                 : TeXRenderer.cc
    Project              : LabPlot
    Description          : TeX renderer class
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2016 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "TeXRenderer.h"
#include "backend/lib/macros.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QColor>
#include <QDir>
#include <QImage>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>

/*!
	\class TeXRenderer
	\brief Implements rendering of latex code to a PNG image.

	Uses latex engine specified by the user (default xelatex) to render LaTeX text

	\ingroup tools
*/
QImage TeXRenderer::renderImageLaTeX(const QString& teXString, bool* success, const TeXRenderer::Formatting& format) {
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

	//create a temporary file
	QTemporaryFile file(tempPath + QDir::separator() + "labplot_XXXXXX.tex");
	// FOR DEBUG: file.setAutoRemove(false);
	// DEBUG("temp file path = " << file.fileName().toUtf8().constData());
	if (file.open()) {
		QDir::setCurrent(tempPath);
	} else {
		WARN(QString("Couldn't open the file " + file.fileName()).toStdString());
		*success = false;
		return QImage();
	}

	//determine latex engine to be used
	KConfigGroup group = KSharedConfig::openConfig()->group("Settings_Worksheet");
	QString engine = group.readEntry("LaTeXEngine", "pdflatex");

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
	// TODO: this fails with pdflatex
	//out << "\\usepackage{mathtools}";
	out << "\\begin{document}";
	out << "\\begin{preview}";
	out << "\\pagecolor[rgb]{" << backgroundColor.redF() << ',' << backgroundColor.greenF() << ',' << backgroundColor.blueF() << "}";
	out << "\\fontsize{" << QString::number(fontSize) << "}{" << QString::number(fontSize) << "}\\selectfont";
	out << "\\color[rgb]{" << fontColor.redF() << ',' << fontColor.greenF() << ',' << fontColor.blueF() << "}";
	out << body;
	out << "\\end{preview}";
	out << "\\end{document}";
	out.flush();
	if (engine == "latex")
		return imageFromDVI(file, dpi, success);
	else
		return imageFromPDF(file, dpi, engine, success);
}

// TEX -> PDF -> PNG
QImage TeXRenderer::imageFromPDF(const QTemporaryFile& file, const int dpi, const QString& engine, bool* success) {
	QFileInfo fi(file.fileName());
	const QString& baseName = fi.completeBaseName();

	// pdflatex: produce the PDF file
	QProcess latexProcess;
#if defined(HAVE_WINDOWS)
	latexProcess.setNativeArguments("-interaction=batchmode " + file.fileName());
	latexProcess.start(engine, QStringList() << QString());
#else
	latexProcess.start(engine, QStringList() << "-interaction=batchmode" << file.fileName());
#endif

	if (!latexProcess.waitForFinished() || latexProcess.exitCode() != 0) {
		WARN("pdflatex process failed, exit code = " << latexProcess.exitCode());
		*success = false;
		QFile::remove(baseName + ".aux");
		QFile::remove(baseName + ".log");
		return QImage();
	}

	// convert: PDF -> PNG
	QProcess convertProcess;
#if defined(HAVE_WINDOWS)
	// need to set path to magick coder modules (which are in the labplot2 directory)
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	env.insert("MAGICK_CODER_MODULE_PATH", qPrintable(qgetenv("PROGRAMFILES") + QString("\\labplot2")));
	convertProcess.setProcessEnvironment(env);
#endif

	const QStringList params{"-density", QString::number(dpi), baseName + ".pdf", baseName + ".png"};
	convertProcess.start("convert", params);

	if (!convertProcess.waitForFinished() || convertProcess.exitCode() != 0) {
		WARN("convert process failed, exit code = " << convertProcess.exitCode());
		*success = false;
		QFile::remove(baseName + ".aux");
		QFile::remove(baseName + ".log");
		QFile::remove(baseName + ".pdf");
		return QImage();
	}

	// read png file
	QImage image(baseName + ".png", "png");

	// final clean up
	QFile::remove(baseName + ".aux");
	QFile::remove(baseName + ".log");
	QFile::remove(baseName + ".pdf");
	QFile::remove(baseName + ".png");

	*success = true;
	return image;
}

// TEX -> DVI -> PS -> PNG
QImage TeXRenderer::imageFromDVI(const QTemporaryFile& file, const int dpi, bool* success) {
	QFileInfo fi(file.fileName());
	const QString& baseName = fi.completeBaseName();

	//latex: produce the DVI file
	QProcess latexProcess;
	latexProcess.start("latex", QStringList() << "-interaction=batchmode" << file.fileName());
	if (!latexProcess.waitForFinished() || latexProcess.exitCode() != 0) {
		WARN("latex process failed, exit code = " << latexProcess.exitCode());
		*success = false;
		QFile::remove(baseName + ".aux");
		QFile::remove(baseName + ".log");
		return QImage();
	}

	// dvips: DVI -> PS
	QProcess dvipsProcess;
	dvipsProcess.start("dvips", QStringList() << "-E" << baseName);
	if (!dvipsProcess.waitForFinished() || dvipsProcess.exitCode() != 0) {
		WARN("dvips process failed, exit code = " << dvipsProcess.exitCode());
		*success = false;
		QFile::remove(baseName + ".aux");
		QFile::remove(baseName + ".log");
		QFile::remove(baseName + ".dvi");
		return QImage();
	}

	// convert: PS -> PNG
	QProcess convertProcess;
#if defined(HAVE_WINDOWS)
	// need to set path to magick coder modules (which are in the labplot2 directory)
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	env.insert("MAGICK_CODER_MODULE_PATH", qPrintable(qgetenv("PROGRAMFILES") + QString("\\labplot2")));
	convertProcess.setProcessEnvironment(env);
#endif

	const QStringList params{"-density", QString::number(dpi), baseName + ".ps", baseName + ".png"};
	convertProcess.start("convert", params);

	if (!convertProcess.waitForFinished() || convertProcess.exitCode() != 0) {
		WARN("convert process failed, exit code = " << convertProcess.exitCode());
		*success = false;
		QFile::remove(baseName + ".aux");
		QFile::remove(baseName + ".log");
		QFile::remove(baseName + ".dvi");
		QFile::remove(baseName + ".ps");
		return QImage();
	}

	// read png file
	QImage image(baseName + ".png", "png");

	// final clean up
	QFile::remove(baseName + ".aux");
	QFile::remove(baseName + ".log");
	QFile::remove(baseName + ".dvi");
	QFile::remove(baseName + ".ps");
	QFile::remove(baseName + ".png");

	*success = true;
	return image;
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

	//engine found, check the presence of other required tools (s.a. TeXRenderer.cpp):
	//to convert the generated PDF/PS files to PNG we need 'convert' from the ImageMagic package
	if (!executableExists(QLatin1String("convert"))) {
		WARN("program \"convert\" does not exist");
		return false;
	}

	//to convert the generated PS files to DVI we need 'dvips'
	if (engine == "latex") {
		if (!executableExists(QLatin1String("dvips"))) {
			WARN("program \"dvips\" does not exist");
			return false;
		}
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

	return true;
}

bool TeXRenderer::executableExists(const QString& exe) {
	return !QStandardPaths::findExecutable(exe).isEmpty();
}
