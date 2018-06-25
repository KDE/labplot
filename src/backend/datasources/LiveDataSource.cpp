/***************************************************************************
File		: LiveDataSource.cpp
Project		: LabPlot
Description	: Represents live data source
--------------------------------------------------------------------
Copyright	: (C) 2009-2017 Alexander Semke (alexander.semke@web.de)
Copyright	: (C) 2017 Fabian Kristof (fkristofszabolcs@gmail.com)
Copyright	: (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/datasources/filters/BinaryFilter.h"
#include "backend/core/Project.h"
#include "kdefrontend/spreadsheet/PlotDataDialog.h"

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
#include <QUdpSocket>

#include <QIcon>
#include <QAction>
#include <KLocalizedString>

#include <QDebug>

/*!
  \class LiveDataSource
  \brief Represents data stored in a file. Reading and writing is done with the help of appropriate I/O-filters.

  \ingroup datasources
*/
LiveDataSource::LiveDataSource(AbstractScriptingEngine* engine, const QString& name, bool loading)
	: Spreadsheet(engine, name, loading),
	  m_fileType(Ascii),
	  m_fileWatched(false),
	  m_fileLinked(false),
	  m_paused(false),
	  m_prepared(false),
	  m_keepLastValues(false),
	  m_bytesRead(0),
	  m_filter(nullptr),
	  m_updateTimer(new QTimer(this)),
	  m_fileSystemWatcher(nullptr),
	  m_file(nullptr),
	  m_localSocket(nullptr),
	  m_tcpSocket(nullptr),
	  m_udpSocket(nullptr),
	  m_serialPort(nullptr),
	  m_device(nullptr) {

	initActions();
	connect(m_updateTimer, &QTimer::timeout, this, &LiveDataSource::read);
}

LiveDataSource::~LiveDataSource() {
	//stop reading before deleting the objects
	pauseReading();

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

/*!
 * depending on the update type, periodically or on data changes, starts the timer or activates the file watchers, respectively.
 */
void LiveDataSource::ready() {
	if (m_updateType == TimeInterval)
		m_updateTimer->start(m_updateInterval);
	else
		watch();
}

void LiveDataSource::initActions() {
	m_reloadAction = new QAction(QIcon::fromTheme("view-refresh"), i18n("Reload"), this);
	connect(m_reloadAction, &QAction::triggered, this, &LiveDataSource::read);

	m_toggleLinkAction = new QAction(i18n("Link the file"), this);
	m_toggleLinkAction->setCheckable(true);
	connect(m_toggleLinkAction, &QAction::triggered, this, &LiveDataSource::linkToggled);

	m_plotDataAction = new QAction(QIcon::fromTheme("office-chart-line"), i18n("Plot data"), this);
	connect(m_plotDataAction, &QAction::triggered, this, &LiveDataSource::plotData);
}

QWidget* LiveDataSource::view() const {
	if (!m_partView)
		m_partView = new SpreadsheetView(const_cast<LiveDataSource*>(this), true);
	return m_partView;
}

/*!
 * \brief Returns a list with the names of the available ports
 */
QStringList LiveDataSource::availablePorts() {
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
QStringList LiveDataSource::supportedBaudRates() {
	QStringList baudRates;

	for(const auto& baud : QSerialPortInfo::standardBaudRates())
		baudRates.append(QString::number(baud));
	return baudRates;
}

/*!
 * \brief Updates this data source at this moment
 */
void LiveDataSource::updateNow() {
	m_updateTimer->stop();
	read();

	//restart the timer after update
	if (m_updateType == TimeInterval)
		m_updateTimer->start(m_updateInterval);
}

/*!
 * \brief Continue reading from the live data source after it was paused.
 */
void LiveDataSource::continueReading() {
	m_paused = false;
	if (m_updateType == TimeInterval)
		m_updateTimer->start(m_updateInterval);
	else if (m_updateType == NewData)
		connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &LiveDataSource::read);
}

/*!
 * \brief Pause the reading of the live data source.
 */
void LiveDataSource::pauseReading() {
	m_paused = true;
	if (m_updateType == TimeInterval)
		m_updateTimer->stop();
	else if (m_updateType == NewData)
		disconnect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &LiveDataSource::read);
}

