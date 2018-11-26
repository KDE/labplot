/***************************************************************************
File                 : ImportFileWidget.cpp
Project              : LabPlot
Description          : import file data widget
--------------------------------------------------------------------
Copyright            : (C) 2009-2018 Stefan Gerlach (stefan.gerlach@uni.kn)
Copyright            : (C) 2009-2018 Alexander Semke (alexander.semke@web.de)
Copyright            : (C) 2017-2018 Fabian Kristof (fkristofszabolcs@gmail.com)
Copyright            : (C) 2018 Kovacs Ferencz (kferike98@gmail.com)

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
#include "backend/datasources/filters/NgspiceRawBinaryFilter.h"
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
#include <QJsonDocument>
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

#ifdef HAVE_MQTT
#include "kdefrontend/widgets/MQTTWillSettingsWidget.h"
#include "MQTTConnectionManagerDialog.h"
#include "MQTTConnectionManagerWidget.h"
#include "backend/core/Project.h"
#include <QMqttClient>
#include <QMqttSubscription>
#include <QMqttTopicFilter>
#include <QMqttMessage>
#include <QMessageBox>
#include <QWidgetAction>
#include <QMenu>
#endif

/*!
   \class ImportFileWidget
   \brief Widget for importing data from a file.

   \ingroup kdefrontend
*/
ImportFileWidget::ImportFileWidget(QWidget* parent, bool liveDataSource, const QString& fileName) : QWidget(parent),
	m_asciiOptionsWidget(nullptr),
	m_binaryOptionsWidget(nullptr),
	m_hdf5OptionsWidget(nullptr),
	m_imageOptionsWidget(nullptr),
	m_netcdfOptionsWidget(nullptr),
	m_fitsOptionsWidget(nullptr),
	m_jsonOptionsWidget(nullptr),
	m_rootOptionsWidget(nullptr),
	m_fileName(fileName),
	m_fileEmpty(false),
	m_liveDataSource(liveDataSource),
	m_suppressRefresh(false)
#ifdef HAVE_MQTT
	,
	m_client(nullptr),
	m_searching(false),
	m_searchTimer(new QTimer(this)),
	m_connectTimeoutTimer(new QTimer(this)),
	m_mqttReadyForPreview (false),
	m_initialisingMQTT(false),
	m_connectionTimedOut(false)
#endif
{
	ui.setupUi(this);

	auto* completer = new QCompleter(this);
	completer->setModel(new QDirModel);
	ui.leFileName->setCompleter(completer);

	//add supported file types
	if (!liveDataSource) {
		ui.cbFileType->addItem(i18n("ASCII data"), AbstractFileFilter::Ascii);
		ui.cbFileType->addItem(i18n("Binary data"), AbstractFileFilter::Binary);
		ui.cbFileType->addItem(i18n("Image"), AbstractFileFilter::Image);
#ifdef HAVE_HDF5
		ui.cbFileType->addItem(i18n("Hierarchical Data Format 5 (HDF5)"), AbstractFileFilter::HDF5);
#endif
#ifdef HAVE_NETCDF
		ui.cbFileType->addItem(i18n("Network Common Data Format (NetCDF)"), AbstractFileFilter::NETCDF);
#endif
#ifdef HAVE_FITS
		ui.cbFileType->addItem(i18n("Flexible Image Transport System Data Format (FITS)"), AbstractFileFilter::FITS);
#endif
		ui.cbFileType->addItem(i18n("JSON data"), AbstractFileFilter::JSON);
#ifdef HAVE_ZIP
		ui.cbFileType->addItem(i18n("ROOT (CERN) Histograms"), AbstractFileFilter::ROOT);
#endif
		ui.cbFileType->addItem(i18n("Ngspice RAW ASCII"), AbstractFileFilter::NgspiceRawAscii);
		ui.cbFileType->addItem(i18n("Ngspice RAW Binary"), AbstractFileFilter::NgspiceRawBinary);

		//hide widgets relevant for live data reading only
		ui.lSourceType->hide();
		ui.cbSourceType->hide();
		ui.gbUpdateOptions->hide();
	} else {
		ui.cbFileType->addItem(i18n("ASCII data"), AbstractFileFilter::Ascii);
		ui.cbFileType->addItem(i18n("Binary data"), AbstractFileFilter::Binary);
		ui.cbFileType->addItem(i18n("Ngspice RAW ASCII"), AbstractFileFilter::NgspiceRawAscii);
		ui.cbFileType->addItem(i18n("Ngspice RAW Binary"), AbstractFileFilter::NgspiceRawBinary);

		ui.lePort->setValidator( new QIntValidator(ui.lePort) );
		ui.cbBaudRate->addItems(LiveDataSource::supportedBaudRates());
		ui.cbSerialPort->addItems(LiveDataSource::availablePorts());

		ui.tabWidget->removeTab(2);

#ifdef HAVE_MQTT
		m_searchTimer->setInterval(10000);
		m_connectTimeoutTimer->setInterval(6000);
#endif
	}

	QStringList filterItems {i18n("Automatic"), i18n("Custom")};
	ui.cbFilter->addItems(filterItems);

	//hide options that will be activated on demand
	ui.gbOptions->hide();
	ui.gbUpdateOptions->hide();
	setMQTTVisible(false);

	ui.cbReadingType->addItem(i18n("Whole file"), LiveDataSource::WholeFile);

	ui.bOpen->setIcon( QIcon::fromTheme(QLatin1String("document-open")) );
	ui.bFileInfo->setIcon( QIcon::fromTheme(QLatin1String("help-about")) );
	ui.bManageFilters->setIcon( QIcon::fromTheme(QLatin1String("configure")) );
	ui.bSaveFilter->setIcon( QIcon::fromTheme(QLatin1String("document-save")) );
	ui.bRefreshPreview->setIcon( QIcon::fromTheme(QLatin1String("view-refresh")) );

	ui.tvJson->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tvJson->setAlternatingRowColors(true);
	showJsonModel(false);

	// the table widget for preview
	m_twPreview = new QTableWidget(ui.tePreview);
	m_twPreview->verticalHeader()->hide();
	m_twPreview->setEditTriggers(QTableWidget::NoEditTriggers);
	auto* layout = new QHBoxLayout;
	layout->addWidget(m_twPreview);
	ui.tePreview->setLayout(layout);
	m_twPreview->hide();

#ifdef HAVE_MQTT
	ui.cbSourceType->addItem(QLatin1String("MQTT"));
	m_configPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() + QLatin1String("MQTT_connections");

	m_willSettings.MQTTUseWill = false;
	m_willSettings.willRetain = false;
	m_willSettings.willQoS = 0;
	m_willSettings.willMessageType = MQTTClient::WillMessageType::OwnMessage;
	m_willSettings.willUpdateType = MQTTClient::WillUpdateType::TimePeriod;
	m_willSettings.willTimeInterval = 10000;
	m_willSettings.willStatistics.fill(false, 15);

	ui.bSubscribe->setIcon(ui.bSubscribe->style()->standardIcon(QStyle::SP_ArrowRight));
	ui.bSubscribe->setToolTip(i18n("Subscribe selected topics"));
	ui.bUnsubscribe->setIcon(ui.bUnsubscribe->style()->standardIcon(QStyle::SP_ArrowLeft));
	ui.bUnsubscribe->setToolTip(i18n("Unsubscribe selected topics"));
	ui.bManageConnections->setIcon(QIcon::fromTheme(QLatin1String("network-server")));
	ui.bManageConnections->setToolTip(i18n("Manage MQTT connections"));
	ui.bWillMessage->setEnabled(false);
	ui.bWillMessage->setToolTip(i18n("Manage MQTT connection's will settings"));
	ui.bWillMessage->setIcon(ui.bWillMessage->style()->standardIcon(QStyle::SP_FileDialogDetailedView));
#endif

	//TODO: implement save/load of user-defined settings later and activate these buttons again
	ui.bSaveFilter->hide();
	ui.bManageFilters->hide();
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

	//read the source type first since settings in fileNameChanged() depend on this
	ui.cbSourceType->setCurrentIndex(conf.readEntry("SourceType").toInt());

	//general settings
	AbstractFileFilter::FileType fileType = static_cast<AbstractFileFilter::FileType>(conf.readEntry("Type", 0));
	for (int i = 0; i < ui.cbFileType->count(); ++i) {
		AbstractFileFilter::FileType itemFileType = static_cast<AbstractFileFilter::FileType>(ui.cbFileType->itemData(i).toInt());
		if (itemFileType == fileType) {
			ui.cbFileType->setCurrentIndex(i);
			break;
		}
	}

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
	//MQTT related settings
	//read available connections
	m_initialisingMQTT = true;
	readMQTTConnections();

	ui.cbConnection->setCurrentIndex(ui.cbConnection->findText(conf.readEntry("Connection", "")));

	m_willSettings.willRetain = conf.readEntry("mqttWillRetain").toInt();
	m_willSettings.willUpdateType = static_cast<MQTTClient::WillUpdateType>(conf.readEntry("mqttWillUpdateType").toInt());
	m_willSettings.willQoS = conf.readEntry("mqttWillQoS").toInt();
	m_willSettings.willOwnMessage = conf.readEntry("mqttWillOwnMessage","");
	m_willSettings.willTimeInterval = conf.readEntry("mqttWillUpdateInterval","").toInt();
	QString willStatistics = conf.readEntry("mqttWillStatistics","");
	QStringList statisticsList = willStatistics.split('|', QString::SplitBehavior::SkipEmptyParts);
	for (auto value : statisticsList) {
		m_willSettings.willStatistics[value.toInt()] = true;
	}
	m_willSettings.willMessageType = static_cast<MQTTClient::WillMessageType>(conf.readEntry("mqttWillMessageType").toInt());
	m_willSettings.MQTTUseWill = conf.readEntry("mqttWillUse").toInt();

	m_initialisingMQTT = false;
#endif

	//initialize the slots after all settings were set in order to avoid unneeded refreshes
	initSlots();

	//update the status of the widgets
	sourceTypeChanged(currentSourceType());
	fileTypeChanged(fileType);
	readingTypeChanged(ui.cbReadingType->currentIndex());

	//all set now, refresh the preview
	m_suppressRefresh = false;
	QTimer::singleShot(100, this, [=] () { refreshPreview(); });
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
	conf.writeEntry("Type", (int)currentFileType());
	conf.writeEntry("Filter", ui.cbFilter->currentIndex());
	conf.writeEntry("LastImportedFile", ui.leFileName->text());

	//live data related settings
	conf.writeEntry("SourceType", (int)currentSourceType());
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
	//MQTT related settings
	conf.writeEntry("Connection", ui.cbConnection->currentText());
	conf.writeEntry("mqttWillMessageType", static_cast<int>(m_willSettings.willMessageType));
	conf.writeEntry("mqttWillUpdateType", static_cast<int>(m_willSettings.willUpdateType));
	conf.writeEntry("mqttWillQoS", QString::number(m_willSettings.willQoS));
	conf.writeEntry("mqttWillOwnMessage", m_willSettings.willOwnMessage);
	conf.writeEntry("mqttWillUpdateInterval", QString::number(m_willSettings.willTimeInterval));
	QString willStatistics;
	for (int i = 0; i < m_willSettings.willStatistics.size(); ++i) {
		if (m_willSettings.willStatistics[i])
			willStatistics += QString::number(i)+"|";
	}
	conf.writeEntry("mqttWillStatistics", willStatistics);
	conf.writeEntry("mqttWillRetain", static_cast<int>(m_willSettings.willRetain));
	conf.writeEntry("mqttWillUse", static_cast<int>(m_willSettings.MQTTUseWill));
#endif

	// data type specific settings
	if (m_asciiOptionsWidget)
		m_asciiOptionsWidget->saveSettings();
	if (m_binaryOptionsWidget)
		m_binaryOptionsWidget->saveSettings();
	if (m_imageOptionsWidget)
		m_imageOptionsWidget->saveSettings();
	if (m_jsonOptionsWidget)
		m_jsonOptionsWidget->saveSettings();
}

