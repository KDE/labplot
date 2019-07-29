/***************************************************************************
File                 : ImportFileWidget.cpp
Project              : LabPlot
Description          : import file data widget
--------------------------------------------------------------------
Copyright            : (C) 2009-2018 Stefan Gerlach (stefan.gerlach@uni.kn)
Copyright            : (C) 2009-2019 Alexander Semke (alexander.semke@web.de)
Copyright            : (C) 2017-2018 Fabian Kristof (fkristofszabolcs@gmail.com)
Copyright            : (C) 2018-2019 Kovacs Ferencz (kferike98@gmail.com)

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
#include "backend/datasources/filters/filters.h"
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

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KUrlComboBox>

#ifdef HAVE_MQTT
#include "kdefrontend/widgets/MQTTWillSettingsWidget.h"
#include "MQTTConnectionManagerDialog.h"
#include "MQTTSubscriptionWidget.h"
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
	m_fileName(fileName),
	m_liveDataSource(liveDataSource)
#ifdef HAVE_MQTT
	,
	m_connectTimeoutTimer(new QTimer(this)),
	m_subscriptionWidget(new MQTTSubscriptionWidget(this))
#endif
{
	ui.setupUi(this);

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
		ui.cbFileType->addItem(i18n("ROOT (CERN)"), AbstractFileFilter::ROOT);
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
#ifdef HAVE_ZIP
		ui.cbFileType->addItem(i18n("ROOT (CERN)"), AbstractFileFilter::ROOT);
#endif
		ui.cbFileType->addItem(i18n("Ngspice RAW ASCII"), AbstractFileFilter::NgspiceRawAscii);
		ui.cbFileType->addItem(i18n("Ngspice RAW Binary"), AbstractFileFilter::NgspiceRawBinary);

		ui.lePort->setValidator( new QIntValidator(ui.lePort) );
		ui.cbBaudRate->addItems(LiveDataSource::supportedBaudRates());
		ui.cbSerialPort->addItems(LiveDataSource::availablePorts());

		ui.tabWidget->removeTab(2);

		ui.chbLinkFile->setToolTip(i18n("If this option is checked, only the link to the file is stored in the project file but not its content."));
		ui.chbRelativePath->setToolTip(i18n("If this option is checked, the relative path of the file (relative to project's folder) will be saved."));

#ifdef HAVE_MQTT
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

	// the combobox for the import path
	m_cbFileName = new KUrlComboBox(KUrlComboBox::Mode::Files, ui.tePreview);
	m_cbFileName->setMaxItems(7);
	auto* gridLayout = dynamic_cast<QGridLayout*>(ui.gbDataSource->layout());
	if (gridLayout)
		gridLayout->addWidget(m_cbFileName, 1, 2, 1, 3);

#ifdef HAVE_MQTT
	ui.cbSourceType->addItem(QLatin1String("MQTT"));
	m_configPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() + QLatin1String("MQTT_connections");

	//add subscriptions widget
	layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->addWidget(m_subscriptionWidget);
	ui.frameSubscriptions->setLayout(layout);

	ui.bManageConnections->setIcon(QIcon::fromTheme(QLatin1String("network-server")));
	ui.bManageConnections->setToolTip(i18n("Manage MQTT connections"));

	QString info = i18n("Specify the 'Last Will and Testament' message (LWT). At least one topic has to be subscribed.");
	ui.lLWT->setToolTip(info);
	ui.bLWT->setToolTip(info);
	ui.bLWT->setEnabled(false);
	ui.bLWT->setIcon(ui.bLWT->style()->standardIcon(QStyle::SP_FileDialogDetailedView));
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
		if (static_cast<AbstractFileFilter::FileType>(ui.cbFileType->itemData(i).toInt()) == fileType) {
			if (ui.cbFileType->currentIndex() == i)
				initOptionsWidget();
			else
				ui.cbFileType->setCurrentIndex(i);

			break;
		}
	}

	if (m_fileName.isEmpty()) {
		ui.cbFilter->setCurrentIndex(conf.readEntry("Filter", 0));
		m_cbFileName->setUrl(conf.readEntry("LastImportedFile", ""));
		QStringList urls = m_cbFileName->urls();
		urls.append(conf.readXdgListEntry("LastImportedFiles"));
		m_cbFileName->setUrls(urls);
		filterChanged(ui.cbFilter->currentIndex());	// needed if filter is not changed
	} else
		m_cbFileName->setUrl(QUrl(m_fileName));


	//live data related settings
	ui.cbBaudRate->setCurrentIndex(conf.readEntry("BaudRate").toInt());
	ui.cbReadingType->setCurrentIndex(conf.readEntry("ReadingType").toInt());
	ui.cbSerialPort->setCurrentIndex(conf.readEntry("SerialPort").toInt());
	ui.cbUpdateType->setCurrentIndex(conf.readEntry("UpdateType").toInt());
	updateTypeChanged(ui.cbUpdateType->currentIndex());
	ui.leHost->setText(conf.readEntry("Host",""));
	ui.sbKeepNValues->setValue(conf.readEntry("KeepNValues").toInt());
	ui.lePort->setText(conf.readEntry("Port",""));
	ui.sbSampleSize->setValue(conf.readEntry("SampleSize").toInt());
	ui.sbUpdateInterval->setValue(conf.readEntry("UpdateInterval").toInt());
	ui.chbLinkFile->setCheckState((Qt::CheckState)conf.readEntry("LinkFile").toInt());
	ui.chbRelativePath->setCheckState((Qt::CheckState)conf.readEntry("RelativePath").toInt());

#ifdef HAVE_MQTT
	//read available MQTT connections
	m_initialisingMQTT = true;
	readMQTTConnections();
	ui.cbConnection->setCurrentIndex(ui.cbConnection->findText(conf.readEntry("Connection", "")));
	m_initialisingMQTT = false;

	m_willSettings.enabled = conf.readEntry("mqttWillEnabled", m_willSettings.enabled);
	m_willSettings.willRetain = conf.readEntry("mqttWillRetain", m_willSettings.willRetain);
	m_willSettings.willUpdateType = static_cast<MQTTClient::WillUpdateType>(conf.readEntry("mqttWillUpdateType", (int)m_willSettings.willUpdateType));
	m_willSettings.willMessageType = static_cast<MQTTClient::WillMessageType>(conf.readEntry("mqttWillMessageType", (int)m_willSettings.willMessageType));
	m_willSettings.willQoS = conf.readEntry("mqttWillQoS", (int)m_willSettings.willQoS);
	m_willSettings.willOwnMessage = conf.readEntry("mqttWillOwnMessage", m_willSettings.willOwnMessage);
	m_willSettings.willTimeInterval = conf.readEntry("mqttWillUpdateInterval", m_willSettings.willTimeInterval);

	const QString& willStatistics = conf.readEntry("mqttWillStatistics","");
	const QStringList& statisticsList = willStatistics.split('|', QString::SplitBehavior::SkipEmptyParts);
	for (auto value : statisticsList)
		m_willSettings.willStatistics[value.toInt()] = true;
#endif

	//initialize the slots after all settings were set in order to avoid unneeded refreshes
	initSlots();

	//update the status of the widgets
	fileTypeChanged(fileType);
	sourceTypeChanged(currentSourceType());
	readingTypeChanged(ui.cbReadingType->currentIndex());

	//all set now, refresh the preview
	m_suppressRefresh = false;
	QTimer::singleShot(0, this, [=] () { refreshPreview(); });
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
	conf.writeEntry("LastImportedFile", m_cbFileName->currentText());
	conf.writeXdgListEntry("LastImportedFiles", m_cbFileName->urls());

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
	conf.writeEntry("LinkFile", (int)ui.chbLinkFile->checkState());
	conf.writeEntry("RelativePath", (int)ui.chbRelativePath->checkState());

#ifdef HAVE_MQTT

	delete m_connectTimeoutTimer;
	delete m_subscriptionWidget;

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
			willStatistics += QString::number(i)+ QLatin1Char('|');
	}
	conf.writeEntry("mqttWillStatistics", willStatistics);
	conf.writeEntry("mqttWillRetain", static_cast<int>(m_willSettings.willRetain));
	conf.writeEntry("mqttWillUse", static_cast<int>(m_willSettings.enabled));
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
	connect(ui.cbSourceType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged),
	        this, static_cast<void (ImportFileWidget::*) (int)>(&ImportFileWidget::sourceTypeChanged));
	connect(m_cbFileName, &KUrlComboBox::urlActivated,
			this, [=](const QUrl &url){fileNameChanged(url.path());});
	connect(ui.leHost, &QLineEdit::textChanged, this, &ImportFileWidget::hostChanged);
	connect(ui.lePort, &QLineEdit::textChanged, this, &ImportFileWidget::portChanged);
	connect(ui.tvJson, &QTreeView::clicked, this, &ImportFileWidget::refreshPreview);
	connect(ui.bOpen, &QPushButton::clicked, this, &ImportFileWidget::selectFile);
	connect(ui.bFileInfo, &QPushButton::clicked, this, &ImportFileWidget::fileInfoDialog);
	connect(ui.bSaveFilter, &QPushButton::clicked, this, &ImportFileWidget::saveFilter);
	connect(ui.bManageFilters, &QPushButton::clicked, this, &ImportFileWidget::manageFilters);
	connect(ui.cbFileType, static_cast<void (KComboBox::*) (int)>(&KComboBox::currentIndexChanged),
	        this, &ImportFileWidget::fileTypeChanged);
	connect(ui.cbUpdateType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged),
	        this, &ImportFileWidget::updateTypeChanged);
	connect(ui.cbReadingType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged),
	        this, &ImportFileWidget::readingTypeChanged);
	connect(ui.cbFilter, static_cast<void (KComboBox::*) (int)>(&KComboBox::activated), this, &ImportFileWidget::filterChanged);
	connect(ui.bRefreshPreview, &QPushButton::clicked, this, &ImportFileWidget::refreshPreview);

#ifdef HAVE_MQTT
	connect(ui.cbConnection, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &ImportFileWidget::mqttConnectionChanged);
	connect(m_connectTimeoutTimer, &QTimer::timeout, this, &ImportFileWidget::mqttConnectTimeout);
	connect(ui.cbFileType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), [this]() {
		emit checkFileType();
	});
	connect(ui.bManageConnections, &QPushButton::clicked, this, &ImportFileWidget::showMQTTConnectionManager);
	connect(ui.bLWT, &QPushButton::clicked, this, &ImportFileWidget::showWillSettings);
	connect(m_subscriptionWidget, &MQTTSubscriptionWidget::makeSubscription, this, &ImportFileWidget::mqttSubscribe);
	connect(m_subscriptionWidget, &MQTTSubscriptionWidget::MQTTUnsubscribeFromTopic, this, &ImportFileWidget::unsubscribeFromTopic);
	connect(m_subscriptionWidget, &MQTTSubscriptionWidget::enableWill, this, &ImportFileWidget::enableWill);
	connect(m_subscriptionWidget, &MQTTSubscriptionWidget::subscriptionChanged, this, &ImportFileWidget::refreshPreview);
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
	return m_cbFileName->currentText();
}

QString ImportFileWidget::selectedObject() const {
	const QString& path = fileName();

	//determine the file name only
	QString name = path.right(path.length() - path.lastIndexOf(QDir::separator()) - 1);

	//strip away the extension if available
	if (name.indexOf('.') != -1)
		name = name.left(name.lastIndexOf('.'));

	//for multi-dimensional formats like HDF, netCDF and FITS add the currently selected object
	const auto format = currentFileType();
	if (format == AbstractFileFilter::HDF5) {
		const QStringList& hdf5Names = m_hdf5OptionsWidget->selectedNames();
		if (hdf5Names.size())
			name += hdf5Names.first(); //the names of the selected HDF5 objects already have '/'
	} else if (format == AbstractFileFilter::NETCDF) {
		const QStringList& names = m_netcdfOptionsWidget->selectedNames();
		if (names.size())
			name += QLatin1Char('/') + names.first();
	} else if (format == AbstractFileFilter::FITS) {
		const QString& extensionName = m_fitsOptionsWidget->currentExtensionName();
		if (!extensionName.isEmpty())
			name += QLatin1Char('/') + extensionName;
	} else if (format == AbstractFileFilter::ROOT) {
		const QStringList& names = m_rootOptionsWidget->selectedNames();
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

	source->setComment( fileName() );
	source->setFileType(fileType);
	currentFileFilter();
	source->setFilter(m_currentFilter.release()); // pass ownership of the filter to the LiveDataSource

	source->setSourceType(sourceType);

	switch (sourceType) {
	case LiveDataSource::SourceType::FileOrPipe:
		source->setFileName(fileName());
		source->setFileLinked(ui.chbLinkFile->isChecked());
		source->setUseRelativePath(ui.chbRelativePath->isChecked());
		break;
	case LiveDataSource::SourceType::LocalSocket:
		source->setFileName(fileName());
		source->setLocalSocketName(fileName());
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

	//reading options
	source->setReadingType(readingType);
	source->setKeepNValues(ui.sbKeepNValues->value());
	source->setUpdateType(updateType);
	if (updateType == LiveDataSource::UpdateType::TimeInterval)
		source->setUpdateInterval(ui.sbUpdateInterval->value());

	if (readingType != LiveDataSource::ReadingType::TillEnd)
		source->setSampleSize(ui.sbSampleSize->value());
}

#ifdef HAVE_MQTT
/*!
	saves the settings to the MQTTClient \c client.
*/
void ImportFileWidget::saveMQTTSettings(MQTTClient* client) const {
	DEBUG("ImportFileWidget::saveMQTTSettings");
	MQTTClient::UpdateType updateType = static_cast<MQTTClient::UpdateType>(ui.cbUpdateType->currentIndex());
	MQTTClient::ReadingType readingType = static_cast<MQTTClient::ReadingType>(ui.cbReadingType->currentIndex());

	client->setComment(fileName());
	currentFileFilter();
	client->setFilter(static_cast<AsciiFilter*>(m_currentFilter.release())); // pass ownership of the filter to MQTTClient

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

	for (int i = 0; i < m_mqttSubscriptions.count(); ++i)
		client->addInitialMQTTSubscriptions(m_mqttSubscriptions[i]->topic(), m_mqttSubscriptions[i]->qos());

	const bool retain = group.readEntry("Retain").toUInt();
	client->setMQTTRetain(retain);

	if (m_willSettings.enabled)
		client->setWillSettings(m_willSettings);
}
#endif

