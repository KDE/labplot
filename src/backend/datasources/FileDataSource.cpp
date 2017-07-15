/***************************************************************************
File		: FileDataSource.cpp
Project		: LabPlot
Description	: Represents file data source
--------------------------------------------------------------------
Copyright	: (C) 2009-2017 Alexander Semke (alexander.semke@web.de)
Copyright   : (C) 2017 Fabian Kristof (fkristofszabolcs@gmail.com)

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
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/datasources/filters/BinaryFilter.h"
#include "backend/core/Project.h"

#include "commonfrontend/spreadsheet/SpreadsheetView.h"

#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QDir>
#include <QMenu>
#include <QFileSystemWatcher>
#include <QFile>
#include <QTimer>
#include <QMessageBox>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTcpSocket>

#include <QIcon>
#include <QAction>
#include <KLocale>

#include <QDebug>

/*!
  \class FileDataSource
  \brief Represents data stored in a file. Reading and writing is done with the help of appropriate I/O-filters.

  \ingroup datasources
*/
FileDataSource::FileDataSource(AbstractScriptingEngine* engine, const QString& name, bool loading)
	: Spreadsheet(engine, name, loading),
	  m_fileType(Ascii),
	  m_fileWatched(false),
	  m_fileLinked(false),
	  m_filter(nullptr),
	  m_fileSystemWatcher(nullptr),
	  m_file(nullptr),
	  m_localSocket(nullptr),
	  m_tcpSocket(nullptr),
	  m_serialPort(nullptr),
	  m_updateTimer(new QTimer(this)),
	  m_paused(false),
	  m_prepared(false),
	  m_newDataAvailable(false),
	  m_bytesRead(0) {
	initActions();
	connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(read()));
}

FileDataSource::~FileDataSource() {
	if (m_filter)
		delete m_filter;

	if (m_fileSystemWatcher)
		delete m_fileSystemWatcher;

	if (m_file)
		delete m_file;

	if (m_localSocket)
		delete m_localSocket;

	if (m_tcpSocket)
		delete m_tcpSocket;

	if (m_serialPort)
		delete m_serialPort;

	delete m_updateTimer;
}

void FileDataSource::ready() {
	if (m_updateType == TimeInterval)
		m_updateTimer->start(m_updateInterval);
}

void FileDataSource::initActions() {
	m_reloadAction = new QAction(QIcon::fromTheme("view-refresh"), i18n("Reload"), this);
	connect(m_reloadAction, SIGNAL(triggered()), this, SLOT(read()));

	m_toggleWatchAction = new QAction(i18n("Watch the file"), this);
	m_toggleWatchAction->setCheckable(true);
	connect(m_toggleWatchAction, SIGNAL(triggered()), this, SLOT(watchToggled()));

	m_toggleLinkAction = new QAction(i18n("Link the file"), this);
	m_toggleLinkAction->setCheckable(true);
	connect(m_toggleLinkAction, SIGNAL(triggered()), this, SLOT(linkToggled()));
}

//TODO make the view customizable (show as a spreadsheet or as a pure text file in an editor)
QWidget *FileDataSource::view() const {
	if (!m_view)
		m_view = new SpreadsheetView(const_cast<FileDataSource*>(this));
	return m_view;
}

/*!
 * \brief Returns a list with the names of the available ports
 */
QStringList FileDataSource::availablePorts() {
	QStringList ports;
	qDebug() << "available ports count:" << QSerialPortInfo::availablePorts().size();

	for(const QSerialPortInfo& sp : QSerialPortInfo::availablePorts()) {
		ports.append(sp.portName());

		qDebug() << sp.description();
		qDebug() << sp.manufacturer();
		qDebug() << sp.portName();
		qDebug() << sp.serialNumber();
		qDebug() << sp.systemLocation();
	}

	return ports;
}

/*!
 * \brief Returns a list with the supported baud rates
 */
QStringList FileDataSource::supportedBaudRates() {
	QStringList baudRates;

	for(const auto& baud : QSerialPortInfo::standardBaudRates())
		baudRates.append(QString::number(baud));
	return baudRates;
}

/*!
 * \brief Updates this data source at this moment
 */
void FileDataSource::updateNow() {
	m_updateTimer->stop();
	read();

	//restart the timer after update
	if (m_updateType == TimeInterval)
		m_updateTimer->start(m_updateInterval);
}

