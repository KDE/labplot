/***************************************************************************
    File                 : TeXRenderer.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    					(use @ for *)
    Description          : TeX renderer class
                           
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
#include <QtConcurrentRun>

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
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		kWarning()<<"/dev/shm failed. using /tmp"<<endl;
#endif
		file.setFileTemplate("/tmp/labplot_XXXXXX.tex");
		if(file.open())
			QDir::setCurrent("/tmp");
		else
			return QImage();
	}

	// create latex code
	QTextStream out(&file);
	out << "\\documentclass[" << fontSize <<  "]{article}\n";
	out << "\\usepackage{color}\n\\usepackage[active,displaymath,textmath,tightpage]{preview}\n";
	out << "\\begin{document}\n";
	out << "\\definecolor{fontcolor}{rgb}{" << fontColor.redF() << ',' << fontColor.greenF() << ','<<fontColor.blueF() << "}\n";
	out << "\\begin{preview}\n";
	out << "{\\color{fontcolor}\n";
	out << teXString;
	out << "\n}\n\\end{preview}\n";
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
		convertProcess.start("convert",  QStringList() << "-density"<< QString::number(dpi) + "x" + QString::number(dpi)
																<< fi.completeBaseName() + ".pdf"
																<< fi.completeBaseName() + ".png");
		//gs doesn't work here. Why?
// 		convertProcess.start("gs", QStringList() << "-sDEVICE=png16m -dTextAlphaBits=4 -r" + QString::number(dpi) + " -dGraphicsAlphaBits=4 -dSAFER -q -dNOPAUSE"
// 																		<< "-sOutputFile=" <<  fi.completeBaseName() + ".png"
// 																		<< fi.completeBaseName() + ".pdf");

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
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		kWarning()<<"pdflatex failed."<<endl;
#endif
	}

	//////////// fallback if pdflatex fails ///////////////

	// latex: TeX -> DVI
	latexProcess.start("latex", QStringList() << "-interaction=batchmode" << file.fileName());
	// also possible: latexmf -C
	if (!latexProcess.waitForFinished()) {
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		kWarning()<<"latex failed."<<endl;
#endif
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
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		kWarning()<<"dvips failed."<<endl;
#endif
		QFile::remove(fi.completeBaseName()+".dvi");
		return QImage();
	}

	// convert: PS -> PNG
	convertProcess.start("convert", QStringList() << "-density" << QString::number(dpi) + "x" + QString::number(dpi)  << fi.completeBaseName()+".ps" << fi.completeBaseName()+".png");
	if (!convertProcess.waitForFinished()) {
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		kWarning()<<"convert failed."<<endl;
#endif
		QFile::remove(fi.completeBaseName()+".ps");
		return QImage();
	}

	// read png file
	QImage image;
	image.load(fi.completeBaseName()+".png");

	//clean up
	QFile::remove(fi.completeBaseName()+".png");
	QFile::remove(fi.completeBaseName()+".aux");
	QFile::remove(fi.completeBaseName()+".log");
	QFile::remove(fi.completeBaseName()+".dvi");
	QFile::remove(fi.completeBaseName()+".ps");

	return image;
}
