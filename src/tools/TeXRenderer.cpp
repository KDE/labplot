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

#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KConfig>

#include <QImage>
#include <QColor>
#include <QDir>
#include <QTemporaryFile>
#include <QTextStream>
#include <QProcess>

/*!
	\class TeXRenderer
	\brief Implements rendering of latex code to a PNG image, uses latex engine specified by the user (default xelatex) to render LaTeX text

	\ingroup tools
*/
QImage TeXRenderer::renderImageLaTeX( const QString& teXString, const QColor& fontColor, const int fontSize, const int dpi){
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
	if(file.open()) {
		QDir::setCurrent(tempPath);
	} else {
		qWarning() << "Couldn't open the file " << file.fileName();
		return QImage();
	}

	//determine latex engine to be used
	KConfigGroup group = KGlobal::config()->group( "General" );
	QString engine = group.readEntry("TeXEngine", "");

	// create latex code
	QTextStream out(&file);
	int headerIndex = teXString.indexOf("\\begin{document}");
	QString body;
	if (headerIndex!=-1) {
		//user provided a complete latex document -> extract the document header and body
		QString header = teXString.left(headerIndex);
		int footerIndex = teXString.indexOf("\\end{document}");
		body = teXString.mid(headerIndex + 16, footerIndex - headerIndex - 16);
		out << header;
	} else {
		//user simply provided a document body (assume it's a math. expression) -> add a minimal header
		out << "\\documentclass{minimal}";
		if (engine == "latex")
			body = '$' + teXString + '$';
		else
			body = teXString;
	}

	if (engine=="xelatex" || engine=="lualatex")
		out<< "\\usepackage{xltxtra}";

	out << "\\usepackage{color}";
	out << "\\usepackage[active,displaymath,textmath,tightpage]{preview}";
	out << "\\begin{document}";
	out << "\\definecolor{fontcolor}{rgb}{" << fontColor.redF() << ',' << fontColor.greenF() << ','<<fontColor.blueF() << "}";
	out << "\\begin{preview}";
	out << "{\\fontsize{" << QString::number(fontSize) << "}{" << QString::number(fontSize) << "}\\selectfont";
	out << "{\\color{fontcolor}";
	out << body;
	out << "}}\\end{preview}";
	out << "\\end{document}";
	out.flush();

	if (engine!="latex")
		return imageFromPDF(file, dpi, engine);
	else
		return imageFromDVI(file, dpi);
}

// TEX -> PDF -> PNG
QImage TeXRenderer::imageFromPDF(const QTemporaryFile& file, const int dpi, const QString& engine) {
	QProcess latexProcess, convertProcess;
	latexProcess.start(engine, QStringList() << "-interaction=batchmode" << file.fileName());

	if (latexProcess.waitForFinished()) { // pdflatex finished
		QFileInfo fi(file.fileName());
		QFile::remove(fi.completeBaseName()+".aux");
		QFile::remove(fi.completeBaseName()+".log");

		//TODO: pdflatex doesn't come back with EX_OK
// 		if(latexProcess.exitCode() != 0)	// skip if pdflatex failed
// 			return QImage();

		// convert: PDF -> PNG
		convertProcess.start("convert",  QStringList() << "-density"<< QString::number(dpi) + 'x' + QString::number(dpi)
														<< fi.completeBaseName() + ".pdf"
														<< fi.completeBaseName() + ".png");

		// clean up and read png file
		if (convertProcess.waitForFinished()) {
			QFile::remove(fi.completeBaseName()+".pdf");

			QImage image;
			image.load(fi.completeBaseName()+".png");
			QFile::remove(fi.completeBaseName()+".png");

			return image;
		}else{
			QFile::remove(fi.completeBaseName()+".pdf");
			return QImage();
		}
	}else{
		kWarning()<<"pdflatex failed."<<endl;
		return QImage();
	}
}

// TEX -> DVI -> PS -> PNG
QImage TeXRenderer::imageFromDVI(const QTemporaryFile& file, const int dpi) {
	QProcess latexProcess, convertProcess;
	latexProcess.start("latex", QStringList() << "-interaction=batchmode" << file.fileName());

	QFileInfo fi(file.fileName());
	if (!latexProcess.waitForFinished()) {
		kWarning()<<"latex failed."<<endl;
		QFile::remove(fi.completeBaseName()+".aux");
		QFile::remove(fi.completeBaseName()+".log");
		return QImage();
	}

	if(latexProcess.exitCode() != 0) // skip if latex failed
		return QImage();

	// dvips: DVI -> PS
	QProcess dvipsProcess;
	dvipsProcess.start("dvips", QStringList() << "-E" << fi.completeBaseName());
	if (!dvipsProcess.waitForFinished()) {
		kWarning()<<"dvips failed."<<endl;
		QFile::remove(fi.completeBaseName()+".dvi");
		return QImage();
	}

	// convert: PS -> PNG
	convertProcess.start("convert", QStringList() << "-density" << QString::number(dpi) + 'x' + QString::number(dpi)  << fi.completeBaseName()+".ps" << fi.completeBaseName()+".png");
	if (!convertProcess.waitForFinished()) {
		kWarning()<<"convert failed."<<endl;
		QFile::remove(fi.completeBaseName()+".ps");
		return QImage();
	}

	// read png file
	QImage image;
	image.load(fi.completeBaseName()+".png", "png");

	//clean up
	QFile::remove(fi.completeBaseName()+".png");
	QFile::remove(fi.completeBaseName()+".aux");
	QFile::remove(fi.completeBaseName()+".log");
	QFile::remove(fi.completeBaseName()+".dvi");
	QFile::remove(fi.completeBaseName()+".ps");

	return image;
}