/*!
	returns the currently used file type.
*/
AbstractFileFilter::FileType ImportFileWidget::currentFileType() const {
	return static_cast<AbstractFileFilter::FileType>(ui.cbFileType->currentData().toInt());
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
	if (m_currentFilter && m_currentFilter->type() != fileType)
		m_currentFilter.reset();

	switch (fileType) {
	case AbstractFileFilter::Ascii: {
		DEBUG("	ASCII");
		if (!m_currentFilter)
			m_currentFilter.reset(new AsciiFilter);
		auto filter = static_cast<AsciiFilter*>(m_currentFilter.get());

		if (ui.cbFilter->currentIndex() == 0)     //"automatic"
			filter->setAutoModeEnabled(true);
		else if (ui.cbFilter->currentIndex() == 1) { //"custom"
			filter->setAutoModeEnabled(false);
			if (m_asciiOptionsWidget)
				m_asciiOptionsWidget->applyFilterSettings(filter);
		} else
			filter->loadFilterSettings(ui.cbFilter->currentText());

		//save the data portion to import
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());

		break;
	}
	case AbstractFileFilter::Binary: {
		DEBUG("	Binary");
		if (!m_currentFilter)
			m_currentFilter.reset(new BinaryFilter);
		auto filter = static_cast<BinaryFilter*>(m_currentFilter.get());
		if ( ui.cbFilter->currentIndex() == 0 ) 	//"automatic"
			filter->setAutoModeEnabled(true);
		else if (ui.cbFilter->currentIndex() == 1) {	//"custom"
			filter->setAutoModeEnabled(false);
			if (m_binaryOptionsWidget)
				m_binaryOptionsWidget->applyFilterSettings(filter);
		} else {
			//TODO: load filter settings
			// 			filter->setFilterName( ui.cbFilter->currentText() );
		}

		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());

		break;
	}
	case AbstractFileFilter::Image: {
		DEBUG("	Image");
		if (!m_currentFilter)
			m_currentFilter.reset(new ImageFilter);
		auto filter = static_cast<ImageFilter*>(m_currentFilter.get());

		filter->setImportFormat(m_imageOptionsWidget->currentFormat());
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());

		break;
	}
	case AbstractFileFilter::HDF5: {
		DEBUG("ImportFileWidget::currentFileFilter(): HDF5");
		if (!m_currentFilter)
			m_currentFilter.reset(new HDF5Filter);
		auto filter = static_cast<HDF5Filter*>(m_currentFilter.get());
		QStringList names = selectedHDF5Names();
		QDEBUG("ImportFileWidget::currentFileFilter(): selected HDF5 names =" << names);
		if (!names.isEmpty())
			filter->setCurrentDataSetName(names[0]);
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());
		DEBUG("ImportFileWidget::currentFileFilter(): OK");

		break;
	}
	case AbstractFileFilter::NETCDF: {
		DEBUG("	NETCDF");
		if (!m_currentFilter)
			m_currentFilter.reset(new NetCDFFilter);
		auto filter = static_cast<NetCDFFilter*>(m_currentFilter.get());

		if (!selectedNetCDFNames().isEmpty())
			filter->setCurrentVarName(selectedNetCDFNames()[0]);
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());

		break;
	}
	case AbstractFileFilter::FITS: {
		DEBUG("	FITS");
		if (!m_currentFilter)
			m_currentFilter.reset(new FITSFilter);
		auto filter = static_cast<FITSFilter*>(m_currentFilter.get());
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());

		break;
	}
	case AbstractFileFilter::JSON: {
		DEBUG("	JSON");
		if (!m_currentFilter)
			m_currentFilter.reset(new JsonFilter);
		auto filter = static_cast<JsonFilter*>(m_currentFilter.get());
		m_jsonOptionsWidget->applyFilterSettings(filter, ui.tvJson->currentIndex());

		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());

		break;
	}
	case AbstractFileFilter::ROOT: {
		DEBUG("	ROOT");
		if (!m_currentFilter)
			m_currentFilter.reset(new ROOTFilter);
		auto filter = static_cast<ROOTFilter*>(m_currentFilter.get());
		QStringList names = selectedROOTNames();
		if (!names.isEmpty())
			filter->setCurrentObject(names.first());

		filter->setStartRow(m_rootOptionsWidget->startRow());
		filter->setEndRow(m_rootOptionsWidget->endRow());
		filter->setColumns(m_rootOptionsWidget->columns());

		break;
	}
	case AbstractFileFilter::NgspiceRawAscii: {
		DEBUG("	NgspiceRawAscii");
		if (!m_currentFilter)
			m_currentFilter.reset(new NgspiceRawAsciiFilter);
		auto filter = static_cast<NgspiceRawAsciiFilter*>(m_currentFilter.get());
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());

		break;
	}
	case AbstractFileFilter::NgspiceRawBinary: {
		DEBUG("	NgspiceRawBinary");
		if (!m_currentFilter)
			m_currentFilter.reset(new NgspiceRawBinaryFilter);
		auto filter = static_cast<NgspiceRawBinaryFilter*>(m_currentFilter.get());
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());

		break;
	}
	}

	return m_currentFilter.get();
}

