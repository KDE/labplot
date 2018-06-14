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
#include <QVector>

#ifdef HAVE_MQTT
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttTopicName>
#endif

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
	Q_ENUMS(FileType)

public:
	enum FileType {Ascii, Binary, Image, HDF5, NETCDF, FITS};
	enum SourceType {
		FileOrPipe = 0,
		NetworkTcpSocket,
		NetworkUdpSocket,
		LocalSocket,
        SerialPort,
        Mqtt
	};

	enum UpdateType {
		TimeInterval = 0,
		NewData
	};

	enum ReadingType {
		ContinousFixed = 0,
		FromEnd,
        TillEnd,
        WholeFile
	};

#ifdef HAVE_MQTT
	enum WillMessageType {
		OwnMessage = 0,
		Statistics,
		LastMessage
	};

	enum WillUpdateType {
		TimePeriod = 0,
		OnClick
	};

	enum WillStatistics {
		Minimum = 0,
		Maximum,
		ArithmeticMean,
		GeometricMean,
		HarmonicMean,
		ContraharmonicMean,
		Median,
		Variance,
		StandardDeviation,
		MeanDeviation,
		MeanDeviationAroundMedian,
		MedianDeviation,
		Skewness,
		Kurtosis,
		Entropy
	};
#endif


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

	int sampleRate() const;
	void setSampleRate(int);

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

#ifdef HAVE_MQTT
	void setMqttClient(const QString&, const quint16&);
	void setMqttClientAuthentication(const QString&, const QString&);
	void setMqttClientId(const QString&);
	QMqttClient mqttClient() const;

	void addMqttSubscriptions(const QMqttTopicFilter&, const quint8&);
	QVector<QMqttTopicName> mqttSubscribtions() const;
#endif

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

#ifdef HAVE_MQTT
	int topicNumber();
	int topicIndex(const QString&);
	QVector<QString> topicVector() const;
	bool checkAllArrived();

	void setMqttWillUse(bool);
	bool mqttWillUse() const;

	void setWillTopic(const QString&);
	QString willTopic() const;

	void setWillRetain(bool);
	bool willRetain() const;

	void setWillQoS(quint8);
	quint8 willQoS() const;

	void setWillMessageType(WillMessageType);
	WillMessageType willMessageType() const;

	void setWillOwnMessage(const QString&);
	QString willOwnMessage() const;

	WillUpdateType willUpdateType() const;
	void setWillUpdateType(WillUpdateType);

	int willTimeInterval() const;
	void setWillTimeInterval(int);

	void startWillTimer() const;
	void stopWillTimer() const;

	void setWillForMqtt() ;

	void setMqttRetain(bool);
	bool mqttRetain() const;

	void clearLastMessage();
	void addWillStatistics(WillStatistics);
	void removeWillStatistics(WillStatistics);
	QVector<bool> willStatistics() const;
#endif

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

	int m_sampleRate;
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

#ifdef HAVE_MQTT
	QMqttClient* m_client;
	QMap<QMqttTopicFilter, quint8> m_topicMap;
	QMap<QMqttTopicName, bool> m_messageArrived;
	QMap<QMqttTopicName, QVector<QMqttMessage>> m_messagePuffer;
	QVector<QString> m_subscriptions;
	bool m_mqttTest;
	bool m_mqttUseWill;
	QString m_willMessage;
	QString m_willTopic;
	bool m_willRetain;
	quint8 m_willQoS;
	WillMessageType m_willMessageType;
	QString m_willOwnMessage;
	QString m_willLastMessage;
	QTimer* m_willTimer;
	int m_willTimeInterval;
	WillUpdateType m_willUpdateType;
	QVector<bool> m_willStatistics;
	bool m_mqttFirstConnectEstablished;
	bool m_mqttRetain;
#endif

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

#ifdef HAVE_MQTT
	void onMqttConnect();
	void mqttSubscribtionMessageReceived(const QMqttMessage&);
	void onAllArrived();
	void mqttErrorChanged(QMqttClient::ClientError);
#endif

signals:

#ifdef HAVE_MQTT
	void mqttAllArrived();
	void mqttSubscribed();
#endif

};

#endif