void ImportFileWidget::initSlots() {
	//SLOTs for the general part of the data source configuration
	connect( ui.cbSourceType, SIGNAL(currentIndexChanged(int)), this, SLOT(sourceTypeChanged(int)));
	connect( ui.leFileName, SIGNAL(textChanged(QString)), SLOT(fileNameChanged(QString)) );
	connect(ui.leHost, SIGNAL(textChanged(QString)), this, SIGNAL(hostChanged()));
	connect(ui.lePort, SIGNAL(textChanged(QString)), this, SIGNAL(portChanged()));
	connect( ui.tvJson, SIGNAL(clicked(QModelIndex)), this, SLOT(refreshPreview()));
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
	connect(ui.cbConnection, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &ImportFileWidget::mqttConnectionChanged);
	connect(ui.bSubscribe,  &QPushButton::clicked, this, &ImportFileWidget::mqttSubscribe);
	connect(ui.bUnsubscribe, &QPushButton::clicked, this,&ImportFileWidget::mqttUnsubscribe);
	connect(this, &ImportFileWidget::newTopic, this, &ImportFileWidget::setTopicCompleter);
	connect(m_searchTimer, &QTimer::timeout, this, &ImportFileWidget::topicTimeout);
	connect(m_connectTimeoutTimer, &QTimer::timeout, this, &ImportFileWidget::mqttConnectTimeout);
	connect(ui.cbFileType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), [this]() {emit checkFileType();});
	connect(ui.leTopics, &QLineEdit::textChanged, this, &ImportFileWidget::scrollToTopicTreeItem);
	connect(ui.leSubscriptions, &QLineEdit::textChanged, this, &ImportFileWidget::scrollToSubsriptionTreeItem);
	connect(ui.bManageConnections, &QPushButton::clicked, this, &ImportFileWidget::showMQTTConnectionManager);
	connect(ui.bWillMessage, &QPushButton::clicked, this, &ImportFileWidget::showWillSettings);
#endif
}

void ImportFileWidget::showAsciiHeaderOptions(bool b) {
	if (m_asciiOptionsWidget)
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
	AbstractFileFilter::FileType fileType = currentFileType();
	auto updateType = static_cast<LiveDataSource::UpdateType>(ui.cbUpdateType->currentIndex());
	LiveDataSource::SourceType sourceType = currentSourceType();
	auto readingType = static_cast<LiveDataSource::ReadingType>(ui.cbReadingType->currentIndex());

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
		source->setFileName(ui.leFileName->text());
		source->setFileLinked(ui.chbLinkFile->isChecked());
		break;
	case LiveDataSource::SourceType::LocalSocket:
		source->setFileName(ui.leFileName->text());
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
	case LiveDataSource::SourceType::MQTT:
		break;
	default:
		break;
	}
}

#ifdef HAVE_MQTT
/*!
	saves the settings to the MQTTClient \c client.
*/
void ImportFileWidget::saveMQTTSettings(MQTTClient* client) const {
	DEBUG("ImportFileWidget::saveMQTTSettings");
	MQTTClient::UpdateType updateType = static_cast<MQTTClient::UpdateType>(ui.cbUpdateType->currentIndex());
	MQTTClient::ReadingType readingType = static_cast<MQTTClient::ReadingType>(ui.cbReadingType->currentIndex());

	client->setComment( ui.leFileName->text() );
	client->setFilter(this->currentFileFilter());

	client->setReadingType(readingType);

	if (updateType == MQTTClient::UpdateType::TimeInterval)
		client->setUpdateInterval(ui.sbUpdateInterval->value());

	client->setKeepNValues(ui.sbKeepNValues->value());

	client->setUpdateType(updateType);

	if (readingType != MQTTClient::ReadingType::TillEnd)
		client->setSampleSize(ui.sbSampleSize->value());

	client->setMQTTClientHostPort(m_client->hostname(), m_client->port());

	KConfig config(m_configPath, KConfig::SimpleConfig);
	KConfigGroup group = config.group(ui.cbConnection->currentText());

	bool useID = group.readEntry("UseID").toUInt();
	bool useAuthentication = group.readEntry("UseAuthentication").toUInt();

	client->setMQTTUseAuthentication(useAuthentication);
	if (useAuthentication)
		client->setMQTTClientAuthentication(m_client->username(), m_client->password());

	client->setMQTTUseID(useID);
	if (useID)
		client->setMQTTClientId(m_client->clientId());

	for (int i = 0; i < m_mqttSubscriptions.count(); ++i) {
		client->addInitialMQTTSubscriptions(m_mqttSubscriptions[i]->topic(), m_mqttSubscriptions[i]->qos());
	}

	bool retain = group.readEntry("Retain").toUInt();
	client->setMQTTRetain(retain);

	client->setWillSettings(m_willSettings);
}
#endif

/*!
	returns the currently used file type.
*/
AbstractFileFilter::FileType ImportFileWidget::currentFileType() const {
	return static_cast<AbstractFileFilter::FileType>( ui.cbFileType->itemData(ui.cbFileType->currentIndex()).toInt() );
}

LiveDataSource::SourceType ImportFileWidget::currentSourceType() const {
	return static_cast<LiveDataSource::SourceType>(ui.cbSourceType->currentIndex());
}

