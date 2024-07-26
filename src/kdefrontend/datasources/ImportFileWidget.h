/*
	File                 : ImportFileWidget.h
	Project              : LabPlot
	Description          : import file data widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2017 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
	SPDX-FileCopyrightText: 2009-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2018 Fabian Kristof <fkristofszabolcs@gmail.com>
	SPDX-FileCopyrightText: 2018-2019 Kovacs Ferencz <kferike98@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTFILEWIDGET_H
#define IMPORTFILEWIDGET_H

#include "backend/datasources/LiveDataSource.h"
#include "ui_importfilewidget.h"
#include <memory>

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
class MQTTSubscriptionWidget;
#endif

#include <QVector>

class AbstractFileFilter;
class AsciiOptionsWidget;
class BinaryOptionsWidget;
class CANOptionsWidget;
class XLSXOptionsWidget;
class FITSOptionsWidget;
class HDF5OptionsWidget;
class ImageOptionsWidget;
class JsonOptionsWidget;
class McapOptionsWidget;
class MatioOptionsWidget;
class NetCDFOptionsWidget;
class OdsOptionsWidget;
class ROOTOptionsWidget;
class TemplateHandler;
class ImportKaggleDatasetWidget;

class QTableWidget;
class QCompleter;
class QTimer;
class QTreeWidgetItem;

class KConfig;
class KUrlComboBox;

class ImportFileWidget : public QWidget {
	Q_OBJECT

public:
	static QString absolutePath(const QString& fileName);

	explicit ImportFileWidget(QWidget*, bool liveDataSource, const QString& fileName = QString(), bool embedded = false);
	~ImportFileWidget() override;

	void showOptions(bool);
	void saveSettings(LiveDataSource*) const;
	void loadSettings();
	AbstractFileFilter::FileType currentFileType() const;
	LiveDataSource::SourceType currentSourceType() const;
	AbstractFileFilter* currentFileFilter() const;
	QString fileName() const;
	QString dbcFileName() const;
	QString selectedObject() const;
	bool importValid() const;
	bool useFirstRowAsColNames() const; // use by XLSX and ODS

	const QStringList selectedXLSXRegionNames() const;
	const QStringList selectedOdsSheetNames() const;
	const QStringList selectedHDF5Names() const;
	const QStringList selectedNetCDFNames() const;
	const QStringList selectedFITSExtensions() const;
	const QStringList selectedROOTNames() const;
	const QStringList selectedMatioNames() const;
	//	const QStringList selectedVectorBLFNames() const;

	QString host() const;
	QString port() const;
	QString serialPort() const;
	int baudRate() const;

	friend class ImportKaggleDatasetWidget;

public Q_SLOTS:
	void dataContainerChanged(AbstractAspect*);

private:
	Ui::ImportFileWidget ui;
	void setMQTTVisible(bool);
	void updateContent(const QString&);
	void initOptionsWidget();
	void initSlots();
	QString fileInfoString(const QString&) const;
	void showJsonModel(bool);
	void enableFirstRowAsColNames(bool enable); // used by XLSX and Ods
	void updateHeaderOptions();

	std::unique_ptr<AsciiOptionsWidget> m_asciiOptionsWidget;
	std::unique_ptr<BinaryOptionsWidget> m_binaryOptionsWidget;
	std::unique_ptr<HDF5OptionsWidget> m_hdf5OptionsWidget;
	std::unique_ptr<ImageOptionsWidget> m_imageOptionsWidget;
	std::unique_ptr<OdsOptionsWidget> m_odsOptionsWidget;
	std::unique_ptr<XLSXOptionsWidget> m_xlsxOptionsWidget;
	std::unique_ptr<NetCDFOptionsWidget> m_netcdfOptionsWidget;
	std::unique_ptr<CANOptionsWidget> m_canOptionsWidget;
	std::unique_ptr<MatioOptionsWidget> m_matioOptionsWidget;
	std::unique_ptr<FITSOptionsWidget> m_fitsOptionsWidget;
	std::unique_ptr<JsonOptionsWidget> m_jsonOptionsWidget;
	std::unique_ptr<McapOptionsWidget> m_mcapOptionsWidget;

	std::unique_ptr<ROOTOptionsWidget> m_rootOptionsWidget;

	mutable std::unique_ptr<AbstractFileFilter> m_currentFilter;

	AbstractAspect* m_targetContainer{nullptr};
	QTableWidget* m_twPreview{nullptr};
	KUrlComboBox* m_cbFileName{nullptr};
	KUrlComboBox* m_cbDBCFileName{nullptr};
	const QString& m_fileName;
	const QString m_dbcFileName;
	bool m_liveDataSource;
	bool m_suppressRefresh{false};
	bool m_embedded{false};
	TemplateHandler* m_templateHandler{nullptr};
	bool mcapTopicsInitialized{false};

Q_SIGNALS:
	void enableImportToMatrix(bool enable);
	void fileNameChanged();
	void sourceTypeChanged();
	void hostChanged();
	void portChanged();
	void error(const QString&);
	void previewReady();

private Q_SLOTS:
	void fileNameChanged(const QString&);
	void fileTypeChanged(int = 0);

	void sourceTypeChanged(int);
	void updateTypeChanged(int);
	void readingTypeChanged(int);
	void firstRowAsColNamesChanged(bool checked);

	void hidePropertyWidgets();
	void filterChanged(int);
	void selectFile();
	void selectDBCFile();
	void showFileInfo();
	void refreshPreview();
	void updateStartRow(int);
	void enableDataPortionSelection(bool);
	void changeTopic();

	// save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

	friend class HDF5OptionsWidget; // to access refreshPreview()
	friend class MatioOptionsWidget; // to access refreshPreview() and others
	friend class NetCDFOptionsWidget; // to access refreshPreview() and others
	friend class FITSOptionsWidget;
	friend class JsonOptionsWidget;
	friend class McapOptionsWidget;
	friend class ROOTOptionsWidget; // to access refreshPreview() and others
	friend class OdsOptionsWidget; // to access refreshPreview()
	friend class XLSXOptionsWidget; // to access refreshPreview()

#ifdef HAVE_MQTT
private:
	QMqttClient* m_client{nullptr};
	QVector<QMqttSubscription*> m_mqttSubscriptions;
	QTimer* m_connectTimeoutTimer{nullptr};
	QMap<QMqttTopicName, QMqttMessage> m_lastMessage;
	QVector<QString> m_subscribedTopicNames;
	QVector<QString> m_addedTopics;
	QString m_configPath;
	bool m_initialisingMQTT{false};
	MQTTClient::MQTTWill m_willSettings;
	MQTTSubscriptionWidget* m_subscriptionWidget{nullptr};

	void disconnectMqttConnection();

public:
	void saveMQTTSettings(MQTTClient*) const;
	bool isMqttValid();

Q_SIGNALS:
	void newTopic(const QString&);
	void subscriptionsChanged();
	void checkFileType();
	void updateSubscriptionTree(const QVector<QString>&);
	void MQTTClearTopics();

private Q_SLOTS:
	void mqttConnectionChanged();
	void onMqttConnect();
	void subscribeTopic(const QString&, uint);
	void unsubscribeTopic(const QString&, QVector<QTreeWidgetItem*>);
	void mqttMessageReceived(const QByteArray&, const QMqttTopicName&);
	void mqttSubscriptionMessageReceived(const QMqttMessage&);
	void onMqttDisconnect();
	void mqttErrorChanged(QMqttClient::ClientError);
	void mqttConnectTimeout();
	void showMQTTConnectionManager();
	void readMQTTConnections();
	void showWillSettings();
	void enableWill(bool);
#endif
};

#endif
