/*
    File		: LiveDataSource.cpp
    Project		: LabPlot
    Description	: Represents live data source
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009-2019 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2017 Fabian Kristof <fkristofszabolcs@gmail.com>
    SPDX-FileCopyrightText: 2018 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/datasources/filters/BinaryFilter.h"
#include "backend/datasources/filters/ROOTFilter.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "kdefrontend/spreadsheet/PlotDataDialog.h"

#include <QAction>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QTcpSocket>
#include <QUdpSocket>
#ifdef HAVE_QTSERIALPORT
#include <QSerialPortInfo>
#endif

#include <KLocalizedString>

/*!
  \class LiveDataSource
  \brief Represents data stored in a file. Reading and writing is done with the help of appropriate I/O-filters.

  \ingroup datasources
*/
LiveDataSource::LiveDataSource(const QString& name, bool loading) : Spreadsheet(name, loading, AspectType::LiveDataSource),
	m_updateTimer(new QTimer(this)), m_watchTimer(new QTimer(this)) {

	initActions();

	//stop reading from the source before removing the child from the project
	connect(this, &AbstractAspect::aspectAboutToBeRemoved, this, &LiveDataSource::pauseReading);

	connect(m_updateTimer, &QTimer::timeout, this, &LiveDataSource::read);
	connect(m_watchTimer, &QTimer::timeout, this, &LiveDataSource::readOnUpdate);
}

LiveDataSource::~LiveDataSource() {
	//stop reading before deleting the objects
	pauseReading();

	delete m_filter;
	delete m_fileSystemWatcher;
	delete m_localSocket;
	delete m_tcpSocket;
#ifdef HAVE_QTSERIALPORT
	delete m_serialPort;
#endif
}

void LiveDataSource::initActions() {
	m_plotDataAction = new QAction(QIcon::fromTheme("office-chart-line"), i18n("Plot data"), this);
	connect(m_plotDataAction, &QAction::triggered, this, &LiveDataSource::plotData);
	m_watchTimer->setSingleShot(true);
	m_watchTimer->setInterval(100);
}

QWidget* LiveDataSource::view() const {
	if (!m_partView) {
		m_view = new SpreadsheetView(const_cast<LiveDataSource*>(this), true);
		m_partView = m_view;
	}
	return m_partView;
}

/*!
 * \brief Returns a list with the names of the available ports
 */
QStringList LiveDataSource::availablePorts() {
	QStringList ports;
// 	qDebug() << "available ports count:" << QSerialPortInfo::availablePorts().size();

#ifdef HAVE_QTSERIALPORT
	for (const QSerialPortInfo& sp : QSerialPortInfo::availablePorts()) {
		ports.append(sp.portName());

		DEBUG(" port " << STDSTRING(sp.portName()) << ": " << STDSTRING(sp.systemLocation()) << STDSTRING(sp.description())
			<< ' ' << STDSTRING(sp.manufacturer()) << ' ' << STDSTRING(sp.serialNumber()));
	}
	// For Testing:
	// ports.append("/dev/pts/26");
#endif

	return ports;
}

/*!
 * \brief Returns a list with the supported baud rates
 */
QStringList LiveDataSource::supportedBaudRates() {
	QStringList baudRates;

#ifdef HAVE_QTSERIALPORT
	for (const auto& baud : QSerialPortInfo::standardBaudRates())
		baudRates.append(QString::number(baud));
#endif
	return baudRates;
}

/*!
 * \brief Updates this data source at this moment
 */
void LiveDataSource::updateNow() {
	DEBUG("LiveDataSource::updateNow() update interval = " << m_updateInterval);
	if (m_updateType == UpdateType::TimeInterval)
		m_updateTimer->stop();
	else
		m_pending = false;
	read();

	//restart the timer after update
	if (m_updateType == UpdateType::TimeInterval && !m_paused)
		m_updateTimer->start(m_updateInterval);
}

/*!
 * \brief Continue reading from the live data source after it was paused
 */
