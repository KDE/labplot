/***************************************************************************
    File                 : TexRenderer.cc
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
#include "TexRenderer.h"

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <KTempDir>
#include <KProcess>
#include <KDebug>
#endif
#include <QDir>
#include <QTemporaryFile>
#include <QTextStream>
#include <QProcess>

// use latex to render LaTeX text
// see tex2im, etc.
// TODO: color, font size
bool TexRenderer::renderImageLaTeX( const QString& teXString, QImage& image, int fontSize){
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	kWarning()<<teXString<<endl;
#endif
	QTemporaryFile file("/dev/shm/labplot_XXXXXX.tex");
	// for debugging *.tex file
	//file.setAutoRemove(false);
	if(! file.open()) {	
		// try /tmp
		file.setFileTemplate("/tmp/labplot_XXXXXX.tex");
		if(! file.open())
			return false;
	}

	// create latex skel
	QTextStream out(&file);
	//TODO: use fontSize (or later in convert process)
	out<<"\\documentclass[12pt]{article}\n\\usepackage{color}\n\\usepackage[dvips]{graphicx}\n\\pagestyle{empty}\n\\begin{document}\n";
	out<<teXString;
	out<<"\n\\end{document}";
	out.flush();

	// latex: TeX -> DVI
	QProcess latexProcess;
	latexProcess.start("latex", QStringList() << "-interaction=batchmode" << file.fileName());
	if (!latexProcess.waitForStarted())
		return false;
	if (!latexProcess.waitForFinished())
		return false;

	// dvips: DVI -> PS
	QProcess dvipsProcess;
	QFileInfo fi(file.fileName());
	dvipsProcess.start("dvips", QStringList() << "-E" << fi.completeBaseName());
	if (!dvipsProcess.waitForStarted())
		return false;
	if (!dvipsProcess.waitForFinished())
		return false;

	// convert: PS -> PNG
	QProcess convertProcess;
	convertProcess.start("convert", QStringList() << "-scale"<< "50%" << "-density" << "200x200" << fi.completeBaseName()+".ps" << fi.completeBaseName()+".png");
	if (!convertProcess.waitForStarted())
		return false;
	if (!convertProcess.waitForFinished())
		return false;

	// read png file
	image.load(fi.completeBaseName()+".png");

	//clean up
	//QFile::remove(fi.completeBaseName()+".png");
	// also possible: latexmf -C
	QFile::remove(fi.completeBaseName()+".aux");
	QFile::remove(fi.completeBaseName()+".log");
	QFile::remove(fi.completeBaseName()+".dvi");
	QFile::remove(fi.completeBaseName()+".ps");

	return true;
}

// old method using texvc to render LaTeX text
//TODO make this function using Qt only?
bool TexRenderer::renderImageTeXvc( const QString& texString, QImage& image){
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	KTempDir *tmpDir = new KTempDir();
	QString dirName = tmpDir->name();
// 	kDebug()<<"temporary directory "<<dirName<<" is used"<<endl;

	KProcess *proc = new KProcess;
	*proc<<"texvc";
	*proc<<"/tmp"<<dirName<<texString;

	int exitCode=proc->execute();
	kDebug()<<"texvc's exit code "<<exitCode<<endl;
	if( exitCode==-2 ) {
		kDebug()<<"Couldn't find texvc."<<endl;
		return false;
	}else if (exitCode==-1){
		kDebug()<<"Texvc crashed."<<endl;
		return false;
	}

	// take resulting image and show it
 	QDir d(dirName);
 	if (d.count()!=3){
 		kDebug()<<"No file created. Check the syntax."<<endl;
 		return false;

	}

 	QString fileName = dirName+QString(d[2]);
// 	kDebug()<<"file name "<<fileName<<" is used"<<endl;
	if ( !image.load(fileName) ){
		kDebug()<<"Error on loading the tex-image"<<endl;
		tmpDir->unlink();
		return false;
	}

 	tmpDir->unlink();
	kDebug()<<"image created."<<endl;
#endif
	return true;
}
