/*
    File                 : LiveDataSource.h
    Project              : LabPlot
    Description          : File data source
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Fabian Kristof (fkristofszabolcs@gmail.com)
    SPDX-FileCopyrightText: 2017-2018 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LIVEDATASOURCE_H
#define LIVEDATASOURCE_H

#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"

#include <QLocalSocket>
#include <QTimer>
#include <QVector>
#include <QMap>
#ifdef HAVE_QTSERIALPORT
#include <QSerialPort>
#endif

class QString;
class AbstractFileFilter;
class QFileSystemWatcher;
class QAction;
class QTcpSocket;
class QUdpSocket;

class LiveDataSource : public Spreadsheet {
	Q_OBJECT
	Q_ENUMS(SourceType)
	Q_ENUMS(UpdateType)
	Q_ENUMS(ReadingType)

public:
	enum class SourceType {
		FileOrPipe = 0,		// regular file or pipe
		NetworkTcpSocket,	// TCP socket
		NetworkUdpSocket,	// UDP socket
		LocalSocket,		// local socket
		SerialPort,		// serial port
		MQTT
	};

	enum class UpdateType {
		TimeInterval = 0,	// update periodically using given interval
		NewData			// update when new data is available
	};

	enum class ReadingType {
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
	void finalizeLoad();

private:
	void initActions();

	QString m_fileName;
	QString m_dirName;
	QString m_serialPortName;
	QString m_localSocketName;
	QString m_host;

	AbstractFileFilter::FileType m_fileType{AbstractFileFilter::FileType::Ascii};
	UpdateType m_updateType;
	SourceType m_sourceType;
	ReadingType m_readingType;

	bool m_fileWatched{false};
	bool m_fileLinked{false};
	bool m_relativePath{false};
	bool m_paused{false};
	bool m_prepared{false};
	bool m_reading{false};
	bool m_pending{false};

	int m_sampleSize{1};
	int m_keepNValues{0};	// number of values to keep (0 - all)
	int m_updateInterval{1000};
	quint16 m_port{1027};
	int m_baudRate{9600};

	qint64 m_bytesRead{0};

	AbstractFileFilter* m_filter{nullptr};

	QTimer* m_updateTimer;
	QTimer* m_watchTimer;
	QFileSystemWatcher* m_fileSystemWatcher{nullptr};

	QLocalSocket* m_localSocket{nullptr};
	QTcpSocket* m_tcpSocket{nullptr};
	QUdpSocket* m_udpSocket{nullptr};
#ifdef HAVE_QTSERIALPORT
	QSerialPort* m_serialPort{nullptr};
#endif
	QIODevice* m_device{nullptr};
	QAction* m_plotDataAction{nullptr};

public slots:
	void read();
	void readOnUpdate();

private slots:
	void plotData();
	void readyRead();

	void localSocketError(QLocalSocket::LocalSocketError);
	void tcpSocketError(QAbstractSocket::SocketError);
#ifdef HAVE_QTSERIALPORT
	void serialPortError(QSerialPort::SerialPortError);
#endif
};

#endif