/*!
 * \brief FileDataSource::stopReading
 */
//TODO: do we want this?
void FileDataSource::stopReading() {
	if (m_updateType == TimeInterval)
		m_updateTimer->stop();
	else if (m_updateType == NewData)
		disconnect(m_fileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged()));
}

/*!
 * \brief Continue reading from the live data source after it was paused.
 */
void FileDataSource::continueReading() {
	m_paused = false;
	if (m_updateType == TimeInterval)
		m_updateTimer->start(m_updateInterval);
	else if (m_updateType == NewData)
		connect(m_fileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged()));
}

/*!
 * \brief Pause the reading of the live data source.
 */
void FileDataSource::pauseReading() {
	m_paused = true;
	if (m_updateType == TimeInterval)
		m_updateTimer->stop();
	else if (m_updateType == NewData)
		disconnect(m_fileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged()));
}

/*!
  returns the list with all supported data file formats.
*/
QStringList FileDataSource::fileTypes() {
// see FileDataSource::FileType
	return (QStringList()<< i18n("ASCII data")
	        << i18n("Binary data")
	        << i18n("Image")
	        << i18n("Hierarchical Data Format (HDF)")
	        << i18n("Network Common Data Format (NetCDF)")
//		<< "CDF"
	        << i18n("Flexible Image Transport System Data Format (FITS)")
//		<< i18n("Sound")
	       );
}

void FileDataSource::setFileName(const QString& name) {
	m_fileName=name;
}

QString FileDataSource::fileName() const {
	return m_fileName;
}

void FileDataSource::setFileType(const FileType type) {
	m_fileType = type;
}

FileDataSource::FileType FileDataSource::fileType() const {
	return m_fileType;
}

void FileDataSource::setFilter(AbstractFileFilter* f) {
	m_filter = f;
}

AbstractFileFilter* FileDataSource::filter() const {
	return m_filter;
}

/*!
  sets whether the file should be watched or not.
  In the first case the data source will be automatically updated on file changes.
*/
void FileDataSource::setFileWatched(const bool b) {
	m_fileWatched = b;
}

bool FileDataSource::isFileWatched() const {
	return m_fileWatched;
}

/*!
 * \brief Sets whether we'll keep the last values or append it to the previous ones
 * \param keepLastValues
 */
void FileDataSource::setKeepLastValues(const bool keepLastValues) {
	m_keepLastValues = keepLastValues;
}

bool FileDataSource::keepLastValues() const {
	return m_keepLastValues;
}

/*!
 * \brief Sets the serial port's baud rate
 * \param baudrate
 */
void FileDataSource::setBaudRate(const int baudrate) {
	m_baudRate = baudrate;
}

int FileDataSource::baudRate() const {
	return m_baudRate;
}

/*!
 * \brief Sets the source's update interval to \c interval
 * \param interval
 */
void FileDataSource::setUpdateInterval(const int interval) {
	m_updateInterval = interval;
	m_updateTimer->start(m_updateInterval);
}

int FileDataSource::updateInterval() const {
	return m_updateInterval;
}

/*!
 * \brief Sets how many values we should store
 * \param keepnvalues
 */
void FileDataSource::setKeepNvalues(const int keepnvalues) {
	m_keepNvalues = keepnvalues;
}

int FileDataSource::keepNvalues() const {
	return m_keepNvalues;
}

/*!
 * \brief Sets the network socket's port to port
 * \param port
 */
void FileDataSource::setPort(const int port) {
	m_port = port;
}

int FileDataSource::port() const {
	return m_port;
}

/*!
 * \brief Sets the serial port's name to name
 * \param name
 */
void FileDataSource::setSerialPort(const QString &name) {
	m_serialPortName = name;
}

QString FileDataSource::serialPortName() const {
	return m_serialPortName;
}

/*!
 * \brief Sets the sample rate to samplerate
 * \param samplerate
 */
void FileDataSource::setSampleRate(const int samplerate) {
	m_sampleRate = samplerate;
}

int FileDataSource::sampleRate() const {
	return m_sampleRate;
}

/*!
 * \brief Sets the source's type to sourcetype
 * \param sourcetype
 */
void FileDataSource::setSourceType(const SourceType sourcetype) {
	m_sourceType = sourcetype;
}

FileDataSource::SourceType FileDataSource::sourceType() const {
	return m_sourceType;
}

