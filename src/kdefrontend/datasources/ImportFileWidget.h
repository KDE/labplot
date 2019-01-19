/***************************************************************************
    File                 : ImportFileWidget.h
    Project              : LabPlot
    Description          : import file data widget
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2017 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
    Copyright            : (C) 2009-2019 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017-2018 Fabian Kristof (fkristofszabolcs@gmail.com)

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
#ifndef IMPORTFILEWIDGET_H
#define IMPORTFILEWIDGET_H

#include "ui_importfilewidget.h"
#include "backend/datasources/LiveDataSource.h"
#include <memory>

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif

#include <QVector>

class AbstractFileFilter;
class AsciiOptionsWidget;
class BinaryOptionsWidget;
class HDF5OptionsWidget;
class ImageOptionsWidget;
class NetCDFOptionsWidget;
class FITSOptionsWidget;
class JsonOptionsWidget;
class ROOTOptionsWidget;
class QTableWidget;
class QCompleter;
class QTimer;
class QTreeWidgetItem;
class QStringList;

class ImportFileWidget : public QWidget {
	Q_OBJECT

public:
	explicit ImportFileWidget(QWidget*, bool liveDataSource, const QString& fileName = QString());
	~ImportFileWidget() override;

	void showOptions(bool);
	void saveSettings(LiveDataSource*) const;
	void loadSettings();
	AbstractFileFilter::FileType currentFileType() const;
	LiveDataSource::SourceType currentSourceType() const;
	AbstractFileFilter* currentFileFilter() const;
	QString fileName() const;
	QString selectedObject() const;
	bool isFileEmpty() const;
	const QStringList selectedHDF5Names() const;
	const QStringList selectedNetCDFNames() const;
	const QStringList selectedFITSExtensions() const;
	const QStringList selectedROOTNames() const;
	void showAsciiHeaderOptions(bool);
	void showJsonModel(bool);

	QString host() const;
	QString port() const;
	QString serialPort() const;
	int baudRate() const;

private:
	Ui::ImportFileWidget ui;
	void setMQTTVisible(bool);
	void updateContent(const QString&);
	void initOptionsWidget();
	void initSlots();

	std::unique_ptr<AsciiOptionsWidget> m_asciiOptionsWidget;
	std::unique_ptr<BinaryOptionsWidget> m_binaryOptionsWidget;
	std::unique_ptr<HDF5OptionsWidget> m_hdf5OptionsWidget;
	std::unique_ptr<ImageOptionsWidget> m_imageOptionsWidget;
	std::unique_ptr<NetCDFOptionsWidget> m_netcdfOptionsWidget;
	std::unique_ptr<FITSOptionsWidget> m_fitsOptionsWidget;
	std::unique_ptr<JsonOptionsWidget> m_jsonOptionsWidget;
	std::unique_ptr<ROOTOptionsWidget> m_rootOptionsWidget;

	mutable std::unique_ptr<AbstractFileFilter> m_currentFilter;

	QTableWidget* m_twPreview;
	const QString& m_fileName;
	bool m_fileEmpty{false};
	bool m_liveDataSource;
	bool m_suppressRefresh{false};

private slots:
	void fileNameChanged(const QString&);
	void fileTypeChanged(int);

	void updateTypeChanged(int);
	void sourceTypeChanged(int);
	void readingTypeChanged(int);

	void saveFilter();
	void manageFilters();
	void filterChanged(int);
	void selectFile();
	void fileInfoDialog();
	void refreshPreview();

signals:
	void fileNameChanged();
	void sourceTypeChanged();
	void hostChanged();
	void portChanged();
	void previewRefreshed();
	void checkedFitsTableToMatrix(const bool enable);

	friend class HDF5OptionsWidget;	// to access refreshPreview()
	friend class NetCDFOptionsWidget;	// to access refreshPreview() and others
	friend class FITSOptionsWidget;
	friend class JsonOptionsWidget;
	friend class ROOTOptionsWidget;	// to access refreshPreview() and others

#ifdef HAVE_MQTT
private:
	void updateSubscriptionCompleter();
	void unsubscribeFromTopic(const QString&);
	void manageCommonLevelSubscriptions();
	void updateSubscriptionTree();

	QMqttClient* m_client{nullptr};
	QVector<QMqttSubscription*> m_mqttSubscriptions;
	QCompleter* m_topicCompleter{nullptr};
	QCompleter* m_subscriptionCompleter{nullptr};
	QStringList m_topicList;
	bool m_searching{false};
	QTimer* m_searchTimer;
	QTimer* m_connectTimeoutTimer;
	QMap<QMqttTopicName, bool> m_messageArrived;
	QMap<QMqttTopicName, QMqttMessage> m_lastMessage;
	bool m_mqttReadyForPreview{false};
	QVector<QString> m_subscribedTopicNames;
	QVector<QString> m_addedTopics;
	QString m_configPath;
	bool m_initialisingMQTT{false};
	bool m_connectionTimedOut{false};
	MQTTClient::MQTTWill m_willSettings;
public:
	void saveMQTTSettings(MQTTClient*) const;
	bool isMqttValid();

signals:
	void newTopic(QString);
	void subscriptionsChanged();
	void checkFileType();

private slots:
	void mqttConnectionChanged();
	void onMqttConnect();
	void mqttAvailableTopicDoubleClicked(QTreeWidgetItem*, int);
	void mqttSubscribedTopicDoubleClicked(QTreeWidgetItem*, int);
	void mqttSubscribe();
	void mqttUnsubscribe();
	void mqttMessageReceived(const QByteArray&, const QMqttTopicName&);
	void setTopicCompleter(const QString&);
	void topicTimeout();
	void mqttSubscriptionMessageReceived(const QMqttMessage& );
	void onMqttDisconnect();
	void mqttErrorChanged(QMqttClient::ClientError);
	void scrollToTopicTreeItem(const QString&);
	void scrollToSubsriptionTreeItem(const QString&);
	void mqttConnectTimeout();
	void showMQTTConnectionManager();
	void readMQTTConnections();
	void showWillSettings();
#endif
};

#endif
