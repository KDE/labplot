/***************************************************************************
File                 : FileDataSource.cpp
Project              : LabPlot/SciDAVis
Description 		: Represents file data source
--------------------------------------------------------------------
Copyright            		: (C) 2009-2013 Alexander Semke
Email (use @ for *)  	: alexander.semke*web.de

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

#include "FileDataSource.h"
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QDir>
#include "backend/datasources/filters/AsciiFilter.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <QIcon>
#else
#include <KIcon>
#include <kdirwatch.h>
#endif

/*!
  \class FileDataSource
  \brief Represents data stored in a file. Reading and writing is done with the help of appropriate I/O-filters.

  \ingroup datasources
*/

FileDataSource::FileDataSource(AbstractScriptingEngine *engine, const QString& name)
     : Spreadsheet(engine, name){

}


FileDataSource::~FileDataSource(){
  if (m_filter)
	delete m_filter;
}

//TODO make the view customizable (show as a spreadsheet or as a pure text file in an editor)
QWidget *FileDataSource::view() const{
	if (!m_view){
		m_view = new SpreadsheetView(const_cast<FileDataSource*>(this));
	}
	return m_view;
}

/*!
  returns the list with all supported data file formats.
*/
QStringList FileDataSource::fileTypes(){
    return (QStringList()<< tr("ASCII vector data")
//                         << tr("BINARY vector data")
//                         << tr("ASCII matrix data")
//                         << tr("BINARY matrix data")
//                         << tr("Image")
//                         << tr("Sound")
//                         << "FITS"
//                         << "NetCDF"
//                         << "HDF"
//                         << "CDF"
                        );
}

void FileDataSource::setFileName(const QString& name){
	m_fileName=name;
}

QString FileDataSource::fileName() const{
	return m_fileName;
}

void FileDataSource::setFileType(const FileType type){
  m_fileType=type;
}

FileDataSource::FileType FileDataSource::fileType() const{
  return m_fileType;
}

void FileDataSource::setFilter(AbstractFileFilter* f){
 	m_filter=f;
}

/*!
  sets whether the file should be watched or not.
  In the first case the data source will be automaticaly updated on file changes.
*/
void FileDataSource::setFileWatched(const bool b){
	m_fileWatched=b;
}

bool FileDataSource::isFileWatched() const{
  	return m_fileWatched;
}
  
/*!
  sets whether only a link to the file is saved in the project file (\c b=true)
  or the whole content of the file (\c b=false).
*/
void FileDataSource::setFileLinked(const bool b){
  m_fileLinked=b;
}

/*!
  returns \c true if only a link to the file is saved in the project file.
  \c false otherwise.
*/
bool FileDataSource::isFileLinked() const{
  return m_fileLinked;
}


QIcon FileDataSource::icon() const{
	QIcon icon;
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	//TODO
#else
  if (m_fileType==AsciiVector || m_fileType==AsciiMatrix)
	  icon = KIcon("text-plain");
  else if (m_fileType==BinaryVector || m_fileType==BinaryMatrix)
	icon = KIcon("application-octet-stream");
  else if (m_fileType==Image)
	icon = KIcon("image-x-generic");
  else if (m_fileType==Sound)
	icon = KIcon("audio-x-generic");
#endif
	return icon;
}

// TODO
QMenu* FileDataSource::createContextMenu(){
	return 0;
}

void FileDataSource::read(){
  if (m_fileName.isEmpty())
	return;

  if (m_filter==0)
	return;

  m_filter->read(m_fileName, this);
  
  //watch the file upon reading for changes if required
//   if (m_fileWatched)
// 	this->watchFile();
}

/*!
    returns a string containing the general information about the file \c name
    and some content specific information
    (number of columns and lines for ASCII, color-depth for images etc.).
 */
QString FileDataSource::fileInfoString(const QString &name){
	QString infoString;
	QFileInfo fileInfo;
	QString fileTypeString;
    QIODevice *file = new QFile(name);

    QString fileName;
    if ( name.left(1)!=QDir::separator()){
        fileName=QDir::homePath() + QDir::separator() + name;
    }else{
        fileName=name;
    }

	if(file==0)
		file = new QFile(fileName);

	if (file->open(QIODevice::ReadOnly)){
		//general information about the file
		infoString += "<u><b>" + fileName + ":</b></u><br>";
		fileInfo.setFile(fileName);

		infoString +=	tr("Readable") + ": ";
		if ( fileInfo.isReadable() )
			infoString += tr("yes");
		else
			infoString += tr("no");

		infoString += "<br>" + tr("Writable") + ": ";
		if ( fileInfo.isWritable() )
			infoString += tr("yes");
		else
			infoString += tr("no");

		infoString += "<br>" + tr("Executable") + ": ";
		if ( fileInfo.isExecutable() )
			infoString += tr("yes");
		else
			infoString += tr("no");

		infoString += "<br>" + tr("Created") + ": " + fileInfo.created().toString();
		infoString += "<br>" + tr("Last modified") + ": " + fileInfo.lastModified().toString();
		infoString += "<br>" + tr("Last read") + ": " + fileInfo.lastRead().toString();
		infoString += "<br>" + tr("Owner") + ": " + fileInfo.owner();
		infoString += "<br>" + tr("Group") + ": " + fileInfo.group();
		infoString += "<br>" + tr("Size") + ": " + QString::number(fileInfo.size()) + " " + tr("cBytes");

        // file type and type specific information about the file
#ifdef Q_OS_LINUX
		QProcess *proc = new QProcess();
		QStringList args;
		args<<"-b"<<fileName;
		proc->start( "file", args);

		if(proc->waitForReadyRead(1000) == false){
            infoString+= tr("Could not open file %1 for reading.").arg(fileName);
		}else{
            fileTypeString = proc->readLine();
            if( fileTypeString.contains(tr("cannot open")) )
                fileTypeString="";
            else {
                fileTypeString.remove(fileTypeString.length()-1,1);	// remove '\n'
            }
		}
		infoString += "<br>" + tr("File type") + ": " + fileTypeString;
#endif

        //TODO depending on the file type, generate additional information about the file:
        //Number of lines for ASCII, color-depth for images etc. Use the specific filters here.
        // port the old labplot1.6 code.
         if( fileTypeString.contains("ASCII")){
            infoString += "<br><br>" + tr("Number of columns") + ": "
                          + QString::number(AsciiFilter::columnNumber(fileName));

            infoString += "<br>" + tr("Number of lines") + ": "
                          + QString::number(AsciiFilter::lineNumber(fileName));
        }
	}else{
		infoString+= tr("Could not open file %1 for reading.").arg(fileName);
	}

    return infoString;
}
/*
FileDataSource::watchFile(){
  KDirWatch dirWatch;
  dirWatch->addFile(m_fileName);
  connect(dirWatch, dirty(const QString&), this, fileChanged(const QString&) );
  
}*/
/*

FileDataSource::fileChanged(){
  
}*/
