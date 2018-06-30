/***************************************************************************
    File                 : LiveDataSource.h
    Project              : LabPlot
    Description          : File data source
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2013 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017 Fabian Kristof (fkristofszabolcs@gmail.com)

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
#ifndef LIVEDATASOURCE_H
#define LIVEDATASOURCE_H

#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"

#include <QSerialPort>
#include <QtNetwork/QLocalSocket>
#include <QTimer>

class QString;
class AbstractFileFilter;
class QFileSystemWatcher;
class QAction;
class QTcpSocket;
class QUdpSocket;
class QFile;

class LiveDataSource : public Spreadsheet {
	Q_OBJECT
	Q_ENUMS(FileType)

public:
	enum FileType {Ascii, Binary, Image, HDF5, NETCDF, FITS, ROOT};
	enum SourceType {
		FileOrPipe = 0,		// regular file or pipe
		NetworkTcpSocket,	// TCP socket
		NetworkUdpSocket,	// UDP socket
		LocalSocket,		// local socket
		SerialPort		// serial port
	};

	enum UpdateType {
		TimeInterval = 0,	// update periodically using given interval
		NewData			// update when new data is available
	};

	enum ReadingType {
		ContinuousFixed = 0,	// read fixed number of samples (aka lines) using given sample size
		FromEnd,		// ?
		TillEnd,		// read until the end
		WholeFile		// reread whole file
	};

	LiveDataSource(AbstractScriptingEngine*, const QString& name, bool loading = false);
	~LiveDataSource() override;

	void ready();

	static QStringList supportedBaudRates();
	static QStringList availablePorts();

	static QStringList fileTypes();
	static QString fileInfoString(const QString&);

	void setFileType(const FileType);
	FileType fileType() const;

	UpdateType updateType() const;
	void setUpdateType(UpdateType);

	SourceType sourceType() const;
	void setSourceType(SourceType);

	ReadingType readingType() const;
	void setReadingType(ReadingType);

	int sampleSize() const;
	void setSampleSize(int);

	void setBytesRead(qint64 bytes);
	int bytesRead() const;

	int port() const;
	void setPort(quint16);

	bool isPaused() const;

	void setSerialPort(const QString& name);
	QString serialPortName() const;

	QString host() const;
	void setHost(const QString&);

	int baudRate() const;
	void setBaudRate(int);

	void setUpdateInterval(int);
	int updateInterval() const;

	void setKeepNvalues(int);
	int keepNvalues() const;

	void setKeepLastValues(bool);
	bool keepLastValues() const;

	void setFileWatched(bool);
	bool isFileWatched() const;

	void setFileLinked(bool);
	bool isFileLinked() const;

	void setFileName(const QString&);
	QString fileName() const;

	void setLocalSocketName(const QString&);
	QString localSocketName() const;

	void updateNow();
	void pauseReading();
	void continueReading();

	void setFilter(AbstractFileFilter*);
	AbstractFileFilter* filter() const;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

private:
	void initActions();
	void watch();

	QString m_fileName;
	QString m_serialPortName;
	QString m_localSocketName;
	QString m_host;

	FileType m_fileType;
	UpdateType m_updateType;
	SourceType m_sourceType;
	ReadingType m_readingType;

	bool m_fileWatched;
	bool m_fileLinked;
	bool m_paused;
	bool m_prepared;
	bool m_keepLastValues;

	int m_sampleSize;
	int m_keepNvalues;
	int m_updateInterval;
	quint16 m_port;
	int m_baudRate;

	qint64 m_bytesRead;

	AbstractFileFilter* m_filter;

	QTimer* m_updateTimer;
	QFileSystemWatcher* m_fileSystemWatcher;

	QFile* m_file;
	QLocalSocket* m_localSocket;
	QTcpSocket* m_tcpSocket;
	QUdpSocket* m_udpSocket;
	QSerialPort* m_serialPort;
	QIODevice* m_device;

	QAction* m_reloadAction;
	QAction* m_toggleLinkAction;
	QAction* m_showEditorAction;
	QAction* m_showSpreadsheetAction;
	QAction* m_plotDataAction;

public slots:
	void read();

private slots:
	void watchToggled();
	void linkToggled();
	void plotData();

	void readyRead();

	void localSocketError(QLocalSocket::LocalSocketError);
	void tcpSocketError(QAbstractSocket::SocketError);
	void serialPortError(QSerialPort::SerialPortError);
};

#endif