/*!
  returns the list with all supported data file formats.
*/
QStringList LiveDataSource::fileTypes() {
// see LiveDataSource::FileType
	return (QStringList()<< i18n("ASCII data")
	        << i18n("Binary data")
	        << i18n("Image")
	        << i18n("Hierarchical Data Format 5 (HDF5)")
	        << i18n("Network Common Data Format (NetCDF)")
//		<< "CDF"
	        << i18n("Flexible Image Transport System Data Format (FITS)")
	        << i18n("JSON data")
//		<< i18n("Sound")
	       );
}

void LiveDataSource::setFileName(const QString& name) {
	m_fileName = name;
}

QString LiveDataSource::fileName() const {
	return m_fileName;
}

/*!
 * \brief Sets the local socket's server name to name
 * \param name
 */
void LiveDataSource::setLocalSocketName(const QString& name) {
	m_localSocketName = name;
}

QString LiveDataSource::localSocketName() const {
	return m_localSocketName;
}

void LiveDataSource::setFileType(FileType type) {
	m_fileType = type;
}

LiveDataSource::FileType LiveDataSource::fileType() const {
	return m_fileType;
}

void LiveDataSource::setFilter(AbstractFileFilter* f) {
	m_filter = f;
}

AbstractFileFilter* LiveDataSource::filter() const {
	return m_filter;
}

/*!
  sets whether the file should be watched or not.
  In the first case the data source will be automatically updated on file changes.
*/
void LiveDataSource::setFileWatched(bool b) {
	m_fileWatched = b;
}

bool LiveDataSource::isFileWatched() const {
	return m_fileWatched;
}

/*!
 * \brief Sets whether we'll keep the last values or append it to the previous ones
 * \param keepLastValues
 */
void LiveDataSource::setKeepLastValues(bool keepLastValues) {
	m_keepLastValues = keepLastValues;
}

bool LiveDataSource::keepLastValues() const {
	return m_keepLastValues;
}

/*!
 * \brief Sets the serial port's baud rate
 * \param baudrate
 */
void LiveDataSource::setBaudRate(int baudrate) {
	m_baudRate = baudrate;
}

int LiveDataSource::baudRate() const {
	return m_baudRate;
}

/*!
 * \brief Sets the source's update interval to \c interval
 * \param interval
 */
void LiveDataSource::setUpdateInterval(int interval) {
	m_updateInterval = interval;
	m_updateTimer->start(m_updateInterval);
}

int LiveDataSource::updateInterval() const {
	return m_updateInterval;
}

/*!
 * \brief Sets how many values we should store
 * \param keepnvalues
 */
void LiveDataSource::setKeepNvalues(int keepnvalues) {
	m_keepNvalues = keepnvalues;
}

int LiveDataSource::keepNvalues() const {
	return m_keepNvalues;
}

/*!
 * \brief Sets the network socket's port to port
 * \param port
 */
void LiveDataSource::setPort(quint16 port) {
	m_port = port;
}

void LiveDataSource::setBytesRead(qint64 bytes) {
	m_bytesRead = bytes;
}

int LiveDataSource::bytesRead() const {
	return m_bytesRead;
}

int LiveDataSource::port() const {
	return m_port;
}

/*!
 * \brief Sets the serial port's name to name
 * \param name
 */
void LiveDataSource::setSerialPort(const QString& name) {
	m_serialPortName = name;
}

QString LiveDataSource::serialPortName() const {
	return m_serialPortName;
}

bool LiveDataSource::isPaused() const {
	return m_paused;
}

/*!
 * \brief Sets the sample rate to samplerate
 * \param samplerate
 */
void LiveDataSource::setSampleRate(int samplerate) {
	m_sampleRate = samplerate;
}

int LiveDataSource::sampleRate() const {
	return m_sampleRate;
}

/*!
 * \brief Sets the source's type to sourcetype
 * \param sourcetype
 */
void LiveDataSource::setSourceType(SourceType sourcetype) {
	m_sourceType = sourcetype;
}

LiveDataSource::SourceType LiveDataSource::sourceType() const {
	return m_sourceType;
}