/*!
	opens a file dialog and lets the user select the file data source.
*/
void ImportFileWidget::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("ImportFileWidget"));
	const QString& dir = conf.readEntry(QLatin1String("LastDir"), "");
	const QString& path = QFileDialog::getOpenFileName(this, i18n("Select the File Data Source"), dir);
	if (path.isEmpty())	//cancel was clicked in the file-dialog
		return;

	int pos = path.lastIndexOf(QDir::separator());
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry(QLatin1String("LastDir"), newDir);
	}

	//process all events after the FileDialog was closed to repaint the widget
	//before we start calculating the preview
	QApplication::processEvents(QEventLoop::AllEvents, 0);

	QStringList urls = m_cbFileName->urls();
	urls.insert(0, "file://"+path); // add type of path
	m_cbFileName->setUrls(urls);
	m_cbFileName->setCurrentText(urls.first());
	fileNameChanged(path); // why do I have to call this function separately
}

/*!
	hides the MQTT related items of the widget
*/
void ImportFileWidget::setMQTTVisible(bool visible) {
	ui.lConnections->setVisible(visible);
	ui.cbConnection->setVisible(visible);
	ui.bManageConnections->setVisible(visible);

	//topics
	if (ui.cbConnection->currentIndex() != -1 && visible) {
		ui.lTopics->setVisible(true);
		ui.frameSubscriptions->setVisible(true);
#ifdef HAVE_MQTT
		m_subscriptionWidget->setVisible(true);
		m_subscriptionWidget->makeVisible(true);
#endif
	} else {
		ui.lTopics->setVisible(false);
		ui.frameSubscriptions->setVisible(false);
#ifdef HAVE_MQTT
		m_subscriptionWidget->setVisible(false);
		m_subscriptionWidget->makeVisible(false);
#endif
	}

	//will message
	ui.lLWT->setVisible(visible);
	ui.bLWT->setVisible(visible);
}

