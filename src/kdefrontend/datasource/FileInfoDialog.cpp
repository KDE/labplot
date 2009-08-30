/***************************************************************************
    File                 : FileInfoDialog.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de,
				    alexander.semke*web.de
    Description          : import file data dialog

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

#include "FileInfoDialog.h"
#include <KDebug>
#include <KLocale>
#include <KFilterDev>
#include <QFileInfo>
#include <QProcess>

 /*!
	\class ImportWidget
	\brief Provides a dialog containing the information about the files to be imported.

	\ingroup kdefrontend
 */

FileInfoDialog::FileInfoDialog(QWidget* parent) : KDialog(parent) {

	textEditWidget.setReadOnly(true);
	textEditWidget.setLineWrapMode(QTextEdit::NoWrap);
	setMainWidget( &textEditWidget );
 	setButtons( KDialog::Ok);
 	setWindowIcon(KIcon("help-about"));
	setCaption(i18n("File info"));
 	resize( QSize(500,300) );
}


void FileInfoDialog::setFiles(QStringList& files){
	QString fileName;
	QString infoString;
	QFileInfo fileInfo;
	QString fileTypeString;

	 for ( int i=0; i<files.size(); i++ ) {
		fileName = files.at(i);
		if(fileName.isEmpty())
			continue;

		QIODevice *file = KFilterDev::deviceForFile(fileName, QString::null, true);
		if(file==0)
			file = new QFile(fileName);

		if (file->open(QIODevice::ReadOnly)){
			if (infoString!="")
				infoString += "<br><br><br>";

			//general information about the file
			infoString += "<u><b>" + fileName + ":</b></u><br>";
			fileInfo.setFile(fileName);

			infoString +=	i18n("Readable") + ": ";
			if ( fileInfo.isReadable() )
				infoString += i18n("yes");
			else
				infoString += i18n("no");

			infoString += "<br>" + i18n("Writable") + ": ";
			if ( fileInfo.isWritable() )
				infoString += i18n("yes");
			else
				infoString += i18n("no");

			infoString += "<br>" + i18n("Executable") + ": ";
			if ( fileInfo.isExecutable() )
				infoString += i18n("yes");
			else
				infoString += i18n("no");

			infoString += "<br>" + i18n("Created") + ": " + fileInfo.created().toString();
			infoString += "<br>" + i18n("Last modified") + ": " + fileInfo.lastModified().toString();
			infoString += "<br>" + i18n("Last read") + ": " + fileInfo.lastRead().toString();
			infoString += "<br>" + i18n("Owner") + ": " + fileInfo.owner();
			infoString += "<br>" + i18n("Group") + ": " + fileInfo.group();
			infoString += "<br>" + i18n("Size") + ": " + QString().setNum(fileInfo.size()) + i18n(" Bytes");


			// file type and type specific information about the file
			QProcess *proc = new QProcess(this);
			QStringList args;
			args<<"-b"<<fileName;
			proc->start( "file", args);

			if(proc->waitForReadyRead(1000) == false){
				kDebug()<<"ERROR: reading the file type of the file"<<fileName<<endl;
				fileTypeString="";
			}else{
				fileTypeString = proc->readLine();
				if( fileTypeString.contains(i18n("cannot open")) )
					fileTypeString="";
				else {
					fileTypeString.remove(fileTypeString.length()-1,1);	// remove '\n'
				}
			}
			infoString += "<br>" + i18n("File type") + ": " + fileTypeString;

		//TODO
		// port the old labplot1.6 code
		}else{
			kDebug()<<"WARNING: Could not open file"<<fileName<<endl;
			infoString+= i18n("Could not open file %1 for reading.").arg(fileName);
		}
	}

	textEditWidget.document()->setHtml(infoString);
}