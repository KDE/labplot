/***************************************************************************
File                 : ImportFileWidget.cpp
Project              : LabPlot
Description          : import file data widget
--------------------------------------------------------------------
Copyright            : (C) 2009-2018 Stefan Gerlach (stefan.gerlach@uni.kn)
Copyright            : (C) 2009-2017 Alexander Semke (alexander.semke@web.de)
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

#include "ImportFileWidget.h"
#include "FileInfoDialog.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/filters/BinaryFilter.h"
#include "backend/datasources/filters/HDF5Filter.h"
#include "backend/datasources/filters/NetCDFFilter.h"
#include "backend/datasources/filters/ImageFilter.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/datasources/filters/JsonFilter.h"
#include "backend/datasources/filters/QJsonModel.h"
#include "backend/datasources/filters/NgspiceRawAsciiFilter.h"
#include "backend/datasources/filters/ROOTFilter.h"
#include "AsciiOptionsWidget.h"
#include "BinaryOptionsWidget.h"
#include "HDF5OptionsWidget.h"
#include "ImageOptionsWidget.h"
#include "NetCDFOptionsWidget.h"
#include "FITSOptionsWidget.h"
#include "JsonOptionsWidget.h"
#include "ROOTOptionsWidget.h"

#include <QCompleter>
#include <QDir>
#include <QDirModel>
#include <QFileDialog>
#include <QImageReader>
#include <QInputDialog>
#include <QIntValidator>
#include <QLocalSocket>
#include <QProcess>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>
#include <QCheckBox>
#include <QTreeWidgetItem>
#include <QStringList>
#include <QBuffer>


#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QtCore/QJsonDocument>
#include <QTreeWidgetItem>
#include <QTimer>
#include <QCompleter>

#ifdef HAVE_MQTT
#include "backend/core/Project.h"
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttSubscription>
#include <QtMqtt/QMqttTopicFilter>
#include <QtMqtt/QMqttMessage>
#include <QMessageBox>
#endif


/*!
   \class ImportFileWidget
   \brief Widget for importing data from a file.

   \ingroup kdefrontend
*/
ImportFileWidget::ImportFileWidget(QWidget* parent, const QString& fileName) : QWidget(parent),
	m_fileName(fileName),
	m_fileEmpty(false),
	m_liveDataSource(true),   
#ifdef HAVE_MQTT
	m_mqttReadyForPreview (false),
	m_searching(false),