/*!
 * \brief Sets the source's reading type to readingType
 * \param readingType
 */
void LiveDataSource::setReadingType(ReadingType readingType) {
	m_readingType = readingType;
}

LiveDataSource::ReadingType LiveDataSource::readingType() const {
	return m_readingType;
}

/*!
 * \brief Sets the source's update type to updatetype and handles this change
 * \param updatetype
 */
void LiveDataSource::setUpdateType(UpdateType updatetype) {
	if (updatetype == NewData) {
		m_updateTimer->stop();
		if (m_fileSystemWatcher == nullptr)
			watch();
		else
			connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &LiveDataSource::read);
	} else {
		if (m_fileSystemWatcher)
			disconnect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &LiveDataSource::read);
	}
	m_updateType = updatetype;
}

LiveDataSource::UpdateType LiveDataSource::updateType() const {
	return m_updateType;
}

/*!
 * \brief Sets the network socket's host
 * \param host
 */
void LiveDataSource::setHost(const QString& host) {
	m_host = host;
}

QString LiveDataSource::host() const {
	return m_host;
}

/*!
  sets whether only a link to the file is saved in the project file (\c b=true)
  or the whole content of the file (\c b=false).
*/
void LiveDataSource::setFileLinked(bool b) {
	m_fileLinked = b;
}

/*!
  returns \c true if only a link to the file is saved in the project file.
  \c false otherwise.
*/
bool LiveDataSource::isFileLinked() const {
	return m_fileLinked;
}

QIcon LiveDataSource::icon() const {
	QIcon icon;
	if (m_fileType == LiveDataSource::Ascii)
		icon = QIcon::fromTheme("text-plain");
	else if (m_fileType == LiveDataSource::Binary)
		icon = QIcon::fromTheme("application-octet-stream");
	else if (m_fileType == LiveDataSource::Image)
		icon = QIcon::fromTheme("image-x-generic");
	// TODO: HDF5, NetCDF, FITS, etc.

	return icon;
}

QMenu* LiveDataSource::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();

	QAction* firstAction = 0;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size() > 1)
		firstAction = menu->actions().at(1);

	menu->insertAction(firstAction, m_plotDataAction);
	menu->insertSeparator(firstAction);

	//TODO: doesnt' always make sense...
// 	if (!m_fileWatched)
// 		menu->insertAction(firstAction, m_reloadAction);
//
// 	m_toggleWatchAction->setChecked(m_fileWatched);
// 	menu->insertAction(firstAction, m_toggleWatchAction);
//
// 	m_toggleLinkAction->setChecked(m_fileLinked);
// 	menu->insertAction(firstAction, m_toggleLinkAction);

	return menu;
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################

/*
 * called periodically or on new data changes (file changed, new data in the socket, etc.)
 */