#ifdef HAVE_MQTT
/*!
 * returns \c true if there is a valid connection to an MQTT broker and the user has subscribed to at least 1 topic,
 * returns \c false otherwise.
 */
bool ImportFileWidget::isMqttValid() {
	if (!m_client)
		return false;

	bool connected = (m_client->state() == QMqttClient::ClientState::Connected);
	bool subscribed = (m_subscriptionWidget->subscriptionCount() > 0);
	bool fileTypeOk = false;
	if (this->currentFileType() == AbstractFileFilter::FileType::Ascii)
		fileTypeOk = true;

	return connected && subscribed && fileTypeOk;
}

/*!
 *\brief Unsubscribes from the given topic, and removes any data connected to it
 *
 * \param topicName the name of a topic we want to unsubscribe from
 */
void ImportFileWidget::unsubscribeFromTopic(const QString& topicName, QVector<QTreeWidgetItem*> children) {
	if (topicName.isEmpty())
		return;

	QMqttTopicFilter filter{topicName};
	m_client->unsubscribe(filter);

	for (int i = 0; i< m_mqttSubscriptions.count(); ++i)
		if (m_mqttSubscriptions[i]->topic().filter() == topicName) {
			m_mqttSubscriptions.remove(i);
			break;
		}

	QMapIterator<QMqttTopicName, QMqttMessage> j(m_lastMessage);
	while (j.hasNext()) {
		j.next();
        if (MQTTSubscriptionWidget::checkTopicContains(topicName, j.key().name()))
			m_lastMessage.remove(j.key());
	}

	for (int i = 0; i < m_subscribedTopicNames.size(); ++i) {
        if (MQTTSubscriptionWidget::checkTopicContains(topicName, m_subscribedTopicNames[i])) {
			m_subscribedTopicNames.remove(i);
			i--;
		}
	}

	if (m_willSettings.willTopic == topicName) {
		if (m_subscriptionWidget->subscriptionCount() > 0)
			m_willSettings.willTopic = children[0]->text(0);
		else
			m_willSettings.willTopic.clear();
	}

	//signals that there was a change among the subscribed topics
	emit subscriptionsChanged();
	refreshPreview();
}
#endif