#endif
	m_suppressRefresh(false) {
	ui.setupUi(this);

#ifdef HAVE_MQTT
	m_searchTimer = new QTimer(this);
	m_searchTimer->setInterval(10000);
	m_timeoutTimer = new QTimer(this);
	m_timeoutTimer->setInterval(5000);
#endif

	QCompleter* completer = new QCompleter(this);
	completer->setModel(new QDirModel);
	ui.leFileName->setCompleter(completer);

	ui.cbFileType->addItems(AbstractFileFilter::fileTypes());
	QStringList filterItems;
	filterItems << i18n("Automatic") << i18n("Custom");
	ui.cbFilter->addItems(filterItems);

	// file type specific option widgets
	QWidget* asciiw = new QWidget();
	m_asciiOptionsWidget = std::unique_ptr<AsciiOptionsWidget>(new AsciiOptionsWidget(asciiw));
	ui.swOptions->insertWidget(AbstractFileFilter::Ascii, asciiw);

	QWidget* binaryw = new QWidget();
	m_binaryOptionsWidget = std::unique_ptr<BinaryOptionsWidget>(new BinaryOptionsWidget(binaryw));
	ui.swOptions->insertWidget(AbstractFileFilter::Binary, binaryw);

	QWidget* imagew = new QWidget();
	m_imageOptionsWidget = std::unique_ptr<ImageOptionsWidget>(new ImageOptionsWidget(imagew));
	ui.swOptions->insertWidget(AbstractFileFilter::Image, imagew);

	QWidget* hdf5w = new QWidget();
	m_hdf5OptionsWidget = std::unique_ptr<HDF5OptionsWidget>(new HDF5OptionsWidget(hdf5w, this));
	ui.swOptions->insertWidget(AbstractFileFilter::HDF5, hdf5w);

	QWidget* netcdfw = new QWidget();
	m_netcdfOptionsWidget = std::unique_ptr<NetCDFOptionsWidget>(new NetCDFOptionsWidget(netcdfw, this));
	ui.swOptions->insertWidget(AbstractFileFilter::NETCDF, netcdfw);

	QWidget* fitsw = new QWidget();
	m_fitsOptionsWidget = std::unique_ptr<FITSOptionsWidget>(new FITSOptionsWidget(fitsw, this));
	ui.swOptions->insertWidget(AbstractFileFilter::FITS, fitsw);

	QWidget* rootw = new QWidget();
	m_rootOptionsWidget = std::unique_ptr<ROOTOptionsWidget>(new ROOTOptionsWidget(rootw, this));
	ui.swOptions->insertWidget(AbstractFileFilter::ROOT, rootw);

	QWidget* jsonw = new QWidget();
	m_jsonOptionsWidget = std::unique_ptr<JsonOptionsWidget>(new JsonOptionsWidget(jsonw, this));
	ui.swOptions->insertWidget(AbstractFileFilter::Json, jsonw);

	ui.tvJson->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tvJson->setAlternatingRowColors(true);
	ui.tvJson->setModel(m_jsonOptionsWidget->model());
	showJsonModel(false);

	// the table widget for preview
	m_twPreview = new QTableWidget(ui.tePreview);
	m_twPreview->verticalHeader()->hide();
	m_twPreview->setEditTriggers(QTableWidget::NoEditTriggers);
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(m_twPreview);
	ui.tePreview->setLayout(layout);
	m_twPreview->hide();

	// default filter
	ui.swOptions->setCurrentIndex(AbstractFileFilter::Ascii);
#if !defined(HAVE_HDF5) || !defined(HAVE_NETCDF) || !defined(HAVE_FITS) || !defined(HAVE_ZIP)
	const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(ui.cbFileType->model());
#endif
#ifndef HAVE_HDF5
	// disable HDF5 item
	QStandardItem* item = model->item(AbstractFileFilter::HDF5);
	item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
#endif
#ifndef HAVE_NETCDF
	// disable NETCDF item
	QStandardItem* item2 = model->item(AbstractFileFilter::NETCDF);
	item2->setFlags(item2->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
#endif
#ifndef HAVE_FITS
	// disable FITS item
	QStandardItem* item3 = model->item(AbstractFileFilter::FITS);
	item3->setFlags(item3->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
#endif
#ifndef HAVE_ZIP
	// disable ROOT item
	QStandardItem* item4 = model->item(AbstractFileFilter::ROOT);
	item4->setFlags(item4->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
#endif
#ifndef HAVE_MQTT
	// disable MQTT item
	QStandardItem* item5 = model->item(LiveDataSource::MQTT);
	item5->setFlags(item5->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
#endif

	ui.cbReadingType->addItem(i18n("Whole file"), LiveDataSource::WholeFile);

	ui.lePort->setValidator( new QIntValidator(ui.lePort) );
	ui.gbOptions->hide();
	ui.gbUpdateOptions->hide();

	ui.bOpen->setIcon( QIcon::fromTheme("document-open") );
	ui.bFileInfo->setIcon( QIcon::fromTheme("help-about") );
	ui.bManageFilters->setIcon( QIcon::fromTheme("configure") );
	ui.bSaveFilter->setIcon( QIcon::fromTheme("document-save") );
	ui.bRefreshPreview->setIcon( QIcon::fromTheme("view-refresh") );
#ifdef HAVE_MQTT
    m_client = new QMqttClient(this);
#endif

	connect( ui.leFileName, SIGNAL(textChanged(QString)), SLOT(fileNameChanged(QString)) );
	connect( ui.bOpen, SIGNAL(clicked()), this, SLOT (selectFile()) );
	connect( ui.bFileInfo, SIGNAL(clicked()), this, SLOT (fileInfoDialog()) );
	connect( ui.bSaveFilter, SIGNAL(clicked()), this, SLOT (saveFilter()) );
	connect( ui.bManageFilters, SIGNAL(clicked()), this, SLOT (manageFilters()) );
	connect( ui.cbFileType, SIGNAL(currentIndexChanged(int)), SLOT(fileTypeChanged(int)) );
	connect( ui.cbUpdateType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTypeChanged(int)));
	connect( ui.cbReadingType, SIGNAL(currentIndexChanged(int)), this, SLOT(readingTypeChanged(int)));
	connect( ui.cbFilter, SIGNAL(activated(int)), SLOT(filterChanged(int)) );
	connect( ui.bRefreshPreview, SIGNAL(clicked()), SLOT(refreshPreview()) );
#ifdef HAVE_MQTT
	connect(ui.chbID, SIGNAL(stateChanged(int)), this, SLOT(idChecked(int)));
	connect(ui.chbAuthentication, SIGNAL(stateChanged(int)), this, SLOT(authenticationChecked(int)));
	connect(ui.bConnect, SIGNAL(clicked()), this, SLOT(mqttConnection()) );
	connect(m_client, SIGNAL(connected()), this, SLOT(onMqttConnect()) );
	connect(ui.bSubscribe,  &QPushButton::clicked, this, &ImportFileWidget::mqttSubscribe);
	connect(ui.bUnsubscribe, &QPushButton::clicked, this,&ImportFileWidget::mqttUnsubscribe);
	connect(m_client, SIGNAL(messageReceived(QByteArray, QMqttTopicName)), this, SLOT(mqttMessageReceived(QByteArray, QMqttTopicName)) );
	connect(this, &ImportFileWidget::newTopic, this, &ImportFileWidget::setCompleter);
	connect(m_searchTimer, &QTimer::timeout, this, &ImportFileWidget::topicTimeout);
	connect(m_client, &QMqttClient::disconnected, this, &ImportFileWidget::onMqttDisconnect);
	connect(m_timeoutTimer, &QTimer::timeout, this, &ImportFileWidget::mqttTimeout);

	connect(ui.chbWill, &QCheckBox::stateChanged, this, &ImportFileWidget::useWillMessage);
	connect(ui.cbWillMessageType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &ImportFileWidget::willMessageTypeChanged);
	connect(ui.cbWillUpdate, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &ImportFileWidget::willUpdateChanged);
	connect(this, &ImportFileWidget::newTopicForWill, this, &ImportFileWidget::updateWillTopics);
	connect(m_client, &QMqttClient::errorChanged, this, &ImportFileWidget::mqttErrorChanged);
	connect(ui.cbFileType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), [this]() {emit checkFileType();});
	connect(ui.leTopics, &QLineEdit::textChanged, this, &ImportFileWidget::searchTreeItem);

	connect(ui.chbAuthentication, &QCheckBox::stateChanged, this, &ImportFileWidget::checkConnectEnable);
	connect(ui.chbID, &QCheckBox::stateChanged, this, &ImportFileWidget::checkConnectEnable);
	connect(ui.leHost, &QLineEdit::textChanged, this, &ImportFileWidget::checkConnectEnable);
	connect(ui.lePort, &QLineEdit::textChanged, this, &ImportFileWidget::checkConnectEnable);
	connect(ui.lePassword, &QLineEdit::textChanged, this, &ImportFileWidget::checkConnectEnable);
	connect(ui.leUsername, &QLineEdit::textChanged, this, &ImportFileWidget::checkConnectEnable);
	connect(ui.leID, &QLineEdit::textChanged, this, &ImportFileWidget::checkConnectEnable);

	ui.bSubscribe->setIcon(ui.bSubscribe->style()->standardIcon(QStyle::SP_ArrowRight));
	ui.bUnsubscribe->setIcon(ui.bUnsubscribe->style()->standardIcon(QStyle::SP_BrowserStop));
#endif

	connect(ui.leHost, SIGNAL(textChanged(QString)), this, SIGNAL(hostChanged()));
	connect(ui.lePort, SIGNAL(textChanged(QString)), this, SIGNAL(portChanged()));

	connect( ui.cbSourceType, SIGNAL(currentIndexChanged(int)), this, SLOT(sourceTypeChanged(int)));

	connect( ui.tvJson, SIGNAL(clicked(const QModelIndex&)), this, SLOT(refreshPreview()));

	//TODO: implement save/load of user-defined settings later and activate these buttons again
	ui.bSaveFilter->hide();
	ui.bManageFilters->hide();

	//defer the loading of settings a bit in order to show the dialog prior to blocking the GUI in refreshPreview()
	QTimer::singleShot( 100, this, SLOT(loadSettings()) );
	hideMQTT();
}

void ImportFileWidget::loadSettings() {
	m_suppressRefresh = true;

	//load last used settings
	QString confName;
	if (m_liveDataSource)
		confName = QLatin1String("LiveDataImport");
	else
		confName = QLatin1String("FileImport");
	KConfigGroup conf(KSharedConfig::openConfig(), confName);

	//settings for data type specific widgets
	m_asciiOptionsWidget->loadSettings();
	m_binaryOptionsWidget->loadSettings();
	m_imageOptionsWidget->loadSettings();
	m_jsonOptionsWidget->loadSettings();

	//read the source type first since settings in fileNameChanged() depend on this
	ui.cbSourceType->setCurrentIndex(conf.readEntry("SourceType").toInt());

	//general settings
	ui.cbFileType->setCurrentIndex(conf.readEntry("Type", 0));
	ui.cbFilter->setCurrentIndex(conf.readEntry("Filter", 0));
	filterChanged(ui.cbFilter->currentIndex());	// needed if filter is not changed
	if (m_fileName.isEmpty())
		ui.leFileName->setText(conf.readEntry("LastImportedFile", ""));
	else
		ui.leFileName->setText(m_fileName);

	//live data related settings
	ui.cbBaudRate->setCurrentIndex(conf.readEntry("BaudRate").toInt());
	ui.cbReadingType->setCurrentIndex(conf.readEntry("ReadingType").toInt());
	ui.cbSerialPort->setCurrentIndex(conf.readEntry("SerialPort").toInt());
	ui.cbUpdateType->setCurrentIndex(conf.readEntry("UpdateType").toInt());
	ui.leHost->setText(conf.readEntry("Host",""));
	ui.sbKeepNValues->setValue(conf.readEntry("KeepNValues").toInt());
	ui.lePort->setText(conf.readEntry("Port",""));
	ui.sbSampleSize->setValue(conf.readEntry("SampleSize").toInt());
	ui.sbUpdateInterval->setValue(conf.readEntry("UpdateInterval").toInt());
#ifdef HAVE_MQTT
	ui.chbID->setChecked(conf.readEntry("mqttUseId").toInt());
	ui.chbAuthentication->setChecked(conf.readEntry("mqttUseAuthentication").toInt());
	ui.chbRetain->setChecked(conf.readEntry("mqttUseRetain").toInt());
	ui.leUsername->setText(conf.readEntry("mqttUsername",""));
	ui.lePassword->setText(conf.readEntry("mqttPassword",""));
	ui.leID->setText(conf.readEntry("mqttId",""));
	ui.chbWillRetain->setChecked(conf.readEntry("mqttWillRetain").toInt());
	ui.cbWillUpdate->setCurrentIndex(conf.readEntry("mqttWillUpdateType").toInt());
	ui.cbWillQoS->setCurrentIndex(conf.readEntry("mqttWillQoS").toInt());
	ui.leWillOwnMessage->setText(conf.readEntry("mqttWillOwnMessage",""));
	ui.leWillUpdateInterval->setText(conf.readEntry("mqttWillUpdateInterval",""));
	QString willStatistics = conf.readEntry("mqttWillStatistics","");
	QStringList statisticsList = willStatistics.split('|', QString::SplitBehavior::SkipEmptyParts);
	for(auto value : statisticsList) {
		QListWidgetItem* item = ui.lwWillStatistics->item(value.toInt());
		item->setCheckState(Qt::Checked);
	}
	ui.cbWillMessageType->setCurrentIndex(conf.readEntry("mqttWillMessageType").toInt());
	ui.chbWill->setChecked(conf.readEntry("mqttWillUse").toInt());
	//chbWill is unchecked by deafult, so if false is loaded it doesn't emit state changed signal, we have to force it
	if(!ui.chbWill->isChecked()) {
		ui.chbWill->setChecked(true);
		ui.chbWill->setChecked(false);
	}
#endif
	m_suppressRefresh = false;
	refreshPreview();
}

ImportFileWidget::~ImportFileWidget() {
	// save current settings
	QString confName;
	if (m_liveDataSource)
		confName = QLatin1String("LiveDataImport");
	else
		confName = QLatin1String("FileImport");
	KConfigGroup conf(KSharedConfig::openConfig(), confName);

	// general settings
	conf.writeEntry("Type", ui.cbFileType->currentIndex());
	conf.writeEntry("Filter", ui.cbFilter->currentIndex());
	conf.writeEntry("LastImportedFile", ui.leFileName->text());

	//live data related settings
	conf.writeEntry("SourceType", ui.cbSourceType->currentIndex());
	conf.writeEntry("UpdateType", ui.cbUpdateType->currentIndex());
	conf.writeEntry("ReadingType", ui.cbReadingType->currentIndex());
	conf.writeEntry("SampleSize", ui.sbSampleSize->value());
	conf.writeEntry("KeepNValues", ui.sbKeepNValues->value());
	conf.writeEntry("BaudRate", ui.cbBaudRate->currentIndex());
	conf.writeEntry("SerialPort", ui.cbSerialPort->currentIndex());
	conf.writeEntry("Host", ui.leHost->text());
	conf.writeEntry("Port", ui.lePort->text());
	conf.writeEntry("UpdateInterval", ui.sbUpdateInterval->value());
#ifdef HAVE_MQTT
	conf.writeEntry("mqttUsername", ui.leUsername->text());
	conf.writeEntry("mqttPassword", ui.lePassword->text());
	conf.writeEntry("mqttId", ui.leID->text());
	conf.writeEntry("mqttWillMessageType", ui.cbWillMessageType->currentIndex());
	conf.writeEntry("mqttWillUpdateType", ui.cbWillUpdate->currentIndex());
	conf.writeEntry("mqttWillQoS", ui.cbWillQoS->currentIndex());
	conf.writeEntry("mqttWillOwnMessage", ui.leWillOwnMessage->text());
	conf.writeEntry("mqttWillUpdateInterval", ui.leWillUpdateInterval->text());
	QString willStatistics;
	for(int i = 0; i < ui.lwWillStatistics->count(); ++i) {
		QListWidgetItem* item = ui.lwWillStatistics->item(i);
		if (item->checkState() == Qt::Checked)
			willStatistics += QString::number(i)+"|";
	}
	conf.writeEntry("mqttWillStatistics", willStatistics);
	conf.writeEntry("mqttWillRetain", static_cast<int>(ui.chbWillRetain->isChecked()));
	conf.writeEntry("mqttWillUse", static_cast<int>(ui.chbWill->isChecked()));
	conf.writeEntry("mqttUseId", static_cast<int>(ui.chbID->isChecked()));
	conf.writeEntry("mqttUseAuthentication", static_cast<int>(ui.chbAuthentication->isChecked()));
	conf.writeEntry("mqttUseRetain", static_cast<int>(ui.chbRetain->isChecked()));
#endif

	// data type specific settings
	m_asciiOptionsWidget->saveSettings();
	m_binaryOptionsWidget->saveSettings();
	m_imageOptionsWidget->saveSettings();
}

void ImportFileWidget::hideDataSource() {
	m_liveDataSource = false;
	ui.gbUpdateOptions->hide();

	ui.chbLinkFile->hide();

	ui.cbBaudRate->hide();
	ui.lBaudRate->hide();

	ui.lHost->hide();
	ui.leHost->hide();

	ui.lPort->hide();
	ui.lePort->hide();

	ui.cbSerialPort->hide();
	ui.lSerialPort->hide();

	ui.lSourceType->hide();
	ui.cbSourceType->hide();

	ui.cbUpdateType->hide();
	ui.lUpdateType->hide();

	ui.sbUpdateInterval->hide();
	ui.lUpdateInterval->hide();

#ifdef HAVE_MQTT
	hideMQTT();
#endif
}

void ImportFileWidget::showAsciiHeaderOptions(bool b) {
	m_asciiOptionsWidget->showAsciiHeaderOptions(b);
}

void ImportFileWidget::showJsonModel(bool b) {
	ui.tvJson->setVisible(b);
	ui.lField->setVisible(b);
}

void ImportFileWidget::showOptions(bool b) {
	ui.gbOptions->setVisible(b);

	if (m_liveDataSource)
		ui.gbUpdateOptions->setVisible(b);

	resize(layout()->minimumSize());
}

QString ImportFileWidget::fileName() const {
	return ui.leFileName->text();
}

QString ImportFileWidget::selectedObject() const {
	const QString& path = ui.leFileName->text();

	//determine the file name only
	QString name = path.right(path.length() - path.lastIndexOf(QDir::separator()) - 1);

	//strip away the extension if available
	if (name.indexOf('.') != -1)
		name = name.left(name.lastIndexOf('.'));

	//for multi-dimensinal formats like HDF, netCDF and FITS add the currently selected object
	const auto format = currentFileType();
	if (format == AbstractFileFilter::HDF5) {
		const QStringList& hdf5Names = m_hdf5OptionsWidget->selectedHDF5Names();
		if (hdf5Names.size())
			name += hdf5Names.first(); //the names of the selected HDF5 objects already have '/'
	} else if (format == AbstractFileFilter::NETCDF) {
		const QStringList& names = m_netcdfOptionsWidget->selectedNetCDFNames();
		if (names.size())
			name += QLatin1Char('/') + names.first();
	} else if (format == AbstractFileFilter::FITS) {
		const QString& extensionName = m_fitsOptionsWidget->currentExtensionName();
		if (!extensionName.isEmpty())
			name += QLatin1Char('/') + extensionName;
	} else if (format == AbstractFileFilter::ROOT) {
		const QStringList& names = m_rootOptionsWidget->selectedROOTNames();
		if (names.size())
			name += QLatin1Char('/') + names.first();
	}

	return name;
}

/*!
 * returns \c true if the number of lines to be imported from the currently selected file is zero ("file is empty"),
 * returns \c false otherwise.
 */
bool ImportFileWidget::isFileEmpty() const {
	return m_fileEmpty;
}

QString ImportFileWidget::host() const {
	return ui.leHost->text();
}

QString ImportFileWidget::port() const {
	return ui.lePort->text();
}

QString ImportFileWidget::serialPort() const {
	return ui.cbSerialPort->currentText();
}

int ImportFileWidget::baudRate() const {
	return ui.cbBaudRate->currentText().toInt();
}

/*!
	saves the settings to the data source \c source.
*/
void ImportFileWidget::saveSettings(LiveDataSource* source) const {
	AbstractFileFilter::FileType fileType = static_cast<AbstractFileFilter::FileType>(ui.cbFileType->currentIndex());
	LiveDataSource::UpdateType updateType = static_cast<LiveDataSource::UpdateType>(ui.cbUpdateType->currentIndex());
	LiveDataSource::SourceType sourceType = static_cast<LiveDataSource::SourceType>(ui.cbSourceType->currentIndex());
	LiveDataSource::ReadingType readingType = static_cast<LiveDataSource::ReadingType>(ui.cbReadingType->currentIndex());

	source->setComment( ui.leFileName->text() );
	source->setFileType(fileType);
	source->setFilter(this->currentFileFilter());

	source->setSourceType(sourceType);
	source->setReadingType(readingType);

	if (updateType == LiveDataSource::UpdateType::TimeInterval)
		source->setUpdateInterval(ui.sbUpdateInterval->value());
	else
		source->setFileWatched(true);

	source->setKeepNValues(ui.sbKeepNValues->value());

	source->setUpdateType(updateType);

	if (readingType != LiveDataSource::ReadingType::TillEnd)
		source->setSampleSize(ui.sbSampleSize->value());

	switch (sourceType) {
	case LiveDataSource::SourceType::FileOrPipe:
		source->setFileName( ui.leFileName->text() );
		source->setFileLinked( ui.chbLinkFile->isChecked() );
		break;
	case LiveDataSource::SourceType::LocalSocket:
		source->setLocalSocketName(ui.leFileName->text());
		break;
	case LiveDataSource::SourceType::NetworkTcpSocket:
	case LiveDataSource::SourceType::NetworkUdpSocket:
		source->setHost(ui.leHost->text());
		source->setPort((quint16)ui.lePort->text().toInt());
		break;
	case LiveDataSource::SourceType::SerialPort:
		source->setBaudRate(ui.cbBaudRate->currentText().toInt());
		source->setSerialPort(ui.cbSerialPort->currentText());
		break;
	default:
		break;
	}
}

#ifdef HAVE_MQTT
void ImportFileWidget::saveMQTTSettings(MQTTClient* client) const {
	qDebug()<<"Saving to MQTT Client";
	MQTTClient::UpdateType updateType = static_cast<MQTTClient::UpdateType>(ui.cbUpdateType->currentIndex());
	MQTTClient::ReadingType readingType = static_cast<MQTTClient::ReadingType>(ui.cbReadingType->currentIndex());

	client->setComment( ui.leFileName->text() );
	client->setFilter(this->currentFileFilter());

	client->setReadingType(readingType);

	if (updateType == MQTTClient::UpdateType::TimeInterval)
		client->setUpdateInterval(ui.sbUpdateInterval->value());

	client->setKeepNvalues(ui.sbKeepNValues->value());

	client->setUpdateType(updateType);

	if (readingType != MQTTClient::ReadingType::TillEnd)
		client->setSampleRate(ui.sbSampleSize->value());

	qDebug()<<"Saving mqtt";
	client->setMqttClientHostPort(m_client->hostname(), m_client->port());

	client->setMQTTUseAuthentication(ui.chbAuthentication->isChecked());
	if(ui.chbAuthentication->isChecked())
		client->setMqttClientAuthentication(m_client->username(), m_client->password());

	client->setMQTTUseID(ui.chbID->isChecked());
	if(ui.chbID->isChecked())
		client->setMqttClientId(m_client->clientId());

	for(int i=0; i<m_mqttSubscriptions.count(); ++i) {
		client->addMqttSubscriptions(m_mqttSubscriptions[i]->topic(), m_mqttSubscriptions[i]->qos());
	}
	client->setMqttRetain(ui.chbRetain->isChecked());
	client->setWillMessageType(static_cast<MQTTClient::WillMessageType>(ui.cbWillMessageType->currentIndex()) );
	client->setWillOwnMessage(ui.leWillOwnMessage->text());
	client->setWillQoS(ui.cbWillQoS->currentIndex() );
	client->setWillRetain(ui.chbWillRetain->isChecked());
	client->setWillTimeInterval(ui.leWillUpdateInterval->text().toInt());
	client->setWillTopic(ui.cbWillTopic->currentText());
	client->setWillUpdateType(static_cast<MQTTClient::WillUpdateType>(ui.cbWillUpdate->currentIndex()) );
	client->setMqttWillUse(ui.chbWill->isChecked());
	for(int i = 0; i < ui.lwWillStatistics->count(); ++i) {
		QListWidgetItem* item = ui.lwWillStatistics->item(i);
		if (item->checkState() == Qt::Checked)
			client->addWillStatistics(static_cast<MQTTClient::WillStatistics> (i));
	}
}
#endif

/*!
	returns the currently used file type.
*/
AbstractFileFilter::FileType ImportFileWidget::currentFileType() const {
	return static_cast<AbstractFileFilter::FileType>(ui.cbFileType->currentIndex());
}

LiveDataSource::SourceType ImportFileWidget::currentSourceType() const {
	return static_cast<LiveDataSource::SourceType>(ui.cbSourceType->currentIndex());
}

/*!
	returns the currently used filter.
*/
AbstractFileFilter* ImportFileWidget::currentFileFilter() const {
	DEBUG("ImportFileWidget::currentFileFilter()");
	AbstractFileFilter::FileType fileType = static_cast<AbstractFileFilter::FileType>(ui.cbFileType->currentIndex());

	switch (fileType) {
	case AbstractFileFilter::Ascii: {
			DEBUG("	ASCII");
//TODO			std::unique_ptr<AsciiFilter> filter(new AsciiFilter());
			AsciiFilter* filter = new AsciiFilter();

			if (ui.cbFilter->currentIndex() == 0)     //"automatic"
				filter->setAutoModeEnabled(true);
			else if (ui.cbFilter->currentIndex() == 1) { //"custom"
				filter->setAutoModeEnabled(false);
				m_asciiOptionsWidget->applyFilterSettings(filter);
			} else
				filter->loadFilterSettings( ui.cbFilter->currentText() );

			//save the data portion to import
			filter->setStartRow( ui.sbStartRow->value());
			filter->setEndRow( ui.sbEndRow->value() );
			filter->setStartColumn( ui.sbStartColumn->value());
			filter->setEndColumn( ui.sbEndColumn->value());

			return filter;
		}
	case AbstractFileFilter::Binary: {
			BinaryFilter* filter = new BinaryFilter();
			if ( ui.cbFilter->currentIndex() == 0 ) 	//"automatic"
				filter->setAutoModeEnabled(true);
			else if ( ui.cbFilter->currentIndex() == 1 ) {	//"custom"
				filter->setAutoModeEnabled(false);
				m_binaryOptionsWidget->applyFilterSettings(filter);
			} else {
				//TODO: load filter settings
// 			filter->setFilterName( ui.cbFilter->currentText() );
			}

			filter->setStartRow( ui.sbStartRow->value() );
			filter->setEndRow( ui.sbEndRow->value() );

			return filter;
		}
	case AbstractFileFilter::Image: {
			ImageFilter* filter = new ImageFilter();

			filter->setImportFormat(m_imageOptionsWidget->currentFormat());
			filter->setStartRow( ui.sbStartRow->value() );
			filter->setEndRow( ui.sbEndRow->value() );
			filter->setStartColumn( ui.sbStartColumn->value() );
			filter->setEndColumn( ui.sbEndColumn->value() );

			return filter;
		}
	case AbstractFileFilter::HDF5: {
			HDF5Filter* filter = new HDF5Filter();
			QStringList names = selectedHDF5Names();
			if (!names.isEmpty())
				filter->setCurrentDataSetName(names[0]);
			filter->setStartRow( ui.sbStartRow->value() );
			filter->setEndRow( ui.sbEndRow->value() );
			filter->setStartColumn( ui.sbStartColumn->value() );
			filter->setEndColumn( ui.sbEndColumn->value() );

			return filter;
		}
	case AbstractFileFilter::NETCDF: {
			NetCDFFilter* filter = new NetCDFFilter();

			if (!selectedNetCDFNames().isEmpty())
				filter->setCurrentVarName(selectedNetCDFNames()[0]);
			filter->setStartRow( ui.sbStartRow->value() );
			filter->setEndRow( ui.sbEndRow->value() );
			filter->setStartColumn( ui.sbStartColumn->value() );
			filter->setEndColumn( ui.sbEndColumn->value() );

			return filter;
		}
	case AbstractFileFilter::FITS: {
			FITSFilter* filter = new FITSFilter();
			filter->setStartRow( ui.sbStartRow->value());
			filter->setEndRow( ui.sbEndRow->value() );
			filter->setStartColumn( ui.sbStartColumn->value());
			filter->setEndColumn( ui.sbEndColumn->value());
			return filter;
		}
	case AbstractFileFilter::Json: {
			JsonFilter* filter = new JsonFilter();
			m_jsonOptionsWidget->applyFilterSettings(filter, ui.tvJson->currentIndex());

			filter->setStartRow( ui.sbStartRow->value() );
			filter->setEndRow( ui.sbEndRow->value() );
			filter->setStartColumn( ui.sbStartColumn->value());
			filter->setEndColumn( ui.sbEndColumn->value());
			return filter;
		}
	case AbstractFileFilter::ROOT: {
			ROOTFilter* filter = new ROOTFilter();
			QStringList names = selectedROOTNames();
			if (!names.isEmpty())
				filter->setCurrentHistogram(names.first());

			filter->setStartBin( m_rootOptionsWidget->startBin() );
			filter->setEndBin( m_rootOptionsWidget->endBin() );
			filter->setColumns( m_rootOptionsWidget->columns() );

			return filter;
		}
	case AbstractFileFilter::NgspiceRawAscii: {
			NgspiceRawAsciiFilter* filter = new NgspiceRawAsciiFilter();
			filter->setStartRow( ui.sbStartRow->value() );
			filter->setEndRow( ui.sbEndRow->value() );
			return filter;
		}
	}

	return 0;
}

/*!
	opens a file dialog and lets the user select the file data source.
*/
void ImportFileWidget::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportFileWidget");
	QString dir = conf.readEntry("LastDir", "");
	QString path = QFileDialog::getOpenFileName(this, i18n("Select the File Data Source"), dir);
	if (path.isEmpty())	//cancel was clicked in the file-dialog
		return;

	int pos = path.lastIndexOf(QDir::separator());
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastDir", newDir);
	}

	ui.leFileName->setText(path);

	//TODO: decide whether the selection of several files should be possible
// 	QStringList filelist = QFileDialog::getOpenFileNames(this,i18n("Select one or more files to open"));
// 	if (! filelist.isEmpty() )
// 		ui.leFileName->setText(filelist.join(";"));
}