/*!
	returns the currently used filter.
*/
AbstractFileFilter* ImportFileWidget::currentFileFilter() const {
	DEBUG("ImportFileWidget::currentFileFilter()");
	AbstractFileFilter::FileType fileType = currentFileType();

	switch (fileType) {
	case AbstractFileFilter::Ascii: {
		DEBUG("	ASCII");
		//TODO			std::unique_ptr<AsciiFilter> filter(new AsciiFilter());
		auto* filter = new AsciiFilter();

		if (ui.cbFilter->currentIndex() == 0)     //"automatic"
			filter->setAutoModeEnabled(true);
		else if (ui.cbFilter->currentIndex() == 1) { //"custom"
			filter->setAutoModeEnabled(false);
			if (m_asciiOptionsWidget)
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
		auto* filter = new BinaryFilter();
		if ( ui.cbFilter->currentIndex() == 0 ) 	//"automatic"
			filter->setAutoModeEnabled(true);
		else if ( ui.cbFilter->currentIndex() == 1 ) {	//"custom"
			filter->setAutoModeEnabled(false);
			if (m_binaryOptionsWidget)
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
		auto* filter = new ImageFilter();

		filter->setImportFormat(m_imageOptionsWidget->currentFormat());
		filter->setStartRow( ui.sbStartRow->value() );
		filter->setEndRow( ui.sbEndRow->value() );
		filter->setStartColumn( ui.sbStartColumn->value() );
		filter->setEndColumn( ui.sbEndColumn->value() );

		return filter;
	}
	case AbstractFileFilter::HDF5: {
		auto* filter = new HDF5Filter();
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
		auto* filter = new NetCDFFilter();

		if (!selectedNetCDFNames().isEmpty())
			filter->setCurrentVarName(selectedNetCDFNames()[0]);
		filter->setStartRow( ui.sbStartRow->value() );
		filter->setEndRow( ui.sbEndRow->value() );
		filter->setStartColumn( ui.sbStartColumn->value() );
		filter->setEndColumn( ui.sbEndColumn->value() );

		return filter;
	}
	case AbstractFileFilter::FITS: {
		auto* filter = new FITSFilter();
		filter->setStartRow( ui.sbStartRow->value());
		filter->setEndRow( ui.sbEndRow->value() );
		filter->setStartColumn( ui.sbStartColumn->value());
		filter->setEndColumn( ui.sbEndColumn->value());
		return filter;
	}
	case AbstractFileFilter::JSON: {
		auto* filter = new JsonFilter();
		m_jsonOptionsWidget->applyFilterSettings(filter, ui.tvJson->currentIndex());

		filter->setStartRow( ui.sbStartRow->value() );
		filter->setEndRow( ui.sbEndRow->value() );
		filter->setStartColumn( ui.sbStartColumn->value());
		filter->setEndColumn( ui.sbEndColumn->value());
		return filter;
	}
	case AbstractFileFilter::ROOT: {
		auto* filter = new ROOTFilter();
		QStringList names = selectedROOTNames();
		if (!names.isEmpty())
			filter->setCurrentHistogram(names.first());

		filter->setStartBin( m_rootOptionsWidget->startBin() );
		filter->setEndBin( m_rootOptionsWidget->endBin() );
		filter->setColumns( m_rootOptionsWidget->columns() );

		return filter;
	}
	case AbstractFileFilter::NgspiceRawAscii: {
		auto* filter = new NgspiceRawAsciiFilter();
		filter->setStartRow( ui.sbStartRow->value() );
		filter->setEndRow( ui.sbEndRow->value() );
		return filter;
	}
	case AbstractFileFilter::NgspiceRawBinary: {
		auto* filter = new NgspiceRawBinaryFilter();
		filter->setStartRow( ui.sbStartRow->value() );
		filter->setEndRow( ui.sbEndRow->value() );
		return filter;
	}
	}

	return nullptr;
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

	//process all events after the FileDialog was closed to repaint the widget
	//before we start calculating the preview
	QApplication::processEvents(QEventLoop::AllEvents, 0);

	ui.leFileName->setText(path);

	//TODO: decide whether the selection of several files should be possible
	// 	QStringList filelist = QFileDialog::getOpenFileNames(this,i18n("Select one or more files to open"));
	// 	if (! filelist.isEmpty() )
	// 		ui.leFileName->setText(filelist.join(";"));
}

/*!
	hides the MQTT related items of the widget
*/
void ImportFileWidget::setMQTTVisible(bool visible) {
	ui.lConnections->setVisible(visible);
	ui.cbConnection->setVisible(visible);
	ui.bManageConnections->setVisible(visible);
	ui.cbQos->setVisible(visible);
	ui.lQos->setVisible(visible);

	//topics
	if (ui.cbConnection->currentIndex() != -1 && visible) {
		ui.lTopics->setVisible(true);
		ui.gbManageSubscriptions->setVisible(true);
	} else {
		ui.lTopics->setVisible(false);
		ui.gbManageSubscriptions->setVisible(false);
	}

	//will message
	ui.lWillMessage->setVisible(visible);
	ui.bWillMessage->setVisible(visible);
}

#ifdef HAVE_MQTT
/*!
 * returns \c true if there is a valid connection to an MQTT broker and the user has subscribed to at least 1 topic,
 * returns \c false otherwise.
 */
bool ImportFileWidget::isMqttValid() {
	bool connected = (m_client->state() == QMqttClient::ClientState::Connected);
	bool subscribed = (ui.twSubscriptions->topLevelItemCount() > 0);
	bool fileTypeOk = false;
	if (this->currentFileType() == AbstractFileFilter::FileType::Ascii)
		fileTypeOk = true;
	return connected && subscribed && fileTypeOk;
}

/*!
 *\brief Checks if a topic contains another one
 *
 * \param superior the name of a topic
 * \param inferior the name of a topic
 * \return	true if superior is equal to or contains(if superior contains wildcards) inferior,
 *			false otherwise
 */
bool ImportFileWidget::checkTopicContains(const QString& superior, const QString& inferior) {
	if (superior == inferior)
		return true;

	if (!superior.contains('/'))
		return false;

	const QStringList& superiorList = superior.split('/', QString::SkipEmptyParts);
	const QStringList& inferiorList = inferior.split('/', QString::SkipEmptyParts);

	//a longer topic can't contain a shorter one
	if (superiorList.size() > inferiorList.size())
		return false;

	bool ok = true;
	for (int i = 0; i < superiorList.size(); ++i) {
		if (superiorList.at(i) != inferiorList.at(i)) {
			if ((superiorList.at(i) != '+') &&
					!(superiorList.at(i) == "#" && i == superiorList.size() - 1)) {
				//if the two topics differ, and the superior's current level isn't + or #(which can be only in the last position)
				//then superior can't contain inferior
				ok = false;
				break;
			} else if (i == superiorList.size() - 1 && (superiorList.at(i) == '+' && inferiorList.at(i) == "#") ) {
				//if the two topics differ at the last level
				//and the superior's current level is + while the inferior's is #(which can be only in the last position)
				//then superior can't contain inferior
				ok = false;
				break;
			}
		}
	}
	return ok;
}

/*!
 *\brief Returns the '+' wildcard containing topic name, which includes the given topic names
 *
 * \param first the name of a topic
 * \param second the name of a topic
 * \return The name of the common topic, if it exists, otherwise ""
 */
QString ImportFileWidget::checkCommonLevel(const QString& first, const QString& second) {
	const QStringList& firstList = first.split('/', QString::SkipEmptyParts);
	if (firstList.isEmpty())
		return QString();

	const QStringList& secondtList = second.split('/', QString::SkipEmptyParts);
	QString commonTopic = "";

	//the two topics have to be the same size and can't be identic
	if (firstList.size() == secondtList.size() && (first != second))	{

		//the index where they differ
		int differIndex = -1;
		for (int i = 0; i < firstList.size(); ++i) {
			if (firstList.at(i) != secondtList.at(i)) {
				differIndex = i;
				break;
			}
		}

		//they can differ at only one level
		bool differ = false;
		if (differIndex > 0) {
			for (int j = differIndex + 1; j < firstList.size(); ++j) {
				if (firstList.at(j) != secondtList.at(j)) {
					differ = true;
					break;
				}
			}
		} else
			differ = true;

		if (!differ) {
			for (int i = 0; i < firstList.size(); ++i) {
				if (i != differIndex) {
					commonTopic.append(firstList.at(i));
				} else {
					//we put '+' wildcard at the level where they differ
					commonTopic.append('+');
				}

				if (i != firstList.size() - 1)
					commonTopic.append('/');
			}
		}
	}

	qDebug() << "Common topic for " << first << " and " << second << " is: " << commonTopic;
	return commonTopic;
}

/*!
 *\brief Returns the index of level where the two topic names differ, if there is a common topic for them
 *
 * \param first the name of a topic
 * \param second the name of a topic
 * \return The index of the unequal level, if there is a common topic, otherwise -1
 */
int ImportFileWidget::commonLevelIndex(const QString& first, const QString& second) {
	QStringList firstList = first.split('/', QString::SkipEmptyParts);
	QStringList secondtList = second.split('/', QString::SkipEmptyParts);
	QString commonTopic = "";
	int differIndex = -1;

	if (!firstList.isEmpty()) {
		//the two topics have to be the same size and can't be identic
		if (firstList.size() == secondtList.size() && (first != second))	{

			//the index where they differ
			for (int i = 0; i < firstList.size(); ++i) {
				if (firstList.at(i) != secondtList.at(i)) {
					differIndex = i;
					break;
				}
			}

			//they can differ at only one level
			bool differ = false;
			if (differIndex > 0) {
				for (int j = differIndex + 1; j < firstList.size(); ++j) {
					if (firstList.at(j) != secondtList.at(j)) {
						differ = true;
						break;
					}
				}
			}
			else
				differ = true;

			if (!differ) {
				for (int i = 0; i < firstList.size(); ++i) {
					if (i != differIndex)
						commonTopic.append(firstList.at(i));
					else
						commonTopic.append('+');

					if (i != firstList.size() - 1)
						commonTopic.append('/');
				}
			}
		}
	}

	//if there is a common topic we return the differIndex
	if (!commonTopic.isEmpty())
		return differIndex;
	else
		return -1;
}

/*!
 *\brief Unsubscribes from the given topic, and removes any data connected to it
 *
 * \param topicName the name of a topic we want to unsubscribe from
 */
void ImportFileWidget::unsubscribeFromTopic(const QString& topicName) {
	if (topicName.isEmpty())
		return;

	QMqttTopicFilter filter{topicName};
	m_client->unsubscribe(filter);

	for (int i = 0; i< m_mqttSubscriptions.count(); ++i)
		if (m_mqttSubscriptions[i]->topic().filter() == topicName) {
			m_mqttSubscriptions.remove(i);
			break;
		}

	m_mqttReadyForPreview = false;

	QMapIterator<QMqttTopicName, bool> i(m_messageArrived);
	while(i.hasNext()) {
		i.next();
		if (checkTopicContains(topicName, i.key().name()))
			m_messageArrived.remove(i.key());
	}

	QMapIterator<QMqttTopicName, QMqttMessage> j(m_lastMessage);
	while(j.hasNext()) {
		j.next();
		if (checkTopicContains(topicName, j.key().name()))
			m_lastMessage.remove(j.key());
	}

	for (int row = 0; row < ui.twSubscriptions->topLevelItemCount(); row++)  {
		if (ui.twSubscriptions->topLevelItem(row)->text(0) == topicName) {
			ui.twSubscriptions->topLevelItem(row)->takeChildren();
			ui.twSubscriptions->takeTopLevelItem(row);
		}
	}

	for (int i = 0; i < m_subscribedTopicNames.size(); ++i) {
		if (checkTopicContains(topicName, m_subscribedTopicNames[i])) {
			m_subscribedTopicNames.remove(i);
			i--;
		}
	}

	if (m_willSettings.willTopic == topicName) {
		QVector<QTreeWidgetItem*> children;
		if (ui.twSubscriptions->topLevelItemCount() > 0) {
			findSubscriptionLeafChildren(children, ui.twSubscriptions->topLevelItem(0));
			m_willSettings.willTopic = children[0]->text(0);
		} else
			m_willSettings.willTopic = "";
	}

	//signals that there was a change among the subscribed topics
	emit subscriptionsChanged();
	refreshPreview();
}

/*!
 *\brief Adds to a # wildcard containing topic, every topic present in twTopics that the former topic contains
 *
 * \param topic pointer to the TreeWidgetItem which was selected before subscribing
 * \param subscription pointer to the TreeWidgetItem which represents the new subscirption,
 *		  we add all of the children to this item
 */
void ImportFileWidget::addSubscriptionChildren(QTreeWidgetItem * topic, QTreeWidgetItem * subscription) {
	//if the topic doesn't have any children we don't do anything
	if (topic->childCount() <= 0)
		return;

	for (int i = 0; i < topic->childCount(); ++i) {
		QTreeWidgetItem* temp = topic->child(i);
		QString name;
		//if it has children, then we add it as a # wildcrad containing topic
		if (topic->child(i)->childCount() > 0) {
			name.append(temp->text(0) + "/#");
			while(temp->parent() != nullptr) {
				temp = temp->parent();
				name.prepend(temp->text(0) + '/');
			}
		}

		//if not then we simply add the topic itself
		else {
			name.append(temp->text(0));
			while(temp->parent() != nullptr) {
				temp = temp->parent();
				name.prepend(temp->text(0) + '/');
			}
		}

		QStringList nameList;
		nameList.append(name);
		QTreeWidgetItem* childItem = new QTreeWidgetItem(nameList);
		subscription->addChild(childItem);
		//we use the function recursively on the given item
		addSubscriptionChildren(topic->child(i), childItem);
	}
}

/*!
 *\brief Fills the children vector, with the root item's (twSubscriptions) leaf children (meaning no wildcard containing topics)
 *
 * \param children vector of TreeWidgetItem pointers
 * \param root pointer to a TreeWidgetItem of twSubscriptions
 */
void ImportFileWidget::findSubscriptionLeafChildren(QVector<QTreeWidgetItem*>& children, QTreeWidgetItem* root) {
	if (root->childCount() == 0)
		children.push_back(root);
	else
		for (int i = 0; i < root->childCount(); ++i)
			findSubscriptionLeafChildren(children, root->child(i));
}

/*!
 *\brief Returns the amount of topics that the '+' wildcard will replace in the level position
 *
 * \param levelIdx the level currently being investigated
 * \param level the level where the new + wildcard will be placed
 * \param commonList the topic name split into levels
 * \param currentItem pointer to a TreeWidgetItem which represents the parent of the level
 *		  represented by levelIdx
 * \return returns the childCount, or -1 if some topics already represented by + wildcard have different
 *		   amount of children
 */
int ImportFileWidget::checkCommonChildCount(int levelIdx, int level, QStringList& commonList, QTreeWidgetItem* currentItem) {
	//we recursively check the number of children, until we get to level-1
	if (levelIdx < level - 1) {
		if (commonList[levelIdx] != '+') {
			for (int j = 0; j < currentItem->childCount(); ++j) {
				if (currentItem->child(j)->text(0) == commonList[levelIdx]) {
					//if the level isn't represented by + wildcard we simply return the amount of children of the corresponding item, recursively
					return checkCommonChildCount(levelIdx + 1, level, commonList, currentItem->child(j));
				}
			}
		} else {
			int childCount = -1;
			bool ok = true;

			//otherwise we check if every + wildcard represented topic has the same number of children, recursively
			for (int j = 0; j < currentItem->childCount(); ++j) {
				int temp = checkCommonChildCount(levelIdx + 1, level, commonList, currentItem->child(j));
				if ((j > 0) && (temp != childCount)) {
					ok = false;
					break;
				}
				childCount = temp;
			}

			//if yes we return this number, otherwise -1
			if (ok)
				return childCount;
			else
				return -1;
		}
	} else if (levelIdx == level - 1) {
		if (commonList[levelIdx] != '+') {
			for (int j = 0; j < currentItem->childCount(); ++j) {
				if (currentItem->child(j)->text(0) == commonList[levelIdx]) {
					//if the level isn't represented by + wildcard we simply return the amount of children of the corresponding item
					return currentItem->child(j)->childCount();
				}
			}
		} else {
			int childCount = -1;
			bool ok = true;

			//otherwise we check if every + wildcard represented topic has the same number of children
			for (int j = 0; j < currentItem->childCount(); ++j) {
				if ((j > 0) && (currentItem->child(j)->childCount() != childCount)) {
					ok = false;
					break;
				}
				childCount = currentItem->child(j)->childCount();
			}

			//if yes we return this number, otherwise -1
			if (ok)
				return childCount;
			else
				return -1;
		}

	} else if (level == 1 && levelIdx == 1)
		return currentItem->childCount();

	return -1;
}

/*!
 *\brief We search in twSubscriptions for topics that can be represented using + wildcards, then merge them.
 *		 We do this until there are no topics to merge
 */
void ImportFileWidget::manageCommonLevelSubscriptions() {
	bool foundEqual = false;

	do{
		foundEqual = false;
		QMap<QString, QVector<QString>> equalTopicsMap;
		QVector<QString> equalTopics;

		//compare the subscriptions present in the TreeWidget
		for (int i = 0; i < ui.twSubscriptions->topLevelItemCount() - 1; ++i) {
			for (int j = i + 1; j < ui.twSubscriptions->topLevelItemCount(); ++j) {
				QString commonTopic = checkCommonLevel(ui.twSubscriptions->topLevelItem(i)->text(0), ui.twSubscriptions->topLevelItem(j)->text(0));

				//if there is a common topic for the 2 compared topics, we add them to the map (using the common topic as key)
				if (!commonTopic.isEmpty()) {
					if (!equalTopicsMap[commonTopic].contains(ui.twSubscriptions->topLevelItem(i)->text(0))) {
						equalTopicsMap[commonTopic].push_back(ui.twSubscriptions->topLevelItem(i)->text(0));
					}

					if (!equalTopicsMap[commonTopic].contains(ui.twSubscriptions->topLevelItem(j)->text(0))) {
						equalTopicsMap[commonTopic].push_back(ui.twSubscriptions->topLevelItem(j)->text(0));
					}
				}
			}
		}

		if (!equalTopicsMap.isEmpty()) {
			qDebug()<<"Manage common topics";

			QVector<QString> commonTopics;
			QMapIterator<QString, QVector<QString>> topics(equalTopicsMap);

			//check for every map entry, if the found topics can be merged or not
			while(topics.hasNext()) {
				topics.next();

				int level = commonLevelIndex(topics.value().last(), topics.value().first());
				QStringList commonList = topics.value().first().split('/', QString::SkipEmptyParts);
				QTreeWidgetItem* currentItem = nullptr;
				//search the corresponding item to the common topics first level(root)
				for (int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
					if (ui.twTopics->topLevelItem(i)->text(0) == commonList.first()) {
						currentItem = ui.twTopics->topLevelItem(i);
						break;
					}
				}

				//calculate the number of topics the new + wildcard could replace
				int childCount = checkCommonChildCount(1, level, commonList, currentItem);
				if (childCount > 0) {
					//if the number of topics found and the calculated number of topics is equal, the topics can be merged
					if (topics.value().size() == childCount) {
						qDebug() << "Found common topic to manage: " << topics.key();
						foundEqual = true;
						commonTopics.push_back(topics.key());
					}
				}
			}

			if (foundEqual) {
				//if there are more common topics, the topics of which can be merged, we choose the one which has the lowest level new '+' wildcard
				int lowestLevel = INT_MAX;
				int topicIdx = -1;
				for (int i = 0; i < commonTopics.size(); ++i) {
					int level = commonLevelIndex(equalTopicsMap[commonTopics[i]].first(), commonTopics[i]);
					if (level < lowestLevel) {
						topicIdx = i;
						lowestLevel = level;
					}
				}
				qDebug() << "Manage: " << commonTopics[topicIdx];
				equalTopics.append(equalTopicsMap[commonTopics[topicIdx]]);

				//Add the common topic ("merging")
				QString commonTopic;
				commonTopic = checkCommonLevel(equalTopics.first(), equalTopics.last());
				QStringList nameList;
				nameList.append(commonTopic);
				QTreeWidgetItem* newTopic = new QTreeWidgetItem(nameList);
				ui.twSubscriptions->addTopLevelItem(newTopic);
				QMqttTopicFilter filter {commonTopic};
				QMqttSubscription *temp_subscription = m_client->subscribe(filter, static_cast<quint8> (ui.cbQos->currentText().toUInt()) );

				if (temp_subscription) {
					m_mqttSubscriptions.push_back(temp_subscription);
					connect(temp_subscription, &QMqttSubscription::messageReceived, this, &ImportFileWidget::mqttSubscriptionMessageReceived);
					emit subscriptionsChanged();
				}

				//remove the "merged" topics
				for (int i = 0; i < equalTopics.size(); ++i) {
					for (int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j) {
						if (ui.twSubscriptions->topLevelItem(j)->text(0) == equalTopics[i]) {
							newTopic->addChild(ui.twSubscriptions->takeTopLevelItem(j));
							unsubscribeFromTopic(equalTopics[i]);
							break;
						}
					}
				}

				//remove any subscription that the new subscription contains
				for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
					if (checkTopicContains(commonTopic, ui.twSubscriptions->topLevelItem(i)->text(0)) &&
							commonTopic != ui.twSubscriptions->topLevelItem(i)->text(0) ) {
						unsubscribeFromTopic(ui.twSubscriptions->topLevelItem(i)->text(0));
						i--;
					}
				}
			}
		}
	} while(foundEqual);
}

/*!
 *\brief Fills twSubscriptions with the subscriptions made by the client
 */
void ImportFileWidget::updateSubscriptionTree() {
	DEBUG("ImportFileWidget::updateSubscriptionTree()");
	ui.twSubscriptions->clear();

	for (int i = 0; i < m_mqttSubscriptions.size(); ++i) {
		QStringList name;
		name.append(m_mqttSubscriptions[i]->topic().filter());

		bool found = false;
		for (int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j) {
			if (ui.twSubscriptions->topLevelItem(j)->text(0) == m_mqttSubscriptions[i]->topic().filter()) {
				found = true;
				break;
			}
		}

		if (!found) {
			//Add the subscription to the tree widget
			QTreeWidgetItem* newItem = new QTreeWidgetItem(name);
			ui.twSubscriptions->addTopLevelItem(newItem);
			name.clear();
			name = m_mqttSubscriptions[i]->topic().filter().split('/', QString::SkipEmptyParts);

			//find the corresponding "root" item in twTopics
			QTreeWidgetItem* topic = nullptr;
			for (int j = 0; j < ui.twTopics->topLevelItemCount(); ++j) {
				if (ui.twTopics->topLevelItem(j)->text(0) == name[0]) {
					topic = ui.twTopics->topLevelItem(j);
					break;
				}
			}

			//restore the children of the subscription
			if (topic != nullptr && topic->childCount() > 0) {
				restoreSubscriptionChildren(topic, newItem, name, 1);
			}
		}
	}
	m_searching = false;
}

/*!
 *\brief Restores the children of a top level item in twSubscriptions if it contains wildcards
 *
 * \param topic pointer to a top level item in twTopics which represents the root of the subscription topic
 * \param subscription pointer to a top level item in twSubscriptions, this is the item whose children will be restored
 * \param list QStringList containing the levels of the subscription topic
 * \param level the level's number which is being investigated
 */
void ImportFileWidget::restoreSubscriptionChildren(QTreeWidgetItem * topic, QTreeWidgetItem * subscription, const QStringList& list, int level) {
	DEBUG("ImportFileWidget::restoreSubscriptionChildren");
	if (list[level] != '+' && list[level] != "#" && level < list.size() - 1) {
		for (int i = 0; i < topic->childCount(); ++i) {
			//if the current level isn't + or # wildcard we recursively continue with the next level
			if (topic->child(i)->text(0) == list[level]) {
				restoreSubscriptionChildren(topic->child(i), subscription, list, level + 1);
				break;
			}
		}
	} else if (list[level] == '+') {
		for (int i = 0; i < topic->childCount(); ++i) {
			//determine the name of the topic, contained by the subscription
			QString name;
			name.append(topic->child(i)->text(0));
			for (int j = level + 1; j < list.size(); ++j) {
				name.append('/' + list[j]);
			}
			QTreeWidgetItem* temp = topic->child(i);
			while(temp->parent() != nullptr) {
				temp = temp->parent();
				name.prepend(temp->text(0) + '/');
			}

			//Add the topic as child of the subscription
			QStringList nameList;
			nameList.append(name);
			QTreeWidgetItem* newItem = new QTreeWidgetItem(nameList);
			subscription->addChild(newItem);
			//Continue adding children recursively to the new item
			restoreSubscriptionChildren(topic->child(i), newItem, list, level + 1);
		}
	} else if (list[level] == "#") {
		//add the children of the # wildcard containing subscription
		addSubscriptionChildren(topic, subscription);
	}
}

/*!
 *\brief Updates the completer for leSubscriptions
 */
void ImportFileWidget::updateSubscriptionCompleter() {
	QStringList subscriptionList;
	for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i)
		subscriptionList.append(ui.twSubscriptions->topLevelItem(i)->text(0));

	if (!subscriptionList.isEmpty()) {
		m_subscriptionCompleter = new QCompleter(subscriptionList, this);
		m_subscriptionCompleter->setCompletionMode(QCompleter::PopupCompletion);
		m_subscriptionCompleter->setCaseSensitivity(Qt::CaseSensitive);
		ui.leSubscriptions->setCompleter(m_subscriptionCompleter);
	} else
		ui.leSubscriptions->setCompleter(nullptr);
}
#endif