/*!
 * \brief Sets the source's reading type to readingType
 * \param readingType
 */
void FileDataSource::setReadingType(const ReadingType readingType) {
	m_readingType = readingType;
}

FileDataSource::ReadingType FileDataSource::readingType() const {
	return m_readingType;
}

/*!
 * \brief Sets the source's update type to updatetype
 * \param updatetype
 */
void FileDataSource::setUpdateType(const UpdateType updatetype) {
	if (updatetype == NewData)
		m_updateTimer->stop();
	m_updateType = updatetype;
}

FileDataSource::UpdateType FileDataSource::updateType() const {
	return m_updateType;
}

/*!
 * \brief Sets the network socket's host
 * \param host
 */
void FileDataSource::setHost(const QString & host) {
	m_host = host;
}

QString FileDataSource::host() const {
	return m_host;
}

/*!
  sets whether only a link to the file is saved in the project file (\c b=true)
  or the whole content of the file (\c b=false).
*/
void FileDataSource::setFileLinked(const bool b) {
	m_fileLinked = b;
}

/*!
  returns \c true if only a link to the file is saved in the project file.
  \c false otherwise.
*/
bool FileDataSource::isFileLinked() const {
	return m_fileLinked;
}


QIcon FileDataSource::icon() const {
	QIcon icon;
	if (m_fileType == FileDataSource::Ascii)
		icon = QIcon::fromTheme("text-plain");
	else if (m_fileType == FileDataSource::Binary)
		icon = QIcon::fromTheme("application-octet-stream");
	else if (m_fileType == FileDataSource::Image)
		icon = QIcon::fromTheme("image-x-generic");
	// TODO: HDF, NetCDF, FITS, etc.

	return icon;
}

QMenu* FileDataSource::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();

	QAction* firstAction = 0;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

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
void FileDataSource::read() {
	if (m_filter == nullptr)
		return;

	if (!m_prepared) {
		switch (m_sourceType) {
		case FileOrPipe:
			m_file = new QFile(m_fileName);
			break;
		case NetworkSocket:
			m_tcpSocket = new QTcpSocket;

			break;
		case LocalSocket:
			m_localSocket = new QLocalSocket;
			m_localSocket->setServerName(m_fileName);

			connect(m_localSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
			m_localSocket->connectToServer(QLocalSocket::ReadOnly);
			connect(m_localSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(localSocketError(QLocalSocket::LocalSocketError)));

			break;
		case SerialPort:
			m_serialPort = new QSerialPort;
			m_serialPort->setBaudRate(m_baudRate);
			m_serialPort->setPortName(m_serialPortName);

			connect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(serialPortError(QSerialPort::SerialPortError)));
			connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(readyRead()));
			break;
		}
		m_prepared = true;
	}
	qint64 bytes;

	switch (m_sourceType) {
	case FileOrPipe:
		switch (m_fileType) {
		case Ascii:
			qDebug() << "reading live ascii file.." ;
			bytes = dynamic_cast<AsciiFilter*>(m_filter)->readFromLiveDevice(*m_file, this, m_bytesRead, AbstractFileFilter::Replace);
			m_bytesRead += bytes;
			qDebug() << "read " << bytes << " bytes, in total: " << m_bytesRead;

			break;
		case Binary:
			//bytes = dynamic_cast<BinaryFilter*>(m_filter)->readFromLiveDevice(*m_file, this, m_bytesRead);
			m_bytesRead += bytes;
		default:
			break;
		}
		break;
	case NetworkSocket:
		break;
	case LocalSocket:
		if (m_newDataAvailable) {
			switch (m_fileType) {
			case Ascii:
				dynamic_cast<AsciiFilter*>(m_filter)->readFromLiveDeviceNotFile(*m_localSocket, this);
				break;
			case Binary:
				//  dynamic_cast<BinaryFilter*>(m_filter)->readFromLiveDeviceNotFile(*m_localSocket, this);
				break;
			default:
				break;
			}
			m_localSocket->abort();
			m_localSocket->connectToServer(m_fileName, QLocalSocket::ReadOnly);
			m_newDataAvailable = false;
		}
		break;
	case SerialPort:
		if (m_newDataAvailable) {
			// copy data from buffer spreadsheet
			switch (m_fileType) {
			case Ascii:
				dynamic_cast<AsciiFilter*>(m_filter)->readFromLiveDeviceNotFile(*m_serialPort, this);
				break;
			case Binary:
				//   dynamic_cast<BinaryFilter*>(m_filter)->readFromLiveDeviceNotFile(*m_serialPort, this);
				break;

			default:
				break;
			}
			m_newDataAvailable = false;
		}
		break;
	}


	watch();
}

