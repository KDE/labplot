/***************************************************************************
    File                 : TeXRenderer.cc
    Project              : LabPlot
    Description          : TeX renderer class
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2013 by Alexander Semke (alexander.semke@web.de)
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

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <KTempDir>
#include <KProcess>
#include <KDebug>
#endif
#include <QDir>
#include <QTemporaryFile>
#include <QTextStream>
#include <QProcess>
#include <QtConcurrent/QtConcurrentRun>

// use (pdf)latex to render LaTeX text (see tex2im, etc.)
// TODO: test convert to svg and render to qimage, test dvipng
/*!
 * use latex to render LaTeX text (see tex2im, etc.)
 */
QImage TeXRenderer::renderImageLaTeX( const QString& teXString, const QColor& fontColor, const int fontSize, const int dpi){
	QTemporaryFile file("/dev/shm/labplot_XXXXXX.tex");
	//file.setAutoRemove(false);
	if(file.open()) {
		QDir::setCurrent("/dev/shm");
	}
	else {
		kWarning()<<"/dev/shm failed. using /tmp"<<endl;
		file.setFileTemplate("/tmp/labplot_XXXXXX.tex");
		if(file.open())
			QDir::setCurrent("/tmp");
		else
			return QImage();
	}

	// create latex code
	QTextStream out(&file);
	out << "\\documentclass{minimal}";
	out << "\\usepackage{color}\\usepackage[active,displaymath,textmath,tightpage]{preview}";
	out << "\\begin{document}";
	out << "\\definecolor{fontcolor}{rgb}{" << fontColor.redF() << ',' << fontColor.greenF() << ','<<fontColor.blueF() << "}";
	out << "\\begin{preview}";
	out << "{\\fontsize{" << QString::number(fontSize) << "}{" << QString::number(fontSize) << "}\\selectfont";
	out << "{\\color{fontcolor}\n";
	out << teXString;
	out << "\n}}\\end{preview}";
	out << "\\end{document}";
	out.flush();

	// pdflatex: TeX -> PDF
	QProcess latexProcess, convertProcess;
	latexProcess.start("pdflatex", QStringList() << "-interaction=batchmode" << file.fileName());

	QFileInfo fi(file.fileName());
	if (latexProcess.waitForFinished()) { 	// pdflatex finished
		QFile::remove(fi.completeBaseName()+".aux");
		QFile::remove(fi.completeBaseName()+".log");

		//TODO: pdflatex doesn't come back with EX_OK
// 		if(latexProcess.exitCode() != 0)	// skip if pdflatex failed
// 			return QImage();

		// convert: PDF -> PNG
		convertProcess.start("convert",  QStringList() << "-density"<< QString::number(dpi) + 'x' + QString::number(dpi)
														<< fi.completeBaseName() + ".pdf"
														<< fi.completeBaseName() + ".png");
		//gs doesn't work here. Why?
// 		convertProcess.start("gs", QStringList()<< "-sDEVICE=png16m"
// 												<< "-dTextAlphaBits=4"
// 												<< "-r" + QString::number(dpi)
// 												<< "-dGraphicsAlphaBits=4"
// 												<< "-sDEVICE=pngalpha"
// 												<< "-dSAFER"
// 												<< "-q"
// 												<< "-dNOPAUSE"
// 												<< "-sOutputFile=" + fi.completeBaseName() + ".png"
// 												<< fi.completeBaseName() + ".pdf");

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
	}

	//////////// fallback if pdflatex fails ///////////////

	// latex: TeX -> DVI
	latexProcess.start("latex", QStringList() << "-interaction=batchmode" << file.fileName());
	// also possible: latexmf -C
	if (!latexProcess.waitForFinished()) {
		kWarning()<<"latex failed."<<endl;
		QFile::remove(fi.completeBaseName()+".aux");
		QFile::remove(fi.completeBaseName()+".log");
		return QImage();
	}
	if(latexProcess.exitCode() != 0)	// skip if latex failed
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