void LiveDataSource::read() {
	DEBUG("LiveDataSource::read()");
	if (m_filter == nullptr)
		return;

	//initialize the device (file, socket, serial port), when calling this function for the first time
	if (!m_prepared) {
		DEBUG("	preparing device");
		switch (m_sourceType) {
		case FileOrPipe:
			m_file = new QFile(m_fileName);
			m_device = m_file;
			break;
		case NetworkTcpSocket:
			m_tcpSocket = new QTcpSocket(this);
			m_device = m_tcpSocket;
			m_tcpSocket->connectToHost(m_host, m_port, QIODevice::ReadOnly);

			connect(m_tcpSocket, &QTcpSocket::readyRead, this, &LiveDataSource::readyRead);
			connect(m_tcpSocket, static_cast<void (QTcpSocket::*) (QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &LiveDataSource::tcpSocketError);

			break;
		case NetworkUdpSocket:
			m_udpSocket = new QUdpSocket(this);
			m_device = m_udpSocket;
			m_udpSocket->bind(QHostAddress(m_host), m_port);
			m_udpSocket->connectToHost(m_host, 0, QUdpSocket::ReadOnly);

			connect(m_udpSocket, &QUdpSocket::readyRead, this, &LiveDataSource::readyRead);
			connect(m_udpSocket, static_cast<void (QUdpSocket::*) (QAbstractSocket::SocketError)>(&QUdpSocket::error), this, &LiveDataSource::tcpSocketError);

			break;
		case LocalSocket:
			m_localSocket = new QLocalSocket(this);
			m_device = m_localSocket;
			m_localSocket->connectToServer(m_localSocketName, QLocalSocket::ReadOnly);

			connect(m_localSocket, &QLocalSocket::readyRead, this, &LiveDataSource::readyRead);
			connect(m_localSocket, static_cast<void (QLocalSocket::*) (QLocalSocket::LocalSocketError)>(&QLocalSocket::error), this, &LiveDataSource::localSocketError);

			break;
		case SerialPort:
			m_serialPort = new QSerialPort;
			m_device = m_serialPort;
			m_serialPort->setBaudRate(m_baudRate);
			m_serialPort->setPortName(m_serialPortName);
			connect(m_serialPort, static_cast<void (QSerialPort::*) (QSerialPort::SerialPortError)>(&QSerialPort::error), this, &LiveDataSource::serialPortError);
			connect(m_serialPort, &QSerialPort::readyRead, this, &LiveDataSource::readyRead);
			break;
		}
		m_prepared = true;
	}

	qint64 bytes = 0;

	switch (m_sourceType) {
	case FileOrPipe:
		switch (m_fileType) {
		case Ascii:
			DEBUG("Reading live ascii file ..");
			if (m_readingType == LiveDataSource::ReadingType::WholeFile) {
				dynamic_cast<AsciiFilter*>(m_filter)->readFromLiveDevice(*m_file, this, 0);
			} else {
				bytes = dynamic_cast<AsciiFilter*>(m_filter)->readFromLiveDevice(*m_file, this, m_bytesRead);
				m_bytesRead += bytes;
			}
			DEBUG("Read " << bytes << " bytes, in total: " << m_bytesRead);

			break;
		case Binary:
			//bytes = dynamic_cast<BinaryFilter*>(m_filter)->readFromLiveDevice(*m_file, this, m_bytesRead);
			m_bytesRead += bytes;
		case Image:
		case HDF5:
		case NETCDF:
		case FITS:
		case Json:
			break;
		}
		break;
	case NetworkTcpSocket:
		DEBUG("reading from TCP socket. state before abort = " << m_tcpSocket->state());
		m_tcpSocket->abort();
		m_tcpSocket->connectToHost(m_host, m_port, QIODevice::ReadOnly);
		DEBUG("reading from TCP socket. state after reconnect = " << m_tcpSocket->state());
		break;
	case NetworkUdpSocket:
		DEBUG("reading from UDP socket. state before abort = " << m_udpSocket->state());
		m_udpSocket->abort();
		m_udpSocket->bind(QHostAddress(m_host), m_port);
		m_udpSocket->connectToHost(m_host, 0, QUdpSocket::ReadOnly);
		DEBUG("reading from UDP socket. state after reconnect = " << m_udpSocket->state());
		break;
	case LocalSocket:
		DEBUG("reading from local socket. state before abort = " << m_localSocket->state());
		m_localSocket->abort();
		m_localSocket->connectToServer(m_localSocketName, QLocalSocket::ReadOnly);
		DEBUG("reading from local socket. state after reconnect = " << m_localSocket->state());
		break;
	case SerialPort:
		DEBUG("reading from the serial port");
		m_serialPort->setBaudRate(m_baudRate);
		m_serialPort->setPortName(m_serialPortName);
		m_device = m_serialPort;
		//TODO
		break;
	}
}

/*!
 * Slot for the signal that is emitted once every time new data is available for reading from the device.
 * It will only be emitted again once new data is available, such as when a new payload of network data has arrived on the network socket,
 * or when a new block of data has been appended to your device.
 */
void LiveDataSource::readyRead() {
	DEBUG("Got new data from the device");

	if (m_fileType == Ascii)
		dynamic_cast<AsciiFilter*>(m_filter)->readFromLiveDeviceNotFile(*m_device, this);
// 	else if (m_fileType == Binary)
	//  dynamic_cast<BinaryFilter*>(m_filter)->readFromLiveDeviceNotFile(*m_device, this);

	//since we won't have the timer to call read() where we create new connections
	//for sequencial devices in read() we just request data/connect to servers
	if (m_updateType == NewData)
		read();
}

void LiveDataSource::localSocketError(QLocalSocket::LocalSocketError socketError) {
	Q_UNUSED(socketError);
	/*disconnect(m_localSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(localSocketError(QLocalSocket::LocalSocketError)));
	disconnect(m_localSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));*/

	/*switch (socketError) {
	case QLocalSocket::ServerNotFoundError:
		QMessageBox::critical(0, i18n("Local Socket Error"),
		                      i18n("The socket was not found. Please check the socket name."));
		break;
	case QLocalSocket::ConnectionRefusedError:
		QMessageBox::critical(0, i18n("Local Socket Error"),
		                      i18n("The connection was refused by the peer"));
		break;
	case QLocalSocket::PeerClosedError:
		QMessageBox::critical(0, i18n("Local Socket Error"),
		                      i18n("The socket has closed the connection."));
		break;
	default:
		QMessageBox::critical(0, i18n("Local Socket Error"),
		                      i18n("The following error occurred: %1.", m_localSocket->errorString()));
	}*/
}

void LiveDataSource::tcpSocketError(QAbstractSocket::SocketError socketError) {
	Q_UNUSED(socketError);
	/*switch (socketError) {
	case QAbstractSocket::ConnectionRefusedError:
		QMessageBox::critical(0, i18n("TCP Socket Error"),
		                      i18n("The connection was refused by the peer. Make sure the server is running and check the host name and port settings."));
		break;
	case QAbstractSocket::RemoteHostClosedError:
		QMessageBox::critical(0, i18n("TCP Socket Error"),
		                      i18n("The remote host closed the connection."));
		break;
	case QAbstractSocket::HostNotFoundError:
		QMessageBox::critical(0, i18n("TCP Socket Error"),
		                      i18n("The host was not found. Please check the host name and port settings."));
		break;
	default:
		QMessageBox::critical(0, i18n("TCP Socket Error"),
		                      i18n("The following error occurred: %1.", m_tcpSocket->errorString()));
	}*/
}

void LiveDataSource::serialPortError(QSerialPort::SerialPortError serialPortError) {
	switch (serialPortError) {
	case QSerialPort::DeviceNotFoundError:
		QMessageBox::critical(0, i18n("Serial Port Error"), i18n("Failed to open the device."));
		break;
	case QSerialPort::PermissionError:
		QMessageBox::critical(0, i18n("Serial Port Error"),
			i18n("Failed to open the device. Please check your permissions on this device."));
		break;
	case QSerialPort::OpenError:
		QMessageBox::critical(0, i18n("Serial Port Error"), i18n("Device already opened."));
		break;
	case QSerialPort::NotOpenError:
		QMessageBox::critical(0, i18n("Serial Port Error"), i18n("The device is not opened."));
		break;
	case QSerialPort::ReadError:
		QMessageBox::critical(0, i18n("Serial Port Error"), i18n("Failed to read data."));
		break;
	case QSerialPort::ResourceError:
		QMessageBox::critical(0, i18n("Serial Port Error"), i18n("Failed to read data. The device is removed."));
		break;
	case QSerialPort::TimeoutError:
		QMessageBox::critical(0, i18n("Serial Port Error"), i18n("The device timed out."));
		break;
#ifndef _MSC_VER
	//MSVC complains about the usage of deprecated enums, g++ and clang complain about missing enums
	case QSerialPort::ParityError:
	case QSerialPort::FramingError:
	case QSerialPort::BreakConditionError:
#endif
	case QSerialPort::WriteError:
	case QSerialPort::UnsupportedOperationError:
	case QSerialPort::UnknownError:
		QMessageBox::critical(0, i18n("Serial Port Error"),
			i18n("The following error occurred: %1.", m_serialPort->errorString()));
		break;
	case QSerialPort::NoError:
		break;
	}
}

void LiveDataSource::watchToggled() {
	m_fileWatched = !m_fileWatched;
	watch();
	project()->setChanged(true);
}

void LiveDataSource::linkToggled() {
	m_fileLinked = !m_fileLinked;
	project()->setChanged(true);
}

//watch the file upon reading for changes if required
void LiveDataSource::watch() {
	if (m_fileWatched) {
		if (!m_fileSystemWatcher) {
			m_fileSystemWatcher = new QFileSystemWatcher;
			connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &LiveDataSource::read);
		}

		if ( !m_fileSystemWatcher->files().contains(m_fileName) )
			m_fileSystemWatcher->addPath(m_fileName);
	} else {
		if (m_fileSystemWatcher)
			m_fileSystemWatcher->removePath(m_fileName);
	}
}