/************** SLOTS **************************************************************/

QString absolutePath(const QString& fileName) {
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
		m_cbFileName->setStyleSheet(QString());
	else
		m_cbFileName->setStyleSheet("QComboBox{background:red;}");

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
		initOptionsWidget();

		emit fileNameChanged();
		return;
	}

	if (currentSourceType() == LiveDataSource::FileOrPipe) {
		const AbstractFileFilter::FileType fileType = AbstractFileFilter::fileType(fileName);
		for (int i = 0; i < ui.cbFileType->count(); ++i) {
			if (static_cast<AbstractFileFilter::FileType>(ui.cbFileType->itemData(i).toInt()) == fileType) {
				// automatically select a new file type
				if (ui.cbFileType->currentIndex() != i) {
					ui.cbFileType->setCurrentIndex(i); // will call the slot fileTypeChanged which updates content and preview

					emit fileNameChanged();
					return;
				} else {
					initOptionsWidget();
					updateContent(fileName);
					break;
				}
			}
		}
	}

	emit fileNameChanged();
	refreshPreview();
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
	AbstractFileFilter::FileType fileType = currentFileType();
	DEBUG("ImportFileWidget::fileTypeChanged " << ENUM_TO_STRING(AbstractFileFilter, FileType, fileType));
	initOptionsWidget();

	//default
	ui.lFilter->show();
	ui.cbFilter->show();

	//different file types show different number of tabs in ui.tabWidget.
	//when switching from the previous file type we re-set the tab widget to its original state
	//and remove/add the required tabs further below
	for (int i = 0; i<ui.tabWidget->count(); ++i)
		ui.tabWidget->removeTab(0);

	ui.tabWidget->addTab(ui.tabDataFormat, i18n("Data format"));
	ui.tabWidget->addTab(ui.tabDataPreview, i18n("Preview"));
	if (!m_liveDataSource)
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
		QString tempFileName = fileName();
		const QString& fileName = absolutePath(tempFileName);
		if (QFile::exists(fileName))
			updateContent(fileName);
	}

	//for file types other than ASCII and binary we support re-reading the whole file only
	//select "read whole file" and deactivate the combobox
	if (m_liveDataSource && (fileType != AbstractFileFilter::Ascii && fileType != AbstractFileFilter::Binary)) {
		ui.cbReadingType->setCurrentIndex(LiveDataSource::ReadingType::WholeFile);
		ui.cbReadingType->setEnabled(false);
	} else
		ui.cbReadingType->setEnabled(true);

	refreshPreview();
}