//for sockets, serial port, network..
void FileDataSource::readyRead() {
	if (!m_newDataAvailable)
		m_newDataAvailable = true;

	//just like for files: the file system watcher emits the signal and we read on new data
	//here new data comes when this is called actually
	if (m_updateType == NewData)
		read();
}

void FileDataSource::localSocketError(QLocalSocket::LocalSocketError socketError) {
	switch (socketError) {
	case QLocalSocket::ServerNotFoundError:
		QMessageBox::information(0, i18n("Local Socket Error"), i18n("The socket was not found. Please check the socket name."));
		break;
	case QLocalSocket::ConnectionRefusedError:
		QMessageBox::information(0, i18n("LabPlot2"),
		                         i18n("The connection was refused by the peer"));
		break;
	case QLocalSocket::PeerClosedError:
		break;
	default:
		QMessageBox::information(0, i18n("LabPlot2"),
		                         i18n("The following error occurred: %1.")
		                         .arg(m_localSocket->errorString()));
	}
}

void FileDataSource::serialPortError(QSerialPort::SerialPortError serialPortError) {
	switch (serialPortError) {
	case QSerialPort::DeviceNotFoundError:
		break;
	case QSerialPort::PermissionError:
		break;
	case QSerialPort::OpenError:
		break;
	case QSerialPort::NotOpenError:
		break;
	case QSerialPort::ReadError:
		break;
	case QSerialPort::ResourceError:
		break;
	case QSerialPort::TimeoutError:
		break;

	default:
		break;
	}
}

void FileDataSource::fileChanged() {
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
	if (m_updateType == UpdateType::NewData) {
		if (m_fileWatched) {
			if (!m_fileSystemWatcher) {
				m_fileSystemWatcher = new QFileSystemWatcher;
				connect (m_fileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged()));
			}

			if ( !m_fileSystemWatcher->files().contains(m_fileName) )
				m_fileSystemWatcher->addPath(m_fileName);
		} else {
			if (m_fileSystemWatcher)
				m_fileSystemWatcher->removePath(m_fileName);
		}
	}
}

/*!
    returns a string containing the general information about the file \c name
    and some content specific information
    (number of columns and lines for ASCII, color-depth for images etc.).
 */