/************** SLOTS **************************************************************/

QString absolutePath(const QString& fileName)
{
#ifndef HAVE_WINDOWS
	// make absolute path // FIXME
	if (!fileName.isEmpty() && fileName.at(0) != QDir::separator())
		return QDir::homePath() + QDir::separator() + fileName;
#endif
	return fileName;
}

/*!
	called on file name changes.
	Determines the file format (ASCII, binary etc.), if the file exists,
	and activates the corresponding options.
*/
void ImportFileWidget::fileNameChanged(const QString& name) {
	DEBUG("ImportFileWidget::fileNameChanged()");
	const QString fileName = absolutePath(name);

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
		//TODO:
// 		m_hdf5OptionsWidget->clear();
// 		m_netcdfOptionsWidget->clear();
// 		m_fitsOptionsWidget->clear();
// 		m_jsonOptionsWidget->clearModel();
// 		m_rootOptionsWidget->clear();

		emit fileNameChanged();
		return;
	}

	if (currentSourceType() == LiveDataSource::FileOrPipe) {
		const AbstractFileFilter::FileType fileType = AbstractFileFilter::fileType(fileName);
		for (int i = 0; i < ui.cbFileType->count(); ++i) {
			AbstractFileFilter::FileType itemFileType = static_cast<AbstractFileFilter::FileType>(ui.cbFileType->itemData(i).toInt());
			if (itemFileType == fileType) {
				ui.cbFileType->setCurrentIndex(i);
				break;
			}
		}
		//TODO: updateContent(fileName, fileType);
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
void ImportFileWidget::fileTypeChanged(int index) {
	Q_UNUSED(index);
	AbstractFileFilter::FileType fileType = currentFileType();//(AbstractFileFilter::FileType)ui.cbFileType->itemData(index).toInt();
	DEBUG("ImportFileWidget::fileTypeChanged " << ENUM_TO_STRING(AbstractFileFilter, FileType, fileType));
	initOptionsWidget(fileType);

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
		ui.lFilter->hide();
		ui.cbFilter->hide();
		ui.lPreviewLines->hide();
		ui.sbPreviewLines->hide();
		break;
	case AbstractFileFilter::NgspiceRawAscii:
	case AbstractFileFilter::NgspiceRawBinary:
		ui.lFilter->hide();
		ui.cbFilter->hide();
		ui.lStartColumn->hide();
		ui.sbStartColumn->hide();
		ui.lEndColumn->hide();
		ui.sbEndColumn->hide();
		ui.tabWidget->removeTab(0);
		ui.tabWidget->setCurrentIndex(0);
		break;
	case AbstractFileFilter::JSON:
		ui.lFilter->hide();
		ui.cbFilter->hide();
		showJsonModel(true);
		break;
	default:
		DEBUG("unknown file type");
	}

	int lastUsedFilterIndex = ui.cbFilter->currentIndex();
	ui.cbFilter->clear();
	ui.cbFilter->addItem( i18n("Automatic") );
	ui.cbFilter->addItem( i18n("Custom") );

	//TODO: populate the combobox with the available pre-defined filter settings for the selected type
	ui.cbFilter->setCurrentIndex(lastUsedFilterIndex);
	filterChanged(lastUsedFilterIndex);

	if (currentSourceType() == LiveDataSource::FileOrPipe) {
		const QString fileName = absolutePath(ui.leFileName->text());

		if (QFile::exists(fileName))
			updateContent(fileName, static_cast<AbstractFileFilter::FileType>(fileType));
	}

	//for file types other than ASCII and binary we support re-reading the whole file only
	//select "read whole file" and deactivate the combobox
	if (m_liveDataSource && (fileType != AbstractFileFilter::Ascii && fileType != AbstractFileFilter::Binary)) {
		ui.cbReadingType->setCurrentIndex(3);
		ui.cbReadingType->setEnabled(false);
	} else
		ui.cbReadingType->setEnabled(true);

	refreshPreview();
}