/************** SLOTS **************************************************************/

/*!
	called on file name changes.
	Determines the file format (ASCII, binary etc.), if the file exists,
	and activates the corresponding options.
*/
void ImportFileWidget::fileNameChanged(const QString& name) {
	QString fileName = name;
#ifndef HAVE_WINDOWS
	// make relative path
	if ( !fileName.isEmpty() && fileName.at(0) != QDir::separator())
		fileName = QDir::homePath() + QDir::separator() + fileName;
#endif

	bool fileExists = QFile::exists(fileName);
	if (fileExists)
		ui.leFileName->setStyleSheet("");
	else
		ui.leFileName->setStyleSheet("QLineEdit{background:red;}");

	ui.gbOptions->setEnabled(fileExists);
	ui.bManageFilters->setEnabled(fileExists);
	ui.cbFilter->setEnabled(fileExists);
	ui.cbFileType->setEnabled(fileExists);
	ui.bFileInfo->setEnabled(fileExists);
	ui.gbUpdateOptions->setEnabled(fileExists);
	if (!fileExists) {
		//file doesn't exist -> delete the content preview that is still potentially
		//available from the previously selected file
		ui.tePreview->clear();
		m_twPreview->clear();
		m_hdf5OptionsWidget->clear();
		m_netcdfOptionsWidget->clear();
		m_fitsOptionsWidget->clear();
		m_jsonOptionsWidget->clearModel();
		m_rootOptionsWidget->clear();

		emit fileNameChanged();
		return;
	}

	if (currentSourceType() == LiveDataSource::FileOrPipe) {
		const AbstractFileFilter::FileType fileType = AbstractFileFilter::fileType(fileName);
		switch(fileType) {
		case AbstractFileFilter::Ascii:
			ui.cbFileType->setCurrentIndex(AbstractFileFilter::Ascii);
			break;
		case AbstractFileFilter::Binary:
			ui.cbFileType->setCurrentIndex(AbstractFileFilter::Binary);
			break;
		case AbstractFileFilter::Image:
			ui.cbFileType->setCurrentIndex(AbstractFileFilter::Image);
			break;
		case AbstractFileFilter::HDF5:
			ui.cbFileType->setCurrentIndex(AbstractFileFilter::HDF5);
			m_hdf5OptionsWidget->updateContent((HDF5Filter*)this->currentFileFilter(), fileName);
			break;
		case AbstractFileFilter::NETCDF:
			ui.cbFileType->setCurrentIndex(AbstractFileFilter::NETCDF);
			m_netcdfOptionsWidget->updateContent((NetCDFFilter*)this->currentFileFilter(), fileName);
			break;
#ifdef HAVE_FITS
		case AbstractFileFilter::FITS:
			ui.cbFileType->setCurrentIndex(AbstractFileFilter::FITS);
			m_fitsOptionsWidget->updateContent((FITSFilter*)this->currentFileFilter(), fileName);
			break;
#endif
		case AbstractFileFilter::Json:
			ui.cbFileType->setCurrentIndex(AbstractFileFilter::Json);
			m_jsonOptionsWidget->loadDocument(fileName);
			break;
		case AbstractFileFilter::ROOT:
			ui.cbFileType->setCurrentIndex(AbstractFileFilter::ROOT);
			m_rootOptionsWidget->updateContent((ROOTFilter*)this->currentFileFilter(), fileName);
			break;
		case AbstractFileFilter::NgspiceRawAscii:
			ui.cbFileType->setCurrentIndex(AbstractFileFilter::NgspiceRawAscii);
			break;
		default:
			ui.cbFileType->setCurrentIndex(AbstractFileFilter::Binary);
		}
	}

	refreshPreview();
	emit fileNameChanged();
}

