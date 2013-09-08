/***************************************************************************
File                 : FileDataSource.cpp
Project              : LabPlot/SciDAVis
Description 		 : Represents file data source
--------------------------------------------------------------------
Copyright            : (C) 2009-2013 Alexander Semke
Email (use @ for *)  : alexander.semke*web.de

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

#include "backend/datasources/FileDataSource.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "backend/core/Project.h"

#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QDir>
#include <QMenu>
#include <QFileSystemWatcher>
#include <QDebug>

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <QIcon>
#include <QAction>
#else
#include <KIcon>
#include <KAction>
#endif

/*!
  \class FileDataSource
  \brief Represents data stored in a file. Reading and writing is done with the help of appropriate I/O-filters.

  \ingroup datasources
*/

FileDataSource::FileDataSource(AbstractScriptingEngine* engine, const QString& name)
     : Spreadsheet(engine, name),
     m_fileType(AsciiVector),
     m_fileWatched(false),
     m_fileLinked(false),
     m_filter(0),
     m_fileSystemWatcher(0)

{ 
	initActions();
}

FileDataSource::~FileDataSource(){
	if (m_filter)
		delete m_filter;
  
	if (m_fileSystemWatcher)
		delete m_fileSystemWatcher;
}

void FileDataSource::initActions(){
	m_reloadAction = new KAction(KIcon("view-refresh"), tr("Reload"), this);
	connect(m_reloadAction, SIGNAL(triggered()), this, SLOT(read()));

	m_toggleWatchAction = new KAction(tr("Watch the file"), this);
	m_toggleWatchAction->setCheckable(true);
	connect(m_toggleWatchAction, SIGNAL(triggered()), this, SLOT(watchToggled()));
	
	m_toggleLinkAction = new KAction(tr("Link the file"), this);
	m_toggleLinkAction->setCheckable(true);
	connect(m_toggleLinkAction, SIGNAL(triggered()), this, SLOT(linkToggled()));
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
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
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

QMenu* FileDataSource::createContextMenu(){
	QMenu* menu = AbstractPart::createContextMenu();
	
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE	
	QAction* firstAction = menu->actions().first();
#else
	QAction* firstAction = 0;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);
#endif

	if (!m_fileWatched)
		menu->insertAction(firstAction, m_reloadAction);

	m_toggleWatchAction->setChecked(m_fileWatched);
	menu->insertAction(firstAction, m_toggleWatchAction);
	
	m_toggleLinkAction->setChecked(m_fileLinked);
	menu->insertAction(firstAction, m_toggleLinkAction);
	
	return menu;
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################
void FileDataSource::read(){
  if (m_fileName.isEmpty())
	return;

  if (m_filter==0)
	return;

  m_filter->read(m_fileName, this);
  watch();
}

void FileDataSource::fileChanged() {
	qDebug()<<"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
	qDebug()<<"fileChanged";
	qDebug()<<"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
	this->read();
}

void FileDataSource::watchToggled() {
	m_fileWatched = !m_fileWatched;
	watch();
	project()->setChanged(true);
}

void FileDataSource::linkToggled() {
	m_fileLinked = !m_fileLinked;
	project()->setChanged(true);
}

//watch the file upon reading for changes if required
void FileDataSource::watch() {
  if (m_fileWatched) {
	  if (!m_fileSystemWatcher) {
		m_fileSystemWatcher = new QFileSystemWatcher();
		connect (m_fileSystemWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChanged()));
	  }

	  if ( !m_fileSystemWatcher->files().contains(m_fileName) )
		m_fileSystemWatcher->addPath(m_fileName);
  }	else {
	  if (m_fileSystemWatcher)
		m_fileSystemWatcher->removePath(m_fileName);
  }
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

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void FileDataSource::save(QXmlStreamWriter* writer) const
{
	writer->writeStartElement("fileDataSource");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
    writer->writeStartElement( "general" );
	writer->writeAttribute( "fileName", m_fileName );
	writer->writeAttribute( "fileType", QString::number(m_fileType) );
	writer->writeAttribute( "fileWatched", QString::number(m_fileWatched) );
	writer->writeAttribute( "fileLinked", QString::number(m_fileLinked) );
	writer->writeEndElement();

	//filter
	m_filter->save(writer);

	//columns
	if (!m_fileLinked) {
		foreach (Column * col, children<Column>(IncludeHidden))
			col->save(writer);
	}

	writer->writeEndElement(); // "fileDataSource"
}

/*!
  Loads from XML.
*/
bool FileDataSource::load(XmlStreamReader* reader) {
    if(!reader->isStartElement() || reader->name() != "fileDataSource") {
        reader->raiseError(tr("no fileDataSource element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = tr("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;

    while (!reader->atEnd()) {
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "fileDataSource")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment") {
            if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == "general"){
            attribs = reader->attributes();

			str = attribs.value("fileName").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'fileName'"));
            else
                m_fileName = str;

            str = attribs.value("fileType").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'fileType'"));
            else
                m_fileType = (FileType)str.toInt();

			str = attribs.value("fileWatched").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'fileWatched'"));
            else
                m_fileWatched = str.toInt();

			str = attribs.value("fileLinked").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'fileLinked'"));
            else
                m_fileLinked = str.toInt();
		} else if (reader->name() == "asciiFilter") {
			m_filter = new AsciiFilter();
			if (!m_filter->load(reader))
				return false;
		} else if(reader->name() == "column") {
			Column* column = new Column("", AbstractColumn::Text);
			if (!column->load(reader)) {
				delete column;
				setColumnCount(0);
				return false;
			}
			addChild(column);
		} else {// unknown element
			reader->raiseWarning(tr("unknown element '%1'").arg(reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	//read the content of the file if it was only linked
	if (m_fileLinked)
		this->read();

	return !reader->hasError();
}