// file type specific option widgets
void ImportFileWidget::initOptionsWidget(AbstractFileFilter::FileType fileType) {
	DEBUG("ImportFileWidget::initOptionsWidget for " << ENUM_TO_STRING(AbstractFileFilter, FileType, fileType));
	switch(fileType) {
	case AbstractFileFilter::Ascii:
		if (!m_asciiOptionsWidget) {
			QWidget* asciiw = new QWidget();
			m_asciiOptionsWidget = std::unique_ptr<AsciiOptionsWidget>(new AsciiOptionsWidget(asciiw));
			m_asciiOptionsWidget->loadSettings();
			ui.swOptions->addWidget(asciiw);
		}
		ui.swOptions->setCurrentWidget(m_asciiOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::Binary:
		if (!m_binaryOptionsWidget) {
			QWidget* binaryw = new QWidget();
			m_binaryOptionsWidget = std::unique_ptr<BinaryOptionsWidget>(new BinaryOptionsWidget(binaryw));
			ui.swOptions->addWidget(binaryw);
			m_binaryOptionsWidget->loadSettings();
		}
		ui.swOptions->setCurrentWidget(m_binaryOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::Image:
		if (!m_imageOptionsWidget) {
			QWidget* imagew = new QWidget();
			m_imageOptionsWidget = std::unique_ptr<ImageOptionsWidget>(new ImageOptionsWidget(imagew));
			ui.swOptions->addWidget(imagew);
			m_imageOptionsWidget->loadSettings();
		}
		ui.swOptions->setCurrentWidget(m_imageOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::HDF5:
		if (!m_hdf5OptionsWidget) {
			QWidget* hdf5w = new QWidget();
			m_hdf5OptionsWidget = std::unique_ptr<HDF5OptionsWidget>(new HDF5OptionsWidget(hdf5w, this));
			ui.swOptions->addWidget(hdf5w);
		} else
			m_hdf5OptionsWidget->clear();
		ui.swOptions->setCurrentWidget(m_hdf5OptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::NETCDF:
		if (!m_netcdfOptionsWidget) {
			QWidget* netcdfw = new QWidget();
			m_netcdfOptionsWidget = std::unique_ptr<NetCDFOptionsWidget>(new NetCDFOptionsWidget(netcdfw, this));
			ui.swOptions->insertWidget(AbstractFileFilter::NETCDF, netcdfw);
		} else
			m_netcdfOptionsWidget->clear();
		ui.swOptions->setCurrentWidget(m_netcdfOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::FITS:
		if (!m_fitsOptionsWidget) {
			QWidget* fitsw = new QWidget();
			m_fitsOptionsWidget = std::unique_ptr<FITSOptionsWidget>(new FITSOptionsWidget(fitsw, this));
			ui.swOptions->addWidget(fitsw);
		}
		ui.swOptions->setCurrentWidget(m_fitsOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::JSON:
		if (!m_jsonOptionsWidget) {
			QWidget* jsonw = new QWidget();
			m_jsonOptionsWidget = std::unique_ptr<JsonOptionsWidget>(new JsonOptionsWidget(jsonw, this));
			ui.tvJson->setModel(m_jsonOptionsWidget->model());
			ui.swOptions->addWidget(jsonw);
			m_jsonOptionsWidget->loadSettings();
		}
		ui.swOptions->setCurrentWidget(m_jsonOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::ROOT:
		if (!m_rootOptionsWidget) {
			QWidget* rootw = new QWidget();
			m_rootOptionsWidget = std::unique_ptr<ROOTOptionsWidget>(new ROOTOptionsWidget(rootw, this));
			ui.swOptions->addWidget(rootw);
		} else
			m_rootOptionsWidget->clear();
		ui.swOptions->setCurrentWidget(m_rootOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::NgspiceRawAscii:
	case AbstractFileFilter::NgspiceRawBinary:
		break;
	}
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
	auto* dlg = new FileInfoDialog(this);
	dlg->setFiles(files);
	dlg->exec();
}

/*!
	enables the options if the filter "custom" was chosen. Disables the options otherwise.
*/
void ImportFileWidget::filterChanged(int index) {
	// ignore filter for these formats
	AbstractFileFilter::FileType fileType = currentFileType();
	if (fileType != AbstractFileFilter::Ascii && fileType != AbstractFileFilter::Binary) {
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

	QString fileName = absolutePath(ui.leFileName->text());
	AbstractFileFilter::FileType fileType = currentFileType();
	LiveDataSource::SourceType sourceType = currentSourceType();
	int lines = ui.sbPreviewLines->value();

	if (sourceType == LiveDataSource::SourceType::FileOrPipe)
		DEBUG("refreshPreview(): file name = " << fileName.toStdString());

	// generic table widget
	if (fileType == AbstractFileFilter::Ascii || fileType == AbstractFileFilter::Binary
		|| fileType == AbstractFileFilter::JSON || fileType == AbstractFileFilter::NgspiceRawAscii
		|| fileType == AbstractFileFilter::NgspiceRawBinary)
		m_twPreview->show();
	else
		m_twPreview->hide();

	bool ok = true;
	QTableWidget* tmpTableWidget = m_twPreview;
	QVector<QStringList> importedStrings;
	QStringList vectorNameList;
	QVector<AbstractColumn::ColumnMode> columnModes;
	DEBUG("Data File Type: " << ENUM_TO_STRING(AbstractFileFilter, FileType, fileType));
	switch (fileType) {
	case AbstractFileFilter::Ascii: {
		ui.tePreview->clear();

		auto* filter = static_cast<AsciiFilter*>(this->currentFileFilter());

		DEBUG("Data Source Type: " << ENUM_TO_STRING(LiveDataSource, SourceType, sourceType));
		switch (sourceType) {
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
			DEBUG("	Port: " << serialPort().toStdString() << ", Settings: " << baudRate() << ',' << sPort.dataBits()
					<< ',' << sPort.parity() << ',' << sPort.stopBits());
			sPort.setPortName(serialPort());
			sPort.setBaudRate(baudRate());

			if (sPort.open(QIODevice::ReadOnly)) {
				if (sPort.waitForReadyRead(2000))
					importedStrings = filter->preview(sPort);
				else
					DEBUG("	ERROR: not ready for read after 2 sec");

				sPort.close();
			} else {
				DEBUG("	ERROR: failed to open serial port. error: " << sPort.error());
			}
			break;
		}
		case LiveDataSource::SourceType::MQTT: {
#ifdef HAVE_MQTT
			qDebug()<<"Start MQTT preview, is it ready:"<<m_mqttReadyForPreview;
			if (m_mqttReadyForPreview) {
				filter->vectorNames().clear();
				QMapIterator<QMqttTopicName, QMqttMessage> i(m_lastMessage);
				while(i.hasNext()) {
					i.next();
					filter->MQTTPreview(importedStrings, QString(i.value().payload().data()), i.key().name() );
					if (importedStrings.isEmpty())
						break;
				}

				QMapIterator<QMqttTopicName, bool> j(m_messageArrived);
				while(j.hasNext()) {
					j.next();
					m_messageArrived[j.key()] = false;
				}
				m_mqttReadyForPreview = false;
			}
#endif
			break;
		}
		}

		vectorNameList = filter->vectorNames();
		columnModes = filter->columnModes();
		break;
	}
	case AbstractFileFilter::Binary: {
		ui.tePreview->clear();
		auto* filter = (BinaryFilter*)this->currentFileFilter();
		importedStrings = filter->preview(fileName, lines);
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
		auto* filter = (HDF5Filter*)this->currentFileFilter();
		lines = m_hdf5OptionsWidget->lines();
		importedStrings = filter->readCurrentDataSet(fileName, nullptr, ok, AbstractFileFilter::Replace, lines);
		tmpTableWidget = m_hdf5OptionsWidget->previewWidget();
		break;
	}
	case AbstractFileFilter::NETCDF: {
		auto* filter = (NetCDFFilter*)this->currentFileFilter();
		lines = m_netcdfOptionsWidget->lines();
		importedStrings = filter->readCurrentVar(fileName, nullptr, AbstractFileFilter::Replace, lines);
		tmpTableWidget = m_netcdfOptionsWidget->previewWidget();
		break;
	}
	case AbstractFileFilter::FITS: {
		auto* filter = (FITSFilter*)this->currentFileFilter();
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
	case AbstractFileFilter::JSON: {
		ui.tePreview->clear();
		m_jsonOptionsWidget->loadDocument(fileName);
		auto* filter = (JsonFilter*)this->currentFileFilter();
		m_jsonOptionsWidget->applyFilterSettings(filter, ui.tvJson->currentIndex());
		importedStrings = filter->preview(fileName);

		vectorNameList = filter->vectorNames();
		columnModes = filter->columnModes();
		break;
	}
	case AbstractFileFilter::ROOT: {
		auto* filter = (ROOTFilter*)this->currentFileFilter();
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
		auto* filter = (NgspiceRawAsciiFilter*)this->currentFileFilter();
		importedStrings = filter->preview(fileName, lines);
		vectorNameList = filter->vectorNames();
		columnModes = filter->columnModes();
		break;
	}
	case AbstractFileFilter::NgspiceRawBinary: {
		ui.tePreview->clear();
		auto* filter = (NgspiceRawBinaryFilter*)this->currentFileFilter();
		importedStrings = filter->preview(fileName, lines);
		vectorNameList = filter->vectorNames();
		columnModes = filter->columnModes();
		break;
	}
	}

	// fill the table widget
	tmpTableWidget->setRowCount(0);
	tmpTableWidget->setColumnCount(0);
	if ( !importedStrings.isEmpty() ) {
		//QDEBUG("importedStrings =" << importedStrings);
		if (!ok) {
			// show imported strings as error message
			tmpTableWidget->setRowCount(1);
			tmpTableWidget->setColumnCount(1);
			auto* item = new QTableWidgetItem();
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
					auto* item = new QTableWidgetItem(importedStrings[i][j]);
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

void ImportFileWidget::updateContent(const QString& fileName, AbstractFileFilter::FileType fileType) {
	initOptionsWidget(fileType);
	if (auto* filter = currentFileFilter()) {
		switch (fileType) {
		case AbstractFileFilter::HDF5:
			m_hdf5OptionsWidget->updateContent((HDF5Filter*)filter, fileName);
			break;
		case AbstractFileFilter::NETCDF:
			m_netcdfOptionsWidget->updateContent((NetCDFFilter*)filter, fileName);
			break;
		case AbstractFileFilter::FITS:
#ifdef HAVE_FITS
			m_fitsOptionsWidget->updateContent((FITSFilter*)filter, fileName);
#endif
			break;
		case AbstractFileFilter::ROOT:
			m_rootOptionsWidget->updateContent((ROOTFilter*)filter, fileName);
			break;
		case AbstractFileFilter::Ascii:
		case AbstractFileFilter::Binary:
		case AbstractFileFilter::Image:
		case AbstractFileFilter::JSON:
		case AbstractFileFilter::NgspiceRawAscii:
		case AbstractFileFilter::NgspiceRawBinary:
			break;
		}
	}
}

void ImportFileWidget::updateTypeChanged(int idx) {
	const auto UpdateType = static_cast<LiveDataSource::UpdateType>(idx);

	switch (UpdateType) {
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
	const auto readingType = static_cast<LiveDataSource::ReadingType>(idx);
	const LiveDataSource::SourceType sourceType = currentSourceType();

	if (sourceType == LiveDataSource::SourceType::NetworkTcpSocket || sourceType == LiveDataSource::SourceType::LocalSocket
			|| sourceType == LiveDataSource::SourceType::SerialPort
			|| readingType == LiveDataSource::ReadingType::TillEnd || readingType == LiveDataSource::ReadingType::WholeFile) {
		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();
	} else {
		ui.lSampleSize->show();
		ui.sbSampleSize->show();
	}

	if (readingType == LiveDataSource::ReadingType::WholeFile) {
		ui.lKeepLastValues->hide();
		ui.sbKeepNValues->hide();
	} else {
		ui.lKeepLastValues->show();
		ui.sbKeepNValues->show();
	}
}

void ImportFileWidget::sourceTypeChanged(int idx) {
	const auto sourceType = static_cast<LiveDataSource::SourceType>(idx);

	// enable/disable "on new data"-option
	const auto* model = qobject_cast<const QStandardItemModel*>(ui.cbUpdateType->model());
	QStandardItem* item = model->item(LiveDataSource::UpdateType::NewData);

	switch (sourceType) {
	case LiveDataSource::SourceType::FileOrPipe:
		ui.lFileName->show();
		ui.leFileName->show();
		ui.bFileInfo->show();
		ui.bOpen->show();
		ui.chbLinkFile->show();

		//option for sample size are available for "continuously fixed" and "from end" reading options
		if (ui.cbReadingType->currentIndex() < 2) {
			ui.lSampleSize->show();
			ui.sbSampleSize->show();
		} else {
			ui.lSampleSize->hide();
			ui.sbSampleSize->hide();
		}

		ui.cbBaudRate->hide();
		ui.lBaudRate->hide();
		ui.lHost->hide();
		ui.leHost->hide();
		ui.lPort->hide();
		ui.lePort->hide();
		ui.cbSerialPort->hide();
		ui.lSerialPort->hide();

		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		fileNameChanged(ui.leFileName->text());
		ui.cbFileType->show();
		ui.lFileType->show();
		setMQTTVisible(false);
		break;
	case LiveDataSource::SourceType::NetworkTcpSocket:
	case LiveDataSource::SourceType::NetworkUdpSocket:
		ui.lHost->show();
		ui.leHost->show();
		ui.lePort->show();
		ui.lPort->show();
		if (sourceType == LiveDataSource::SourceType::NetworkTcpSocket) {
			ui.lSampleSize->hide();
			ui.sbSampleSize->hide();
		} else {
			ui.lSampleSize->show();
			ui.sbSampleSize->show();
		}

		ui.lBaudRate->hide();
		ui.cbBaudRate->hide();
		ui.lSerialPort->hide();
		ui.cbSerialPort->hide();

		ui.lFileName->hide();
		ui.leFileName->hide();
		ui.bFileInfo->hide();
		ui.bOpen->hide();
		ui.chbLinkFile->hide();

		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));

		ui.gbOptions->setEnabled(true);
		ui.bManageFilters->setEnabled(true);
		ui.cbFilter->setEnabled(true);
		ui.cbFileType->setEnabled(true);
		ui.cbFileType->show();
		ui.lFileType->show();
		setMQTTVisible(false);
		break;
	case LiveDataSource::SourceType::LocalSocket:
		ui.lFileName->show();
		ui.leFileName->show();
		ui.bOpen->show();
		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();

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

		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		ui.gbOptions->setEnabled(true);
		ui.bManageFilters->setEnabled(true);
		ui.cbFilter->setEnabled(true);
		ui.cbFileType->setEnabled(true);
		ui.cbFileType->show();
		ui.lFileType->show();
		setMQTTVisible(false);
		break;
	case LiveDataSource::SourceType::SerialPort:
		ui.lBaudRate->show();
		ui.cbBaudRate->show();
		ui.lSerialPort->show();
		ui.cbSerialPort->show();
		ui.lSampleSize->show();
		ui.sbSampleSize->show();

		ui.lHost->hide();
		ui.leHost->hide();
		ui.lePort->hide();
		ui.lPort->hide();
		ui.lFileName->hide();
		ui.leFileName->hide();
		ui.bFileInfo->hide();
		ui.bOpen->hide();
		ui.chbLinkFile->hide();

		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));

		ui.cbFileType->setEnabled(true);
		ui.cbFileType->show();
		ui.gbOptions->setEnabled(true);
		ui.bManageFilters->setEnabled(true);
		ui.cbFilter->setEnabled(true);
		ui.lFileType->show();
		setMQTTVisible(false);
		break;
	case LiveDataSource::SourceType::MQTT:
#ifdef HAVE_MQTT
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		//for MQTT we read ascii data only, hide the file type options
		int idx = ui.cbFileType->findText("ASCII data");
		ui.cbFileType->setCurrentIndex(idx);
		ui.cbFileType->hide();
		ui.lFileType->hide();

		ui.lBaudRate->hide();
		ui.cbBaudRate->hide();
		ui.lSerialPort->hide();
		ui.cbSerialPort->hide();
		ui.lHost->hide();
		ui.leHost->hide();
		ui.lPort->hide();
		ui.lePort->hide();
		ui.lFileName->hide();
		ui.leFileName->hide();
		ui.bFileInfo->hide();
		ui.bOpen->hide();
		ui.chbLinkFile->hide();

		setMQTTVisible(true);

		ui.cbFileType->setEnabled(true);
		ui.gbOptions->setEnabled(true);
		ui.bManageFilters->setEnabled(true);
		ui.cbFilter->setEnabled(true);

		//in case there are already connections defined, show the available topics for the currently selected connection
		mqttConnectionChanged();
#endif
		break;
	}

	//deactivate/activate options that are specific to file of pipe sources only
	auto* typeModel = qobject_cast<const QStandardItemModel*>(ui.cbFileType->model());
	if (sourceType != LiveDataSource::FileOrPipe) {
		//deactivate file types other than ascii and binary
		for (int i = 2; i < ui.cbFileType->count(); ++i)
			typeModel->item(i)->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));

		//"whole file" read option is available for file or pipe only, disable it
		typeModel = qobject_cast<const QStandardItemModel*>(ui.cbReadingType->model());
		QStandardItem* item = typeModel->item(LiveDataSource::ReadingType::WholeFile);
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));

		//"update options" groupbox can be deactivated for "file and pipe" if the file is invalid.
		//Activate the groupbox when switching from "file and pipe" to a different source type.
		ui.gbUpdateOptions->setEnabled(true);
	} else {
		for (int i = 2; i < ui.cbFileType->count(); ++i)
			typeModel->item(i)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		//enable "whole file" item for file or pipe
		typeModel = qobject_cast<const QStandardItemModel*>(ui.cbReadingType->model());
		QStandardItem* item = typeModel->item(LiveDataSource::ReadingType::WholeFile);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}

	emit sourceTypeChanged();
	refreshPreview();
}

#ifdef HAVE_MQTT

/*!
 *\brief called when a different MQTT connection is selected in the connection ComboBox.
 * connects to the MQTT broker according to the connection settings.
 */
void ImportFileWidget::mqttConnectionChanged() {
	if (m_initialisingMQTT)
		return;

	if (m_client == nullptr || m_client->state() == QMqttClient::ClientState::Disconnected) {
		if (ui.cbConnection->currentIndex() == -1)
			return;
		WAIT_CURSOR;

		delete m_client;
		m_client = new QMqttClient;
		connect(m_client, &QMqttClient::connected, this, &ImportFileWidget::onMqttConnect);
		connect(m_client, &QMqttClient::disconnected, this, &ImportFileWidget::onMqttDisconnect);
		connect(m_client, &QMqttClient::messageReceived, this, &ImportFileWidget::mqttMessageReceived);
		connect(m_client, &QMqttClient::errorChanged, this, &ImportFileWidget::mqttErrorChanged);

		ui.cbConnection->setEnabled(false);
		ui.bManageConnections->setEnabled(false);

		//determine the current connection's settings
		KConfig config(m_configPath, KConfig::SimpleConfig);
		KConfigGroup group = config.group(ui.cbConnection->currentText());

		m_client->setHostname(group.readEntry("Host"));
		m_client->setPort(group.readEntry("Port").toUInt());

		const bool useID = group.readEntry("UseID").toUInt();
		if (useID)
			m_client->setClientId(group.readEntry("ClientID"));
		else
			m_client->setClientId("");

		const bool useAuthentication = group.readEntry("UseAuthentication").toUInt();
		if (useAuthentication) {
			m_client->setUsername(group.readEntry("UserName"));
			m_client->setPassword(group.readEntry("Password"));
		} else {
			m_client->setUsername("");
			m_client->setPassword("");
		}

		qDebug()<< "Use ID" << useID << " " << m_client->clientId();
		qDebug()<< "Use authentication" << useAuthentication << " " << m_client->username() << " " << m_client->password();
		qDebug()<< m_client->hostname() << "   " << m_client->port();
		qDebug()<< "Trying to connect";
		m_connectTimeoutTimer->start();
		m_client->connectToHost();
	} else if (m_client->state() == QMqttClient::ClientState::Connected) {
		WAIT_CURSOR;
		ui.cbConnection->setEnabled(false);
		ui.bManageConnections->setEnabled(false);
		qDebug()<<"Disconnecting from MQTT broker"	;
		m_client->disconnectFromHost();
	}
}

/*!
 *\brief called when the client connects to the broker successfully, it subscribes to every topic (# wildcard)
 * in order to later list every available topic
 */
void ImportFileWidget::onMqttConnect() {
	if (m_client->error() == QMqttClient::NoError) {
		m_connectTimeoutTimer->stop();
		ui.gbManageSubscriptions->setVisible(true);

		//subscribing to every topic (# wildcard) in order to later list every available topic
		QMqttTopicFilter globalFilter{"#"};
		m_mainSubscription = m_client->subscribe(globalFilter, 1);
		if (!m_mainSubscription)
			QMessageBox::critical(this, i18n("Couldn't subscribe"), i18n("Couldn't subscribe. Something went wrong"));
	}
	ui.cbConnection->setEnabled(true);
	ui.bManageConnections->setEnabled(true);
	emit subscriptionsChanged();
	RESET_CURSOR;
}

/*!
 *\brief called when the client disconnects from the broker successfully
 * removes every information about the former connection
 */
void ImportFileWidget::onMqttDisconnect() {
	m_lastMessage.clear();
	m_messageArrived.clear();
	m_mqttSubscriptions.clear();
	m_topicList.clear();
	ui.twSubscriptions->clear();
	ui.twTopics->clear();

	m_searchTimer->stop();
	m_connectTimeoutTimer->stop();

	ui.gbManageSubscriptions->setVisible(false);
	ui.cbSourceType->setEnabled(true);
	ui.cbConnection->setEnabled(true);
	ui.bManageConnections->setEnabled(true);

	m_mqttReadyForPreview = false;
	m_searching = false;
	m_topicCompleter = new QCompleter;
	m_subscriptionCompleter = new QCompleter;

	emit subscriptionsChanged();
	RESET_CURSOR;

	if (!m_initialisingMQTT) {
		if (!m_connectionTimedOut)
			QTimer::singleShot(300, this, &ImportFileWidget::mqttConnectionChanged);
		else
			m_connectionTimedOut = false;
	}
}

/*!
 *\brief called when the subscribe button is pressed
 * subscribes to the topic represented by the current item of twTopics
 */
void ImportFileWidget::mqttSubscribe() {
	QTreeWidgetItem* item = ui.twTopics->currentItem();
	if (!item) {
		QMessageBox::warning(this, i18n("Warning"), i18n("You didn't select any item from the Tree Widget"));
		return;
	}

	//determine the topic name that the current item represents
	QTreeWidgetItem* tempItem = item;
	QString name = item->text(0);
	if (item->childCount() != 0)
		name.append("/#");

	while(tempItem->parent() != nullptr) {
		tempItem = tempItem->parent();
		name.prepend(tempItem->text(0) + '/');
	}

	//check if the subscription already exists
	const QList<QTreeWidgetItem*>& topLevelList = ui.twSubscriptions->findItems(name, Qt::MatchExactly);
	if (topLevelList.isEmpty() || topLevelList.first()->parent() != nullptr) {
		qDebug() << "Subscribe to: " << name;
		bool foundSuperior = false;

		for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
			//if the new subscirptions contains an already existing one, we remove the inferior one
			if (checkTopicContains(name, ui.twSubscriptions->topLevelItem(i)->text(0))
					&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
				unsubscribeFromTopic(ui.twSubscriptions->topLevelItem(i)->text(0));
				--i;
				continue;
			}

			//if there is a subscription containing the new one we set foundSuperior true
			if (checkTopicContains(ui.twSubscriptions->topLevelItem(i)->text(0), name)
					&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
				foundSuperior = true;
				qDebug()<<"Can't continue subscribe. Found superior for " << name <<" : "<< ui.twSubscriptions->topLevelItem(i)->text(0);
				break;
			}
		}

		//if there wasn't a superior subscription we can subscribe to the new topic
		if (!foundSuperior) {
			QStringList toplevelName;
			toplevelName.push_back(name);
			QTreeWidgetItem* newTopLevelItem = new QTreeWidgetItem(toplevelName);
			ui.twSubscriptions->addTopLevelItem(newTopLevelItem);

			const QMqttTopicFilter filter {name};
			QMqttSubscription *tempSubscription = m_client->subscribe(filter, static_cast<quint8>(ui.cbQos->currentText().toUInt()) );

			if (tempSubscription) {
				m_mqttSubscriptions.push_back(tempSubscription);
				connect(tempSubscription, &QMqttSubscription::messageReceived, this, &ImportFileWidget::mqttSubscriptionMessageReceived);
				emit subscriptionsChanged();
			}

			if (name.endsWith('#')) {
				//adding every topic that the subscription contains to twSubscriptions
				addSubscriptionChildren(item, newTopLevelItem);

				//if an already existing subscription contains a topic that the new subscription also contains
				//we decompose the already existing subscription
				//by unsubscribing from its topics, that are present in the new subscription as well
				const QStringList nameList = name.split('/', QString::SkipEmptyParts);
				const QString& root = nameList.first();
				QVector<QTreeWidgetItem*> children;
				for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
					if (ui.twSubscriptions->topLevelItem(i)->text(0).startsWith(root)
							&& name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
						children.clear();
						//get the "leaf" children of the inspected subscription
						findSubscriptionLeafChildren(children, ui.twSubscriptions->topLevelItem(i));
						for (int j = 0; j < children.size(); ++j) {
							if (checkTopicContains(name, children[j]->text(0))) {
								//if the new subscription contains a topic, we unsubscribe from it
								ui.twSubscriptions->setCurrentItem(children[j]);
								mqttUnsubscribe();
								--i;
							}
						}
					}
				}
			}

			manageCommonLevelSubscriptions();
			updateSubscriptionCompleter();

			if (!ui.bWillMessage->isEnabled())
				ui.bWillMessage->setEnabled(true);
		} else
			QMessageBox::warning(this, i18n("Warning"), i18n("You already subscribed to a topic containing this one"));
	} else
		QMessageBox::warning(this, i18n("Warning"), i18n("You already subscribed to this topic"));
}

/*!
 *\brief called when the unsubscribe button is pressed
 * unsubscribes from the topic represented by the current item of twSubscription
 */
void ImportFileWidget::mqttUnsubscribe() {
	QTreeWidgetItem* unsubscribeItem = ui.twSubscriptions->currentItem();
	if (!unsubscribeItem) {
		QMessageBox::warning(this, i18n("Warning"), i18n("You didn't select any item from the Tree Widget"));
		return;
	}

	qDebug() << "Unsubscribe from: " << unsubscribeItem->text(0);
	//if it is a top level item, meaning a topic that we really subscribed to(not one that belongs to a subscription)
	//we can simply unsubscribe from it
	if (unsubscribeItem->parent() == nullptr)
		unsubscribeFromTopic(unsubscribeItem->text(0));

	//otherwise we remove the selected item, but subscribe to every other topic, that was contained by
	//the selected item's parent subscription(top level item of twSubscriptions)
	else {
		while(unsubscribeItem->parent() != nullptr) {
			for (int i = 0; i < unsubscribeItem->parent()->childCount(); ++i) {
				if (unsubscribeItem->text(0) != unsubscribeItem->parent()->child(i)->text(0)) {
					const QMqttTopicFilter filter {unsubscribeItem->parent()->child(i)->text(0)};
					QMqttSubscription *tempSubscription = m_client->subscribe(filter, static_cast<quint8>(ui.cbQos->currentText().toUInt()) );

					ui.twSubscriptions->addTopLevelItem(unsubscribeItem->parent()->takeChild(i));

					if (tempSubscription) {
						m_mqttSubscriptions.push_back(tempSubscription);
						connect(tempSubscription, &QMqttSubscription::messageReceived, this, &ImportFileWidget::mqttSubscriptionMessageReceived);
						emit subscriptionsChanged();
					}
					--i;
				}
			}
			unsubscribeItem = unsubscribeItem->parent();
		}
		unsubscribeFromTopic(unsubscribeItem->text(0));

		//check if any common topics were subscribed, if possible merge them
		manageCommonLevelSubscriptions();
	}
	updateSubscriptionCompleter();

	if (ui.twSubscriptions->topLevelItemCount() <= 0)
		ui.bWillMessage->setEnabled(false);
}

/*!
 *\brief called when the client receives a message
 * if the message arrived from a new topic, the topic is put in twTopics
 */
void ImportFileWidget::mqttMessageReceived(const QByteArray& message, const QMqttTopicName& topic) {
	Q_UNUSED(message);
	if (m_addedTopics.contains(topic.name()))
		return;

	m_addedTopics.push_back(topic.name());
	QStringList name;
	QString rootName;
	const QChar sep = '/';

	if (topic.name().contains(sep)) {
		const QStringList& list = topic.name().split(sep, QString::SkipEmptyParts);

		if (!list.isEmpty()) {
			rootName = list.at(0);
			name.append(list.at(0));
			int topItemIdx = -1;
			//check whether the first level of the topic can be found in twTopics
			for (int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
				if (ui.twTopics->topLevelItem(i)->text(0) == list.at(0)) {
					topItemIdx = i;
					break;
				}
			}

			QTreeWidgetItem* currentItem = nullptr;
			//if not we simply add every level of the topic to the tree
			if (topItemIdx < 0) {
				currentItem = new QTreeWidgetItem(name);
				ui.twTopics->addTopLevelItem(currentItem);
				for (int i = 1; i < list.size(); ++i) {
					name.clear();
					name.append(list.at(i));
					currentItem->addChild(new QTreeWidgetItem(name));
					currentItem = currentItem->child(0);
				}
			}
			//otherwise we search for the first level that isn't part of the tree,
			//then add every level of the topic to the tree from that certain level
			else {
				currentItem = ui.twTopics->topLevelItem(topItemIdx);
				int listIdx = 1;
				for (; listIdx < list.size(); ++listIdx) {
					QTreeWidgetItem* childItem = nullptr;
					bool found = false;
					for (int j = 0; j < currentItem->childCount(); ++j) {
						childItem = currentItem->child(j);
						if (childItem->text(0) == list.at(listIdx)) {
							found = true;
							currentItem = childItem;
							break;
						}
					}
					if (!found) {
						//this is the level that isn't present in the tree
						break;
					}
				}

				//add every level to the tree starting with the first level that isn't part of the tree
				for (; listIdx < list.size(); ++listIdx) {
					name.clear();
					name.append(list.at(listIdx));
					currentItem->addChild(new QTreeWidgetItem(name));
					currentItem = currentItem->child(currentItem->childCount() - 1);
				}
			}
		}
	} else {
		rootName = topic.name();
		name.append(topic.name());
		ui.twTopics->addTopLevelItem(new QTreeWidgetItem(name));
	}

	//if a subscribed topic contains the new topic, we have to update twSubscriptions
	for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
		const QStringList subscriptionName = ui.twSubscriptions->topLevelItem(i)->text(0).split('/', QString::SkipEmptyParts);
		if (!subscriptionName.isEmpty()) {
			if (rootName == subscriptionName.first()) {
				updateSubscriptionTree();
				break;
			}
		}
	}

	//signals that a newTopic was added, in order to fill the completer of leTopics
	emit newTopic(rootName);
}

/*!
 *\brief called when a new topic is added to the tree(twTopics)
 * appends the topic's root to the topicList if it isn't in the list already
 * then sets the completer for leTopics
 */
void ImportFileWidget::setTopicCompleter(const QString& topic) {
	if (!m_topicList.contains(topic)) {
		m_topicList.append(topic);
		if (!m_searching) {
			m_topicCompleter = new QCompleter(m_topicList, this);
			m_topicCompleter->setCompletionMode(QCompleter::PopupCompletion);
			m_topicCompleter->setCaseSensitivity(Qt::CaseSensitive);
			ui.leTopics->setCompleter(m_topicCompleter);
		}
	}
}

/*!
 *\brief called when 10 seconds passed since the last time the user searched for a certain root in twTopics
 * enables updating the completer for le
 */
void ImportFileWidget::topicTimeout() {
	m_searching = false;
	m_searchTimer->stop();
}

/*!
 *\brief called when the client receives a message from a subscribed topic (that isn't the "#" wildcard)
 */
void ImportFileWidget::mqttSubscriptionMessageReceived(const QMqttMessage &msg) {
	qDebug()<<"message received from: "<<msg.topic().name();
	if (!m_subscribedTopicNames.contains(msg.topic().name())) {
		m_messageArrived[msg.topic()] = true;
		m_subscribedTopicNames.push_back(msg.topic().name());
	}

	if (!m_messageArrived[msg.topic()])
		m_messageArrived[msg.topic()] = true;

	//updates the last message of the topic
	m_lastMessage[msg.topic()] = msg;

	//check if the client received a message from every subscribed topic, since the last time the preview was refreshed
	bool check = true;
	QMapIterator<QMqttTopicName, bool> i(m_messageArrived);
	while(i.hasNext()) {
		i.next();
		if (i.value() == false ) {
			check = false;
			break;
		}
	}

	//if there is a message from every subscribed topic, we refresh the preview
	if (check) {
		m_mqttReadyForPreview = true;
		refreshPreview();
	}
}

/*!
 *\brief called when use will message is changed in the settings widget
 * Updates the will settings
 */
void ImportFileWidget::useWillMessage(int state) {
	if (state == Qt::Checked)
		m_willSettings.MQTTUseWill = true;
	else if (state == Qt::Unchecked)
		m_willSettings.MQTTUseWill = false;
}

/*!
 *\brief called when the selected will message type is changed in the settings widget
 * Updates the will settings
 */
void ImportFileWidget::willMessageTypeChanged(int type) {
	m_willSettings.willMessageType = static_cast<MQTTClient::WillMessageType>(type);
}

/*!
 *\brief called when the selected will message' type's retain flag is changed in the settings widget
 * Updates the will settings
 */
void ImportFileWidget::willRetainChanged(bool retain) {
	m_willSettings.willRetain = retain;
}

/*!
 *\brief called when the selected will update interval is changed in the settings widget
 * Updates the will settings
 */
void ImportFileWidget::willTimeIntervalChanged(int interval) {
	m_willSettings.willTimeInterval = interval;
}

/*!
 *\brief called when the selected will own message is changed in the settings widget
 * Updates the will settings
 */
void ImportFileWidget::willOwnMessageChanged(const QString& msg) {
	m_willSettings.willOwnMessage = msg;
}

/*!
 *\brief called when the selected will topic is changed in the settings widget
 * Updates the will settings
 */
void ImportFileWidget::willTopicChanged(const QString& topic) {
	m_willSettings.willTopic = topic;
}

/*!
 *\brief called when the selected will statistics are changed in the settings widget
 * Updates the will settings
 */
void ImportFileWidget::willStatisticsChanged(int index) {
	m_willSettings.willStatistics[index] = !m_willSettings.willStatistics[index];
}

/*!
 *\brief called when the selected will message's QoS is changed in the settings widget
 * Updates the will settings
 */
void ImportFileWidget::willQoSChanged(int qos) {
	m_willSettings.willQoS = qos;
}

/*!
 *\brief called when the selected will update type is changed in the settings widget
 * Updates the will settings
 */
void ImportFileWidget::willUpdateTypeChanged(int updateType) {
	qDebug() << "update type changed: " <<updateType;
	m_willSettings.willUpdateType = static_cast<MQTTClient::WillUpdateType>(updateType);
}

/*!
 *\brief called when the clientError of the MQTT client changes
 *
 * \param clientError the current error of the client
 */
void ImportFileWidget::mqttErrorChanged(QMqttClient::ClientError clientError) {
	switch (clientError) {
	case QMqttClient::BadUsernameOrPassword:
		QMessageBox::critical(this, i18n("Couldn't connect"), i18n("Wrong username or password"));
		break;
	case QMqttClient::IdRejected:
		QMessageBox::critical(this, i18n("Couldn't connect"), i18n("The client ID wasn't accepted"));
		break;
	case QMqttClient::ServerUnavailable:
		QMessageBox::critical(this, i18n("Server unavailable"), i18n("The broker couldn't be reached."));
		break;
	case QMqttClient::NotAuthorized:
		QMessageBox::critical(this, i18n("Not authorized"), i18n("The client is not authorized to connect."));
		break;
	case QMqttClient::UnknownError:
		QMessageBox::critical(this, i18n("Unknown MQTT error"), i18n("An unknown error occurred."));
		break;
	case QMqttClient::NoError:
	case QMqttClient::InvalidProtocolVersion:
	case QMqttClient::TransportInvalid:
	case QMqttClient::ProtocolViolation:
		break;
	default:
		break;
	}
}

/*!
 *\brief called when leTopics' text is changed
 *		 if the rootName can be found in twTopics, then we scroll it to the top of the tree widget
 *
 * \param rootName the current text of leTopics
 */
void ImportFileWidget::scrollToTopicTreeItem(const QString& rootName) {
	m_searching = true;
	m_searchTimer->start();

	int topItemIdx = -1;
	for (int i = 0; i < ui.twTopics->topLevelItemCount(); ++i)
		if (ui.twTopics->topLevelItem(i)->text(0) == rootName) {
			topItemIdx = i;
			break;
		}

	if (topItemIdx >= 0)
		ui.twTopics->scrollToItem(ui.twTopics->topLevelItem(topItemIdx), QAbstractItemView::ScrollHint::PositionAtTop);
}

/*!
 *\brief called when leSubscriptions' text is changed
 *		 if the rootName can be found in twSubscriptions, then we scroll it to the top of the tree widget
 *
 * \param rootName the current text of leSubscriptions
 */
void ImportFileWidget::scrollToSubsriptionTreeItem(const QString& rootName) {
	m_searching = true;
	m_searchTimer->start();

	int topItemIdx = -1;
	for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i)
		if (ui.twSubscriptions->topLevelItem(i)->text(0) == rootName) {
			topItemIdx = i;
			break;
		}

	if (topItemIdx >= 0)
		ui.twSubscriptions->scrollToItem(ui.twSubscriptions->topLevelItem(topItemIdx), QAbstractItemView::ScrollHint::PositionAtTop);
}

/*!
 *\brief called when m_connectTimeoutTimer ticks,
 *		 meaning that the client couldn't connect to the broker in 5 seconds
 *		 disconnects the client, stops the timer, and warns the user
 */
void ImportFileWidget::mqttConnectTimeout() {
	m_connectionTimedOut = true;
	m_client->disconnectFromHost();
	m_connectTimeoutTimer->stop();
	QMessageBox::warning(this, i18n("Warning"), i18n("Connecting to the given broker timed out! Try changing the settings"));
	RESET_CURSOR;
}

/*!
	Shows the MQTT connection manager where the connections are created and edited.
	The selected connection is selected in the connection combo box in this widget.
*/
void ImportFileWidget::showMQTTConnectionManager() {
	bool previousConnectionChanged = false;
	MQTTConnectionManagerDialog* dlg = new MQTTConnectionManagerDialog(this, ui.cbConnection->currentText(), previousConnectionChanged);

	if (dlg->exec() == QDialog::Accepted) {
		//re-read the available connections to be in sync with the changes in MQTTConnectionManager
		m_initialisingMQTT = true;
		const QString& prevConn = ui.cbConnection->currentText();
		ui.cbConnection->clear();
		readMQTTConnections();
		m_initialisingMQTT = false;

		//select the connection the user has selected in MQTTConnectionManager
		const QString& conn = dlg->connection();

		int index = ui.cbConnection->findText(conn);
		if (conn != prevConn) {//Current connection isn't the previous one
			if (ui.cbConnection->currentIndex() != index)
				ui.cbConnection->setCurrentIndex(index);
			else
				mqttConnectionChanged();
		} else if (dlg->initialConnectionChanged()) {//Current connection is the same with previous one but it changed
			if (ui.cbConnection->currentIndex() == index)
				mqttConnectionChanged();
			else
				ui.cbConnection->setCurrentIndex(index);
		} else { //Previous connection wasn't changed
			m_initialisingMQTT = true;
			ui.cbConnection->setCurrentIndex(index);
			m_initialisingMQTT = false;
		}
	}
	delete dlg;
}

/*!
	loads all available saved MQTT nconnections
*/
void ImportFileWidget::readMQTTConnections() {
	qDebug()<< "ImportFileWidget: reading available MQTT connections";
	KConfig config(m_configPath, KConfig::SimpleConfig);
	for (const auto& name : config.groupList())
		ui.cbConnection->addItem(name);
}

/*!
 * \brief Shows the mqtt will settings widget, which allows the user to modify the will settings
 */
void ImportFileWidget::showWillSettings() {
	QMenu menu;

	QVector<QTreeWidgetItem*> children;
	for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i)
		findSubscriptionLeafChildren(children, ui.twSubscriptions->topLevelItem(i));

	QVector<QString> topics;
	for (int i = 0; i < children.size(); ++i)
		topics.append(children[i]->text(0));

	MQTTWillSettingsWidget willSettings(&menu, m_willSettings, topics);

	connect(&willSettings, &MQTTWillSettingsWidget::useChanged, this, &ImportFileWidget::useWillMessage);
	connect(&willSettings, &MQTTWillSettingsWidget::messageTypeChanged, this, &ImportFileWidget::willMessageTypeChanged);
	connect(&willSettings, &MQTTWillSettingsWidget::updateTypeChanged, this, &ImportFileWidget::willUpdateTypeChanged);
	connect(&willSettings, &MQTTWillSettingsWidget::retainChanged, this, &ImportFileWidget::willRetainChanged);
	connect(&willSettings, &MQTTWillSettingsWidget::intervalChanged, this, &ImportFileWidget::willTimeIntervalChanged);
	connect(&willSettings, &MQTTWillSettingsWidget::ownMessageChanged, this, &ImportFileWidget::willOwnMessageChanged);
	connect(&willSettings, &MQTTWillSettingsWidget::topicChanged, this, &ImportFileWidget::willTopicChanged);
	connect(&willSettings, &MQTTWillSettingsWidget::statisticsChanged, this, &ImportFileWidget::willStatisticsChanged);
	connect(&willSettings, &MQTTWillSettingsWidget::QoSChanged, this, &ImportFileWidget::willQoSChanged);
	connect(&willSettings, SIGNAL(canceled()), &menu, SLOT(close()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&willSettings);
	menu.addAction(widgetAction);

	const QPoint pos(ui.bWillMessage->sizeHint().width(),ui.bWillMessage->sizeHint().height());
	menu.exec(ui.bWillMessage->mapToGlobal(pos));
}
#endif