/*!
  saves the current filter settings
*/
void ImportFileWidget::saveFilter() {
	bool ok;
	QString text = QInputDialog::getText(this, i18n("Save Filter Settings as"),
	                                     i18n("Filter name:"), QLineEdit::Normal, i18n("new filter"), &ok);
	if (ok && !text.isEmpty()) {
		//TODO
		//AsciiFilter::saveFilter()
	}
}

/*!
  opens a dialog for managing all available predefined filters.
*/
void ImportFileWidget::manageFilters() {
	//TODO
}

/*!
	Depending on the selected file type, activates the corresponding options in the data portion tab
	and populates the combobox with the available pre-defined fllter settings for the selected type.
*/
void ImportFileWidget::fileTypeChanged(int fileType) {
	ui.swOptions->setCurrentIndex(fileType);

	//default
	ui.lFilter->show();
	ui.cbFilter->show();

	//different file types show different number of tabs in ui.tabWidget.
	//when switching from the previous file type we re-set the tab widget to its original state
	//and remove/add the required tabs further below
	for (int i = 0; i<ui.tabWidget->count(); ++i)
		ui.tabWidget->count();

	ui.tabWidget->addTab(ui.tabDataFormat, i18n("Data format"));
	ui.tabWidget->addTab(ui.tabDataPreview, i18n("Preview"));
	ui.tabWidget->addTab(ui.tabDataPortion, i18n("Data portion to read"));

	ui.lPreviewLines->show();
	ui.sbPreviewLines->show();
	ui.lStartColumn->show();
	ui.sbStartColumn->show();
	ui.lEndColumn->show();
	ui.sbEndColumn->show();

	showJsonModel(false);

	switch (fileType) {
	case AbstractFileFilter::Ascii:
		break;
	case AbstractFileFilter::Binary:
		ui.lStartColumn->hide();
		ui.sbStartColumn->hide();
		ui.lEndColumn->hide();
		ui.sbEndColumn->hide();
		break;
	case AbstractFileFilter::ROOT:
		ui.tabWidget->removeTab(1);
		// falls through
	case AbstractFileFilter::HDF5:
	case AbstractFileFilter::NETCDF:
	case AbstractFileFilter::FITS:
		ui.lFilter->hide();
		ui.cbFilter->hide();
		// hide global preview tab. we have our own
		ui.tabWidget->setTabText(0, i18n("Data format && preview"));
		ui.tabWidget->removeTab(1);
		ui.tabWidget->setCurrentIndex(0);
		break;
	case AbstractFileFilter::Image:
		ui.lPreviewLines->hide();
		ui.sbPreviewLines->hide();
		ui.lFilter->hide();
		ui.cbFilter->hide();
		break;
	case AbstractFileFilter::NgspiceRawAscii:
		ui.lStartColumn->hide();
		ui.sbStartColumn->hide();
		ui.lEndColumn->hide();
		ui.sbEndColumn->hide();
		ui.tabWidget->removeTab(0);
		ui.tabWidget->setCurrentIndex(0);
		break;
	case AbstractFileFilter::Json:
		ui.lFilter->hide();
		ui.cbFilter->hide();
		showJsonModel(true);
		break;
	default:
		DEBUG("unknown file type");
	}

	m_hdf5OptionsWidget->clear();
	m_netcdfOptionsWidget->clear();
	m_rootOptionsWidget->clear();

	int lastUsedFilterIndex = ui.cbFilter->currentIndex();
	ui.cbFilter->clear();
	ui.cbFilter->addItem( i18n("Automatic") );
	ui.cbFilter->addItem( i18n("Custom") );

	//TODO: populate the combobox with the available pre-defined filter settings for the selected type
	ui.cbFilter->setCurrentIndex(lastUsedFilterIndex);
	filterChanged(lastUsedFilterIndex);

	refreshPreview();
}


const QStringList ImportFileWidget::selectedHDF5Names() const {
	return m_hdf5OptionsWidget->selectedHDF5Names();
}

const QStringList ImportFileWidget::selectedNetCDFNames() const {
	return m_netcdfOptionsWidget->selectedNetCDFNames();
}

const QStringList ImportFileWidget::selectedFITSExtensions() const {
	return m_fitsOptionsWidget->selectedFITSExtensions();
}

const QStringList ImportFileWidget::selectedROOTNames() const {
	return m_rootOptionsWidget->selectedROOTNames();
}

/*!
	shows the dialog with the information about the file(s) to be imported.
*/
void ImportFileWidget::fileInfoDialog() {
	QStringList files = ui.leFileName->text().split(';');
	FileInfoDialog* dlg = new FileInfoDialog(this);
	dlg->setFiles(files);
	dlg->exec();
}

/*!
	enables the options if the filter "custom" was chosen. Disables the options otherwise.
*/
void ImportFileWidget::filterChanged(int index) {
	// ignore filter for these formats
	if (ui.cbFileType->currentIndex() == AbstractFileFilter::HDF5 ||
		ui.cbFileType->currentIndex() == AbstractFileFilter::NETCDF ||
		ui.cbFileType->currentIndex() == AbstractFileFilter::Image ||
		ui.cbFileType->currentIndex() == AbstractFileFilter::FITS ||
		ui.cbFileType->currentIndex() == AbstractFileFilter::Json ||
		ui.cbFileType->currentIndex() == AbstractFileFilter::ROOT) {
		ui.swOptions->setEnabled(true);
		return;
	}

	if (index == 0) { // "automatic"
		ui.swOptions->setEnabled(false);
		ui.bSaveFilter->setEnabled(false);
	} else if (index == 1) { //custom
		ui.swOptions->setEnabled(true);
		ui.bSaveFilter->setEnabled(true);
	} else {
		// predefined filter settings were selected.
		//load and show them in the GUI.
		//TODO
	}
}