// file type specific option widgets
void ImportFileWidget::initOptionsWidget() {
	DEBUG("ImportFileWidget::initOptionsWidget for " << ENUM_TO_STRING(AbstractFileFilter, FileType, currentFileType()));
	switch (currentFileType()) {
	case AbstractFileFilter::Ascii: {
		if (!m_asciiOptionsWidget) {
			QWidget* asciiw = new QWidget();
			m_asciiOptionsWidget = std::unique_ptr<AsciiOptionsWidget>(new AsciiOptionsWidget(asciiw));
			m_asciiOptionsWidget->loadSettings();
			ui.swOptions->addWidget(asciiw);
		}

		//for MQTT topics we don't allow to set the vector names since the different topics
		//can have different number of columns
		bool isMQTT = (currentSourceType() == LiveDataSource::MQTT);
		m_asciiOptionsWidget->showAsciiHeaderOptions(!isMQTT);
		m_asciiOptionsWidget->showTimestampOptions(isMQTT);

		ui.swOptions->setCurrentWidget(m_asciiOptionsWidget->parentWidget());
		break;
	}
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
		} else
			m_fitsOptionsWidget->clear();
		ui.swOptions->setCurrentWidget(m_fitsOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::JSON:
		if (!m_jsonOptionsWidget) {
			QWidget* jsonw = new QWidget();
			m_jsonOptionsWidget = std::unique_ptr<JsonOptionsWidget>(new JsonOptionsWidget(jsonw, this));
			ui.tvJson->setModel(m_jsonOptionsWidget->model());
			ui.swOptions->addWidget(jsonw);
			m_jsonOptionsWidget->loadSettings();
		} else
			m_jsonOptionsWidget->clearModel();
		ui.swOptions->setCurrentWidget(m_jsonOptionsWidget->parentWidget());
		showJsonModel(true);
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
	return m_hdf5OptionsWidget->selectedNames();
}

const QStringList ImportFileWidget::selectedNetCDFNames() const {
	return m_netcdfOptionsWidget->selectedNames();
}

const QStringList ImportFileWidget::selectedFITSExtensions() const {
	return m_fitsOptionsWidget->selectedExtensions();
}

const QStringList ImportFileWidget::selectedROOTNames() const {
	return m_rootOptionsWidget->selectedNames();
}

/*!
	shows the dialog with the information about the file(s) to be imported.
*/
void ImportFileWidget::fileInfoDialog() {
	QStringList files = fileName().split(';');
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

	DEBUG("ImportFileWidget::refreshPreview()");
	WAIT_CURSOR;

	QString tempFileName = fileName();
	QString fileName = absolutePath(tempFileName);
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

		auto filter = static_cast<AsciiFilter*>(currentFileFilter());

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
			} else
				DEBUG("failed connect to local socket " << fileName.toStdString() << " - " << lsocket.errorString().toStdString());

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
			} else
				DEBUG("failed to connect to TCP socket " << " - " << tcpSocket.errorString().toStdString());

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
			} else
				DEBUG("failed to connect to UDP socket " << " - " << udpSocket.errorString().toStdString());

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
			} else
				DEBUG("	ERROR: failed to open serial port. error: " << sPort.error());

			break;
		}
		case LiveDataSource::SourceType::MQTT: {
#ifdef HAVE_MQTT
			//show the preview for the currently selected topic
			auto* item = m_subscriptionWidget->currentItem();
			if (item && item->childCount() == 0) { //only preview if the lowest level (i.e. a topic) is selected
				const QString& topicName = item->text(0);
				auto i = m_lastMessage.find(topicName);
				if (i != m_lastMessage.end())
					importedStrings = filter->preview(i.value().payload().data());
				else
					importedStrings << QStringList{i18n("No data arrived yet for the selected topic")};
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
		auto filter = static_cast<BinaryFilter*>(currentFileFilter());
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
		DEBUG("ImportFileWidget::refreshPreview: HDF5");
		auto filter = static_cast<HDF5Filter*>(currentFileFilter());
		lines = m_hdf5OptionsWidget->lines();

		importedStrings = filter->readCurrentDataSet(fileName, nullptr, ok, AbstractFileFilter::Replace, lines);
		tmpTableWidget = m_hdf5OptionsWidget->previewWidget();
		break;
	}
	case AbstractFileFilter::NETCDF: {
		auto filter = static_cast<NetCDFFilter*>(currentFileFilter());
		lines = m_netcdfOptionsWidget->lines();

		importedStrings = filter->readCurrentVar(fileName, nullptr, AbstractFileFilter::Replace, lines);
		tmpTableWidget = m_netcdfOptionsWidget->previewWidget();
		break;
	}
	case AbstractFileFilter::FITS: {
		auto filter = static_cast<FITSFilter*>(currentFileFilter());
		lines = m_fitsOptionsWidget->lines();

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
		auto filter = static_cast<JsonFilter*>(currentFileFilter());
		m_jsonOptionsWidget->applyFilterSettings(filter, ui.tvJson->currentIndex());
		importedStrings = filter->preview(fileName);

		vectorNameList = filter->vectorNames();
		columnModes = filter->columnModes();
		break;
	}
	case AbstractFileFilter::ROOT: {
		auto filter = static_cast<ROOTFilter*>(currentFileFilter());
		lines = m_rootOptionsWidget->lines();
		m_rootOptionsWidget->setNRows(filter->rowsInCurrentObject(fileName));
		importedStrings = filter->previewCurrentObject(
		                      fileName,
		                      m_rootOptionsWidget->startRow(),
		                      qMin(m_rootOptionsWidget->startRow() + m_rootOptionsWidget->lines() - 1,
		                           m_rootOptionsWidget->endRow())
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
		auto filter = static_cast<NgspiceRawAsciiFilter*>(currentFileFilter());
		importedStrings = filter->preview(fileName, lines);
		vectorNameList = filter->vectorNames();
		columnModes = filter->columnModes();
		break;
	}
	case AbstractFileFilter::NgspiceRawBinary: {
		ui.tePreview->clear();
		auto filter = static_cast<NgspiceRawBinaryFilter*>(currentFileFilter());
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
				const int cols = importedStrings[i].size() > maxColumns ? maxColumns : importedStrings[i].size();
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
	} else
		m_fileEmpty = true;

	emit previewRefreshed();

	RESET_CURSOR;
}

void ImportFileWidget::updateContent(const QString& fileName) {
	QDEBUG("ImportFileWidget::updateContent(): file name = " << fileName);
	if (auto filter = currentFileFilter()) {
		switch (filter->type()) {
		case AbstractFileFilter::HDF5:
			m_hdf5OptionsWidget->updateContent(static_cast<HDF5Filter*>(filter), fileName);
			break;
		case AbstractFileFilter::NETCDF:
			m_netcdfOptionsWidget->updateContent(static_cast<NetCDFFilter*>(filter), fileName);
			break;
		case AbstractFileFilter::FITS:
#ifdef HAVE_FITS
			m_fitsOptionsWidget->updateContent(static_cast<FITSFilter*>(filter), fileName);
#endif
			break;
		case AbstractFileFilter::ROOT:
			m_rootOptionsWidget->updateContent(static_cast<ROOTFilter*>(filter), fileName);
			break;
		case AbstractFileFilter::JSON:
			m_jsonOptionsWidget->loadDocument(fileName);
			ui.tvJson->setExpanded( m_jsonOptionsWidget->model()->index(0, 0), true); //expand the root node
			break;
		case AbstractFileFilter::Ascii:
		case AbstractFileFilter::Binary:
		case AbstractFileFilter::Image:
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
		m_cbFileName->show();
		ui.bFileInfo->show();
		ui.bOpen->show();
		ui.lRelativePath->show();
		ui.chbRelativePath->show();
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

		fileNameChanged(fileName());
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
		m_cbFileName->hide();
		ui.bFileInfo->hide();
		ui.bOpen->hide();
		ui.lRelativePath->hide();
		ui.chbRelativePath->hide();
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
		m_cbFileName->show();
		ui.bFileInfo->hide();
		ui.bOpen->show();
		ui.lRelativePath->hide();
		ui.chbRelativePath->hide();

		ui.lSampleSize->hide();
		ui.sbSampleSize->hide();
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
		m_cbFileName->hide();
		ui.bFileInfo->hide();
		ui.bOpen->hide();
		ui.lRelativePath->hide();
		ui.chbRelativePath->hide();
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
		for (int i = 0; i < ui.cbFileType->count(); ++i) {
			if (static_cast<AbstractFileFilter::FileType>(ui.cbFileType->itemData(i).toInt()) == AbstractFileFilter::Ascii) {
				if (ui.cbFileType->currentIndex() == i)
					initOptionsWidget();
				else
					ui.cbFileType->setCurrentIndex(i);

				break;
			}
		}
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
		m_cbFileName->hide();
		ui.bFileInfo->hide();
		ui.bOpen->hide();
		ui.lRelativePath->hide();
		ui.chbRelativePath->hide();
		ui.chbLinkFile->hide();

		setMQTTVisible(true);

		ui.cbFileType->setEnabled(true);
		ui.gbOptions->setEnabled(true);
		ui.bManageFilters->setEnabled(true);
		ui.cbFilter->setEnabled(true);

		//in case there are already connections defined,
		//show the available topics for the currently selected connection
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
		if (ui.cbFileType->currentIndex() > 1)
			ui.cbFileType->setCurrentIndex(1);

		//"whole file" read option is available for file or pipe only, disable it
		typeModel = qobject_cast<const QStandardItemModel*>(ui.cbReadingType->model());
		QStandardItem* item = typeModel->item(LiveDataSource::WholeFile);
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (static_cast<LiveDataSource::ReadingType>(ui.cbReadingType->currentIndex()) == LiveDataSource::WholeFile)
			ui.cbReadingType->setCurrentIndex(LiveDataSource::TillEnd);

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
	if (m_initialisingMQTT || ui.cbConnection->currentIndex() == -1)
		return;

	WAIT_CURSOR;

	//disconnected from the broker that was selected before, if this is the case
	if (m_client && m_client->state() == QMqttClient::ClientState::Connected) {
		emit MQTTClearTopics();
		disconnect(m_client, &QMqttClient::disconnected, this, &ImportFileWidget::onMqttDisconnect);
		QDEBUG("Disconnecting from " << m_client->hostname());
		m_client->disconnectFromHost();
		delete m_client;
	}

	//determine the connection settings for the new broker and initialize the mqtt client
	KConfig config(m_configPath, KConfig::SimpleConfig);
	KConfigGroup group = config.group(ui.cbConnection->currentText());

	m_client = new QMqttClient;
	connect(m_client, &QMqttClient::connected, this, &ImportFileWidget::onMqttConnect);
	connect(m_client, &QMqttClient::disconnected, this, &ImportFileWidget::onMqttDisconnect);
	connect(m_client, &QMqttClient::messageReceived, this, &ImportFileWidget::mqttMessageReceived);
	connect(m_client, &QMqttClient::errorChanged, this, &ImportFileWidget::mqttErrorChanged);

	m_client->setHostname(group.readEntry("Host"));
	m_client->setPort(group.readEntry("Port").toUInt());

	const bool useID = group.readEntry("UseID").toUInt();
	if (useID)
		m_client->setClientId(group.readEntry("ClientID"));

	const bool useAuthentication = group.readEntry("UseAuthentication").toUInt();
	if (useAuthentication) {
		m_client->setUsername(group.readEntry("UserName"));
		m_client->setPassword(group.readEntry("Password"));
	}

	//connect to the selected broker
	QDEBUG("Connect to " << m_client->hostname() << ":" << m_client->port());
	m_connectTimeoutTimer->start();
	m_client->connectToHost();
}

/*!
 *\brief called when the client connects to the broker successfully.
 * subscribes to every topic (# wildcard) in order to later list every available topic
 */
void ImportFileWidget::onMqttConnect() {
	if (m_client->error() == QMqttClient::NoError) {
		m_connectTimeoutTimer->stop();
		ui.frameSubscriptions->setVisible(true);
		m_subscriptionWidget->setVisible(true);
		m_subscriptionWidget->makeVisible(true);

		if (!m_client->subscribe(QMqttTopicFilter(QLatin1String("#")), 1))
			QMessageBox::critical(this, i18n("Couldn't subscribe"), i18n("Couldn't subscribe to all available topics. Something went wrong"));
	}
	emit subscriptionsChanged();
	RESET_CURSOR;
}

/*!
 *\brief called when the client disconnects from the broker successfully
 * removes every information about the former connection
 */
void ImportFileWidget::onMqttDisconnect() {
	DEBUG("Disconected from " << m_client->hostname().toStdString());
	m_connectTimeoutTimer->stop();

	ui.lTopics->hide();
	ui.frameSubscriptions->hide();
	ui.lLWT->hide();
	ui.bLWT->hide();

	ui.cbConnection->setItemText(ui.cbConnection->currentIndex(), ui.cbConnection->currentText() + " " + i18n("(Disconnected)"));

	emit subscriptionsChanged();
	RESET_CURSOR;
	QMessageBox::critical(this, i18n("Disconnected"),
	                      i18n("Disconnected from the broker '%1' before the connection was successful.", m_client->hostname()));
}

/*!
 *\brief called when the subscribe button is pressed
 * subscribes to the topic represented by the current item of twTopics
 */
void ImportFileWidget::mqttSubscribe(const QString& name, uint QoS) {
	const QMqttTopicFilter filter {name};
	QMqttSubscription* tempSubscription = m_client->subscribe(filter, static_cast<quint8>(QoS) );

	if (tempSubscription) {
		m_mqttSubscriptions.push_back(tempSubscription);
		connect(tempSubscription, &QMqttSubscription::messageReceived, this, &ImportFileWidget::mqttSubscriptionMessageReceived);
		emit subscriptionsChanged();
	}
}

/*!
 *\brief called when the client receives a message
 * if the message arrived from a new topic, the topic is put in twTopics
 */
void ImportFileWidget::mqttMessageReceived(const QByteArray& message, const QMqttTopicName& topic) {
	Q_UNUSED(message);
// 	qDebug()<<"received " << topic.name();
	if (m_addedTopics.contains(topic.name()))
		return;

	m_addedTopics.push_back(topic.name());
	m_subscriptionWidget->setTopicTreeText(i18n("Available (%1)", m_addedTopics.size()));
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
			for (int i = 0; i < m_subscriptionWidget->topicCount(); ++i) {
				if (m_subscriptionWidget->topLevelTopic(i)->text(0) == list.at(0)) {
					topItemIdx = i;
					break;
				}
			}

			//if not we simply add every level of the topic to the tree
			if (topItemIdx < 0) {
				QTreeWidgetItem* currentItem = new QTreeWidgetItem(name);
				m_subscriptionWidget->addTopic(currentItem);
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
				QTreeWidgetItem* currentItem = m_subscriptionWidget->topLevelTopic(topItemIdx);
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
		m_subscriptionWidget->addTopic(new QTreeWidgetItem(name));
	}

	//if a subscribed topic contains the new topic, we have to update twSubscriptions
	for (int i = 0; i < m_subscriptionWidget->subscriptionCount(); ++i) {
		const QStringList subscriptionName = m_subscriptionWidget->topLevelSubscription(i)->text(0).split('/', QString::SkipEmptyParts);
		if (!subscriptionName.isEmpty()) {
			if (rootName == subscriptionName.first()) {
				QVector<QString> subscriptions;
				for(int i = 0; i < m_mqttSubscriptions.size(); ++i)
					subscriptions.push_back(m_mqttSubscriptions[i]->topic().filter());
				emit updateSubscriptionTree(subscriptions);
				break;
			}
		}
	}

	//signals that a newTopic was added, in order to fill the completer of leTopics
	emit newTopic(rootName);
}

/*!
 *\brief called when the client receives a message from a subscribed topic (that isn't the "#" wildcard)
 */
void ImportFileWidget::mqttSubscriptionMessageReceived(const QMqttMessage &msg) {
	QDEBUG("message received from: " << msg.topic().name());

	if (!m_subscribedTopicNames.contains(msg.topic().name()))
		m_subscribedTopicNames.push_back(msg.topic().name());

	//update the last message for the topic
	m_lastMessage[msg.topic()] = msg;
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
	case QMqttClient::Mqtt5SpecificError:
		break;
	default:
		break;
	}
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
	RESET_CURSOR;
	QMessageBox::warning(this, i18n("Warning"), i18n("Connecting to the given broker timed out! Try changing the settings"));
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
	DEBUG("ImportFileWidget: reading available MQTT connections");
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
	for (int i = 0; i < m_subscriptionWidget->subscriptionCount(); ++i)
        MQTTSubscriptionWidget::findSubscriptionLeafChildren(children, m_subscriptionWidget->topLevelSubscription(i));

	QVector<QString> topics;
	for (int i = 0; i < children.size(); ++i)
		topics.append(children[i]->text(0));

	MQTTWillSettingsWidget willSettingsWidget(&menu, m_willSettings, topics);

	connect(&willSettingsWidget, &MQTTWillSettingsWidget::applyClicked, [this, &menu, &willSettingsWidget]() {
		m_willSettings = willSettingsWidget.will();
		menu.close();
	});
	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&willSettingsWidget);
	menu.addAction(widgetAction);

	const QPoint pos(ui.bLWT->sizeHint().width(),ui.bLWT->sizeHint().height());
	menu.exec(ui.bLWT->mapToGlobal(pos));
}

void ImportFileWidget::enableWill(bool enable) {
	if(enable) {
		if(!ui.bLWT->isEnabled())
			ui.bLWT->setEnabled(enable);
	} else
		ui.bLWT->setEnabled(enable);
}

#endif
