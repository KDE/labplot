/***************************************************************************
    File                 : ImportFileWidget.h
    Project              : LabPlot
    Description          : import file data widget
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2017 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
    Copyright            : (C) 2009-2015 Alexander Semke (alexander.semke@web.de)
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
#ifndef IMPORTFILEWIDGET_H
#define IMPORTFILEWIDGET_H

#include "ui_importfilewidget.h"
#include "backend/datasources/LiveDataSource.h"
#include <memory>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/qmqttclient.h>
#include <QtMqtt/QMqttSubscription>
#include <QtMqtt/qmqttsubscription.h>
#include <QtMqtt/QMqttTopicName>
#include <QtMqtt/QMqttTopicFilter>
#include <QVector>
#include <QCompleter>
#include <QStringList>
#include <QTimer>

class AbstractFileFilter;
class AsciiOptionsWidget;
class BinaryOptionsWidget;
class HDF5OptionsWidget;
class ImageOptionsWidget;
class NetCDFOptionsWidget;
class FITSOptionsWidget;
class QTableWidget;

class ImportFileWidget : public QWidget {
	Q_OBJECT

public:
	explicit ImportFileWidget(QWidget*, const QString& fileName = QString());
	~ImportFileWidget();

	void showOptions(bool);
	void saveSettings(LiveDataSource*) const;
	LiveDataSource::FileType currentFileType() const;
	LiveDataSource::SourceType currentSourceType() const;
	AbstractFileFilter* currentFileFilter() const;
	QString fileName() const;
	QString selectedObject() const;
	bool isFileEmpty() const;
	const QStringList selectedHDF5Names() const;
	const QStringList selectedNetCDFNames() const;
	const QStringList selectedFITSExtensions() const;
	void hideDataSource();
	void showAsciiHeaderOptions(bool);

	QString host() const;
	QString port() const;
    QString serialPort() const;
    int baudRate() const;
	void initializeAndFillPortsAndBaudRates();
    bool isMqttValid();
private:
	Ui::ImportFileWidget ui;

	std::unique_ptr<AsciiOptionsWidget> m_asciiOptionsWidget;
	std::unique_ptr<BinaryOptionsWidget> m_binaryOptionsWidget;
	std::unique_ptr<HDF5OptionsWidget> m_hdf5OptionsWidget;
	std::unique_ptr<ImageOptionsWidget> m_imageOptionsWidget;
	std::unique_ptr<NetCDFOptionsWidget> m_netcdfOptionsWidget;
	std::unique_ptr<FITSOptionsWidget> m_fitsOptionsWidget;
	QTableWidget* m_twPreview;
	const QString& m_fileName;
	bool m_fileEmpty;
	bool m_liveDataSource;
    bool m_suppressRefresh;
    QMqttClient *m_client;
    QMqttSubscription *m_mainSubscription;
    QMqttTopicFilter *m_filter;
    QVector <QMqttSubscription*> m_mqttSubscriptions;
    QCompleter *m_completer;
    QStringList m_topicList;
    bool m_editing;
	QTimer *m_timer;
	QMap<QMqttTopicName, bool> m_messageArrived;
	QMap<QMqttTopicName, QMqttMessage> m_lastMessage;
	bool m_mqttReadyForPreview;
	QString m_mqttNewTopic;


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
	void loadSettings();
    void idChecked(int);
    void authenticationChecked(int);
    void mqttConnection();
    void onMqttConnect();
    void mqttSubscribe();
    void mqttMessageReceived(const QByteArray&, const QMqttTopicName&);
    void setCompleter(QString);
    void topicBeingTyped(const QString);
    void topicTimeout();
	void mqttSubscriptionMessageReceived(const QMqttMessage& );

signals:
	void fileNameChanged();
	void sourceTypeChanged();
	void hostChanged();
	void portChanged();
    void newTopic(QString);
    void subscriptionMade();

	void checkedFitsTableToMatrix(const bool enable);

	friend class HDF5OptionsWidget;	// to access refreshPreview()
	friend class NetCDFOptionsWidget;	// to access refreshPreview() and others
	friend class FITSOptionsWidget;
};

#endif