void ImportFileWidget::refreshPreview() {
	if (m_suppressRefresh)
		return;

	WAIT_CURSOR;

	QString fileName = ui.leFileName->text();
#ifndef HAVE_WINDOWS
	if (!fileName.isEmpty() && fileName.at(0) != QDir::separator())
		fileName = QDir::homePath() + QDir::separator() + fileName;
#endif
	DEBUG("refreshPreview(): file name = " << fileName.toStdString());

	QVector<QStringList> importedStrings;
	AbstractFileFilter::FileType fileType = (AbstractFileFilter::FileType)ui.cbFileType->currentIndex();

	// generic table widget
	if (fileType == AbstractFileFilter::Ascii || fileType == AbstractFileFilter::Binary
		|| fileType == AbstractFileFilter::Json || fileType == AbstractFileFilter::NgspiceRawAscii)
		m_twPreview->show();
	else
		m_twPreview->hide();

	int lines = ui.sbPreviewLines->value();

	bool ok = true;
	QTableWidget* tmpTableWidget{nullptr};
	QStringList vectorNameList;
	QVector<AbstractColumn::ColumnMode> columnModes;
	DEBUG("Data File Type: " << ENUM_TO_STRING(AbstractFileFilter, FileType, fileType));
	switch (fileType) {
	case AbstractFileFilter::Ascii: {
			ui.tePreview->clear();

			AsciiFilter* filter = static_cast<AsciiFilter*>(this->currentFileFilter());

			DEBUG("Data Source Type: " << ENUM_TO_STRING(LiveDataSource, SourceType, currentSourceType()));
			switch (currentSourceType()) {
			case LiveDataSource::SourceType::FileOrPipe: {
					importedStrings = filter->preview(fileName, lines);
					break;
				}
			case LiveDataSource::SourceType::LocalSocket: {
					QLocalSocket lsocket{this};
					DEBUG("Local socket: CONNECT PREVIEW");
					lsocket.connectToServer(fileName, QLocalSocket::ReadOnly);
					if (lsocket.waitForConnected()) {
						DEBUG("connected to local socket " << fileName.toStdString());
						if (lsocket.waitForReadyRead())
							importedStrings = filter->preview(lsocket);
						DEBUG("Local socket: DISCONNECT PREVIEW");
						lsocket.disconnectFromServer();
						// read-only socket is disconnected immediately (no waitForDisconnected())
					} else {
						DEBUG("failed connect to local socket " << fileName.toStdString() << " - " << lsocket.errorString().toStdString());
					}

					break;
				}
			case LiveDataSource::SourceType::NetworkTcpSocket: {
					QTcpSocket tcpSocket{this};
					tcpSocket.connectToHost(host(), port().toInt(), QTcpSocket::ReadOnly);
					if (tcpSocket.waitForConnected()) {
						DEBUG("connected to TCP socket");
						if ( tcpSocket.waitForReadyRead() )
							importedStrings = filter->preview(tcpSocket);

						tcpSocket.disconnectFromHost();
					} else {
						DEBUG("failed to connect to TCP socket " << " - " << tcpSocket.errorString().toStdString());
					}

					break;
				}
			case LiveDataSource::SourceType::NetworkUdpSocket: {
					QUdpSocket udpSocket{this};
					DEBUG("UDP Socket: CONNECT PREVIEW, state = " << udpSocket.state());
					udpSocket.bind(QHostAddress(host()), port().toInt());
					udpSocket.connectToHost(host(), 0, QUdpSocket::ReadOnly);
					if (udpSocket.waitForConnected()) {
						DEBUG("	connected to UDP socket " << host().toStdString() << ':' << port().toInt());
						if (!udpSocket.waitForReadyRead(2000) )
							DEBUG("	ERROR: not ready for read after 2 sec");
						if (udpSocket.hasPendingDatagrams()) {
							DEBUG("	has pending data");
						} else {
							DEBUG("	has no pending data");
						}
						importedStrings = filter->preview(udpSocket);

						DEBUG("UDP Socket: DISCONNECT PREVIEW, state = " << udpSocket.state());
						udpSocket.disconnectFromHost();
					} else {
						DEBUG("failed to connect to UDP socket " << " - " << udpSocket.errorString().toStdString());
					}

					break;
				}
			case LiveDataSource::SourceType::SerialPort: {
					QSerialPort sPort{this};
					sPort.setBaudRate(baudRate());
					sPort.setPortName(serialPort());
					if (sPort.open(QIODevice::ReadOnly)) {
						bool canread = sPort.waitForReadyRead(500);
						if (canread)
							importedStrings = filter->preview(sPort);

						sPort.close();
					}
					break;
				}
			case LiveDataSource::SourceType::MQTT: {
#ifdef HAVE_MQTT
				qDebug()<<"preview mqtt, is it ready:"<<m_mqttReadyForPreview;
				if(m_mqttReadyForPreview)
				{
					filter->vectorNames().clear();
					QMapIterator<QMqttTopicName, QMqttMessage> i(m_lastMessage);
					while(i.hasNext()) {
						i.next();
						  qDebug()<<"calling ascii mqtt preview"<< importedStrings << "  "<<QString(i.value().payload().data())
								 << "    "<< i.key().name();
						filter->mqttPreview(importedStrings, QString(i.value().payload().data()), i.key().name() );
						if(importedStrings.isEmpty())
							break;
					}

					QMapIterator<QMqttTopicName, bool> j(m_messageArrived);
					while(j.hasNext()) {
						j.next();
						qDebug()<<"Set false after preview: "<<j.key().name();
						m_messageArrived[j.key()] = false;
					}
					m_mqttReadyForPreview = false;
				}
#endif
				break;
			}
			}

			tmpTableWidget = m_twPreview;
			vectorNameList = filter->vectorNames();
			columnModes = filter->columnModes();
			break;
		}
	case AbstractFileFilter::Binary: {
			ui.tePreview->clear();
			BinaryFilter *filter = (BinaryFilter *)this->currentFileFilter();
			importedStrings = filter->preview(fileName, lines);
			tmpTableWidget = m_twPreview;
			break;
		}
	case AbstractFileFilter::Image: {
			ui.tePreview->clear();

			QImage image(fileName);
			QTextCursor cursor = ui.tePreview->textCursor();
			cursor.insertImage(image);
			RESET_CURSOR;
			return;
		}
	case AbstractFileFilter::HDF5: {
			HDF5Filter *filter = (HDF5Filter *)this->currentFileFilter();
			lines = m_hdf5OptionsWidget->lines();
			importedStrings = filter->readCurrentDataSet(fileName, NULL, ok, AbstractFileFilter::Replace, lines);
			tmpTableWidget = m_hdf5OptionsWidget->previewWidget();
			break;
		}
	case AbstractFileFilter::NETCDF: {
			NetCDFFilter *filter = (NetCDFFilter *)this->currentFileFilter();
			lines = m_netcdfOptionsWidget->lines();
			importedStrings = filter->readCurrentVar(fileName, NULL, AbstractFileFilter::Replace, lines);
			tmpTableWidget = m_netcdfOptionsWidget->previewWidget();
			break;
		}
	case AbstractFileFilter::FITS: {
			FITSFilter* filter = (FITSFilter*)this->currentFileFilter();
			lines = m_fitsOptionsWidget->lines();

			// update file name (may be any file type)
			m_fitsOptionsWidget->updateContent(filter, fileName);
			QString extensionName = m_fitsOptionsWidget->extensionName(&ok);
			if (!extensionName.isEmpty()) {
				DEBUG("	extension name = " << extensionName.toStdString());
				fileName = extensionName;
			}
			DEBUG("	file name = " << fileName.toStdString());

			bool readFitsTableToMatrix;
			importedStrings = filter->readChdu(fileName, &readFitsTableToMatrix, lines);
			emit checkedFitsTableToMatrix(readFitsTableToMatrix);

			tmpTableWidget = m_fitsOptionsWidget->previewWidget();
			break;
		}
	case AbstractFileFilter::Json: {
			ui.tePreview->clear();
			m_jsonOptionsWidget->loadDocument(fileName);
			JsonFilter *filter = (JsonFilter*)this->currentFileFilter();
			m_jsonOptionsWidget->applyFilterSettings(filter, ui.tvJson->currentIndex());
			importedStrings = filter->preview(fileName);
			tmpTableWidget = m_twPreview;
			columnModes = filter->columnModes();
			break;
		}
	case AbstractFileFilter::ROOT: {
			ROOTFilter *filter = (ROOTFilter *)this->currentFileFilter();
			lines = m_rootOptionsWidget->lines();
			m_rootOptionsWidget->setNBins(filter->binsInCurrentHistogram(fileName));
			importedStrings = filter->previewCurrentHistogram(
				fileName,
				m_rootOptionsWidget->startBin(),
				qMin(m_rootOptionsWidget->startBin() + m_rootOptionsWidget->lines() - 1,
				     m_rootOptionsWidget->endBin())
			);
			tmpTableWidget = m_rootOptionsWidget->previewWidget();
			// the last vector element contains the column names
			vectorNameList = importedStrings.last();
			importedStrings.removeLast();
			columnModes = QVector<AbstractColumn::ColumnMode>(vectorNameList.size(), AbstractColumn::Numeric);
			break;
		}
	case AbstractFileFilter::NgspiceRawAscii: {
			ui.tePreview->clear();
			NgspiceRawAsciiFilter* filter = (NgspiceRawAsciiFilter*)this->currentFileFilter();
			importedStrings = filter->preview(fileName, lines);
			tmpTableWidget = m_twPreview;
			vectorNameList = filter->vectorNames();
			columnModes = filter->columnModes();
			break;
		}
	}

	// fill the table widget
	tmpTableWidget->setRowCount(0);
	tmpTableWidget->setColumnCount(0);
	if( !importedStrings.isEmpty() ) {
		//QDEBUG("importedStrings =" << importedStrings);
		if (!ok) {
			// show imported strings as error message
			tmpTableWidget->setRowCount(1);
			tmpTableWidget->setColumnCount(1);
			QTableWidgetItem* item = new QTableWidgetItem();
			item->setText(importedStrings[0][0]);
			tmpTableWidget->setItem(0, 0, item);
		} else {
			//TODO: maxrows not used
			const int rows = qMax(importedStrings.size(), 1);
			const int maxColumns = 300;
			tmpTableWidget->setRowCount(rows);

			for (int i = 0; i < rows; ++i) {
// 				QDEBUG("imported string " << importedStrings[i]);

				int cols = importedStrings[i].size() > maxColumns ? maxColumns : importedStrings[i].size();	// new
				if (cols > tmpTableWidget->columnCount())
					tmpTableWidget->setColumnCount(cols);

				for (int j = 0; j < cols; ++j) {
					QTableWidgetItem* item = new QTableWidgetItem(importedStrings[i][j]);
					tmpTableWidget->setItem(i, j, item);
				}
			}

			// set header if columnMode available
			for (int i = 0; i < qMin(tmpTableWidget->columnCount(), columnModes.size()); ++i) {
				QString columnName = QString::number(i+1);
				if (i < vectorNameList.size())
					columnName = vectorNameList[i];
				auto* item = new QTableWidgetItem(columnName + QLatin1String(" {") + ENUM_TO_STRING(AbstractColumn, ColumnMode, columnModes[i]) + QLatin1String("}"));
				item->setTextAlignment(Qt::AlignLeft);
				item->setIcon(AbstractColumn::iconForMode(columnModes[i]));

				tmpTableWidget->setHorizontalHeaderItem(i, item);
			}
		}

		tmpTableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
		m_fileEmpty = false;
	} else {
		m_fileEmpty = true;
	}

	emit previewRefreshed();

	RESET_CURSOR;
}

void ImportFileWidget::updateTypeChanged(int idx) {
	LiveDataSource::UpdateType type = static_cast<LiveDataSource::UpdateType>(idx);

	switch (type) {
	case LiveDataSource::UpdateType::TimeInterval:
		ui.lUpdateInterval->show();
		ui.sbUpdateInterval->show();
		break;
	case LiveDataSource::UpdateType::NewData:
		ui.lUpdateInterval->hide();
		ui.sbUpdateInterval->hide();
	}
}

void ImportFileWidget::readingTypeChanged(int idx) {
	LiveDataSource::ReadingType type = static_cast<LiveDataSource::ReadingType>(idx);

	if (type == LiveDataSource::ReadingType::TillEnd || type == LiveDataSource::ReadingType::WholeFile) {
		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();
	} else {
		ui.lSampleSize->show();
		ui.sbSampleSize->show();
	}

	if (type == LiveDataSource::ReadingType::WholeFile) {
		ui.lKeepLastValues->hide();
		ui.sbKeepNValues->hide();
	} else {
		ui.lKeepLastValues->show();
		ui.sbKeepNValues->show();
	}
}

void ImportFileWidget::sourceTypeChanged(int idx) {
	LiveDataSource::SourceType type = static_cast<LiveDataSource::SourceType>(idx);

		switch (type) {
		case LiveDataSource::SourceType::FileOrPipe:
			ui.lFileName->show();
			ui.leFileName->show();
			ui.bFileInfo->show();
			ui.bOpen->show();
			ui.chbLinkFile->show();

			ui.cbBaudRate->hide();
			ui.lBaudRate->hide();
			ui.lHost->hide();
			ui.leHost->hide();
			ui.lPort->hide();
			ui.lePort->hide();
			ui.cbSerialPort->hide();
			ui.lSerialPort->hide();

			fileNameChanged(ui.leFileName->text());
			break;
		case LiveDataSource::SourceType::LocalSocket:
			ui.lFileName->show();
			ui.leFileName->show();
			ui.bOpen->show();

			ui.bFileInfo->hide();
			ui.cbBaudRate->hide();
			ui.lBaudRate->hide();
			ui.lHost->hide();
			ui.leHost->hide();
			ui.lPort->hide();
			ui.lePort->hide();
			ui.cbSerialPort->hide();
			ui.lSerialPort->hide();
			ui.chbLinkFile->hide();

			ui.gbOptions->setEnabled(true);
			ui.bManageFilters->setEnabled(true);
			ui.cbFilter->setEnabled(true);
			ui.cbFileType->setEnabled(true);
			break;
		case LiveDataSource::SourceType::NetworkTcpSocket:
		case LiveDataSource::SourceType::NetworkUdpSocket:
			ui.lHost->show();
			ui.leHost->show();
			ui.lePort->show();
			ui.lPort->show();

			ui.lBaudRate->hide();
			ui.cbBaudRate->hide();
			ui.lSerialPort->hide();
			ui.cbSerialPort->hide();

			ui.lFileName->hide();
			ui.leFileName->hide();
			ui.bFileInfo->hide();
			ui.bOpen->hide();
			ui.chbLinkFile->hide();

			ui.gbOptions->setEnabled(true);
			ui.bManageFilters->setEnabled(true);
			ui.cbFilter->setEnabled(true);
			ui.cbFileType->setEnabled(true);
			break;
		case LiveDataSource::SourceType::SerialPort:
			ui.lBaudRate->show();
			ui.cbBaudRate->show();
			ui.lSerialPort->show();
			ui.cbSerialPort->show();

			ui.lHost->hide();
			ui.leHost->hide();
			ui.lePort->hide();
			ui.lPort->hide();
			ui.lFileName->hide();
			ui.leFileName->hide();
			ui.bFileInfo->hide();
			ui.bOpen->hide();
			ui.chbLinkFile->hide();
			ui.cbFileType->setEnabled(true);

			ui.gbOptions->setEnabled(true);
			ui.bManageFilters->setEnabled(true);
			ui.cbFilter->setEnabled(true);
			break;

	case LiveDataSource::SourceType::MQTT:
#ifdef HAVE_MQTT

		ui.lBaudRate->hide();
		ui.cbBaudRate->hide();
		ui.lSerialPort->hide();
		ui.cbSerialPort->hide();

		ui.lHost->show();
		ui.leHost->show();
		ui.lePort->show();
		ui.lPort->show();
		ui.lFileName->hide();
		ui.leFileName->hide();
		ui.bFileInfo->hide();
		ui.bOpen->hide();
		ui.chbLinkFile->hide();
		ui.cbFileType->setEnabled(true);

		ui.leID->hide();
		ui.lMqttID->hide();
		ui.lePassword->hide();
		ui.lPassword->hide();
		ui.leUsername->hide();
		ui.lUsername->hide();
		ui.cbQos->show();
		ui.lQos->show();
		ui.twTopics->show();
		//ui.lTopicTree->show();
		ui.lTopicSearch->show();
		ui.leTopics->show();
		ui.twSubscriptions->show();
		//ui.lSubscriptions->show();
		ui.chbAuthentication->show();
		ui.chbID->show();
		ui.bSubscribe->show();
		ui.bUnsubscribe->show();
		ui.bConnect->show();

		ui.gbOptions->setEnabled(true);
		ui.bManageFilters->setEnabled(true);
		ui.cbFilter->setEnabled(true);

		ui.gbMqttWill->show();
		ui.chbWill->show();
		ui.chbWillRetain->hide();
		ui.cbWillQoS->hide();
		ui.cbWillMessageType->hide();
		ui.cbWillTopic->hide();
		ui.cbWillUpdate->hide();
		ui.leWillOwnMessage->hide();
		ui.leWillUpdateInterval->setValidator(new QIntValidator(2, 1000000) );
		ui.leWillUpdateInterval->hide();
		ui.lWillMessageType->hide();
		ui.lWillOwnMessage->hide();
		ui.lWillQos->hide();
		ui.lWillTopic->hide();
		ui.lWillUpdate->hide();
		ui.lWillUpdateInterval->hide();
		ui.lwWillStatistics->hide();
		ui.lWillStatistics->hide();
		ui.gbManageSubscriptions->setEnabled(false);

		checkConnectEnable();

		if(ui.chbWill->isChecked()) {
			ui.chbWillRetain->show();
			ui.cbWillQoS->show();
			ui.cbWillMessageType->show();
			ui.cbWillTopic->show();
			ui.cbWillUpdate->show();
			ui.lWillMessageType->show();
			ui.lWillQos->show();
			ui.lWillTopic->show();
			ui.lWillUpdate->show();

			if (ui.cbWillMessageType->currentIndex() == static_cast<int>(MQTTClient::WillMessageType::OwnMessage) ) {
				ui.leWillOwnMessage->show();
				ui.lWillOwnMessage->show();
			}
			else if(ui.cbWillMessageType->currentIndex() == static_cast<int>(MQTTClient::WillMessageType::Statistics) ){
				qDebug()<<"source type changed show statistics";
				ui.lWillStatistics->show();
				ui.lwWillStatistics->show();
			}

			if(ui.cbWillUpdate->currentIndex() == 0) {
				ui.leWillUpdateInterval->show();
				ui.lWillUpdateInterval->show();
			}
			else if (ui.cbWillUpdate->currentIndex() == 1)
			{
				ui.leWillUpdateInterval->hide();
				ui.lWillUpdateInterval->hide();
			}
		}
#endif
		break;
	}

	// "whole file" item
	const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(ui.cbReadingType->model());
	QStandardItem* item = model->item(LiveDataSource::ReadingType::WholeFile);
	if (type == LiveDataSource::SourceType::FileOrPipe)
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	else
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));

	//"update options" groupbox can be deactived for "file and pipe" if the file is invalid.
	//Activate the groupbox when switching from "file and pipe" to a different source type.
	if (type != LiveDataSource::SourceType::FileOrPipe)
		ui.gbUpdateOptions->setEnabled(true);

	emit sourceTypeChanged();
	refreshPreview();
}