/*!
    returns a string containing the general information about the file \c name
    and some content specific information
    (number of columns and lines for ASCII, color-depth for images etc.).
 */
QString LiveDataSource::fileInfoString(const QString &name) {
	QString infoString;
	QFileInfo fileInfo;
	QString fileTypeString;
	QIODevice *file = new QFile(name);

	QString fileName;
#ifdef Q_OS_WIN
	if (name.at(1) != QLatin1Char(':'))
		fileName = QDir::homePath() + name;
	else
		fileName = name;
#else
	if (name.at(0) != QDir::separator())
		fileName = QDir::homePath() + QDir::separator() + name;
	else
		fileName = name;
#endif
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
			infoStrings << i18n("Images: %1", QString::number(FITSFilter::imagesCount(fileName) ));
			infoStrings << i18n("Tables: %1", QString::number(FITSFilter::tablesCount(fileName) ));
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

void LiveDataSource::plotData() {
	PlotDataDialog* dlg = new PlotDataDialog(this);
	dlg->exec();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void LiveDataSource::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("LiveDataSource");
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
		writer->writeAttribute("updateInterval", QString::number(m_updateInterval));

	if (m_readingType != TillEnd)
		writer->writeAttribute("sampleRate", QString::number(m_sampleRate));

	switch (m_sourceType) {
	case SerialPort:
		writer->writeAttribute("baudRate", QString::number(m_baudRate));
		writer->writeAttribute("serialPortName", m_serialPortName);

		break;
	case NetworkTcpSocket:
	case NetworkUdpSocket:
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
		for (auto* col : children<Column>(IncludeHidden))
			col->save(writer);
	}

	writer->writeEndElement(); // "LiveDataSource"
}

/*!
  Loads from XML.
*/
bool LiveDataSource::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "LiveDataSource")
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
				reader->raiseWarning(attributeWarning.subs("fileName").toString());
			else
				m_fileName = str;

			str = attribs.value("fileType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("fileType").toString());
			else
				m_fileType = (FileType)str.toInt();

			str = attribs.value("fileWatched").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("fileWatched").toString());
			else
				m_fileWatched = str.toInt();

			str = attribs.value("fileLinked").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("fileLinked").toString());
			else
				m_fileLinked = str.toInt();

			str = attribs.value("updateType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("updateType").toString());
			else
				m_updateType =  static_cast<UpdateType>(str.toInt());

			str = attribs.value("sourceType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("sourceType").toString());
			else
				m_sourceType =  static_cast<SourceType>(str.toInt());

			str = attribs.value("readingType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("readingType").toString());
			else
				m_readingType =  static_cast<ReadingType>(str.toInt());

			if (m_updateType == TimeInterval) {
				str = attribs.value("updateInterval").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("updateInterval").toString());
				else
					m_updateInterval = str.toInt();
			}

			if (m_readingType != TillEnd) {
				str = attribs.value("sampleRate").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("sampleRate").toString());
				else
					m_sampleRate = str.toInt();
			}

			switch (m_sourceType) {
			case SerialPort:
				str = attribs.value("baudRate").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("baudRate").toString());
				else
					m_baudRate = str.toInt();

				str = attribs.value("serialPortName").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("serialPortName").toString());
				else
					m_serialPortName = str;

				break;
			case NetworkTcpSocket:
			case NetworkUdpSocket:
				str = attribs.value("host").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("host").toString());
				else
					m_host = str;

				str = attribs.value("port").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("port").toString());
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
			if (!column->load(reader, preview)) {
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