QString FileDataSource::fileInfoString(const QString &name) {
	QString infoString;
	QFileInfo fileInfo;
	QString fileTypeString;
	QIODevice *file = new QFile(name);

	QString fileName;
	if (name.at(0) != QDir::separator())
		fileName = QDir::homePath() + QDir::separator() + name;
	else
		fileName = name;

	if(file==0)
		file = new QFile(fileName);

	if (file->open(QIODevice::ReadOnly)) {
		QStringList infoStrings;

		//general information about the file
		infoStrings << "<u><b>" + fileName + "</b></u><br>";
		fileInfo.setFile(fileName);

		infoStrings << i18n("Readable: %1", fileInfo.isReadable() ? i18n("yes") : i18n("no"));
		infoStrings << i18n("Writable: %1", fileInfo.isWritable() ? i18n("yes") : i18n("no"));
		infoStrings << i18n("Executable: %1", fileInfo.isExecutable() ? i18n("yes") : i18n("no"));

		infoStrings << i18n("Created: %1", fileInfo.created().toString());
		infoStrings << i18n("Last modified: %1", fileInfo.lastModified().toString());
		infoStrings << i18n("Last read: %1", fileInfo.lastRead().toString());
		infoStrings << i18n("Owner: %1", fileInfo.owner());
		infoStrings << i18n("Group: %1", fileInfo.group());
		infoStrings << i18n("Size: %1", i18np("%1 cByte", "%1 cBytes", fileInfo.size()));

#ifdef HAVE_FITS
		if (fileName.endsWith(QLatin1String(".fits"))) {
			FITSFilter* fitsFilter = new FITSFilter;

			infoStrings << i18n("Images: %1", QString::number(fitsFilter->imagesCount(fileName) ));
			infoStrings << i18n("Tables: %1", QString::number(fitsFilter->tablesCount(fileName) ));

			delete fitsFilter;
		}
#endif

		// file type and type specific information about the file
#ifdef Q_OS_LINUX
		QProcess *proc = new QProcess();
		QStringList args;
		args<<"-b"<<fileName;
		proc->start( "file", args);

		if(proc->waitForReadyRead(1000) == false)
			infoStrings << i18n("Could not open file %1 for reading.", fileName);
		else {
			fileTypeString = proc->readLine();
			if( fileTypeString.contains(i18n("cannot open")) )
				fileTypeString="";
			else {
				fileTypeString.remove(fileTypeString.length()-1,1);	// remove '\n'
			}
		}
		infoStrings << i18n("File type: %1", fileTypeString);
#endif

		//TODO depending on the file type, generate additional information about the file:
		//Number of lines for ASCII, color-depth for images etc. Use the specific filters here.
		// port the old labplot1.6 code.
		if( fileTypeString.contains("ASCII")) {
			infoStrings << "<br/>";
			//TODO: consider choosen separator
			infoStrings << i18n("Number of columns: %1", AsciiFilter::columnNumber(fileName));

			infoStrings << i18n("Number of lines: %1", AsciiFilter::lineNumber(fileName));
		}
		infoString += infoStrings.join("<br/>");
	} else
		infoString += i18n("Could not open file %1 for reading.", fileName);

	return infoString;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void FileDataSource::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("fileDataSource");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement("general");
	writer->writeAttribute("fileName", m_fileName);
	writer->writeAttribute("fileType", QString::number(m_fileType));
	writer->writeAttribute("fileWatched", QString::number(m_fileWatched));
	writer->writeAttribute("fileLinked", QString::number(m_fileLinked));
	writer->writeAttribute("updateType", QString::number(m_updateType));
	writer->writeAttribute("readingType", QString::number(m_readingType));
	writer->writeAttribute("sourceType", QString::number(m_sourceType));
	writer->writeAttribute("keepValues", QString::number(m_keepNvalues));

	if (m_updateType == TimeInterval)
		writer->writeAttribute("updateFrequency", QString::number(m_updateInterval));

	if (m_readingType != TillEnd)
		writer->writeAttribute("sampleRate", QString::number(m_sampleRate));

	switch (m_sourceType) {
	case SerialPort:
		writer->writeAttribute("baudRate", QString::number(m_baudRate));
		writer->writeAttribute("serialPortName", m_serialPortName);

		break;
	case NetworkSocket:
		writer->writeAttribute("host", m_host);
		writer->writeAttribute("port", QString::number(m_port));
		break;
	case FileOrPipe:
		break;
	case LocalSocket:
		break;
	default:
		break;
	}

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
		reader->raiseError(i18n("no fileDataSource element found"));
		return false;
	}

	if (!readBasicAttributes(reader))
		return false;

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
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
		} else if (reader->name() == "general") {
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

			str = attribs.value("updateType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'updateType'"));
			else
				m_updateType =  static_cast<UpdateType>(str.toInt());

			str = attribs.value("sourceType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'sourceType'"));
			else
				m_sourceType =  static_cast<SourceType>(str.toInt());

			str = attribs.value("readingType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'readingType'"));
			else
				m_readingType =  static_cast<ReadingType>(str.toInt());

			if (m_updateType == TimeInterval) {
				str = attribs.value("updateFrequency").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'updateFrequency'"));
				else
					m_updateInterval = str.toInt();
			}

			if (m_readingType != TillEnd) {
				str = attribs.value("sampleRate").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'sampleRate'"));
				else
					m_sampleRate = str.toInt();
			}

			switch (m_sourceType) {
			case SerialPort:
				str = attribs.value("baudRate").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'baudRate'"));
				else
					m_baudRate = str.toInt();

				str = attribs.value("serialPortName").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'serialPortName'"));
				else
					m_serialPortName = str;

				break;
			case NetworkSocket:
				str = attribs.value("host").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'host'"));
				else
					m_host = str;

				str = attribs.value("port").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'port'"));
				else
					m_host = str;
				break;
			case FileOrPipe:
				break;
			case LocalSocket:
				break;
			default:
				break;
			}

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
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	//read the content of the file if it was only linked
	if (m_fileLinked)
		this->read();

	return !reader->hasError();
}