void ImportFileWidget::initializeAndFillPortsAndBaudRates() {
	for (int i = 2; i < ui.swOptions->count(); ++i)
		ui.swOptions->removeWidget(ui.swOptions->widget(i));

	const int size = ui.cbFileType->count();
	for (int i = 2; i < size; ++i)
		ui.cbFileType->removeItem(2);

	ui.cbBaudRate->hide();
	ui.lBaudRate->hide();

	ui.lHost->hide();
	ui.leHost->hide();

	ui.lPort->hide();
	ui.lePort->hide();

	ui.cbSerialPort->hide();
	ui.lSerialPort->hide();

	ui.cbBaudRate->addItems(LiveDataSource::supportedBaudRates());
	ui.cbSerialPort->addItems(LiveDataSource::availablePorts());

	ui.tabWidget->removeTab(2);
}

#ifdef HAVE_MQTT
void ImportFileWidget::idChecked(int state)
{
	if (state == 2)
	{
		ui.leID->show();
		ui.lMqttID->show();
	}
	else if (state == 0)
	{
		ui.leID->hide();
		ui.lMqttID->hide();
	}
}

void ImportFileWidget::authenticationChecked(int state)
{
	if(state == 2)
	{
		ui.leUsername->show();
		ui.lePassword->show();
		ui.lPassword->show();
		ui.lUsername->show();
	}
	else if (state == 0)
	{
		ui.leUsername->hide();
		ui.lePassword->hide();
		ui.lUsername->hide();
		ui.lPassword->hide();
	}
}

void ImportFileWidget::mqttConnection()
{
	if(m_client->state() == QMqttClient::ClientState::Disconnected)	{
		const bool hostSet = !ui.leHost->text().isEmpty();
		const bool portSet = !ui.lePort->text().isEmpty();
		const bool idUsed = ui.chbID->isChecked();
		const bool idSet = !ui.leID->text().isEmpty();
		const bool idValid = !(idUsed && !idSet);
		const bool authenticationUsed = ui.chbAuthentication->isChecked();
		const bool usernameSet = !ui.leUsername->text().isEmpty();
		const bool passwordSet = !ui.lePassword->text().isEmpty();
		const bool authenticationValid = ! (authenticationUsed && ( !usernameSet || !passwordSet) );
		const bool valid =hostSet && portSet && idValid && authenticationValid;
		if (valid) {
			m_client->setHostname(ui.leHost->text().simplified());
			m_client->setPort(ui.lePort->text().toUInt());
			if(ui.chbID->isChecked())
				m_client->setClientId(ui.leID->text());
			if(ui.chbAuthentication->isChecked()) {
				m_client->setUsername(ui.leUsername->text());
				m_client->setPassword(ui.lePassword->text());
			}
			qDebug()<< ui.leHost->text() << " " << m_client->hostname() << "   " << m_client->port();
			qDebug()<<"Trying to connect";
			m_client->connectToHost();
			m_timeoutTimer->start();
		}
	}
	else if (m_client->state() == QMqttClient::ClientState::Connected) {
		qDebug()<<"Disconnecting from mqtt broker"	;
		m_client->disconnectFromHost();
	}
}

void ImportFileWidget::onMqttConnect() {
	if(m_client->error() == QMqttClient::NoError) {
		m_timeoutTimer->stop();
		ui.gbManageSubscriptions->setEnabled(true);
		ui.bConnect->setText("Disconnect");
		ui.leHost->setEnabled(false);
		ui.lePort->setEnabled(false);
		ui.lePassword->setEnabled(false);
		ui.leUsername->setEnabled(false);
		ui.leID->setEnabled(false);
		ui.cbSourceType->setEnabled(false);
		ui.chbAuthentication->setEnabled(false);
		ui.chbID->setEnabled(false);
		ui.chbRetain->setEnabled(false);
		QMessageBox::information(this, "Connection successful", "Connection established");
		QMqttTopicFilter globalFilter{"#"};
		m_mainSubscription = m_client->subscribe(globalFilter, 1);
		if(!m_mainSubscription)
			QMessageBox::information(this, "Couldn't subscribe", "Something went wrong");
	}
}

void ImportFileWidget::mqttSubscribe() {	
	QString name;
	QTreeWidgetItem *item = ui.twTopics->currentItem();
	if(item != nullptr) {
		QTreeWidgetItem *tempItem = item;
		name.prepend(item->text(0));
		if(item->childCount() != 0)
			name.append("/#");

		while(tempItem->parent() != nullptr) {
			tempItem = tempItem->parent();
			name.prepend(tempItem->text(0) + "/");
		}

		QList<QTreeWidgetItem *> topLevelList = ui.twSubscriptions->findItems(name, Qt::MatchExactly);

		if(topLevelList.isEmpty() || topLevelList.first()->parent() != nullptr) {

			qDebug() << name;
			bool foundSuperior = false;
			bool foundEqual = false;
			QVector<QString> equalTopics;

			for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
				qDebug()<<i<<" "<<ui.twSubscriptions->topLevelItemCount();
				if(checkTopicContains(name, ui.twSubscriptions->topLevelItem(i)->text(0))
						&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
					qDebug()<<"1"<<name<<" "<< ui.twSubscriptions->topLevelItem(i)->text(0);
					unsubscribeFromTopic(ui.twSubscriptions->topLevelItem(i)->text(0));
					qDebug()<<"After Delete";
					i--;
					continue;
				}
				qDebug()<<"checked inferior";

				if(checkTopicContains(ui.twSubscriptions->topLevelItem(i)->text(0), name)
						&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
					foundSuperior = true;
					qDebug()<<"2"<<name<<" "<< ui.twSubscriptions->topLevelItem(i)->text(0);
					break;
				}
				qDebug()<<"checked superior";

				QString commonTopic = checkCommonLevel(ui.twSubscriptions->topLevelItem(i)->text(0), name);
				if(!commonTopic.isEmpty()) {
					equalTopics.push_back(ui.twSubscriptions->topLevelItem(i)->text(0));
				}
				qDebug()<<"checked common";
			}

			if(!equalTopics.isEmpty()) {
				qDebug()<<"Equal topics not empty";
				int level = commonLevelIndex(name, equalTopics.first());
				QStringList commonList = name.split('/', QString::SkipEmptyParts);
				QTreeWidgetItem* currentItem;
				for(int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
					if(ui.twTopics->topLevelItem(i)->text(0) == commonList.first()) {
						currentItem = ui.twTopics->topLevelItem(i);
						break;
					}
				}

				int levelIdx = 1;
				while(levelIdx < level) {
					for(int j = 0; j < currentItem->childCount(); ++j) {
						if(currentItem->child(j)->text(0) == commonList[levelIdx]) {
							qDebug()<<"Child: "<<currentItem->child(j)->text(0);
							currentItem = currentItem->child(j);
							levelIdx++;
							break;
						}
					}
				}

				qDebug()<<currentItem->text(0)<<"  "<<currentItem->childCount();
				if(equalTopics.size() == currentItem->childCount() - 1) {
					qDebug()<<"foundEqual set true";
					foundEqual = true;
				}
			}

			if(!foundSuperior) {
				qDebug()<<"Adding new topic";
				QStringList toplevelName;
				toplevelName.push_back(name);
				QTreeWidgetItem* newTopLevelItem = new QTreeWidgetItem(toplevelName);
				ui.twSubscriptions->addTopLevelItem(newTopLevelItem);

				QMqttTopicFilter filter {name};
				QMqttSubscription *temp_subscription = m_client->subscribe(filter, static_cast<quint8> (ui.cbQos->currentText().toUInt()) );

				if(temp_subscription) {
					m_mqttSubscriptions.push_back(temp_subscription);
					connect(temp_subscription, &QMqttSubscription::messageReceived, this, &ImportFileWidget::mqttSubscriptionMessageReceived);
					m_mqttNewTopic = temp_subscription->topic().filter();
					emit subscriptionMade();
				}

				if(name.endsWith("#")) {
					addSubscriptionChildren(item, newTopLevelItem);
				}
			}

			if(foundEqual) {
				QString commonTopic;

				commonTopic = checkCommonLevel(equalTopics.first(), name);
				QStringList nameList;
				nameList.append(commonTopic);
				QTreeWidgetItem* newTopic = new QTreeWidgetItem(nameList);
				ui.twSubscriptions->addTopLevelItem(newTopic);
				QMqttTopicFilter filter {commonTopic};
				QMqttSubscription *temp_subscription = m_client->subscribe(filter, static_cast<quint8> (ui.cbQos->currentText().toUInt()) );

				if(temp_subscription) {
					m_mqttSubscriptions.push_back(temp_subscription);
					connect(temp_subscription, &QMqttSubscription::messageReceived, this, &ImportFileWidget::mqttSubscriptionMessageReceived);
					m_mqttNewTopic = temp_subscription->topic().filter();
					emit subscriptionMade();
				}

				for(int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j){
					if(ui.twSubscriptions->topLevelItem(j)->text(0) == name) {
						newTopic->addChild(ui.twSubscriptions->takeTopLevelItem(j));
						unsubscribeFromTopic(name);
						break;
					}
				}

				for(int i = 0; i < equalTopics.size(); ++i) {
					for(int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j){
						if(ui.twSubscriptions->topLevelItem(j)->text(0) == equalTopics[i]) {
							newTopic->addChild(ui.twSubscriptions->takeTopLevelItem(j));
							unsubscribeFromTopic(equalTopics[i]);
							break;
						}
					}
				}

				for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
					if(checkTopicContains(commonTopic, ui.twSubscriptions->topLevelItem(i)->text(0)) &&
							commonTopic != ui.twSubscriptions->topLevelItem(i)->text(0) ) {
						unsubscribeFromTopic(ui.twSubscriptions->topLevelItem(i)->text(0));
						i--;
					}
				}

				bool foundNewEqual;
				do{
					foundNewEqual = false;
					equalTopics.clear();

					for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
						if(!checkCommonLevel(ui.twSubscriptions->topLevelItem(i)->text(0), commonTopic).isEmpty()) {
							equalTopics.push_back(ui.twSubscriptions->topLevelItem(i)->text(0));
						}
					}

					if(!equalTopics.isEmpty()) {
						int level = commonLevelIndex(name, equalTopics.first());
						QStringList commonList = name.split('/', QString::SkipEmptyParts);
						QTreeWidgetItem* currentItem;
						for(int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
							if(ui.twTopics->topLevelItem(i)->text(0) == commonList.first()) {
								currentItem = ui.twTopics->topLevelItem(i);
								break;
							}
						}

						int levelIdx = 1;
						while(levelIdx < level) {
							for(int j = 0; j < currentItem->childCount(); ++j) {
								if(currentItem->child(j)->text(0) == commonList[j]) {
									currentItem = currentItem->child(j);
									levelIdx++;
									break;
								}
							}
						}

						qDebug()<<currentItem->text(0)<<"  "<<currentItem->childCount();
						if(equalTopics.size() == currentItem->childCount() - 1) {
							foundNewEqual = true;
						}
					}

					if(foundNewEqual) {
						QString newCommonTopic = checkCommonLevel(equalTopics.first(), commonTopic);
						nameList.clear();
						nameList.append(newCommonTopic);
						QTreeWidgetItem* oldTopic = newTopic;
						newTopic = new QTreeWidgetItem(nameList);
						ui.twSubscriptions->addTopLevelItem(newTopic);

						QMqttTopicFilter filter {newCommonTopic};
						QMqttSubscription *temp_subscription = m_client->subscribe(filter, static_cast<quint8> (ui.cbQos->currentText().toUInt()) );

						if(temp_subscription) {
							m_mqttSubscriptions.push_back(temp_subscription);
							connect(temp_subscription, &QMqttSubscription::messageReceived, this, &ImportFileWidget::mqttSubscriptionMessageReceived);
							m_mqttNewTopic = temp_subscription->topic().filter();
							emit subscriptionMade();
						}

						nameList.clear();
						nameList.append(commonTopic);
						newTopic->addChild(ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(oldTopic)));
						unsubscribeFromTopic(commonTopic);


						for(int i = 0; i < equalTopics.size(); ++i) {
							for(int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j){
								if(ui.twSubscriptions->topLevelItem(j)->text(0) == equalTopics[i]) {
									nameList.clear();
									nameList.append(equalTopics[i]);
									newTopic->addChild(ui.twSubscriptions->takeTopLevelItem(j));
									unsubscribeFromTopic(equalTopics[i]);
									break;
								}
							}
						}

						for(int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
							if(checkTopicContains(newCommonTopic, ui.twSubscriptions->topLevelItem(i)->text(0)) &&
									newCommonTopic != ui.twSubscriptions->topLevelItem(i)->text(0) ) {
								unsubscribeFromTopic(ui.twSubscriptions->topLevelItem(i)->text(0));
								i--;
							}
						}

						commonTopic = newCommonTopic;
					}
				} while(foundNewEqual);
			}

			if(foundSuperior) {
				QMessageBox::warning(this, "Warning", "You already subscribed to a topic containing this one");
			}
		}
		else
			QMessageBox::warning(this, "Warning", "You already subscribed to this topic");
	}
	else
		QMessageBox::warning(this, "Warning", "You didn't select any item from the Tree Widget");
}

