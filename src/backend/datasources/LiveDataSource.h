/***************************************************************************
    File                 : LiveDataSource.h
    Project              : LabPlot
    Description          : File data source
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Fabian Kristof (fkristofszabolcs@gmail.com)
    Copyright            : (C) 2017-2018 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include <QLocalSocket>
#include <QSerialPort>
#include <QTimer>
#include <QVector>
#include <QMap>

class QString;
class AbstractFileFilter;
class QFileSystemWatcher;
class QAction;
class QTcpSocket;
class QUdpSocket;
class QFile;

class LiveDataSource : public Spreadsheet {
	Q_OBJECT
	Q_ENUMS(SourceType)
	Q_ENUMS(UpdateType)
	Q_ENUMS(ReadingType)

public:
	enum SourceType {
		FileOrPipe = 0,		// regular file or pipe
		NetworkTcpSocket,	// TCP socket
		NetworkUdpSocket,	// UDP socket
		LocalSocket,		// local socket
		SerialPort,		// serial port
		MQTT
	};

	enum UpdateType {
		TimeInterval = 0,	// update periodically using given interval
		NewData			// update when new data is available
	};

	enum ReadingType {
		ContinuousFixed = 0,	// read continuously sampleSize number of samples (lines)
		FromEnd,		// read sampleSize number of samples (lines) from end
		TillEnd,		// read until the end
		WholeFile		// reread whole file
	};

	explicit LiveDataSource(const QString& name, bool loading = false);
	~LiveDataSource() override;

	static QStringList supportedBaudRates();
	static QStringList availablePorts();

	void setFileType(const AbstractFileFilter::FileType);
	AbstractFileFilter::FileType fileType() const;

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

	void setKeepNValues(int);
	int keepNValues() const;

	void setKeepLastValues(bool);
	bool keepLastValues() const;

	void setFileLinked(bool);
	bool isFileLinked() const;

	void setUseRelativePath(bool);
	bool useRelativePath() const;

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

	QString m_fileName;
	QString m_serialPortName;
	QString m_localSocketName;
	QString m_host;

	AbstractFileFilter::FileType m_fileType{AbstractFileFilter::Ascii};
	UpdateType m_updateType;
	SourceType m_sourceType;
	ReadingType m_readingType;

	bool m_fileWatched{false};
	bool m_fileLinked{false};
	bool m_relativePath{false};
	bool m_paused{false};
	bool m_prepared{false};
	bool m_reading{false};

	int m_sampleSize{1};
	int m_keepNValues{0};	// number of values to keep (0 - all)
	int m_updateInterval{1000};
	quint16 m_port{1027};
	int m_baudRate{9600};

	qint64 m_bytesRead{0};

	AbstractFileFilter* m_filter{nullptr};

	QTimer* m_updateTimer;
	QFileSystemWatcher* m_fileSystemWatcher{nullptr};

	QFile* m_file{nullptr};
	QLocalSocket* m_localSocket{nullptr};
	QTcpSocket* m_tcpSocket{nullptr};
	QUdpSocket* m_udpSocket{nullptr};
	QSerialPort* m_serialPort{nullptr};
	QIODevice* m_device{nullptr};
	QAction* m_plotDataAction{nullptr};

public slots:
	void read();

private slots:
	void linkToggled();
	void plotData();

	void readyRead();

	void localSocketError(QLocalSocket::LocalSocketError);
	void tcpSocketError(QAbstractSocket::SocketError);
	void serialPortError(QSerialPort::SerialPortError);
};

#endif