void LiveDataSource::continueReading() {
	m_paused = false;
	if (m_pending) {
		m_pending = false;
		updateNow();
	}
}

/*!
 * \brief Pause the reading of the live data source
 */
void LiveDataSource::pauseReading() {
	m_paused = true;
	if (m_updateType == UpdateType::TimeInterval) {
		m_pending = true;
		m_updateTimer->stop();
	}
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

void LiveDataSource::setFileType(AbstractFileFilter::FileType type) {
	m_fileType = type;
}

AbstractFileFilter::FileType LiveDataSource::fileType() const {
	return m_fileType;
}

void LiveDataSource::setFilter(AbstractFileFilter* f) {
	delete m_filter;
	m_filter = f;
}

AbstractFileFilter* LiveDataSource::filter() const {
	return m_filter;
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
	if (!m_paused)
		m_updateTimer->start(m_updateInterval);
}

int LiveDataSource::updateInterval() const {
	return m_updateInterval;
}

/*!
 * \brief Sets how many values we should keep when keepLastValues is true
 * \param keepnvalues
 */
void LiveDataSource::setKeepNValues(int keepnvalues) {
	m_keepNValues = keepnvalues;
}

int LiveDataSource::keepNValues() const {
	return m_keepNValues;
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
 * \brief Sets the sample size to size
 * \param size
 */
void LiveDataSource::setSampleSize(int size) {
	m_sampleSize = size;
}

int LiveDataSource::sampleSize() const {
	return m_sampleSize;
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
	switch (updatetype) {
	case UpdateType::NewData: {
		m_updateTimer->stop();
		if (!m_fileSystemWatcher)
			m_fileSystemWatcher = new QFileSystemWatcher(this);

		m_fileSystemWatcher->addPath(m_fileName);
		QFileInfo file(m_fileName);
		// If the watched file currently does not exist (because it is recreated for instance), watch its containing
		// directory instead. Once the file exists again, switch to watching the file in readOnUpdate().
		// Reading will only start 100ms after the last update, to prevent continuous re-reading while the file is updated.
		// If the watched file intentionally is updated more often than that, the user should switch to periodic reading.
		if (m_fileSystemWatcher->files().contains(m_fileName))
			m_fileSystemWatcher->removePath(file.absolutePath());
		else
			m_fileSystemWatcher->addPath(file.absolutePath());

		connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, [&](){ m_watchTimer->start(); });
		connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, [&](){ m_watchTimer->start(); });
		break;
	}
	case UpdateType::TimeInterval:
		delete m_fileSystemWatcher;
		m_fileSystemWatcher = nullptr;
		break;
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
	m_host = host.simplified();
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

void LiveDataSource::setUseRelativePath(bool b) {
	m_relativePath = b;
}

bool LiveDataSource::useRelativePath() const {
	return m_relativePath;
}

QIcon LiveDataSource::icon() const {
	QIcon icon;

	switch (m_fileType) {
	case AbstractFileFilter::FileType::Ascii:
		icon = QIcon::fromTheme("text-plain");
		break;
	case AbstractFileFilter::FileType::Binary:
		icon = QIcon::fromTheme("application-octet-stream");
		break;
	case AbstractFileFilter::FileType::Image:
		icon = QIcon::fromTheme("image-x-generic");
		break;
	// TODO: missing icons
	case AbstractFileFilter::FileType::HDF5:
	case AbstractFileFilter::FileType::NETCDF:
		break;
	case AbstractFileFilter::FileType::FITS:
		icon = QIcon::fromTheme("kstars_fitsviewer");
		break;
	case AbstractFileFilter::FileType::JSON:
		icon = QIcon::fromTheme("application-json");
		break;
	case AbstractFileFilter::FileType::MATIO:
		icon = QIcon::fromTheme("matlab");
		break;
	case AbstractFileFilter::FileType::READSTAT:
		icon = QIcon::fromTheme("view-statistics");
		break;
	case AbstractFileFilter::FileType::ROOT:
		icon = QIcon::fromTheme("application-x-root");
		break;
	case AbstractFileFilter::FileType::NgspiceRawAscii:
	case AbstractFileFilter::FileType::NgspiceRawBinary:
		break;
	}

	return icon;
}

QMenu* LiveDataSource::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();

	QAction* firstAction = nullptr;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size() > 1)
		firstAction = menu->actions().at(1);

	menu->insertAction(firstAction, m_plotDataAction);
	menu->insertSeparator(firstAction);

	return menu;
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################

/*
 * Called when the watch timer times out, i.e. when modifying the file or directory
 * presumably has finished. Also see LiveDataSource::setUpdateType().
 */
void LiveDataSource::readOnUpdate() {
	if (!m_fileSystemWatcher->files().contains(m_fileName)) {
		m_fileSystemWatcher->addPath(m_fileName);
		QFileInfo file(m_fileName);
		if (m_fileSystemWatcher->files().contains(m_fileName))
			m_fileSystemWatcher->removePath(file.absolutePath());
		else {
			m_fileSystemWatcher->addPath(file.absolutePath());
			return;
		}
	}
	if (m_paused)
		// flag file for reading, once the user decides to continue reading
		m_pending = true;
	else
		read();
}

/*
 * called periodically or on new data changes (file changed, new data in the socket, etc.)
 */
void LiveDataSource::read() {
	DEBUG("\nLiveDataSource::read()");
	if (!m_filter)
		return;

	if (m_reading)
		return;

	m_reading = true;

	//initialize the device (file, socket, serial port) when calling this function for the first time
	if (!m_prepared) {
		DEBUG("	Preparing device: update type = " << ENUM_TO_STRING(LiveDataSource, UpdateType, m_updateType));
		switch (m_sourceType) {
		case SourceType::FileOrPipe:
			delete m_device;
			m_device = new QFile(m_fileName);
			break;
		case SourceType::NetworkTcpSocket:
			m_tcpSocket = new QTcpSocket(this);
			m_device = m_tcpSocket;
			m_tcpSocket->connectToHost(m_host, m_port, QIODevice::ReadOnly);

			connect(m_tcpSocket, &QTcpSocket::readyRead, this, &LiveDataSource::readyRead);
			connect(m_tcpSocket, static_cast<void (QTcpSocket::*) (QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &LiveDataSource::tcpSocketError);

			break;
		case SourceType::NetworkUdpSocket:
			m_udpSocket = new QUdpSocket(this);
			m_device = m_udpSocket;
			m_udpSocket->bind(QHostAddress(m_host), m_port);
			m_udpSocket->connectToHost(m_host, 0, QUdpSocket::ReadOnly);

			// only connect to readyRead when update is on new data
			if (m_updateType == UpdateType::NewData)
				connect(m_udpSocket, &QUdpSocket::readyRead, this, &LiveDataSource::readyRead);
			connect(m_udpSocket, static_cast<void (QUdpSocket::*) (QAbstractSocket::SocketError)>(&QUdpSocket::error), this, &LiveDataSource::tcpSocketError);

			break;
		case SourceType::LocalSocket:
			m_localSocket = new QLocalSocket(this);
			m_device = m_localSocket;
			m_localSocket->connectToServer(m_localSocketName, QLocalSocket::ReadOnly);

			connect(m_localSocket, &QLocalSocket::readyRead, this, &LiveDataSource::readyRead);
			connect(m_localSocket, static_cast<void (QLocalSocket::*) (QLocalSocket::LocalSocketError)>(&QLocalSocket::error), this, &LiveDataSource::localSocketError);

			break;
		case SourceType::SerialPort:
#ifdef HAVE_QTSERIALPORT
			m_serialPort = new QSerialPort(this);
			m_device = m_serialPort;
			DEBUG("	Serial: " << STDSTRING(m_serialPortName) << ", " << m_baudRate);
			m_serialPort->setBaudRate(m_baudRate);
			m_serialPort->setPortName(m_serialPortName);
			m_serialPort->open(QIODevice::ReadOnly);

			// only connect to readyRead when update is on new data
			if (m_updateType == UpdateType::NewData)
				connect(m_serialPort, &QSerialPort::readyRead, this, &LiveDataSource::readyRead);
			connect(m_serialPort, static_cast<void (QSerialPort::*) (QSerialPort::SerialPortError)>(&QSerialPort::error), this, &LiveDataSource::serialPortError);
#endif
			break;
		case SourceType::MQTT:
			break;
		}
		m_prepared = true;
	}

	qint64 bytes = 0;

	switch (m_sourceType) {
	case SourceType::FileOrPipe:
		DEBUG("Reading FileOrPipe. type = " << ENUM_TO_STRING(AbstractFileFilter, FileType, m_fileType));
		switch (m_fileType) {
		case AbstractFileFilter::FileType::Ascii:
			if (m_readingType == LiveDataSource::ReadingType::WholeFile) {
				static_cast<AsciiFilter*>(m_filter)->readFromLiveDevice(*m_device, this, 0);
			} else {
				bytes = static_cast<AsciiFilter*>(m_filter)->readFromLiveDevice(*m_device, this, m_bytesRead);
				m_bytesRead += bytes;
				DEBUG("Read " << bytes << " bytes, in total: " << m_bytesRead);
			}
			break;
		case AbstractFileFilter::FileType::Binary:
			//TODO: not implemented yet
			// bytes = qSharedPointerCast<BinaryFilter>(m_filter)->readFromLiveDevice(*m_file, this, m_bytesRead);
// 			m_bytesRead += bytes;
		case AbstractFileFilter::FileType::ROOT:
		case AbstractFileFilter::FileType::NgspiceRawAscii:
		case AbstractFileFilter::FileType::NgspiceRawBinary:
			//only re-reading of the whole file is supported
			m_filter->readDataFromFile(m_fileName, this);
			break;
		//TODO: other types not implemented yet
		case AbstractFileFilter::FileType::Image:
		case AbstractFileFilter::FileType::HDF5:
		case AbstractFileFilter::FileType::NETCDF:
		case AbstractFileFilter::FileType::FITS:
		case AbstractFileFilter::FileType::JSON:
		case AbstractFileFilter::FileType::READSTAT:
		case AbstractFileFilter::FileType::MATIO:
			break;
		}
		break;
	case SourceType::NetworkTcpSocket:
		DEBUG("reading from TCP socket. state before abort = " << m_tcpSocket->state());
		m_tcpSocket->abort();
		m_tcpSocket->connectToHost(m_host, m_port, QIODevice::ReadOnly);
		DEBUG("reading from TCP socket. state after reconnect = " << m_tcpSocket->state());
		break;
	case SourceType::NetworkUdpSocket:
		DEBUG("	Reading from UDP socket. state = " << m_udpSocket->state());

		// reading data here
		if (m_fileType == AbstractFileFilter::FileType::Ascii)
			static_cast<AsciiFilter*>(m_filter)->readFromLiveDeviceNotFile(*m_device, this);
		break;
	case SourceType::LocalSocket:
		DEBUG("	Reading from local socket. state before abort = " << m_localSocket->state());
		if (m_localSocket->state() == QLocalSocket::ConnectingState)
			m_localSocket->abort();
		m_localSocket->connectToServer(m_localSocketName, QLocalSocket::ReadOnly);
		if (m_localSocket->waitForConnected())
			m_localSocket->waitForReadyRead();
		DEBUG("	Reading from local socket. state after reconnect = " << m_localSocket->state());
		break;
	case SourceType::SerialPort:
		DEBUG("	Reading from serial port");
#ifdef HAVE_QTSERIALPORT

		// reading data here
		if (m_fileType == AbstractFileFilter::FileType::Ascii)
			static_cast<AsciiFilter*>(m_filter)->readFromLiveDeviceNotFile(*m_device, this);
#endif
		break;
	case SourceType::MQTT:
		break;
	}

	m_reading = false;
}

/*!
 * Slot for the signal that is emitted once every time new data is available for reading from the device (not UDP or Serial).
 * It will only be emitted again once new data is available, such as when a new payload of network data has arrived on the network socket,
 * or when a new block of data has been appended to your device.
 */
void LiveDataSource::readyRead() {
	DEBUG("LiveDataSource::readyRead() update type = " << ENUM_TO_STRING(LiveDataSource,UpdateType,m_updateType));
	DEBUG("	REMAINING TIME = " << m_updateTimer->remainingTime());

	if (m_fileType == AbstractFileFilter::FileType::Ascii)
		static_cast<AsciiFilter*>(m_filter)->readFromLiveDeviceNotFile(*m_device, this);

//TODO: not implemented yet
//	else if (m_fileType == AbstractFileFilter::FileType::Binary)
//		dynamic_cast<BinaryFilter*>(m_filter)->readFromLiveDeviceNotFile(*m_device, this);

	//since we won't have the timer to call read() where we create new connections
	//for sequential devices in read() we just request data/connect to servers
	if (m_updateType == UpdateType::NewData)
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

#ifdef HAVE_QTSERIALPORT
void LiveDataSource::serialPortError(QSerialPort::SerialPortError serialPortError) {
	switch (serialPortError) {
	case QSerialPort::DeviceNotFoundError:
		QMessageBox::critical(nullptr, i18n("Serial Port Error"), i18n("Failed to open the device."));
		break;
	case QSerialPort::PermissionError:
		QMessageBox::critical(nullptr, i18n("Serial Port Error"),
			i18n("Failed to open the device. Please check your permissions on this device."));
		break;
	case QSerialPort::OpenError:
		QMessageBox::critical(nullptr, i18n("Serial Port Error"), i18n("Device already opened."));
		break;
	case QSerialPort::NotOpenError:
		QMessageBox::critical(nullptr, i18n("Serial Port Error"), i18n("The device is not opened."));
		break;
	case QSerialPort::ReadError:
		QMessageBox::critical(nullptr, i18n("Serial Port Error"), i18n("Failed to read data."));
		break;
	case QSerialPort::ResourceError:
		QMessageBox::critical(nullptr, i18n("Serial Port Error"), i18n("Failed to read data. The device is removed."));
		break;
	case QSerialPort::TimeoutError:
		QMessageBox::critical(nullptr, i18n("Serial Port Error"), i18n("The device timed out."));
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
		QMessageBox::critical(nullptr, i18n("Serial Port Error"),
			i18n("The following error occurred: %1.", m_serialPort->errorString()));
		break;
	case QSerialPort::NoError:
		break;
	}
}
#endif

void LiveDataSource::plotData() {
	auto* dlg = new PlotDataDialog(this);
	dlg->exec();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void LiveDataSource::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("liveDataSource");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement("general");

	switch (m_sourceType) {
	case SourceType::FileOrPipe:
		writer->writeAttribute("fileType", QString::number(static_cast<int>(m_fileType)));
		writer->writeAttribute("fileLinked", QString::number(m_fileLinked));
		writer->writeAttribute("relativePath", QString::number(m_relativePath));
		if (m_relativePath) {
			//convert from the absolute to the relative path and save it
			const Project* p = const_cast<LiveDataSource*>(this)->project();
			QFileInfo fi(p->fileName());
			writer->writeAttribute("fileName", fi.dir().relativeFilePath(m_fileName));
		}else
			writer->writeAttribute("fileName", m_fileName);

		break;
	case SourceType::SerialPort:
		writer->writeAttribute("baudRate", QString::number(m_baudRate));
		writer->writeAttribute("serialPortName", m_serialPortName);
		break;
	case SourceType::NetworkTcpSocket:
	case SourceType::NetworkUdpSocket:
		writer->writeAttribute("host", m_host);
		writer->writeAttribute("port", QString::number(m_port));
		break;
	case SourceType::LocalSocket:
		break;
	case SourceType::MQTT:
		break;
	}

	writer->writeAttribute("updateType", QString::number(static_cast<int>(m_updateType)));
	writer->writeAttribute("readingType", QString::number(static_cast<int>(m_readingType)));
	writer->writeAttribute("sourceType", QString::number(static_cast<int>(m_sourceType)));
	writer->writeAttribute("keepNValues", QString::number(m_keepNValues));

	if (m_updateType == UpdateType::TimeInterval)
		writer->writeAttribute("updateInterval", QString::number(m_updateInterval));

	if (m_readingType != ReadingType::TillEnd)
		writer->writeAttribute("sampleSize", QString::number(m_sampleSize));
	writer->writeEndElement(); //general

	//filter
	m_filter->save(writer);

	//columns
	if (!m_fileLinked) {
		for (auto* col : children<Column>(ChildIndexFlag::IncludeHidden))
			col->save(writer);
	}

	writer->writeEndElement(); // "liveDataSource"
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
		if (reader->isEndElement()
			&& (reader->name() == "liveDataSource" || reader->name() == "LiveDataSource")) //TODO: remove "LiveDataSources" in couple of releases
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == "general") {
			attribs = reader->attributes();

			str = attribs.value("fileName").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("fileName").toString());
			else
				m_fileName = str;

			str = attribs.value("fileType").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("fileType").toString());
			else
				m_fileType = (AbstractFileFilter::FileType)str.toInt();

			str = attribs.value("fileLinked").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("fileLinked").toString());
			else
				m_fileLinked = str.toInt();

			str = attribs.value("relativePath").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("relativePath").toString());
			else
				m_relativePath = str.toInt();

			str = attribs.value("updateType").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("updateType").toString());
			else
				m_updateType =  static_cast<UpdateType>(str.toInt());

			str = attribs.value("sourceType").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("sourceType").toString());
			else
				m_sourceType =  static_cast<SourceType>(str.toInt());

			str = attribs.value("readingType").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("readingType").toString());
			else
				m_readingType =  static_cast<ReadingType>(str.toInt());

			if (m_updateType == UpdateType::TimeInterval) {
				str = attribs.value("updateInterval").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("updateInterval").toString());
				else
					m_updateInterval = str.toInt();
			}

			if (m_readingType != ReadingType::TillEnd) {
				str = attribs.value("sampleSize").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("sampleSize").toString());
				else
					m_sampleSize = str.toInt();
			}

			switch (m_sourceType) {
			case SourceType::SerialPort:
				str = attribs.value("baudRate").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("baudRate").toString());
				else
					m_baudRate = str.toInt();

				str = attribs.value("serialPortName").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("serialPortName").toString());
				else
					m_serialPortName = str;

				break;
			case SourceType::NetworkTcpSocket:
			case SourceType::NetworkUdpSocket:
				str = attribs.value("host").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("host").toString());
				else
					m_host = str;

				str = attribs.value("port").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("port").toString());
				else
					m_host = str;
				break;
			case SourceType::MQTT:
				break;
			case SourceType::FileOrPipe:
				break;
			case SourceType::LocalSocket:
				break;
			}

		} else if (reader->name() == "asciiFilter") {
			setFilter(new AsciiFilter);
			if (!m_filter->load(reader))
				return false;
		} else if (reader->name() == "rootFilter") {
			setFilter(new ROOTFilter);
			if (!m_filter->load(reader))
				return false;
		} else if (reader->name() == "column") {
			Column* column = new Column(QString(), AbstractColumn::ColumnMode::Text);
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

	return !reader->hasError();
}

void LiveDataSource::finalizeLoad() {
	//convert from the relative path saved in the project file to the absolute file to work with
	if (m_relativePath) {
		QFileInfo fi(project()->fileName());
		m_fileName = fi.dir().absoluteFilePath(m_fileName);
	}

	//read the content of the file if it was only linked
	if (m_fileLinked && QFile::exists(m_fileName))
		this->read();

	//call setUpdateType() to start watching the file for changes, is required
	setUpdateType(m_updateType);
}