void ImportFileWidget::mqttUnsubscribe() {
	QTreeWidgetItem* unsubscribeItem = ui.twSubscriptions->currentItem();

	if(unsubscribeItem != nullptr) {
		if(unsubscribeItem->parent() == nullptr)
			unsubscribeFromTopic(unsubscribeItem->text(0));
		else{
			while(unsubscribeItem->parent() != nullptr) {
				for(int i = 0; i < unsubscribeItem->parent()->childCount(); ++i) {
					if(unsubscribeItem->text(0) != unsubscribeItem->parent()->child(i)->text(0)) {
						QMqttTopicFilter filter {unsubscribeItem->parent()->child(i)->text(0)};
						QMqttSubscription *temp_subscription = m_client->subscribe(filter, static_cast<quint8> (ui.cbQos->currentText().toUInt()) );

						ui.twSubscriptions->addTopLevelItem(unsubscribeItem->parent()->takeChild(i));

						if(temp_subscription) {
							m_mqttSubscriptions.push_back(temp_subscription);
							connect(temp_subscription, &QMqttSubscription::messageReceived, this, &ImportFileWidget::mqttSubscriptionMessageReceived);
							m_mqttNewTopic = temp_subscription->topic().filter();
							emit subscriptionMade();
						}
						i--;
					}
				}
				unsubscribeItem = unsubscribeItem->parent();
			}
			unsubscribeFromTopic(unsubscribeItem->text(0));
		}
	}
	else
		QMessageBox::warning(this, "Warning", "You didn't select any item from the Tree Widget");
}

void ImportFileWidget::mqttMessageReceived(const QByteArray &message , const QMqttTopicName &topic) {
	if(!m_addedTopics.contains(topic.name())) {
		m_addedTopics.push_back(topic.name());
		QStringList name;
		QChar sep = '/';
		QString rootName;
		if(topic.name().contains(sep)) {
			QStringList list = topic.name().split(sep, QString::SkipEmptyParts);

			rootName = list.at(0);
			name.append(list.at(0));
			QTreeWidgetItem* currentItem;
			int topItemIdx = -1;
			for(int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
				if(ui.twTopics->topLevelItem(i)->text(0) == list.at(0)) {
					topItemIdx = i;
					break;
				}
			}
			if( topItemIdx < 0) {
				currentItem = new QTreeWidgetItem(name);
				ui.twTopics->addTopLevelItem(currentItem);
				for(int i = 1; i < list.size(); ++i) {
					name.clear();
					name.append(list.at(i));
					currentItem->addChild(new QTreeWidgetItem(name));
					currentItem = currentItem->child(0);
				}
			} else {
				currentItem = ui.twTopics->topLevelItem(topItemIdx);
				int listIdx = 1;
				for(; listIdx < list.size(); ++listIdx) {
					QTreeWidgetItem* childItem = nullptr;
					bool found = false;
					for(int j = 0; j < currentItem->childCount(); ++j) {
						childItem = currentItem->child(j);
						if(childItem->text(0) == list.at(listIdx)) {
							found = true;
							currentItem = childItem;
							break;
						}
					}
					if(!found)
						break;
				}

				for(; listIdx < list.size(); ++listIdx) {
					name.clear();
					name.append(list.at(listIdx));
					currentItem->addChild(new QTreeWidgetItem(name));
					currentItem = currentItem->child(currentItem->childCount() - 1);
				}
			}
		}
		else {
			rootName = topic.name();
			name.append(topic.name());
			ui.twTopics->addTopLevelItem(new QTreeWidgetItem(name));
		}
		emit newTopic(rootName);
	}
}

void ImportFileWidget::setCompleter(const QString& topic) {
	if(!m_searching) {
		if(!m_topicList.contains(topic)) {
			m_topicList.append(topic);
			m_completer = new QCompleter(m_topicList, this);
			m_completer->setCompletionMode(QCompleter::PopupCompletion);
			m_completer->setCaseSensitivity(Qt::CaseSensitive);
			ui.leTopics->setCompleter(m_completer);
		}
	}
}

void ImportFileWidget::topicTimeout() {
	qDebug()<<"lejart ido";
	m_searching = false;
	m_searchTimer->stop();
}

bool ImportFileWidget::isMqttValid(){
	bool connected = (m_client->state() == QMqttClient::ClientState::Connected);
	bool subscribed = (ui.twSubscriptions->topLevelItemCount() > 0);
	bool fileTypeOk = false;
	if(this->currentFileType() == AbstractFileFilter::FileType::Ascii)
		fileTypeOk = true;
	return connected && subscribed && fileTypeOk;
}

void ImportFileWidget::mqttSubscriptionMessageReceived(const QMqttMessage &msg) {
	qDebug()<<"message received from: "<<msg.topic().name();
	if(!m_subscribedTopicNames.contains(msg.topic().name())) {
		m_messageArrived[msg.topic()] = true;
		qDebug()<<msg.topic().name()<<"set true";
		m_subscribedTopicNames.push_back(msg.topic().name());
		emit newTopicForWill();
	}

	if(m_messageArrived[msg.topic()] == false) {
		qDebug()<<msg.topic().name()<<"set true";
		m_messageArrived[msg.topic()] = true;
	}

	m_lastMessage[msg.topic()]= msg;
	bool check = true;
	QMapIterator<QMqttTopicName, bool> i(m_messageArrived);
	while(i.hasNext()) {
		i.next();
		if(i.value() == false ) {
			qDebug()<<"Found false: "<<i.key().name();
			check = false;
			break;
		}
	}
	if (check == true)
		m_mqttReadyForPreview = true;

	if(m_mqttReadyForPreview && checkTopicContains(m_mqttNewTopic, msg.topic().name())) {
		qDebug() << "New topic for preview:  " << m_mqttNewTopic;
		m_mqttNewTopic.clear();
		refreshPreview();
	}
}

void ImportFileWidget::onMqttDisconnect() {
	ui.gbManageSubscriptions->setEnabled(false);
	ui.bConnect->setText("Connect");

	ui.leHost->setEnabled(true);
	ui.leHost->clear();

	ui.lePort->setEnabled(true);
	ui.lePort->clear();

	ui.lePassword->setEnabled(true);
	ui.lePassword->clear();

	ui.leUsername->setEnabled(true);
	ui.leUsername->clear();

	ui.leID->setEnabled(true);
	ui.leID->clear();

	ui.twSubscriptions->clear();
	ui.twTopics->clear();

	ui.chbRetain->setEnabled(true);
	ui.cbSourceType->setEnabled(true);
	ui.chbAuthentication->setEnabled(true);
	ui.chbID->setEnabled(true);

	m_mqttNewTopic.clear();
	m_mqttReadyForPreview = false;
	m_mqttSubscriptions.clear();

	m_completer = new QCompleter;

	m_topicList.clear();
	m_searching = false;
	m_searchTimer->stop();
	m_messageArrived.clear();
	m_lastMessage.clear();
}

void ImportFileWidget::useWillMessage(int state) {
	if(state == Qt::Checked) {
		ui.chbWillRetain->show();
		ui.cbWillQoS->show();
		ui.cbWillMessageType->show();
		ui.cbWillTopic->show();
		ui.cbWillUpdate->show();
		ui.lWillMessageType->show();
		ui.lWillQos->show();
		ui.lWillTopic->show();
		ui.lWillUpdate->show();

		if (ui.cbWillMessageType->currentIndex() == static_cast<int>(MQTTClient::WillMessageType::OwnMessage) ) {
			ui.leWillOwnMessage->show();
			ui.lWillOwnMessage->show();
		}
		else if(ui.cbWillMessageType->currentIndex() == static_cast<int>(MQTTClient::WillMessageType::Statistics) ){
			qDebug()<<"will use checked show statistics";
			ui.lWillStatistics->show();
			ui.lwWillStatistics->show();
		}


		if(ui.cbWillUpdate->currentIndex() == 0) {
			ui.leWillUpdateInterval->show();
			ui.lWillUpdateInterval->show();
		}
	}
	else if (state == Qt::Unchecked) {

		qDebug()<<"will use unchecked";
		ui.chbWillRetain->hide();
		ui.cbWillQoS->hide();
		ui.cbWillMessageType->hide();
		ui.cbWillTopic->hide();
		ui.cbWillUpdate->hide();
		ui.leWillOwnMessage->hide();
		ui.leWillUpdateInterval->hide();
		ui.lWillMessageType->hide();
		ui.lWillOwnMessage->hide();
		ui.lWillQos->hide();
		ui.lWillTopic->hide();
		ui.lWillUpdate->hide();
		ui.lWillUpdateInterval->hide();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	}
}

void ImportFileWidget::willMessageTypeChanged(int type) {
	if(static_cast<MQTTClient::WillMessageType> (type) == MQTTClient::WillMessageType::OwnMessage) {
		ui.leWillOwnMessage->show();
		ui.lWillOwnMessage->show();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	}
	else if(static_cast<MQTTClient::WillMessageType> (type) == MQTTClient::WillMessageType::LastMessage) {
		ui.leWillOwnMessage->hide();
		ui.lWillOwnMessage->hide();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	}
	else if(static_cast<MQTTClient::WillMessageType> (type) == MQTTClient::WillMessageType::Statistics) {
		qDebug()<<"will message type changed show statistics";
		ui.lWillStatistics->show();
		ui.lwWillStatistics->show();
		ui.leWillOwnMessage->hide();
		ui.lWillOwnMessage->hide();
	}
}

void ImportFileWidget::updateWillTopics() {
	ui.cbWillTopic->clear();
	for(int i = 0; i < m_subscribedTopicNames.size(); ++i) {
		ui.cbWillTopic->addItem(m_subscribedTopicNames[i]);
	}
}

void ImportFileWidget::willUpdateChanged(int updateType) {
	if(static_cast<MQTTClient::WillUpdateType>(updateType) == MQTTClient::WillUpdateType::TimePeriod) {
		ui.leWillUpdateInterval->show();
		ui.lWillUpdateInterval->show();
	}
	else if (static_cast<MQTTClient::WillUpdateType>(updateType) == MQTTClient::WillUpdateType::OnClick) {
		ui.leWillUpdateInterval->hide();
		ui.lWillUpdateInterval->hide();
	}
}
#endif

void ImportFileWidget::hideMQTT() {
	ui.leID->hide();
	ui.lMqttID->hide();
	ui.lePassword->hide();
	ui.lPassword->hide();
	ui.leUsername->hide();
	ui.lUsername->hide();
	ui.cbQos->hide();
	ui.lQos->hide();
	ui.twTopics->hide();
	//ui.lTopicTree->hide();
	ui.lTopicSearch->hide();
	ui.leTopics->hide();
	ui.twSubscriptions->hide();
	//ui.lSubscriptions->hide();
	ui.chbAuthentication->hide();
	ui.chbID->hide();
	ui.bSubscribe->hide();
	ui.bUnsubscribe->hide();
	ui.bConnect->hide();

	ui.gbMqttWill->hide();
	ui.chbWill->hide();
	ui.chbWillRetain->hide();
	ui.cbWillQoS->hide();
	ui.cbWillMessageType->hide();
	ui.cbWillTopic->hide();
	ui.cbWillUpdate->hide();
	ui.leWillOwnMessage->hide();
	ui.leWillUpdateInterval->setValidator(new QIntValidator(2, 1000000) );
	ui.leWillUpdateInterval->hide();
	ui.lWillMessageType->hide();
	ui.lWillOwnMessage->hide();
	ui.lWillQos->hide();
	ui.lWillTopic->hide();
	ui.lWillUpdate->hide();
	ui.lWillUpdateInterval->hide();
	ui.lwWillStatistics->hide();
	ui.lWillStatistics->hide();
}

#ifdef HAVE_MQTT
void ImportFileWidget::mqttErrorChanged(QMqttClient::ClientError clientError) {
	switch (clientError) {
	case QMqttClient::BadUsernameOrPassword:
		QMessageBox::warning(this, "Couldn't connect", "Bad username or password");		
		break;	
	case QMqttClient::IdRejected:
		QMessageBox::warning(this, "Couldn't connect", "The client ID wasn't accepted");
		break;
	case QMqttClient::ServerUnavailable:
		QMessageBox::warning(this, "Server unavailable", "The network connection has been established, but the service is unavailable on the broker side.");
		break;
	case QMqttClient::NotAuthorized:
		QMessageBox::warning(this, "Couldn't connect", "The client is not authorized to connect.");
		break;
	case QMqttClient::UnknownError:
		QMessageBox::warning(this, "Unknown MQTT error", "An unknown error occurred.");
		break;
	default:
		break;
	}
}

bool ImportFileWidget::checkTopicContains(const QString& superior, const QString& inferior) {
	if (superior == inferior)
		return true;
	else {
		if(superior.contains("/")) {
			QStringList superiorList = superior.split('/', QString::SkipEmptyParts);
			QStringList inferiorList = inferior.split('/', QString::SkipEmptyParts);

			if(superiorList.size() > inferiorList.size())
				return false;

			bool ok = true;
			for(int i = 0; i < superiorList.size(); ++i) {
				if(superiorList.at(i) != inferiorList.at(i)) {
					if((superiorList.at(i) != "+") &&
							!(superiorList.at(i) == "#" && i == superiorList.size() - 1)) {
						qDebug() <<superiorList.at(i)<<"  "<<inferiorList.at(i);
						ok = false;
						break;
					}
				}
			}
			return ok;
		}

		return false;
	}
}

QString ImportFileWidget::checkCommonLevel(const QString& first, const QString& second) {
	qDebug()<<first<<"  "<<second;
	QStringList firstList = first.split('/', QString::SkipEmptyParts);
	QStringList secondtList = second.split('/', QString::SkipEmptyParts);
	QString commonTopic = "";

	if(!firstList.isEmpty()) {
		if(firstList.size() == secondtList.size() && (first != second))	{
			int matchIndex = -1;
			for(int i = 0; i < firstList.size(); ++i) {
				if(firstList.at(i) != secondtList.at(i)) {
					matchIndex = i;
					break;
				}
			}
			bool differ = false;
			if(matchIndex > 0) {
				for(int j = matchIndex +1; j < firstList.size(); ++j) {
					if(firstList.at(j) != secondtList.at(j)) {
						differ = true;
						break;
					}
				}
			}
			else
				differ = true;

			if(!differ)
			{
				for(int i = 0; i < firstList.size(); ++i) {
					if(i != matchIndex)
						commonTopic.append(firstList.at(i));
					else
						commonTopic.append("+");

					if(i != firstList.size() - 1)
						commonTopic.append("/");
				}
			}
		}
	}
	qDebug() << "Common topic: "<<commonTopic;
	return commonTopic;
}

int ImportFileWidget::commonLevelIndex(const QString& first, const QString& second) {
	qDebug()<<first<<"  "<<second;
	QStringList firstList = first.split('/', QString::SkipEmptyParts);
	QStringList secondtList = second.split('/', QString::SkipEmptyParts);
	QString commonTopic = "";
	int matchIndex = -1;

	if(!firstList.isEmpty()) {
		if(firstList.size() == secondtList.size() && (first != second))	{
			for(int i = 0; i < firstList.size(); ++i) {
				if(firstList.at(i) != secondtList.at(i)) {
					matchIndex = i;
					break;
				}
			}
			bool differ = false;
			if(matchIndex > 0) {
				for(int j = matchIndex +1; j < firstList.size(); ++j) {
					if(firstList.at(j) != secondtList.at(j)) {
						differ = true;
						break;
					}
				}
			}
			else
				differ = true;

			if(!differ)
			{
				for(int i = 0; i < firstList.size(); ++i) {
					if(i != matchIndex)
						commonTopic.append(firstList.at(i));
					else
						commonTopic.append("+");

					if(i != firstList.size() - 1)
						commonTopic.append("/");
				}
			}
		}
	}

	qDebug() << "Common topic: "<<commonTopic;
	if(!commonTopic.isEmpty())
		return matchIndex;
	else
		return -1;
}

void ImportFileWidget::searchTreeItem(const QString& rootName) {
	m_searching = true;
	m_searchTimer->start();

	qDebug()<<rootName;
	int topItemIdx = -1;
	for(int i = 0; i< ui.twTopics->topLevelItemCount(); ++i)
		if(ui.twTopics->topLevelItem(i)->text(0) == rootName) {
			topItemIdx = i;
			break;
		}

	if(topItemIdx >= 0) {
		qDebug() << "Scroll";
		ui.twTopics->scrollToItem(ui.twTopics->topLevelItem(topItemIdx), QAbstractItemView::ScrollHint::PositionAtTop);
	}
}

void ImportFileWidget::unsubscribeFromTopic(const QString& topicName) {
	if(!topicName.isEmpty()) {
		QMqttTopicFilter filter{topicName};
		m_client->unsubscribe(filter);

		qDebug()<<"unsubscribe occured";

		for(int i = 0; i< m_mqttSubscriptions.count(); ++i)
			if(m_mqttSubscriptions[i]->topic().filter() == topicName) {
				qDebug()<<"1 subscription found at  "<<i <<"and removed";
				m_mqttSubscriptions.remove(i);
				break;
			}

		if(m_mqttNewTopic == topicName)
			m_mqttNewTopic.clear();
		m_mqttReadyForPreview = false;

		QMapIterator<QMqttTopicName, bool> i(m_messageArrived);
		while(i.hasNext()) {
			i.next();
			if(checkTopicContains(topicName, i.key().name())) {
				m_messageArrived.remove(i.key());
				qDebug()<<"2 subscription found at  "<<i.key() <<"and removed";
				break;
			}
		}

		QMapIterator<QMqttTopicName, QMqttMessage> j(m_lastMessage);
		while(j.hasNext()) {
			j.next();
			if(checkTopicContains(topicName, j.key().name())) {
				m_lastMessage.remove(j.key());
				qDebug()<<"3 subscription found at  "<<j.key() <<"and removed";
				break;
			}
		}

		for(int row = 0; row<ui.twSubscriptions->topLevelItemCount(); row++)  {
			if(ui.twSubscriptions->topLevelItem(row)->text(0) == topicName) {
				qDebug()<<"4 subscription found at  "<<ui.twSubscriptions->topLevelItem(row)->text(0) <<"and removed";
				ui.twSubscriptions->topLevelItem(row)->takeChildren();
				ui.twSubscriptions->takeTopLevelItem(row);
			}
		}

		for(int i = 0; i < m_subscribedTopicNames.size(); ++i) {
			if(checkTopicContains(topicName, m_subscribedTopicNames[i])) {
				m_subscribedTopicNames.remove(i);
				i--;
			}
		}

		for(int item = 0; item < ui.cbWillTopic->count(); ++item) {
			if(checkTopicContains(topicName, ui.cbWillTopic->itemText(item))) {
				ui.cbWillTopic->removeItem(item);
				item--;
			}
		}

		emit subscriptionMade();
		refreshPreview();
	}
}

void ImportFileWidget::addSubscriptionChildren(QTreeWidgetItem * topic, QTreeWidgetItem * subscription) {
	if(topic->childCount() > 0) {
		for(int i = 0; i < topic->childCount(); ++i) {
			QTreeWidgetItem* temp = topic->child(i);
			QString name;
			if(topic->child(i)->childCount() > 0) {
				name.append(temp->text(0) + "/#");
				while(temp->parent() != nullptr) {
					temp = temp->parent();
					name.prepend(temp->text(0) + "/");
				}

			} else {
				name.append(temp->text(0));
				while(temp->parent() != nullptr) {
					temp = temp->parent();
					name.prepend(temp->text(0) + "/");
				}
			}
			QStringList nameList;
			nameList.append(name);
			QTreeWidgetItem* childItem = new QTreeWidgetItem(nameList);
			subscription->addChild(childItem);
			addSubscriptionChildren(topic->child(i), childItem);
		}
	}
}

void ImportFileWidget::mqttTimeout() {
	m_client->disconnectFromHost();
	m_timeoutTimer->stop();
	QMessageBox::warning(this, "Warning", "Couldn't connect to the given broker");
}

void ImportFileWidget::checkConnectEnable() {
	bool authenticationUsed = ui.chbAuthentication->isChecked();
	bool idUsed = ui.chbID->isChecked();
	bool authenticationFilled = !ui.leUsername->text().isEmpty() && !ui.lePassword->text().isEmpty();
	bool idFilled = !ui.leID->text().isEmpty();
	bool authenticationOK = !authenticationUsed || (authenticationUsed && authenticationFilled);
	bool idOK = !idUsed || (idUsed && idFilled);
	bool hostOK = !ui.leHost->text().isEmpty();
	bool portOK = !ui.lePort->text().isEmpty();
	bool enable = authenticationOK && idOK && hostOK && portOK;
	ui.bConnect->setEnabled(enable);
}
#endif
