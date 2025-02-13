/*
	File                 : ImportFileWidget.cpp
	Project              : LabPlot
	Description          : import file data widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2025 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2009-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2018 Fabian Kristof <fkristofszabolcs@gmail.com>
	SPDX-FileCopyrightText: 2018-2019 Kovacs Ferencz <kferike98@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImportFileWidget.h"
#include "AsciiOptionsWidget.h"
#include "BinaryOptionsWidget.h"
#include "CANOptionsWidget.h"
#include "FITSOptionsWidget.h"
#include "HDF5OptionsWidget.h"
#include "ImageOptionsWidget.h"
#include "JsonOptionsWidget.h"
#include "MatioOptionsWidget.h"
#include "McapOptionsWidget.h"
#include "NetCDFOptionsWidget.h"
#include "OdsOptionsWidget.h"
#include "ROOTOptionsWidget.h"
#include "XLSXOptionsWidget.h"
#include "backend/core/Settings.h"
#include "backend/datasources/filters/filters.h"
#include "backend/lib/hostprocess.h"
#include "backend/lib/macros.h"
#include "frontend/TemplateHandler.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KUrlComboBox>

#include <QCompleter>
#include <QDir>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QIntValidator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QProcess>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTcpSocket>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QUdpSocket>
#include <QWhatsThis>

#ifdef HAVE_MQTT
#include "MQTTConnectionManagerDialog.h"
#include "MQTTSubscriptionWidget.h"
#include "frontend/widgets/MQTTWillSettingsWidget.h"
#include <QJsonDocument>
#include <QMenu>
#include <QMqttClient>
#include <QMqttMessage>
#include <QMqttSubscription>
#include <QMqttTopicFilter>
#include <QWidgetAction>
#endif

namespace {
	enum FilterSettingsHandlingIndex {
		Automatic = 0,
		Custom = 1,
	};
}

QString ImportFileWidget::absolutePath(const QString& fileName) {
	if (fileName.isEmpty())
		return fileName;

#ifdef HAVE_WINDOWS
	if (fileName.size() == 1 || (fileName.at(0) != QLatin1Char('/') && fileName.at(1) != QLatin1Char(':')))
#else
	if (fileName.at(0) != QLatin1Char('/'))
#endif
		return QDir::homePath() + QLatin1Char('/') + fileName;

	return fileName;
}

/*!
   \class ImportFileWidget
   \brief Widget for importing data from a file.

   \ingroup frontend
*/
ImportFileWidget::ImportFileWidget(QWidget* parent, bool liveDataSource, const QString& fileName, bool embedded)
	: QWidget(parent)
	, m_fileName(fileName)
	, m_liveDataSource(liveDataSource)
	, m_embedded(embedded)
#ifdef HAVE_MQTT
	, m_subscriptionWidget(new MQTTSubscriptionWidget(this))
#endif
{
	ui.setupUi(this);

	// add supported file types	(see also ExportSpreadsheetDialog.cpp)
	if (!liveDataSource) {
		ui.cbFileType->addItem(i18n("ASCII data"), static_cast<int>(AbstractFileFilter::FileType::Ascii));
		ui.cbFileType->addItem(i18n("Binary data"), static_cast<int>(AbstractFileFilter::FileType::Binary));
		ui.cbFileType->addItem(i18n("Image"), static_cast<int>(AbstractFileFilter::FileType::Image));
#ifdef HAVE_QXLSX
		ui.cbFileType->addItem(i18n("Excel 2007+ (XSLX)"), static_cast<int>(AbstractFileFilter::FileType::XLSX));
#endif
#ifdef HAVE_ORCUS
		ui.cbFileType->addItem(i18n("OpenDocument Spreadsheet (ODS)"), static_cast<int>(AbstractFileFilter::FileType::Ods));
#endif
#ifdef HAVE_HDF5
		ui.cbFileType->addItem(i18n("Hierarchical Data Format 5 (HDF5)"), static_cast<int>(AbstractFileFilter::FileType::HDF5));
#endif
#ifdef HAVE_NETCDF
		ui.cbFileType->addItem(i18n("Network Common Data Format (NetCDF)"), static_cast<int>(AbstractFileFilter::FileType::NETCDF));
#endif
#ifdef HAVE_VECTOR_BLF
		ui.cbFileType->addItem(i18n("Vector Binary Logfile (BLF)"), static_cast<int>(AbstractFileFilter::FileType::VECTOR_BLF));
#endif
#ifdef HAVE_FITS
		ui.cbFileType->addItem(i18n("Flexible Image Transport System Data Format (FITS)"), static_cast<int>(AbstractFileFilter::FileType::FITS));
#endif
		ui.cbFileType->addItem(i18n("JSON Data"), static_cast<int>(AbstractFileFilter::FileType::JSON));
#ifdef HAVE_ZIP
		ui.cbFileType->addItem(i18n("ROOT (CERN)"), static_cast<int>(AbstractFileFilter::FileType::ROOT));
#endif
		ui.cbFileType->addItem(i18n("Spice"), static_cast<int>(AbstractFileFilter::FileType::Spice));
#ifdef HAVE_READSTAT
		ui.cbFileType->addItem(i18n("SAS, Stata or SPSS"), static_cast<int>(AbstractFileFilter::FileType::READSTAT));
#endif
#ifdef HAVE_MATIO
		ui.cbFileType->addItem(i18n("MATLAB MAT file"), static_cast<int>(AbstractFileFilter::FileType::MATIO));
#endif
#ifdef HAVE_MCAP
		ui.cbFileType->addItem(i18n("MCAP Data"), static_cast<int>(AbstractFileFilter::FileType::MCAP));
#endif
		// hide widgets relevant for live data reading only
		ui.lRelativePath->hide();
		ui.chbRelativePath->hide();
		ui.lSourceType->hide();
		ui.cbSourceType->hide();
		ui.gbUpdateOptions->hide();
	} else { // Live data source
		ui.cbFileType->addItem(i18n("ASCII Data"), static_cast<int>(AbstractFileFilter::FileType::Ascii));
		ui.cbFileType->addItem(i18n("Binary Data"), static_cast<int>(AbstractFileFilter::FileType::Binary));
#ifdef HAVE_ZIP
		ui.cbFileType->addItem(i18n("ROOT (CERN)"), static_cast<int>(AbstractFileFilter::FileType::ROOT));
#endif
		ui.cbFileType->addItem(i18n("Spice"), static_cast<int>(AbstractFileFilter::FileType::Spice));

		ui.lePort->setValidator(new QIntValidator(ui.lePort));
		ui.cbBaudRate->addItems(LiveDataSource::supportedBaudRates());
		ui.cbSerialPort->addItems(LiveDataSource::availablePorts());

		ui.tabWidget->removeTab(2);

		ui.chbLinkFile->setToolTip(i18n("If this option is checked, only the link to the file is stored in the project file but not its content."));
		ui.chbRelativePath->setToolTip(i18n("If this option is checked, the relative path of the file (relative to project's folder) will be saved."));
	}

	// hide options that will be activated on demand
	ui.gbOptions->hide();
	ui.gbUpdateOptions->hide();
	setMQTTVisible(false);

	ui.cbReadingType->addItem(i18n("Continuously Fixed"), static_cast<int>(LiveDataSource::ReadingType::ContinuousFixed));
	ui.cbReadingType->addItem(i18n("From End"), static_cast<int>(LiveDataSource::ReadingType::FromEnd));
	ui.cbReadingType->addItem(i18n("Till End"), static_cast<int>(LiveDataSource::ReadingType::TillEnd));
	ui.cbReadingType->addItem(i18n("Whole File"), static_cast<int>(LiveDataSource::ReadingType::WholeFile));

	ui.bOpen->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
	ui.bOpenDBC->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
	ui.bFileInfo->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
	ui.bRefreshPreview->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));

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
	m_cbFileName = new KUrlComboBox(KUrlComboBox::Mode::Files, this);
	m_cbFileName->setMaxItems(7);
	auto* gridLayout = dynamic_cast<QGridLayout*>(ui.gbDataSource->layout());
	if (gridLayout)
		gridLayout->addWidget(m_cbFileName, 1, 2, 1, 3);

	m_cbDBCFileName = new KUrlComboBox(KUrlComboBox::Mode::Files, this);
	m_cbDBCFileName->setMaxItems(7);
	gridLayout = dynamic_cast<QGridLayout*>(ui.gbDataSource->layout());
	if (gridLayout)
		gridLayout->addWidget(m_cbDBCFileName, 2, 2, 1, 3);

	// tooltips
	QString info = i18n(
		"Specify how the data source has to be processed on every read:"
		"<ul>"
		"<li>Continuously fixed - fixed amount of samples is processed starting from the beginning of the newly received data.</li>"
		"<li>From End - fixed amount of samples is processed starting from the end of the newly received data.</li>"
		"<li>Till the End - all newly received data is processed.</li>"
		"<li>Whole file - on every read the whole file is re-read completely and processed. Only available for \"File Or Named Pipe\" data sources.</li>"
		"</ul>");
	ui.lReadingType->setToolTip(info);
	ui.cbReadingType->setToolTip(info);

	info = i18n(
		"Number of samples (lines) to be processed on every read.\n"
		"Only needs to be specified for the reading mode \"Continuously Fixed\" and \"From End\".");
	ui.lSampleSize->setToolTip(info);
	ui.sbSampleSize->setToolTip(info);

	info = i18n(
		"Specify when and how frequently the data source needs to be read:"
		"<ul>"
		"<li>Periodically - the data source is read periodically with user specified time interval.</li>"
		"<li>On New Data - the data source is read when new data arrives.</li>"
		"</ul>");
	ui.lUpdateType->setToolTip(info);
	ui.cbUpdateType->setToolTip(info);

	info = i18n("Specify how frequently the data source has to be read.");
	ui.lUpdateInterval->setToolTip(info);
	ui.sbUpdateInterval->setToolTip(info);

	info = i18n(
		"Specify how many samples need to be kept in memory after reading.\n"
		"Use \"All\" if all data has to be kept.");
	ui.lKeepLastValues->setToolTip(info);
	ui.sbKeepNValues->setToolTip(info);

	info = i18n("Enable to use the first row of the selected data region for the column names of the spreadsheet.");
	ui.lFirstRowAsColNames->setToolTip(info);
	ui.chbFirstRowAsColName->setToolTip(info);
#ifdef HAVE_MQTT
	ui.cbSourceType->addItem(QStringLiteral("MQTT"));
	m_configPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst() + QStringLiteral("MQTT_connections");

	// add subscriptions widget
	layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->addWidget(m_subscriptionWidget);
	ui.frameSubscriptions->setLayout(layout);

	ui.bManageConnections->setIcon(QIcon::fromTheme(QStringLiteral("network-server")));
	ui.bManageConnections->setToolTip(i18n("Manage MQTT connections"));

	info = i18n("Specify the 'Last Will and Testament' message (LWT). At least one topic has to be subscribed.");
	ui.lLWT->setToolTip(info);
	ui.bLWT->setToolTip(info);
	ui.bLWT->setEnabled(false);
	ui.bLWT->setIcon(ui.bLWT->style()->standardIcon(QStyle::SP_FileDialogDetailedView));
#endif

	// templates for filter properties
	m_templateHandler = new TemplateHandler(this, QLatin1String("import"), false);
	m_templateHandler->setSaveDefaultAvailable(false);
	m_templateHandler->setLoadAvailable(false);
	m_templateHandler->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
	ui.hLayoutFilter->addWidget(m_templateHandler);
	connect(m_templateHandler, &TemplateHandler::saveConfigRequested, this, &ImportFileWidget::saveConfigAsTemplate);
	connect(ui.sbEndRow, &QSpinBox::valueChanged, [this] (int value) {
		if (value > 0) {
			ui.sbPreviewLines->setEnabled(false);
			ui.sbPreviewLines->setToolTip(i18n("The number of rows is calculated from the values of the data portion tab"));
		} else {
			ui.sbPreviewLines->setEnabled(true);
			ui.sbPreviewLines->setToolTip(QStringLiteral(""));
		}
	});
	connect(ui.sbPreviewPrecision, &QSpinBox::valueChanged, [this] (int value) {
		if (m_currentFilter)
			m_currentFilter->setPreviewPrecision(value);
	});
}

void ImportFileWidget::loadSettings() {
	m_suppressRefresh = true;

	// load last used settings
	QString confName;
	if (m_liveDataSource)
		confName = QStringLiteral("LiveDataImport");
	else
		confName = QStringLiteral("FileImport");
	KConfigGroup conf = Settings::group(confName);

	// read the source type first since settings in fileNameChanged() depend on this
	ui.cbSourceType->setCurrentIndex(conf.readEntry("SourceType").toInt());

	// general settings
	auto fileType = static_cast<AbstractFileFilter::FileType>(conf.readEntry("Type", 0));
	for (int i = 0; i < ui.cbFileType->count(); ++i) {
		if (static_cast<AbstractFileFilter::FileType>(ui.cbFileType->itemData(i).toInt()) == fileType) {
			if (ui.cbFileType->currentIndex() == i)
				initOptionsWidget();
			else
				ui.cbFileType->setCurrentIndex(i);

			break;
		}
	}

	auto urls = m_cbFileName->urls();
	urls.append(conf.readXdgListEntry("LastImportedFiles"));
	m_cbFileName->setUrls(urls);
	if (m_fileName.isEmpty())
		m_cbFileName->setUrl(QUrl(conf.readEntry("LastImportedFile", "")));
	else {
		if (m_fileName.contains(QLatin1Char('\\')))	// Windows path
			m_cbFileName->setUrl(QUrl::fromLocalFile(m_fileName));
		else
			m_cbFileName->setUrl(QUrl(m_fileName));
	}

	urls = m_cbDBCFileName->urls();
	urls.append(conf.readXdgListEntry("LastImportedDBCFiles"));
	m_cbDBCFileName->setUrls(urls);
	if (m_dbcFileName.isEmpty())
		m_cbDBCFileName->setUrl(QUrl(conf.readEntry("LastImportedDBCFile", "")));
	else {
		if (m_fileName.contains(QLatin1Char('\\')))	// Windows path
			m_cbDBCFileName->setUrl(QUrl::fromLocalFile(m_dbcFileName));
		else
			m_cbDBCFileName->setUrl(QUrl(m_dbcFileName));
	}

	ui.sbPreviewLines->setValue(conf.readEntry("PreviewLines", 100));
	ui.chbFirstRowAsColName->setChecked(conf.readEntry("ExcelFirstLineAsColNames", false));

	// live data related settings
	ui.cbBaudRate->setCurrentIndex(conf.readEntry("BaudRate", 13)); // index for bautrate 19200b/s
	ui.cbReadingType->setCurrentIndex(conf.readEntry("ReadingType", static_cast<int>(LiveDataSource::ReadingType::WholeFile)));
	ui.cbSerialPort->setCurrentIndex(conf.readEntry("SerialPort").toInt());
	ui.cbUpdateType->setCurrentIndex(conf.readEntry("UpdateType", static_cast<int>(LiveDataSource::UpdateType::NewData)));
	ui.leHost->setText(conf.readEntry("Host", ""));
	ui.sbKeepNValues->setValue(conf.readEntry("KeepNValues", 0)); // keep all values
	ui.lePort->setText(conf.readEntry("Port", ""));
	ui.sbSampleSize->setValue(conf.readEntry("SampleSize", 1));
	ui.sbUpdateInterval->setValue(conf.readEntry("UpdateInterval", 1000));
	ui.chbLinkFile->setChecked(conf.readEntry("LinkFile", false));
	ui.chbRelativePath->setChecked(conf.readEntry("RelativePath", false));

#ifdef HAVE_MQTT
	// read available MQTT connections
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

	const QString& willStatistics = conf.readEntry("mqttWillStatistics", "");
	const QStringList& statisticsList = willStatistics.split(QLatin1Char('|'), Qt::SkipEmptyParts);
	for (auto value : statisticsList)
		m_willSettings.willStatistics[value.toInt()] = true;
#endif

	// initialize the slots after all settings were set in order to avoid unneeded refreshes
	initSlots();

	// update the status of the widgets
	sourceTypeChanged(static_cast<int>(currentSourceType()));
	fileTypeChanged(); // call it to load the filter templates for the current file type and to select the last used index in cbFilter below
	if (automaticAllowed(currentSourceType())) {
		ui.cbFilter->setCurrentIndex(conf.readEntry("Filter", (int)FilterSettingsHandlingIndex::Automatic));
	}

	filterChanged(ui.cbFilter->currentIndex());
	updateTypeChanged(ui.cbUpdateType->currentIndex());
	readingTypeChanged(ui.cbReadingType->currentIndex());

	// all set now, refresh the content of the file and the preview for the selected dataset
	m_suppressRefresh = false;
	QTimer::singleShot(100, this, [=]() {
		WAIT_CURSOR_AUTO_RESET;
		if (currentSourceType() == LiveDataSource::SourceType::FileOrPipe) {
			const QString& file = absolutePath(fileName());
			if (QFile::exists(file))
				updateContent(file);
		}

		refreshPreview();
	});
}

bool ImportFileWidget::automaticAllowed(LiveDataSource::SourceType sourceType) {
	return sourceType != LiveDataSource::SourceType::SerialPort;
}

void ImportFileWidget::updateFilterHandlingSettings(LiveDataSource::SourceType sourceType) {
	if (automaticAllowed(sourceType)) {
		// No need to change
		ui.cbFilter->setEnabled(true);
	} else {
		ui.cbFilter->setCurrentIndex((int)FilterSettingsHandlingIndex::Custom);
		ui.cbFilter->setEnabled(false);
	}
}

ImportFileWidget::~ImportFileWidget() {
	// clean up and deletions
#ifdef HAVE_MQTT
	delete m_connectTimeoutTimer;
	delete m_subscriptionWidget;
#endif

	if (m_embedded)
		return;

	// save current settings
	QString confName;
	if (m_liveDataSource)
		confName = QStringLiteral("LiveDataImport");
	else
		confName = QStringLiteral("FileImport");
	KConfigGroup conf = Settings::group(confName);

	// general settings
	conf.writeEntry("Type", (int)currentFileType());
	conf.writeEntry("Filter", ui.cbFilter->currentIndex());
	conf.writeEntry("LastImportedFile", m_cbFileName->currentText());
	conf.writeXdgListEntry("LastImportedFiles", m_cbFileName->urls());
	conf.writeEntry("LastImportedDBCFile", m_cbDBCFileName->currentText());
	conf.writeXdgListEntry("LastImportedDBCFiles", m_cbDBCFileName->urls());
	conf.writeEntry("PreviewLines", ui.sbPreviewLines->value());
	conf.writeEntry("ExcelFirstLineAsColNames", ui.chbFirstRowAsColName->isChecked());

	// live data related settings
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
	conf.writeEntry("LinkFile", ui.chbLinkFile->isChecked());
	conf.writeEntry("RelativePath", ui.chbRelativePath->isChecked());

#ifdef HAVE_MQTT
	// MQTT related settings
	conf.writeEntry("Connection", ui.cbConnection->currentText());
	conf.writeEntry("mqttWillMessageType", static_cast<int>(m_willSettings.willMessageType));
	conf.writeEntry("mqttWillUpdateType", static_cast<int>(m_willSettings.willUpdateType));
	conf.writeEntry("mqttWillQoS", QString::number(m_willSettings.willQoS));
	conf.writeEntry("mqttWillOwnMessage", m_willSettings.willOwnMessage);
	conf.writeEntry("mqttWillUpdateInterval", QString::number(m_willSettings.willTimeInterval));
	QString willStatistics;
	for (int i = 0; i < m_willSettings.willStatistics.size(); ++i) {
		if (m_willSettings.willStatistics[i])
			willStatistics += QString::number(i) + QLatin1Char('|');
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
	if (m_mcapOptionsWidget)
		m_mcapOptionsWidget->saveSettings();
	if (m_canOptionsWidget)
		m_canOptionsWidget->saveSettings();
}

void ImportFileWidget::initSlots() {
	// SLOTs for the general part of the data source configuration
	connect(ui.cbSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, QOverload<int>::of(&ImportFileWidget::sourceTypeChanged));
	connect(m_cbFileName, &KUrlComboBox::urlActivated, this, [=](const QUrl& url) {
		fileNameChanged(url.toLocalFile());
	});
	connect(ui.leHost, &QLineEdit::textChanged, this, &ImportFileWidget::hostChanged);
	connect(ui.lePort, &QLineEdit::textChanged, this, &ImportFileWidget::portChanged);
	connect(ui.cbSerialPort, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &ImportFileWidget::portChanged);
	connect(ui.tvJson, &QTreeView::clicked, this, &ImportFileWidget::refreshPreview);

	connect(ui.bOpen, &QPushButton::clicked, this, &ImportFileWidget::selectFile);
	connect(ui.bOpenDBC, &QPushButton::clicked, this, &ImportFileWidget::selectDBCFile);
	connect(ui.bFileInfo, &QPushButton::clicked, this, &ImportFileWidget::showFileInfo);
	connect(ui.cbFileType, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &ImportFileWidget::fileTypeChanged);
	connect(ui.cbUpdateType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportFileWidget::updateTypeChanged);
	connect(ui.cbReadingType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportFileWidget::readingTypeChanged);
	connect(ui.cbFilter, QOverload<int>::of(&KComboBox::activated), this, &ImportFileWidget::filterChanged);
	connect(ui.bRefreshPreview, &QPushButton::clicked, this, &ImportFileWidget::refreshPreview);

	if (m_asciiOptionsWidget) {
		connect(m_asciiOptionsWidget.get(), &AsciiOptionsWidget::headerLineChanged, this, &ImportFileWidget::updateStartRow);
		connect(m_asciiOptionsWidget.get(), &AsciiOptionsWidget::columnModesChanged, this, &ImportFileWidget::checkValid);
	}

	connect(ui.cbMcapTopics, QOverload<int>::of(&KComboBox::activated), this, &ImportFileWidget::changeMcapTopic);

#ifdef HAVE_MQTT
	connect(ui.cbConnection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportFileWidget::mqttConnectionChanged);
	connect(ui.cbFileType, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
		Q_EMIT checkFileType();
	});
	connect(ui.bManageConnections, &QPushButton::clicked, this, &ImportFileWidget::showMQTTConnectionManager);
	connect(ui.bLWT, &QPushButton::clicked, this, &ImportFileWidget::showWillSettings);
	connect(m_subscriptionWidget, &MQTTSubscriptionWidget::makeSubscription, this, &ImportFileWidget::subscribeTopic);
	connect(m_subscriptionWidget, &MQTTSubscriptionWidget::MQTTUnsubscribeFromTopic, this, &ImportFileWidget::unsubscribeTopic);
	connect(m_subscriptionWidget, &MQTTSubscriptionWidget::enableWill, this, &ImportFileWidget::enableWill);
	connect(m_subscriptionWidget, &MQTTSubscriptionWidget::subscriptionChanged, this, &ImportFileWidget::refreshPreview);
#endif
}

/*!
 * \brief Called when the current target data containter was changed in ImportDilaog
 */
void ImportFileWidget::dataContainerChanged(AbstractAspect* aspect) {
	m_targetContainer = aspect;
	updateHeaderOptions();
}

void ImportFileWidget::enableFirstRowAsColNames(bool enable) {
	ui.chbFirstRowAsColName->setEnabled(enable);
}

/*!
 *  update header specific options that are available for some filter types (ASCII, XLSX and Ods)
 *  and for some target data containers (Spreadsheet) only
 */
void ImportFileWidget::updateHeaderOptions() {
	// disable the header options for non-file sources because:
	//* for sockets we allow to import one single value only at the moment
	//* for MQTT topics we don't allow to set the vector names since the different topics can have different number of columns
	// For files this option still can be useful if the user have to re-read the whole file
	// and wants to use the header to set the column names or the user provides manually the column names.
	// TODO: adjust this logic later once we allow to import multiple columns from sockets,
	// it should be possible to provide the names of the columns

	auto fileType = currentFileType();
	bool spreadsheet = true; // assume it's spreadsheet on default if no container is selected yet
	if (m_targetContainer)
		spreadsheet = m_targetContainer->type() == AspectType::Spreadsheet;

	// handle ASCII
	bool visible = (fileType == AbstractFileFilter::FileType::Ascii) && spreadsheet && currentSourceType() == LiveDataSource::SourceType::FileOrPipe;
	if (m_asciiOptionsWidget)
		m_asciiOptionsWidget->showAsciiHeaderOptions(visible);

	// handle XLSX or ODS
	visible = (fileType == AbstractFileFilter::FileType::XLSX || fileType == AbstractFileFilter::FileType::Ods) && spreadsheet;
	ui.lFirstRowAsColNames->setVisible(visible);
	ui.chbFirstRowAsColName->setVisible(visible);
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

QString ImportFileWidget::dbcFileName() const {
	return m_cbDBCFileName->currentText();
}

QString ImportFileWidget::selectedObject() const {
	DEBUG(Q_FUNC_INFO)
	const QString& path = fileName();

	// determine the file name only
	QString name = path.right(path.length() - path.lastIndexOf(QLatin1Char('/')) - 1);

	// strip away the extension if existing
	if (name.indexOf(QLatin1Char('.')) != -1)
		name = name.left(name.lastIndexOf(QLatin1Char('.')));

	// for multi-dimensional formats add the currently selected object
	const auto format = currentFileType();
	if (format == AbstractFileFilter::FileType::HDF5) {
		const QStringList& names = m_hdf5OptionsWidget->selectedNames();
		if (!names.isEmpty())
			name += names.first(); // the names of the selected HDF5 objects already have '/'
	} else if (format == AbstractFileFilter::FileType::NETCDF) {
		const QStringList& names = m_netcdfOptionsWidget->selectedNames();
		if (!names.isEmpty())
			name += QLatin1Char('/') + names.first();
	} else if (format == AbstractFileFilter::FileType::FITS) {
		const QString& extensionName = m_fitsOptionsWidget->currentExtensionName();
		if (!extensionName.isEmpty())
			name += QLatin1Char('/') + extensionName;
	} else if (format == AbstractFileFilter::FileType::ROOT) {
		const QStringList& names = m_rootOptionsWidget->selectedNames();
		if (!names.isEmpty())
			name += QLatin1Char('/') + names.first();
	} else if (format == AbstractFileFilter::FileType::MATIO) {
		const QStringList& names = m_matioOptionsWidget->selectedNames();
		if (!names.isEmpty())
			name += QLatin1Char('/') + names.first();
	} else if (format == AbstractFileFilter::FileType::XLSX) {
		const auto& names = m_xlsxOptionsWidget->selectedXLSXRegionNames();
		if (!names.isEmpty())
			name += QLatin1Char('/') + names.first();
	} else if (format == AbstractFileFilter::FileType::Ods) {
		const auto& names = m_odsOptionsWidget->selectedOdsSheetNames();
		QDEBUG(Q_FUNC_INFO << ", selected sheet names =")
		if (!names.isEmpty()) { // name == "start-end", names.first() == "start-end.ods!Sheet2"
			name += QLatin1Char('!') + names.first().split(QLatin1Char('!')).last();
		}
	}
	return name;
}

QString ImportFileWidget::host() const {
	const auto t = ui.leHost->text();
	if (t.compare(QStringLiteral("localhost"), Qt::CaseSensitivity::CaseInsensitive) == 0)
		return QStringLiteral("127.0.0.1");
	return t;
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

bool ImportFileWidget::importValid() const {
	return false;
}

/*!
	saves the settings to the data source \c source.
*/
void ImportFileWidget::saveSettings(LiveDataSource* source) const {
	// file type
	const auto fileType = currentFileType();
	source->setFileType(fileType);
	source->setFilter(m_currentFilter.release()); // pass ownership of the filter to the LiveDataSource

	// source type
	const auto sourceType = currentSourceType();
	source->setSourceType(sourceType);
	switch (sourceType) {
	case LiveDataSource::SourceType::FileOrPipe:
		source->setFileName(fileName());
		source->setFileLinked(ui.chbLinkFile->isChecked());
		source->setComment(fileName());
		if (m_liveDataSource)
			source->setUseRelativePath(ui.chbRelativePath->isChecked());
		break;
	case LiveDataSource::SourceType::LocalSocket:
		source->setFileName(fileName());
		source->setLocalSocketName(fileName());
		source->setComment(fileName());
		break;
	case LiveDataSource::SourceType::NetworkTCPSocket:
	case LiveDataSource::SourceType::NetworkUDPSocket:
		source->setHost(host());
		source->setPort((quint16)port().toInt());
		break;
	case LiveDataSource::SourceType::SerialPort:
		source->setBaudRate(ui.cbBaudRate->currentText().toInt());
		source->setSerialPort(ui.cbSerialPort->currentText());
		break;
	case LiveDataSource::SourceType::MQTT:
		break;
	}

	// reading options
	const auto readingType = static_cast<LiveDataSource::ReadingType>(ui.cbReadingType->currentData().toInt());
	const auto updateType = static_cast<LiveDataSource::UpdateType>(ui.cbUpdateType->currentIndex());
	source->setReadingType(readingType);
	source->setKeepNValues(ui.sbKeepNValues->value());
	source->setUpdateType(updateType);
	if (updateType == LiveDataSource::UpdateType::TimeInterval)
		source->setUpdateInterval(ui.sbUpdateInterval->value());

	if (readingType != LiveDataSource::ReadingType::TillEnd)
		source->setSampleSize(ui.sbSampleSize->value());
}

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
	auto fileType = currentFileType();
	if (m_currentFilter && m_currentFilter->type() != fileType)
		m_currentFilter.reset();

	switch (fileType) {
	case AbstractFileFilter::FileType::Ascii: {
		DEBUG(Q_FUNC_INFO << ", ASCII");
		if (!m_currentFilter)
			m_currentFilter.reset(new AsciiFilter);
		auto filter = static_cast<AsciiFilter*>(m_currentFilter.get());
		auto properties = filter->defaultProperties();

		// set the data portion to import
		properties.startRow = ui.sbStartRow->value();
		properties.endRow = ui.sbEndRow->value();
		properties.startColumn = ui.sbStartColumn->value();
		properties.endColumn = ui.sbEndColumn->value();

		const bool automatic = ui.cbFilter->currentIndex() == FilterSettingsHandlingIndex::Automatic;
		if ((!m_liveDataSource || currentSourceType() == LiveDataSource::SourceType::FileOrPipe) && automatic) {
			properties.automaticSeparatorDetection = true;
			properties.removeQuotes = true;
			if (m_asciiOptionsWidget)
				m_asciiOptionsWidget->updateWidgets(properties);
		} else { //"custom" and templates
			properties.automaticSeparatorDetection = false;

			// set the remaining filter settings
			if (m_asciiOptionsWidget && !automatic)
				m_asciiOptionsWidget->applyFilterSettings(properties);
		}
		// Both required, because initialize might fail and then no properties are set
		// Initialize is required for livedata (sequential devices)
		filter->setProperties(properties);
		filter->initialize(properties);

		break;
	}
	case AbstractFileFilter::FileType::Binary: {
		DEBUG(Q_FUNC_INFO << ", Binary");
		if (!m_currentFilter)
			m_currentFilter.reset(new BinaryFilter);
		auto filter = static_cast<BinaryFilter*>(m_currentFilter.get());

		if (ui.cbFilter->currentIndex() == FilterSettingsHandlingIndex::Automatic)
			filter->setAutoModeEnabled(true);
		else { //"custom" and templates
			filter->setAutoModeEnabled(false);
			if (m_binaryOptionsWidget)
				m_binaryOptionsWidget->applyFilterSettings(filter);
		}

		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());

		break;
	}
	case AbstractFileFilter::FileType::XLSX: {
		DEBUG(Q_FUNC_INFO << ", XLSX");

		if (!m_currentFilter)
			m_currentFilter.reset(new XLSXFilter);

		auto filter = static_cast<XLSXFilter*>(m_currentFilter.get());
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());
		filter->setFirstRowAsColumnNames(ui.chbFirstRowAsColName->isChecked());

		const auto& sxrn = selectedXLSXRegionNames();
		if (!sxrn.isEmpty()) {
			const auto& firstRegion = sxrn.last();
			const auto& nameSplit = firstRegion.split(QLatin1Char('!'));
			const auto& sheet = nameSplit.at(0);
			const auto& range = nameSplit.at(1);
			filter->setCurrentRange(range);
			filter->setCurrentSheet(sheet);
		}

		break;
	}
	case AbstractFileFilter::FileType::Ods: {
		DEBUG(Q_FUNC_INFO << ", ODS");

		if (!m_currentFilter)
			m_currentFilter.reset(new OdsFilter);

		auto filter = static_cast<OdsFilter*>(m_currentFilter.get());
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());
		filter->setFirstRowAsColumnNames(ui.chbFirstRowAsColName->isChecked());

		const auto& sorn = selectedOdsSheetNames();
		QDEBUG(Q_FUNC_INFO << ", selected Ods sheet names = " << sorn)
		if (!sorn.isEmpty())
			filter->setSelectedSheetNames(sorn);

		break;
	}
	case AbstractFileFilter::FileType::Image: {
		DEBUG(Q_FUNC_INFO << ", Image");
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
	case AbstractFileFilter::FileType::HDF5: {
		DEBUG(Q_FUNC_INFO << ", HDF5");
		if (!m_currentFilter)
			m_currentFilter.reset(new HDF5Filter);
		auto filter = static_cast<HDF5Filter*>(m_currentFilter.get());
		QStringList names = selectedHDF5Names();
		QDEBUG(Q_FUNC_INFO << ", selected HDF5 names =" << names);
		if (!names.isEmpty())
			filter->setCurrentDataSetName(names.at(0));
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());
		DEBUG(Q_FUNC_INFO << ", OK");

		break;
	}
	case AbstractFileFilter::FileType::NETCDF: {
		DEBUG(Q_FUNC_INFO << ", NetCDF");
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
	case AbstractFileFilter::FileType::VECTOR_BLF: {
		DEBUG(Q_FUNC_INFO << ", VECTOR_BLF");
		if (!m_currentFilter) {
			auto filter = new VectorBLFFilter;
			filter->setDBCFile(dbcFileName());
			m_currentFilter.reset(filter);
		}
		auto filter = static_cast<VectorBLFFilter*>(m_currentFilter.get());
		if (m_canOptionsWidget)
			m_canOptionsWidget->applyFilterSettings(filter);

		break;
	}
	case AbstractFileFilter::FileType::FITS: {
		DEBUG(Q_FUNC_INFO << ", FITS");
		if (!m_currentFilter)
			m_currentFilter.reset(new FITSFilter);
		auto filter = static_cast<FITSFilter*>(m_currentFilter.get());
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());

		break;
	}
	case AbstractFileFilter::FileType::JSON: {
		DEBUG(Q_FUNC_INFO << ", JSON");
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
	case AbstractFileFilter::FileType::MCAP: {
		DEBUG(Q_FUNC_INFO << ", MCAP");
		if (!m_currentFilter)
			m_currentFilter.reset(new McapFilter);
		auto filter = static_cast<McapFilter*>(m_currentFilter.get());

		m_mcapOptionsWidget->applyFilterSettings(filter);

		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());
		filter->setCurrentTopic(ui.cbMcapTopics->currentText());

		break;
	}
	case AbstractFileFilter::FileType::ROOT: {
		DEBUG(Q_FUNC_INFO << ", ROOT");
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
	case AbstractFileFilter::FileType::Spice: {
		DEBUG(Q_FUNC_INFO << ", Spice");
		if (!m_currentFilter)
			m_currentFilter.reset(new SpiceFilter());
		auto filter = static_cast<SpiceFilter*>(m_currentFilter.get());
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());

		break;
	}
	case AbstractFileFilter::FileType::READSTAT: {
		DEBUG(Q_FUNC_INFO << ", READSTAT");
		if (!m_currentFilter)
			m_currentFilter.reset(new ReadStatFilter);
		auto filter = static_cast<ReadStatFilter*>(m_currentFilter.get());
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());

		break;
	}
	case AbstractFileFilter::FileType::MATIO: {
		DEBUG(Q_FUNC_INFO << ", MATIO");
		if (!m_currentFilter)
			m_currentFilter.reset(new MatioFilter);
		auto filter = static_cast<MatioFilter*>(m_currentFilter.get());
		if (!selectedMatioNames().isEmpty())
			filter->setSelectedVarNames(selectedMatioNames());
		filter->setStartRow(ui.sbStartRow->value());
		filter->setEndRow(ui.sbEndRow->value());
		filter->setStartColumn(ui.sbStartColumn->value());
		filter->setEndColumn(ui.sbEndColumn->value());

		break;
	}
	}
	ui.sbPreviewPrecision->setValue(m_currentFilter->previewPrecision());
	return m_currentFilter.get();
}

/*!
	opens a file dialog and lets the user select the file data source.
*/
void ImportFileWidget::selectFile() {
	DEBUG(Q_FUNC_INFO)
	KConfigGroup conf = Settings::group(QStringLiteral("ImportFileWidget"));
	const QString& dir = conf.readEntry(QStringLiteral("LastDir"), "");
	const QString& path = QFileDialog::getOpenFileName(this, i18nc("@title:window", "Select the File Data Source"), dir);
	DEBUG("	dir = " << STDSTRING(dir))
	DEBUG("	path = " << STDSTRING(path))
	if (path.isEmpty()) // cancel was clicked in the file-dialog
		return;

	int pos = path.lastIndexOf(QLatin1Char('/'));
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry(QStringLiteral("LastDir"), newDir);
	}

	// process all events after the FileDialog was closed to repaint the widget
	// before we start calculating the preview
	QApplication::processEvents(QEventLoop::AllEvents, 0);

	QStringList urls = m_cbFileName->urls();
	urls.insert(0, QUrl::fromLocalFile(path).url()); // add type of path
	m_cbFileName->setUrls(urls);
	m_cbFileName->setCurrentText(urls.first());
	DEBUG("	combobox text = " << STDSTRING(m_cbFileName->currentText()))
	fileNameChanged(path); // why do I have to call this function separately
}

void ImportFileWidget::selectDBCFile() {
	DEBUG(Q_FUNC_INFO)
	const QString entry = QStringLiteral("DBCDir");
	KConfigGroup conf = Settings::group(QStringLiteral("ImportFileWidget"));
	const QString& dir = conf.readEntry(entry, "");
	const QString& path = QFileDialog::getOpenFileName(this, i18nc("@title:window", "Select the DBC file"), dir, i18n("DBC file (*.dbc)"));
	DEBUG("	dir = " << STDSTRING(dir))
	DEBUG("	path = " << STDSTRING(path))
	if (path.isEmpty()) // cancel was clicked in the file-dialog
		return;

	int pos = path.lastIndexOf(QLatin1Char('/'));
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry(entry, newDir);
	}

	// process all events after the FileDialog was closed to repaint the widget
	// before we start calculating the preview
	QApplication::processEvents(QEventLoop::AllEvents, 0);

	QStringList urls = m_cbDBCFileName->urls();
	urls.insert(0, QUrl::fromLocalFile(path).url()); // add type of path
	m_cbDBCFileName->setUrls(urls);
	m_cbDBCFileName->setCurrentText(urls.first());
	DEBUG("	combobox text = " << STDSTRING(m_cbDBCFileName->currentText()))

	refreshPreview();
}

/*!
	hides the MQTT related items of the widget
*/
void ImportFileWidget::setMQTTVisible(bool visible) {
	ui.lConnections->setVisible(visible);
	ui.cbConnection->setVisible(visible);
	ui.bManageConnections->setVisible(visible);

	// topics
	if (ui.cbConnection->currentIndex() != -1 && visible) {
		ui.lMqttTopics->setVisible(true);
		ui.frameSubscriptions->setVisible(true);
#ifdef HAVE_MQTT
		m_subscriptionWidget->setVisible(true);
		m_subscriptionWidget->makeVisible(true);
#endif
	} else {
		ui.lMqttTopics->setVisible(false);
		ui.frameSubscriptions->setVisible(false);
#ifdef HAVE_MQTT
		m_subscriptionWidget->setVisible(false);
		m_subscriptionWidget->makeVisible(false);
#endif
	}

	// will message
	ui.lLWT->setVisible(visible);
	ui.bLWT->setVisible(visible);
}

/************** SLOTS **************************************************************/
/*!
	called on file name changes.
	Determines the file format (ASCII, binary etc.), if the file exists,
	and activates the corresponding options.
*/
void ImportFileWidget::fileNameChanged(const QString& name) {
	DEBUG(Q_FUNC_INFO << ", file name = " << STDSTRING(name))
	Q_EMIT error(QString()); // clear previous errors

	const QString fileName = absolutePath(name);
	bool fileExists = QFile::exists(fileName);
	ui.gbOptions->setEnabled(fileExists);
	ui.cbFilter->setEnabled(fileExists);
	ui.cbFileType->setEnabled(fileExists);
	ui.bFileInfo->setEnabled(fileExists);
	ui.gbUpdateOptions->setEnabled(fileExists);
	if (!fileExists) {
		// file doesn't exist -> delete the content preview that is still potentially
		// available from the previously selected file
		ui.tePreview->clear();
		m_twPreview->clear();
		initOptionsWidget();

		Q_EMIT fileNameChanged();
		return;
	}

	// warn about opening project files
	bool isProjectFile = false;
	if (name.toLower().endsWith(QLatin1String(".opj"))) {
		Q_EMIT error(i18n("Origin Project files need to be opened with \"Import -> Origin Project\"!"));
		isProjectFile = true;
	} else if (name.toLower().endsWith(QLatin1String(".lml"))) {
		Q_EMIT error(i18n("LabPlot Project files need to be opened with \"Import -> LabPlot Project\"!"));
		isProjectFile = true;
	}
	if (isProjectFile) {
		ui.tePreview->clear();
		m_twPreview->clear();
		Q_EMIT fileNameChanged();
		return;
	}

	if (currentSourceType() == LiveDataSource::SourceType::FileOrPipe) {
		const auto fileType = AbstractFileFilter::fileType(fileName);
		const auto* model = qobject_cast<const QStandardItemModel*>(ui.cbFileType->model());
		for (int i = 0; i < ui.cbFileType->count(); ++i) {
			const auto type = static_cast<AbstractFileFilter::FileType>(ui.cbFileType->itemData(i).toInt());
			// disable item if exclusive
			if (AbstractFileFilter::exclusiveFileType(type)) {
				auto* item = model->item(i);
				if (item)
					item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
			}
		}
		for (int i = 0; i < ui.cbFileType->count(); ++i) {
			if (static_cast<AbstractFileFilter::FileType>(ui.cbFileType->itemData(i).toInt()) == fileType) {
				// enable item if exlusive
				if (AbstractFileFilter::exclusiveFileType(fileType)) {
					auto* item = model->item(i);
					if (item)
						item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
				}

				// automatically select a new file type
				if (ui.cbFileType->currentIndex() != i) {
					ui.cbFileType->setCurrentIndex(i); // will call the slot fileTypeChanged which updates content and preview

					// automatically set the comma separator if a csv file was selected
					if (fileType == AbstractFileFilter::FileType::Ascii && name.endsWith(QLatin1String("csv"), Qt::CaseInsensitive))
						m_asciiOptionsWidget->setSeparatingCharacter(QLatin1Char(','));

					Q_EMIT fileNameChanged();
					return;
				} else {
					initOptionsWidget();

					// automatically set the comma separator if a csv file was selected
					if (fileType == AbstractFileFilter::FileType::Ascii && name.endsWith(QLatin1String("csv"), Qt::CaseInsensitive))
						m_asciiOptionsWidget->setSeparatingCharacter(QLatin1Char(','));

					updateContent(fileName);
					break;
				}
			}
		}
	}
	mcapTopicsInitialized = false;
	Q_EMIT fileNameChanged();
	refreshPreview();
}

/*!
  saves the current filter settings as a template
*/
void ImportFileWidget::saveConfigAsTemplate(KConfig& config) {
	auto fileType = currentFileType();
	KConfigGroup group;
	if (fileType == AbstractFileFilter::FileType::Ascii) {
		m_asciiOptionsWidget->saveConfigAsTemplate(config);
		group = config.group(QLatin1String("ImportAscii"));
	} else if (fileType == AbstractFileFilter::FileType::Binary) {
		m_binaryOptionsWidget->saveConfigAsTemplate(config);
		group = config.group(QLatin1String("ImportBinary"));
	}

	// save additionally the "data portion to read"-settings which are not
	// part of the options widgets and were not saved above
	group.writeEntry(QLatin1String("StartRow"), ui.sbStartRow->value());
	group.writeEntry(QLatin1String("EndRow"), ui.sbStartRow->value());
	group.writeEntry(QLatin1String("StartColumn"), ui.sbStartRow->value());
	group.writeEntry(QLatin1String("EndColumn"), ui.sbStartRow->value());

	// add the currently added name of the template and make it current
	auto name = TemplateHandler::templateName(config);
	ui.cbFilter->addItem(name);
	ui.cbFilter->setCurrentText(name);
}

/*!
  loads the settings for the current filter from a template
*/
void ImportFileWidget::loadConfigFromTemplate(KConfig& config) {
	auto fileType = currentFileType();
	KConfigGroup group;
	if (fileType == AbstractFileFilter::FileType::Ascii) {
		m_asciiOptionsWidget->loadConfigFromTemplate(config);
		group = config.group(QLatin1String("ImportAscii"));
	} else if (fileType == AbstractFileFilter::FileType::Binary) {
		m_binaryOptionsWidget->loadConfigFromTemplate(config);
		group = config.group(QLatin1String("ImportBinary"));
	}

	// load additionally the "data portion to read"-settings which are not
	// part of the options widgets and were not loaded above
	ui.sbStartRow->setValue(group.readEntry(QLatin1String("StartRow"), -1));
	ui.sbEndRow->setValue(group.readEntry(QLatin1String("EndRow"), -1));
	ui.sbStartColumn->setValue(group.readEntry(QLatin1String("StartColumn"), -1));
	ui.sbEndColumn->setValue(group.readEntry(QLatin1String("EndColumn"), -1));
}

/*!
 * \brief ImportFileWidget::hidePropertyWidgets
 * Hide all Widgets related to the properties. This should be the default
 * Currently every filetype is turning off, but it makes sense to turn on
 * only when needed
 * TODO: move all widgets in here
 */
void ImportFileWidget::hidePropertyWidgets() {
	ui.lDBCDatabase->hide();
	ui.bOpenDBC->hide();
	m_cbDBCFileName->hide();
	ui.lWarningLimitedMessages->hide();
}

/*!
	Depending on the selected file type, activates the corresponding options in the data portion tab
	and populates the combobox with the available pre-defined filter settings for the selected type.
*/
void ImportFileWidget::fileTypeChanged(int /*index*/) {
	auto fileType = currentFileType();
	DEBUG(Q_FUNC_INFO << ", " << ENUM_TO_STRING(AbstractFileFilter, FileType, fileType));
	Q_EMIT error(QString()); // clear the potential error message that was shown for the previous file type
	initOptionsWidget();

	// enable the options widgets, should be avaible for all types where there is no "automatic" vs "custom",
	// will be disabled for "automatic" for the relevant data types
	ui.swOptions->setEnabled(true);

	// default
	hidePropertyWidgets();
	ui.lFilter->hide();
	ui.cbFilter->hide();
	m_templateHandler->hide();

	// different file types show different number of tabs in ui.tabWidget.
	// when switching from the previous file type we re-set the tab widget to its original state
	// and remove/add the required tabs further below
	for (int i = 0; i < ui.tabWidget->count(); ++i)
		ui.tabWidget->removeTab(0);

	ui.tabWidget->addTab(ui.tabDataFormat, i18n("Data Format"));
	ui.tabWidget->addTab(ui.tabDataPreview, i18n("Preview"));
	if (!m_liveDataSource)
		ui.tabWidget->addTab(ui.tabDataPortion, i18n("Data Portion to Read"));

	ui.lPreviewLines->show();
	ui.sbPreviewLines->show();
	ui.lStartColumn->show();
	ui.sbStartColumn->show();
	ui.lEndColumn->show();
	ui.sbEndColumn->show();

	showJsonModel(false);

	ui.lMcapTopics->hide();
	ui.cbMcapTopics->hide();

	switch (fileType) {
	case AbstractFileFilter::FileType::Ascii:
		ui.lFilter->show();
		ui.cbFilter->show();
		m_templateHandler->show();
		m_templateHandler->setClassName(QLatin1String("AsciiFilter"));
		break;
	case AbstractFileFilter::FileType::Binary:
		ui.lFilter->show();
		ui.cbFilter->show();
		m_templateHandler->show();
		m_templateHandler->setClassName(QLatin1String("BinaryFilter"));
		ui.lStartColumn->hide();
		ui.sbStartColumn->hide();
		ui.lEndColumn->hide();
		ui.sbEndColumn->hide();
		break;
	case AbstractFileFilter::FileType::ROOT:
		ui.tabWidget->removeTab(1);
	// falls through
	case AbstractFileFilter::FileType::HDF5:
	case AbstractFileFilter::FileType::NETCDF:
	case AbstractFileFilter::FileType::FITS:
	case AbstractFileFilter::FileType::MATIO:
	case AbstractFileFilter::FileType::XLSX:
	case AbstractFileFilter::FileType::Ods:
		ui.lFilter->hide();
		ui.cbFilter->hide();
		// hide global preview tab. we have our own
		ui.tabWidget->setTabText(0, i18n("Data format && preview"));
		ui.tabWidget->removeTab(1);
		ui.tabWidget->setCurrentIndex(0);
		break;
	case AbstractFileFilter::FileType::VECTOR_BLF:
		ui.lDBCDatabase->show();
		ui.bOpenDBC->show();
		m_cbDBCFileName->show();
		ui.lWarningLimitedMessages->show();
		ui.lStartColumn->hide();
		ui.sbStartColumn->hide();
		ui.lEndColumn->hide();
		ui.sbEndColumn->hide();
		ui.tabWidget->setCurrentIndex(0);
		break;
	case AbstractFileFilter::FileType::Image:
		ui.lPreviewLines->hide();
		ui.sbPreviewLines->hide();
		break;
	case AbstractFileFilter::FileType::Spice:
		ui.lStartColumn->hide();
		ui.sbStartColumn->hide();
		ui.lEndColumn->hide();
		ui.sbEndColumn->hide();
		ui.tabWidget->removeTab(0);
		ui.tabWidget->setCurrentIndex(0);
		break;
	case AbstractFileFilter::FileType::JSON:
		showJsonModel(true);
		break;
	case AbstractFileFilter::FileType::MCAP:
		ui.lMcapTopics->show();
		ui.cbMcapTopics->show();
		break;
	case AbstractFileFilter::FileType::READSTAT:
		ui.tabWidget->removeTab(0);
		ui.tabWidget->setCurrentIndex(0);
		break;
	}

	// update header specific options that are available for some filter types
	// and for some target data containers (Spreadsheet) only
	updateHeaderOptions();

	if (fileType == AbstractFileFilter::FileType::Ascii || fileType == AbstractFileFilter::FileType::Binary) {
		int lastUsedFilterIndex = ui.cbFilter->currentIndex();
		ui.cbFilter->clear();
		ui.cbFilter->addItem(i18n("Automatic"));
		ui.cbFilter->addItem(i18n("Custom"));

		// add templates
		const auto& names = m_templateHandler->templateNames();
		if (!names.isEmpty()) {
			ui.cbFilter->insertSeparator(2);
			ui.cbFilter->addItems(names);
		}

		if (lastUsedFilterIndex != -1) {
			// if one of the custom and filter specific templates was selected, switch to "Automatic" when
			// switching to a different file/filter type and keep the previous selection "Automatic" or "Custom" otherwise
			if (lastUsedFilterIndex > 2)
				lastUsedFilterIndex = 0;

			ui.cbFilter->setCurrentIndex(lastUsedFilterIndex);
			filterChanged(lastUsedFilterIndex);
		}
	}

	if (currentSourceType() == LiveDataSource::SourceType::FileOrPipe) {
		const QString& file = absolutePath(fileName());
		if (QFile::exists(file))
			updateContent(file);
	}

	// for file types other than ASCII and binary we support re-reading the whole file only
	// select "read whole file" and deactivate the combobox
	if (m_liveDataSource && (fileType != AbstractFileFilter::FileType::Ascii && fileType != AbstractFileFilter::FileType::Binary)) {
		ui.cbReadingType->setCurrentIndex(static_cast<int>(LiveDataSource::ReadingType::WholeFile));
		ui.cbReadingType->setEnabled(false);
	} else
		ui.cbReadingType->setEnabled(true);

	refreshPreview();
}

// file type specific option widgets
void ImportFileWidget::initOptionsWidget() {
	DEBUG(Q_FUNC_INFO << ", for " << ENUM_TO_STRING(AbstractFileFilter, FileType, currentFileType()));
	switch (currentFileType()) {
	case AbstractFileFilter::FileType::Ascii: {
		if (!m_asciiOptionsWidget) {
			auto* asciiw = new QWidget();
			m_asciiOptionsWidget = std::unique_ptr<AsciiOptionsWidget>(new AsciiOptionsWidget(asciiw, m_liveDataSource));
			m_asciiOptionsWidget->loadSettings();

			// allow to add timestamp column for live data sources
			if (m_liveDataSource) {
				m_asciiOptionsWidget->showTimestampOptions(true);
			}
			ui.swOptions->addWidget(asciiw);
		}

		ui.swOptions->setCurrentWidget(m_asciiOptionsWidget->parentWidget());
		break;
	}
	case AbstractFileFilter::FileType::Binary:
		if (!m_binaryOptionsWidget) {
			auto* binaryw = new QWidget();
			m_binaryOptionsWidget = std::unique_ptr<BinaryOptionsWidget>(new BinaryOptionsWidget(binaryw));
			ui.swOptions->addWidget(binaryw);
			m_binaryOptionsWidget->loadSettings();
		}
		ui.swOptions->setCurrentWidget(m_binaryOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::FileType::Image:
		if (!m_imageOptionsWidget) {
			auto* imagew = new QWidget();
			m_imageOptionsWidget = std::unique_ptr<ImageOptionsWidget>(new ImageOptionsWidget(imagew));
			ui.swOptions->addWidget(imagew);
			m_imageOptionsWidget->loadSettings();
		}
		ui.swOptions->setCurrentWidget(m_imageOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::FileType::XLSX:
		if (!m_xlsxOptionsWidget) {
			QWidget* xlsxw = new QWidget();
			m_xlsxOptionsWidget = std::unique_ptr<XLSXOptionsWidget>(new XLSXOptionsWidget(xlsxw, this));
			ui.swOptions->addWidget(xlsxw);
			connect(dynamic_cast<XLSXOptionsWidget*>(m_xlsxOptionsWidget.get()),
					&XLSXOptionsWidget::enableDataPortionSelection,
					this,
					&ImportFileWidget::enableDataPortionSelection);
		}
		ui.swOptions->setCurrentWidget(m_xlsxOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::FileType::Ods:
		if (!m_odsOptionsWidget) {
			QWidget* odsw = new QWidget();
			m_odsOptionsWidget = std::unique_ptr<OdsOptionsWidget>(new OdsOptionsWidget(odsw, this));
			ui.swOptions->addWidget(odsw);
			connect(dynamic_cast<OdsOptionsWidget*>(m_odsOptionsWidget.get()),
					&OdsOptionsWidget::enableDataPortionSelection,
					this,
					&ImportFileWidget::enableDataPortionSelection);
		}
		ui.swOptions->setCurrentWidget(m_odsOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::FileType::HDF5:
		if (!m_hdf5OptionsWidget) {
			auto* hdf5w = new QWidget();
			m_hdf5OptionsWidget = std::unique_ptr<HDF5OptionsWidget>(new HDF5OptionsWidget(hdf5w, this));
			ui.swOptions->addWidget(hdf5w);
		} else
			m_hdf5OptionsWidget->clear();
		ui.swOptions->setCurrentWidget(m_hdf5OptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::FileType::NETCDF:
		if (!m_netcdfOptionsWidget) {
			auto* netcdfw = new QWidget();
			m_netcdfOptionsWidget = std::unique_ptr<NetCDFOptionsWidget>(new NetCDFOptionsWidget(netcdfw, this));
			ui.swOptions->insertWidget(static_cast<int>(AbstractFileFilter::FileType::NETCDF), netcdfw);
		} else
			m_netcdfOptionsWidget->clear();
		ui.swOptions->setCurrentWidget(m_netcdfOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::FileType::VECTOR_BLF:
		if (!m_canOptionsWidget) {
			auto* vectorBLF = new QWidget();
			m_canOptionsWidget = std::unique_ptr<CANOptionsWidget>(new CANOptionsWidget(vectorBLF));
			ui.swOptions->addWidget(vectorBLF);
		}
		ui.swOptions->setCurrentWidget(m_canOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::FileType::FITS:
		if (!m_fitsOptionsWidget) {
			auto* fitsw = new QWidget();
			m_fitsOptionsWidget = std::unique_ptr<FITSOptionsWidget>(new FITSOptionsWidget(fitsw, this));
			ui.swOptions->addWidget(fitsw);
		} else
			m_fitsOptionsWidget->clear();
		ui.swOptions->setCurrentWidget(m_fitsOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::FileType::JSON:
		if (!m_jsonOptionsWidget) {
			auto* jsonw = new QWidget();
			m_jsonOptionsWidget = std::unique_ptr<JsonOptionsWidget>(new JsonOptionsWidget(jsonw));
			ui.tvJson->setModel(m_jsonOptionsWidget->model());
			ui.swOptions->addWidget(jsonw);
			m_jsonOptionsWidget->loadSettings();
			connect(m_jsonOptionsWidget.get(), &JsonOptionsWidget::error, this, &ImportFileWidget::error);
		} else
			m_jsonOptionsWidget->clearModel();
		ui.swOptions->setCurrentWidget(m_jsonOptionsWidget->parentWidget());
		showJsonModel(true);
		break;
	case AbstractFileFilter::FileType::MCAP:
		if (!m_mcapOptionsWidget) {
			auto* jsonw = new QWidget();
			m_mcapOptionsWidget = std::unique_ptr<McapOptionsWidget>(new McapOptionsWidget(jsonw));
			ui.swOptions->addWidget(jsonw);
			m_mcapOptionsWidget->loadSettings();

			connect(m_mcapOptionsWidget.get(), &McapOptionsWidget::error, this, &ImportFileWidget::error);
		}
		ui.swOptions->setCurrentWidget(m_mcapOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::FileType::ROOT:
		if (!m_rootOptionsWidget) {
			auto* rootw = new QWidget();
			m_rootOptionsWidget = std::unique_ptr<ROOTOptionsWidget>(new ROOTOptionsWidget(rootw, this));
			ui.swOptions->addWidget(rootw);
		} else
			m_rootOptionsWidget->clear();
		ui.swOptions->setCurrentWidget(m_rootOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::FileType::MATIO:
		if (!m_matioOptionsWidget) {
			auto* matiow = new QWidget();
			m_matioOptionsWidget = std::unique_ptr<MatioOptionsWidget>(new MatioOptionsWidget(matiow, this));
			ui.swOptions->insertWidget(static_cast<int>(AbstractFileFilter::FileType::MATIO), matiow);
		} else
			m_matioOptionsWidget->clear();
		ui.swOptions->setCurrentWidget(m_matioOptionsWidget->parentWidget());
		break;
	case AbstractFileFilter::FileType::Spice:
	case AbstractFileFilter::FileType::READSTAT:
		break;
	}
}

const QStringList ImportFileWidget::selectedHDF5Names() const {
	return m_hdf5OptionsWidget->selectedNames();
}

// const QStringList ImportFileWidget::selectedVectorBLFNames() const {
//	return m_vectorBLFOptionsWidget->selectedNames();
// }

const QStringList ImportFileWidget::selectedNetCDFNames() const {
	return m_netcdfOptionsWidget->selectedNames();
}

const QStringList ImportFileWidget::selectedMatioNames() const {
	return m_matioOptionsWidget->selectedNames();
}

const QStringList ImportFileWidget::selectedFITSExtensions() const {
	return m_fitsOptionsWidget->selectedExtensions();
}

const QStringList ImportFileWidget::selectedROOTNames() const {
	return m_rootOptionsWidget->selectedNames();
}

const QStringList ImportFileWidget::selectedXLSXRegionNames() const {
	return m_xlsxOptionsWidget->selectedXLSXRegionNames();
}

const QStringList ImportFileWidget::selectedOdsSheetNames() const {
	return m_odsOptionsWidget->selectedOdsSheetNames();
}

bool ImportFileWidget::useFirstRowAsColNames() const {
	return ui.chbFirstRowAsColName->isChecked();
}

/*!
	shows the dialog with the information about the file(s) to be imported.
*/
void ImportFileWidget::showFileInfo() {
	const QString& info = fileInfoString(fileName());
	QWhatsThis::showText(ui.bFileInfo->mapToGlobal(QPoint(0, 0)), info, ui.bFileInfo);
}

/*!
	returns a string containing the general information about the file \c name
	and some content specific information
	(number of columns and lines for ASCII, color-depth for images etc.).
*/
QString ImportFileWidget::fileInfoString(const QString& name) const {
	DEBUG(Q_FUNC_INFO << ", file name = " << STDSTRING(name))
	QString infoString;
	QFileInfo fileInfo;
	QString fileTypeString;
	QIODevice* file = new QFile(name);

	QString fileName = absolutePath(name);

	if (!file)
		file = new QFile(fileName);

	if (file->open(QIODevice::ReadOnly)) {
		QStringList infoStrings;

		infoStrings << QStringLiteral("<u><b>") + fileName + QStringLiteral("</b></u><br>");

		// File type given by "file"
#ifdef Q_OS_LINUX
		const QString fileFullPath = safeExecutableName(QStringLiteral("file"));
		if (fileFullPath.isEmpty())
			return i18n("file command not found");

		QProcess proc;
		QStringList args;
		args << QStringLiteral("-b") << fileName;
		startHostProcess(proc, fileFullPath, args);

		if (proc.waitForReadyRead(1000) == false)
			infoStrings << i18n("Reading from file %1 failed.", fileName);
		else {
			fileTypeString = QLatin1String(proc.readLine());
			if (fileTypeString.contains(i18n("cannot open")))
				fileTypeString.clear();
			else
				fileTypeString.remove(fileTypeString.length() - 1, 1); // remove '\n'
		}
		infoStrings << i18n("<b>File type:</b> %1", fileTypeString);
#endif

		// General:
		fileInfo.setFile(fileName);
		infoStrings << QStringLiteral("<b>") << i18n("General:") << QStringLiteral("</b>");

		infoStrings << i18n("Readable: %1", fileInfo.isReadable() ? i18n("yes") : i18n("no"));
		infoStrings << i18n("Writable: %1", fileInfo.isWritable() ? i18n("yes") : i18n("no"));
		infoStrings << i18n("Executable: %1", fileInfo.isExecutable() ? i18n("yes") : i18n("no"));

		infoStrings << i18n("Birth time: %1", fileInfo.birthTime().toString());
		infoStrings << i18n("Last metadata changed: %1", fileInfo.metadataChangeTime().toString());
		infoStrings << i18n("Last modified: %1", fileInfo.lastModified().toString());
		infoStrings << i18n("Last read: %1", fileInfo.lastRead().toString());
		infoStrings << i18n("Owner: %1", fileInfo.owner());
		infoStrings << i18n("Group: %1", fileInfo.group());
		infoStrings << i18n("Size: %1", i18np("%1 cByte", "%1 cBytes", fileInfo.size()));

		// Summary:
		infoStrings << QStringLiteral("<b>") << i18n("Summary:") << QStringLiteral("</b>");
		// depending on the file type, generate summary and content information about the file
		// TODO: content information (in BNF) for more types
		// TODO: introduce a function in the base class and work with infoStrings << currentFileFilter()->fileInfoString(fileName);
		// instead of this big switch-case.
		switch (AbstractFileFilter::fileType(fileName)) {
		case AbstractFileFilter::FileType::Ascii:
			infoStrings << AsciiFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FileType::Binary:
			infoStrings << BinaryFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FileType::XLSX:
			infoStrings << XLSXFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FileType::Ods:
			infoStrings << OdsFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FileType::Image:
			infoStrings << ImageFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FileType::HDF5:
			infoStrings << HDF5Filter::fileInfoString(fileName);
			infoStrings << QStringLiteral("<b>") << i18n("Content:") << QStringLiteral("</b>");
			infoStrings << HDF5Filter::fileDDLString(fileName);
			break;
		case AbstractFileFilter::FileType::NETCDF:
			infoStrings << NetCDFFilter::fileInfoString(fileName);
			infoStrings << QStringLiteral("<b>") << i18n("Content:") << QStringLiteral("</b>");
			infoStrings << NetCDFFilter::fileCDLString(fileName);
			break;
		case AbstractFileFilter::FileType::VECTOR_BLF:
			infoStrings << VectorBLFFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FileType::FITS:
			infoStrings << FITSFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FileType::JSON:
			infoStrings << JsonFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FileType::MCAP:
			infoStrings << McapFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FileType::ROOT:
			infoStrings << ROOTFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FileType::Spice:
			infoStrings << SpiceFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FileType::READSTAT:
			infoStrings << ReadStatFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FileType::MATIO:
			infoStrings << MatioFilter::fileInfoString(fileName);
			break;
		}

		infoString += infoStrings.join(QLatin1String("<br>"));
	} else
		infoString += i18n("Could not open file %1 for reading.", fileName);

	return infoString;
}

/*!
 * called when the filter settings type (custom, automatic, from a template) was changed.
 * enables the options if the filter "custom" was chosen. Disables the options otherwise.
 */
void ImportFileWidget::filterChanged(int index) {
	DEBUG(Q_FUNC_INFO)

	// filter settings are available for ASCII and Binary only, ignore for other file types
	auto fileType = currentFileType();
	if (fileType != AbstractFileFilter::FileType::Ascii && fileType != AbstractFileFilter::FileType::Binary) {
		ui.swOptions->setEnabled(true);
		return;
	}

	if (index == FilterSettingsHandlingIndex::Automatic) {
		ui.swOptions->setEnabled(false);
		m_templateHandler->hide();
	} else if (index == FilterSettingsHandlingIndex::Custom) {
		ui.swOptions->setEnabled(true);
		m_templateHandler->show();
	} else { // templates
		ui.swOptions->setEnabled(false);
		m_templateHandler->hide();
		auto config = m_templateHandler->config(ui.cbFilter->currentText());
		this->loadConfigFromTemplate(config);
	}
}

inline void delay(int millisecondsWait)
{
	QEventLoop loop;
	QTimer t;
	t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
	t.start(millisecondsWait);
	loop.exec();
}

void ImportFileWidget::refreshPreview() {
	DEBUG(Q_FUNC_INFO)
	// don't generate any preview if it was explicitly suppressed
	// or if the options box together with the preview widget is not visible
	if (m_suppressRefresh || !ui.gbOptions->isVisible())
		return;

	auto* currentFilter = currentFileFilter();
	currentFilter->setLastError(QString()); // clear the last error message, if any available

	auto file = absolutePath(fileName());
	const auto sourceType = currentSourceType();

	if (sourceType == LiveDataSource::SourceType::FileOrPipe && file.isEmpty())
		return; // initial open with no file selected yet, nothing to preview

	const auto fileType = currentFileType();
	DEBUG(Q_FUNC_INFO << ", Data File Type: " << ENUM_TO_STRING(AbstractFileFilter, FileType, fileType));
	const auto& dbcFile = dbcFileName();
	int lines = ui.sbPreviewLines->value();
	currentFilter->setPreviewPrecision(ui.sbPreviewPrecision->value());

	// default preview widget
	if (fileType == AbstractFileFilter::FileType::Ascii || fileType == AbstractFileFilter::FileType::Binary || fileType == AbstractFileFilter::FileType::JSON
		|| fileType == AbstractFileFilter::FileType::MCAP || fileType == AbstractFileFilter::FileType::Spice
		|| fileType == AbstractFileFilter::FileType::VECTOR_BLF || fileType == AbstractFileFilter::FileType::READSTAT)
		m_twPreview->show();
	else
		m_twPreview->hide();

	bool ok = true;
	auto* tmpTableWidget = m_twPreview;
	QVector<QStringList> importedStrings;
	QStringList vectorNameList;
	QVector<AbstractColumn::ColumnMode> columnModes;

	DEBUG(Q_FUNC_INFO << ", Data File Type: " << ENUM_TO_STRING(AbstractFileFilter, FileType, fileType));

	WAIT_CURSOR_AUTO_RESET;

	QString errorMessage;

	switch (fileType) {
	case AbstractFileFilter::FileType::Ascii: {
		ui.tePreview->clear();

		auto filter = static_cast<AsciiFilter*>(currentFilter);
		const auto& properties = filter->properties();
		if (properties.endRow > 0)
			lines = properties.endRow - properties.startRow + 1;
		if (lines <= 0) {
			Q_EMIT error(i18n("Invalid number rows. Please check 'End Row' and 'Start Row'. 'End Row' must be larger than 'Start Row' or -1"));
			return;
		}
		filter->clearLastError();
		filter->clearLastWarnings();

		// sequential
		if (!automaticAllowed(sourceType)) {
			auto p = filter->properties();
			filter->initialize(p);
			if (!filter->lastError().isEmpty()) {
				Q_EMIT error(i18n("Preview: Initialization failed: %1", filter->lastError()));
				return;
			}
		}

		DEBUG(Q_FUNC_INFO << ", Data Source Type: " << ENUM_TO_STRING(LiveDataSource, SourceType, sourceType));
		switch (sourceType) {
		case LiveDataSource::SourceType::FileOrPipe: {
			DEBUG(Q_FUNC_INFO << ", file name = " << STDSTRING(file));
			importedStrings = filter->preview(file, lines);
			break;
		}
		case LiveDataSource::SourceType::LocalSocket: {
			QLocalSocket lsocket{this};
			DEBUG("Local socket: CONNECT PREVIEW");
			lsocket.connectToServer(file, QLocalSocket::ReadOnly);
			if (lsocket.waitForConnected()) {
				DEBUG("connected to local socket " << STDSTRING(file));
				if (lsocket.waitForReadyRead())
					importedStrings = filter->preview(lsocket, lines, true);
				DEBUG("Local socket: DISCONNECT PREVIEW");
				lsocket.disconnectFromServer();
				// read-only socket is disconnected immediately (no waitForDisconnected())
			} else {
				DEBUG("failed connect to local socket " << STDSTRING(file) << " - " << STDSTRING(lsocket.errorString()));
				Q_EMIT error(i18n("Preview: Failed to connect to local socket %1 - %2", file, lsocket.errorString()));
				return;
			}

			break;
		}
		case LiveDataSource::SourceType::NetworkTCPSocket: {
			QTcpSocket tcpSocket{this};
			tcpSocket.connectToHost(host(), port().toInt(), QTcpSocket::ReadOnly);
			constexpr auto timeoutTime_ms = 5000;
			if (tcpSocket.waitForConnected(timeoutTime_ms)) {
				DEBUG("connected to TCP socket");
				if (tcpSocket.waitForReadyRead(timeoutTime_ms)) {
					delay(1000); // Wait to collect some data
					importedStrings = filter->preview(tcpSocket, lines, true);
				} else {
					DEBUG("failed connect to TCP socket " << STDSTRING(tcpSocket.errorString()));
					errorMessage = i18n("Preview: Failed to connect to TCP socket - %1", tcpSocket.errorString());
				}
				tcpSocket.disconnectFromHost();
			} else
				DEBUG("failed to connect to TCP socket within " << timeoutTime_ms << "ms"
					  << " - " << STDSTRING(tcpSocket.errorString()));

			break;
		}
		case LiveDataSource::SourceType::NetworkUDPSocket: {
			QUdpSocket udpSocket{this};
			DEBUG("UDP Socket: CONNECT PREVIEW, state = " << udpSocket.state());
			if (udpSocket.bind(QHostAddress(host()), port().toInt())) {
				udpSocket.connectToHost(host(), 0, QUdpSocket::ReadOnly);
				if (udpSocket.waitForConnected()) {
					DEBUG("	connected to UDP socket " << STDSTRING(host()) << ':' << port().toInt());
					if (!udpSocket.waitForReadyRead(2000)) {
						DEBUG("	ERROR: not ready for read after 2 sec" << udpSocket.errorString().toStdString());
						errorMessage = i18n("Not ready for read after 2s:") + udpSocket.errorString();
					} else {
						if (udpSocket.hasPendingDatagrams()) {
							DEBUG("	has pending data");
						} else {
							DEBUG("	has no pending data");
						}
						delay(1000); // Wait to collect some data
						importedStrings = filter->preview(udpSocket, lines, true);
					}

					DEBUG("UDP Socket: DISCONNECT PREVIEW, state = " << udpSocket.state());
					udpSocket.disconnectFromHost();
				} else {
					DEBUG("failed to connect to UDP socket "
						  << " - " << STDSTRING(udpSocket.errorString()));
					errorMessage = i18n("Unable to connect to host: ") + udpSocket.errorString();
				}
			} else {
				DEBUG("Unable to bind" << udpSocket.errorString().toStdString());
				errorMessage = i18n("Unable to bind: ") + udpSocket.errorString();
			}

			break;
		}
		case LiveDataSource::SourceType::SerialPort: {
#ifdef HAVE_QTSERIALPORT
			QSerialPort sPort{this};
			DEBUG("	Port: " << STDSTRING(serialPort()) << ", Settings: " << baudRate() << ',' << sPort.dataBits() << ',' << sPort.parity() << ','
							<< sPort.stopBits());
			sPort.setPortName(serialPort());
			sPort.setBaudRate(baudRate());

			if (sPort.open(QIODevice::ReadOnly)) {
				if (sPort.waitForReadyRead(2000)) {
					importedStrings = filter->preview(sPort, lines, false, true);
					if (!filter->lastError().isEmpty())
						errorMessage = i18n("Parse Error: %1", filter->lastError());
				} else
					errorMessage = i18n("ERROR: not ready for read after 2 sec");

				sPort.close();
			} else
				errorMessage = i18n("ERROR: failed to open serial port. error: %1", sPort.error());
#endif
			break;
		}
		case LiveDataSource::SourceType::MQTT: {
#ifdef HAVE_MQTT
			// show the preview for the currently selected topic
			auto* item = m_subscriptionWidget->currentItem();
			if (!item)
				errorMessage = QStringLiteral("Please select a topic.");
			else if (item->childCount() != 0)
				errorMessage = QStringLiteral("Please select lowest level of the topic.");
			else { // only preview if the lowest level (i.e. a topic) is selected
				const QString& topicName = item->text(0);
				auto i = m_lastMessage.find(topicName);
				if (i != m_lastMessage.end()) {
					BufferReader reader(i.value().payload());
					importedStrings = filter->preview(reader, lines, false);
				} else
					importedStrings << QStringList{i18n("No data arrived yet for the selected topic")};
			}
#endif
			break;
		}
		}

		vectorNameList = filter->columnNames();
		columnModes = filter->columnModes();
		break;
	} // AbstractFileFilter::FileType::Ascii
	case AbstractFileFilter::FileType::Binary: {
		ui.tePreview->clear();
		auto filter = static_cast<BinaryFilter*>(currentFilter);
		importedStrings = filter->preview(file, lines);
		break;
	}
	case AbstractFileFilter::FileType::XLSX:
		// update own preview (Nothing else to do)
		m_xlsxOptionsWidget->dataRegionSelectionChanged();
		// TODO: needed for import (why?)
		importedStrings = m_xlsxOptionsWidget->previewString();
		break;
	case AbstractFileFilter::FileType::Ods:
		// update own preview (Nothing else to do)
		m_odsOptionsWidget->sheetSelectionChanged();
		// TODO: needed for import (why?)
		importedStrings = m_odsOptionsWidget->previewString();
		break;
	case AbstractFileFilter::FileType::Image: {
		ui.tePreview->clear();

		QImage image(file);
		auto cursor = ui.tePreview->textCursor();
		cursor.insertImage(image);
		error(currentFilter->lastError());
		return;
	}
	case AbstractFileFilter::FileType::HDF5: {
		DEBUG(Q_FUNC_INFO << ", HDF5");
		auto filter = static_cast<HDF5Filter*>(currentFilter);
		lines = m_hdf5OptionsWidget->lines();

		importedStrings = filter->readCurrentDataSet(file, nullptr, ok, AbstractFileFilter::ImportMode::Replace, lines);
		tmpTableWidget = m_hdf5OptionsWidget->previewWidget();
		break;
	}
	case AbstractFileFilter::FileType::NETCDF: {
		DEBUG(Q_FUNC_INFO << ", NetCDF");
		auto filter = static_cast<NetCDFFilter*>(currentFilter);
		lines = m_netcdfOptionsWidget->lines();

		importedStrings = filter->readCurrentVar(file, nullptr, AbstractFileFilter::ImportMode::Replace, lines);
		tmpTableWidget = m_netcdfOptionsWidget->previewWidget();
		break;
	}
	case AbstractFileFilter::FileType::VECTOR_BLF: {
		ui.tePreview->clear();
		auto filter = static_cast<VectorBLFFilter*>(currentFilter);
		filter->setDBCFile(dbcFile);
		importedStrings = filter->preview(file, lines);
		vectorNameList = filter->vectorNames();
		columnModes = filter->columnModes();
		break;
	}
	case AbstractFileFilter::FileType::FITS: {
		DEBUG(Q_FUNC_INFO << ", FITS");
		auto filter = static_cast<FITSFilter*>(currentFilter);
		lines = m_fitsOptionsWidget->lines();

		QString extensionName = m_fitsOptionsWidget->extensionName(&ok);
		if (!extensionName.isEmpty()) {
			DEBUG(Q_FUNC_INFO << ", extension name = " << STDSTRING(extensionName));
			file = extensionName;
		}

		bool readFitsTableToMatrix;
		importedStrings = filter->readChdu(file, &readFitsTableToMatrix, lines);
		Q_EMIT enableImportToMatrix(readFitsTableToMatrix);

		tmpTableWidget = m_fitsOptionsWidget->previewWidget();
		break;
	}
	case AbstractFileFilter::FileType::JSON: {
		ui.tePreview->clear();
		auto filter = static_cast<JsonFilter*>(currentFilter);
		m_jsonOptionsWidget->applyFilterSettings(filter, ui.tvJson->currentIndex());
		importedStrings = filter->preview(file, lines);
		vectorNameList = filter->vectorNames();
		columnModes = filter->columnModes();
		break;
	}
	case AbstractFileFilter::FileType::MCAP: {
		DEBUG(Q_FUNC_INFO << ", MCAP");

		ui.tePreview->clear();
		auto filter = static_cast<McapFilter*>(currentFileFilter());

		if (!mcapTopicsInitialized) {
			ui.cbMcapTopics->clear();
			auto s = filter->getValidTopics(file);
			for (int i = 0; i < s.size(); i++) {
				ui.cbMcapTopics->addItem(s[i]);
			}
			mcapTopicsInitialized = true;
		}

		const auto& currentMcapTopic = ui.cbMcapTopics->currentText();
		DEBUG("Current selected topic" << STDSTRING(currentMcapTopic));
		filter->setCurrentTopic(currentMcapTopic);
		importedStrings = filter->preview(file, lines);
		vectorNameList = filter->vectorNames();
		columnModes = filter->columnModes();
		break;
	}
	case AbstractFileFilter::FileType::ROOT: {
		auto filter = static_cast<ROOTFilter*>(currentFilter);
		lines = m_rootOptionsWidget->lines();
		m_rootOptionsWidget->setNRows(filter->rowsInCurrentObject(file));
		importedStrings = filter->previewCurrentObject(file,
													   m_rootOptionsWidget->startRow(),
													   std::min(m_rootOptionsWidget->startRow() + lines - 1, m_rootOptionsWidget->endRow()));
		tmpTableWidget = m_rootOptionsWidget->previewWidget();
		// the last vector element contains the column names
		vectorNameList = importedStrings.last();
		importedStrings.removeLast();
		columnModes = QVector<AbstractColumn::ColumnMode>(vectorNameList.size(), AbstractColumn::ColumnMode::Double);
		break;
	}
	case AbstractFileFilter::FileType::Spice: {
		ui.tePreview->clear();
		auto filter = static_cast<SpiceFilter*>(currentFilter);
		importedStrings = filter->preview(file, lines);
		vectorNameList = filter->vectorNames();
		columnModes = filter->columnModes();
		break;
	}
	case AbstractFileFilter::FileType::READSTAT: {
		ui.tePreview->clear();
		auto filter = static_cast<ReadStatFilter*>(currentFilter);
		importedStrings = filter->preview(file, lines);
		vectorNameList = filter->vectorNames();
		columnModes = filter->columnModes();
		DEBUG(Q_FUNC_INFO << ", got " << columnModes.size() << " columns and " << importedStrings.size() << " rows")
		break;
	}
	case AbstractFileFilter::FileType::MATIO: {
		auto filter = static_cast<MatioFilter*>(currentFilter);
		lines = m_matioOptionsWidget->lines();

		QVector<QStringList> strings;
		// loop over all selected vars
		for (const QString& var : filter->selectedVarNames()) {
			// DEBUG(Q_FUNC_INFO << ", reading variable: " << STDSTRING(var))
			filter->setCurrentVarName(var);
			strings = filter->readCurrentVar(file, nullptr, AbstractFileFilter::ImportMode::Replace, lines);
			if (importedStrings.size() == 0) // first var
				importedStrings = strings;
			else { // append
				if (importedStrings.size() < strings.size()) { // more rows than before
					const int oldSize = importedStrings.size();
					importedStrings.resize(strings.size());
					for (int row = oldSize; row < strings.size(); row++) // fill new items
						for (int col = 0; col < importedStrings.at(0).size(); col++)
							importedStrings[row] << QString();
				}
				for (int i = 0; i < strings.size(); i++)
					importedStrings[i] << strings.at(i);
			}
		}

		tmpTableWidget = m_matioOptionsWidget->previewWidget();
		break;
	}
	}
	// fill the table widget
	tmpTableWidget->setRowCount(0);
	tmpTableWidget->setColumnCount(0);
	if (!importedStrings.isEmpty()) {
		if (!ok) {
			// show imported strings as error message
			tmpTableWidget->setRowCount(1);
			tmpTableWidget->setColumnCount(1);
			auto* item = new QTableWidgetItem();
			item->setText(importedStrings[0][0]);
			tmpTableWidget->setItem(0, 0, item);
		} else {
			const int rowCount = std::max(importedStrings.size(), static_cast<qsizetype>(1));
			const int maxColumns = 300;
			tmpTableWidget->setRowCount(rowCount);

			for (int row = 0; row < rowCount; ++row) {
				const int colCount = importedStrings.at(row).size() > maxColumns ? maxColumns : importedStrings.at(row).size();
				if (colCount > tmpTableWidget->columnCount())
					tmpTableWidget->setColumnCount(colCount);

				for (int col = 0; col < colCount; ++col) {
					auto* item = new QTableWidgetItem(importedStrings[row][col]);
					tmpTableWidget->setItem(row, col, item);
				}
			}

			// XLSX and Ods has special h/vheader, don't overwrite the preview table
			if (fileType != AbstractFileFilter::FileType::XLSX && fileType != AbstractFileFilter::FileType::Ods) {
				// set header if columnMode available
				for (int i = 0; i < std::min(static_cast<qsizetype>(tmpTableWidget->columnCount()), columnModes.size()); ++i) {
					QString columnName = QString::number(i + 1);
					if (i < vectorNameList.size())
						columnName = vectorNameList.at(i);

					auto* item = new QTableWidgetItem(columnName + QStringLiteral(" {")
													  + QLatin1String(ENUM_TO_STRING(AbstractColumn, ColumnMode, columnModes.at(i))) + QStringLiteral("}"));
					item->setTextAlignment(Qt::AlignLeft);
					item->setIcon(AbstractColumn::modeIcon(columnModes.at(i)));

					DEBUG("COLUMN " << i + 1 << " NAME = " << STDSTRING(columnName))
					tmpTableWidget->setHorizontalHeaderItem(i, item);
				}
			}
		}

		tmpTableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	}

	if (!errorMessage.isEmpty())
		Q_EMIT error(errorMessage);
	else if (!currentFilter->lastError().isEmpty())
		Q_EMIT error(currentFilter->lastError());
	else
		Q_EMIT error(QStringLiteral(""));

	if (currentFilter->lastError().isEmpty() && errorMessage.isEmpty())
		Q_EMIT previewReady();
}

void ImportFileWidget::updateStartRow(int line) {
	if (line >= ui.sbStartRow->value())
		ui.sbStartRow->setValue(line + 1);
}

void ImportFileWidget::checkValid() {
	QString errorMessage;
	if (m_asciiOptionsWidget && !m_asciiOptionsWidget->isValid(errorMessage))
		Q_EMIT error(errorMessage);
	else
		Q_EMIT error({});
}

void ImportFileWidget::updateContent(const QString& fileName) {
	DEBUG(Q_FUNC_INFO << ", file name = " << STDSTRING(fileName));

	if (m_suppressRefresh)
		return;

	QApplication::processEvents(QEventLoop::AllEvents, 0);
	WAIT_CURSOR;
	CleanupNoArguments cleanup([] () {
		RESET_CURSOR;
	});

	if (auto filter = currentFileFilter()) {
		switch (filter->type()) {
		case AbstractFileFilter::FileType::HDF5: {
			int status = m_hdf5OptionsWidget->updateContent(static_cast<HDF5Filter*>(filter), fileName);
			if (status != 0) { // parsing failed: switch to binary filter
				ui.cbFileType->setCurrentIndex(ui.cbFileType->findData(static_cast<int>(AbstractFileFilter::FileType::Binary)));
				Q_EMIT error(i18n("Not a HDF5 file: %1", fileName));
			}
			break;
		}
		case AbstractFileFilter::FileType::NETCDF:
			// TODO: check status (see HDF5)
			m_netcdfOptionsWidget->updateContent(static_cast<NetCDFFilter*>(filter), fileName);
			break;
			//		case AbstractFileFilter::FileType::VECTOR_BLF:
			//			m_vectorBLFOptionsWidget->updateContent(static_cast<VectorBLFFilter*>(filter), fileName);
			//			break;
		case AbstractFileFilter::FileType::FITS:
#ifdef HAVE_FITS
			// TODO: check status (see HDF5)
			m_fitsOptionsWidget->updateContent(static_cast<FITSFilter*>(filter), fileName);
#endif
			break;
		case AbstractFileFilter::FileType::ROOT:
			// TODO: check status (see HDF5)
			m_rootOptionsWidget->updateContent(static_cast<ROOTFilter*>(filter), fileName);
			break;
		case AbstractFileFilter::FileType::JSON:
			m_jsonOptionsWidget->loadDocument(fileName);
			ui.tvJson->setExpanded(m_jsonOptionsWidget->model()->index(0, 0), true); // expand the root node
			break;
		case AbstractFileFilter::FileType::MCAP: {
#ifdef HAVE_MCAP
			DEBUG(Q_FUNC_INFO << "loadDocument, file name = " << STDSTRING(fileName));
			auto* mcap_filter = static_cast<McapFilter*>(filter);

			if (!mcapTopicsInitialized) {
				ui.cbMcapTopics->clear();
				const auto& mcapTopics = mcap_filter->getValidTopics(fileName);
				for (int i = 0; i < mcapTopics.size(); i++)
					ui.cbMcapTopics->addItem(mcapTopics.at(i));
				mcapTopicsInitialized = true;
			}

			const auto& currentMcapTopic = ui.cbMcapTopics->currentText();
			DEBUG("Current selected topic" << STDSTRING(currentMcapTopic));
			mcap_filter->setCurrentTopic(currentMcapTopic);
#endif
			break;
		}
		case AbstractFileFilter::FileType::MATIO:
			// TODO: check status (see HDF5)
			m_matioOptionsWidget->updateContent(static_cast<MatioFilter*>(filter), fileName);
			break;
		case AbstractFileFilter::FileType::XLSX:
#ifdef HAVE_QXLSX
			// TODO: check status (see HDF5)
			m_xlsxOptionsWidget->updateContent(static_cast<XLSXFilter*>(filter), fileName);
#endif
			break;
		case AbstractFileFilter::FileType::Ods: {
#ifdef HAVE_ORCUS
			bool status = m_odsOptionsWidget->updateContent(static_cast<OdsFilter*>(filter), fileName);
			if (!status)
				Q_EMIT error(i18n("Parsing ODS file %1 failed.", fileName));

#endif
			break;
		}
		case AbstractFileFilter::FileType::Ascii:
		case AbstractFileFilter::FileType::Binary:
		case AbstractFileFilter::FileType::Image:
		case AbstractFileFilter::FileType::Spice:
		case AbstractFileFilter::FileType::READSTAT:
		case AbstractFileFilter::FileType::VECTOR_BLF:
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

	if (sourceType == LiveDataSource::SourceType::NetworkTCPSocket || sourceType == LiveDataSource::SourceType::LocalSocket
		|| sourceType == LiveDataSource::SourceType::SerialPort || readingType == LiveDataSource::ReadingType::TillEnd
		|| readingType == LiveDataSource::ReadingType::WholeFile) {
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

void ImportFileWidget::firstRowAsColNamesChanged(bool checked) {
	if (checked) {
		if (ui.sbStartRow->value() == 1)
			ui.sbStartRow->setValue(2);
	} else
		ui.sbStartRow->setValue(1);
}

void ImportFileWidget::sourceTypeChanged(int idx) {
	const auto sourceType = static_cast<LiveDataSource::SourceType>(idx);

#ifdef HAVE_MQTT
	// when switching from mqtt to another source type, make sure we disconnect from
	// the current broker, if connected, in order not to get any notification anymore
	if (sourceType != LiveDataSource::SourceType::MQTT)
		disconnectMqttConnection();
#endif

	// enable/disable "on new data"-option
	const auto* model = qobject_cast<const QStandardItemModel*>(ui.cbUpdateType->model());
	auto* item = model->item(static_cast<int>(LiveDataSource::UpdateType::NewData));

	ui.gbOptions->setEnabled(true);

	switch (sourceType) {
	case LiveDataSource::SourceType::FileOrPipe:
		ui.lFileName->show();
		m_cbFileName->show();
		ui.bFileInfo->show();
		ui.bOpen->show();
		if (m_liveDataSource) {
			ui.lRelativePath->show();
			ui.chbRelativePath->show();
		}
		ui.chbLinkFile->show();

		// option for sample size are available for "continuously fixed" and "from end" reading options
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
	case LiveDataSource::SourceType::NetworkTCPSocket:
	case LiveDataSource::SourceType::NetworkUDPSocket:
		ui.lHost->show();
		ui.leHost->show();
		ui.lePort->show();
		ui.lPort->show();
		if (sourceType == LiveDataSource::SourceType::NetworkTCPSocket) {
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

		// don't allow to select "New Data" for network sockets.
		// select "Periodically" in the combo box in case "New Data" was selected before
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		ui.cbUpdateType->setCurrentIndex(0);

		ui.gbOptions->setEnabled(true);
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

		// don't allow to select "New Data" serial port.
		// select "Periodically" in the combo box in case "New Data" was selected before
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		ui.cbUpdateType->setCurrentIndex(0);

		ui.cbFileType->setEnabled(true);
		ui.cbFileType->show();
		ui.gbOptions->setEnabled(true);
		ui.cbFilter->setEnabled(true);
		ui.lFileType->show();
		setMQTTVisible(false);
		break;
	case LiveDataSource::SourceType::MQTT:
#ifdef HAVE_MQTT
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		// for MQTT we read ascii data only, hide the file type options
		for (int i = 0; i < ui.cbFileType->count(); ++i) {
			if (static_cast<AbstractFileFilter::FileType>(ui.cbFileType->itemData(i).toInt()) == AbstractFileFilter::FileType::Ascii) {
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
		ui.cbFilter->setEnabled(true);

		// in case there are already connections defined,
		// show the available topics for the currently selected connection
		mqttConnectionChanged();
#endif
		break;
	}

	// deactivate/activate options that are specific to file of pipe sources only
	auto* typeModel = qobject_cast<const QStandardItemModel*>(ui.cbFileType->model());
	if (sourceType != LiveDataSource::SourceType::FileOrPipe) {
		// deactivate file types other than ascii and binary
		for (int i = 2; i < ui.cbFileType->count(); ++i)
			typeModel->item(i)->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (ui.cbFileType->currentIndex() > 1)
			ui.cbFileType->setCurrentIndex(1);

		//"whole file" read option is available for file or pipe only, disable it
		typeModel = qobject_cast<const QStandardItemModel*>(ui.cbReadingType->model());
		auto* item = typeModel->item(static_cast<int>(LiveDataSource::ReadingType::WholeFile));
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (static_cast<LiveDataSource::ReadingType>(ui.cbReadingType->currentIndex()) == LiveDataSource::ReadingType::WholeFile)
			ui.cbReadingType->setCurrentIndex(static_cast<int>(LiveDataSource::ReadingType::TillEnd));

		//"update options" groupbox can be deactivated for "file and pipe" if the file is invalid.
		// Activate the groupbox when switching from "file and pipe" to a different source type.
		ui.gbUpdateOptions->setEnabled(true);
	} else {
		// enable "whole file" item for file or pipe
		typeModel = qobject_cast<const QStandardItemModel*>(ui.cbReadingType->model());
		auto* item = typeModel->item(static_cast<int>(LiveDataSource::ReadingType::WholeFile));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}

	updateFilterHandlingSettings(sourceType);

	updateHeaderOptions();

	Q_EMIT sourceTypeChanged();
	refreshPreview();
}

void ImportFileWidget::enableDataPortionSelection(bool enabled) {
	ui.tabWidget->setTabEnabled(ui.tabWidget->indexOf(ui.tabDataPortion), enabled);
}

void ImportFileWidget::changeMcapTopic() {
	auto filter = currentFileFilter();
	if (filter->type() != AbstractFileFilter::FileType::MCAP)
		return;

	auto* mcap_filter = static_cast<McapFilter*>(filter);
	if (!(mcap_filter->getCurrentTopic() == ui.cbMcapTopics->currentText()))
		refreshPreview();
}

#ifdef HAVE_MQTT
/*!
 *\brief called when a different MQTT connection is selected in the connection ComboBox.
 * connects to the MQTT broker according to the connection settings.
 */
void ImportFileWidget::mqttConnectionChanged() {
	if (m_initialisingMQTT || ui.cbConnection->currentIndex() == -1) {
		ui.lLWT->hide();
		ui.bLWT->hide();
		ui.lMqttTopics->hide();
		return;
	}

	WAIT_CURSOR;
	CleanupNoArguments cleanup([] () {
		RESET_CURSOR;
	});
	Q_EMIT error(QString());

	// disconnected from the broker that was selected before
	disconnectMqttConnection();

	delete m_client;
	m_client = new QMqttClient;
	connect(m_client, &QMqttClient::connected, this, &ImportFileWidget::onMqttConnect);
	connect(m_client, &QMqttClient::disconnected, this, &ImportFileWidget::onMqttDisconnect);
	connect(m_client, &QMqttClient::messageReceived, this, &ImportFileWidget::mqttMessageReceived);
	connect(m_client, &QMqttClient::errorChanged, this, &ImportFileWidget::mqttErrorChanged);

	// determine the connection settings for the new broker and initialize the mqtt client
	KConfig config(m_configPath, KConfig::SimpleConfig);
	KConfigGroup group = config.group(ui.cbConnection->currentText());
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

	// connect to the selected broker
	QDEBUG("Connect to " << m_client->hostname() << ":" << m_client->port());
	if (!m_connectTimeoutTimer) {
		m_connectTimeoutTimer = new QTimer(this);
		m_connectTimeoutTimer->setInterval(6000);
		connect(m_connectTimeoutTimer, &QTimer::timeout, this, &ImportFileWidget::mqttConnectTimeout);
	}
	m_connectTimeoutTimer->start();
	m_client->connectToHost();
}

void ImportFileWidget::disconnectMqttConnection() {
	if (m_client && m_client->state() == QMqttClient::ClientState::Connected) {
		Q_EMIT MQTTClearTopics();
		disconnect(m_client, &QMqttClient::disconnected, this, &ImportFileWidget::onMqttDisconnect);
		QDEBUG("Disconnecting from " << m_client->hostname());
		m_client->disconnectFromHost();
		delete m_client;
		m_client = nullptr;
	}
}

/*!
 * returns \c true if there is a valid connection to an MQTT broker and the user has subscribed to at least 1 topic,
 * returns \c false otherwise.
 */
bool ImportFileWidget::isMqttValid() {
	if (!m_client)
		return false;
	if (m_client->state() != QMqttClient::ClientState::Connected)
		return false;
	if (!m_subscriptionWidget->subscriptionCount())
		return false;
	if (this->currentFileType() != AbstractFileFilter::FileType::Ascii)
		return false;

	return true;
}

/*!
 *\brief called when the client connects to the broker successfully.
 * subscribes to every topic (# wildcard) in order to later list every available topic
 */
void ImportFileWidget::onMqttConnect() {
	m_connectTimeoutTimer->stop();
	if (m_client->error() == QMqttClient::NoError) {
		ui.frameSubscriptions->setVisible(true);
		m_subscriptionWidget->setVisible(true);
		m_subscriptionWidget->makeVisible(true);

		if (!m_client->subscribe(QMqttTopicFilter(QLatin1String("#")), 1))
			Q_EMIT error(i18n("Couldn't subscribe to all available topics."));
		else {
			Q_EMIT error(QString());
			ui.lLWT->show();
			ui.bLWT->show();
			ui.lMqttTopics->show();
		}
	} else
		Q_EMIT error(i18n("Failed to connect to '%1'. Error %2.", m_client->hostname(), QString::number(m_client->error())));

	Q_EMIT subscriptionsChanged();
	RESET_CURSOR;
}

/*!
 *\brief called when the client disconnects from the broker successfully
 * removes every information about the former connection
 */
void ImportFileWidget::onMqttDisconnect() {
	DEBUG("Disconnected from " << STDSTRING(m_client->hostname()));
	m_connectTimeoutTimer->stop();

	ui.lMqttTopics->hide();
	ui.frameSubscriptions->hide();
	ui.lLWT->hide();
	ui.bLWT->hide();

	ui.cbConnection->setCurrentIndex(-1);

	Q_EMIT subscriptionsChanged();
	Q_EMIT error(i18n("Disconnected from '%1'.", m_client->hostname()));
	RESET_CURSOR;
}

/*!
 *\brief called when the subscribe button is pressed
 * subscribes to the topic represented by the current item of twTopics
 */
void ImportFileWidget::subscribeTopic(const QString& name, uint QoS) {
	auto* tempSubscription = m_client->subscribe(QMqttTopicFilter(name), static_cast<quint8>(QoS));
	if (tempSubscription) {
		m_mqttSubscriptions.push_back(tempSubscription);
		connect(tempSubscription, &QMqttSubscription::messageReceived, this, &ImportFileWidget::mqttSubscriptionMessageReceived);
		Q_EMIT subscriptionsChanged();
	}
}

/*!
 *\brief Unsubscribes from the given topic, and removes any data connected to it
 *
 * \param topicName the name of a topic we want to unsubscribe from
 */
void ImportFileWidget::unsubscribeTopic(const QString& topicName, QVector<QTreeWidgetItem*> children) {
	if (topicName.isEmpty())
		return;

	for (int i = 0; i < m_mqttSubscriptions.count(); ++i) {
		if (m_mqttSubscriptions[i]->topic().filter() == topicName) {
			// explicitly disconnect from the signal, callling QMqttClient::unsubscribe() below is not enough
			disconnect(m_mqttSubscriptions.at(i), &QMqttSubscription::messageReceived, this, &ImportFileWidget::mqttSubscriptionMessageReceived);
			m_mqttSubscriptions.remove(i);
			break;
		}
	}

	m_client->unsubscribe(QMqttTopicFilter(topicName));

	QMapIterator<QMqttTopicName, QMqttMessage> j(m_lastMessage);
	while (j.hasNext()) {
		j.next();
		if (MQTTSubscriptionWidget::checkTopicContains(topicName, j.key().name()))
			m_lastMessage.remove(j.key());
	}

	if (m_willSettings.willTopic == topicName) {
		if (m_subscriptionWidget->subscriptionCount() > 0)
			m_willSettings.willTopic = children[0]->text(0);
		else
			m_willSettings.willTopic.clear();
	}

	// signals that there was a change among the subscribed topics
	Q_EMIT subscriptionsChanged();
	refreshPreview();
}

/*!
 *\brief called when the client receives a message
 * if the message arrived from a new topic, the topic is put in twTopics
 */
void ImportFileWidget::mqttMessageReceived(const QByteArray& /*message*/, const QMqttTopicName& topic) {
	// 	qDebug()<<"received " << topic.name();
	if (m_addedTopics.contains(topic.name()))
		return;

	m_addedTopics.push_back(topic.name());
	m_subscriptionWidget->setTopicTreeText(i18n("Available (%1)", m_addedTopics.size()));
	QStringList name;
	QString rootName;
	const QChar sep = QLatin1Char('/');

	if (topic.name().contains(sep)) {
		const QStringList& list = topic.name().split(sep, Qt::SkipEmptyParts);

		if (!list.isEmpty()) {
			rootName = list.at(0);
			name.append(list.at(0));
			int topItemIdx = -1;
			// check whether the first level of the topic can be found in twTopics
			for (int i = 0; i < m_subscriptionWidget->topicCount(); ++i) {
				if (m_subscriptionWidget->topLevelTopic(i)->text(0) == list.at(0)) {
					topItemIdx = i;
					break;
				}
			}

			// if not we simply add every level of the topic to the tree
			if (topItemIdx < 0) {
				auto* currentItem = new QTreeWidgetItem(name);
				m_subscriptionWidget->addTopic(currentItem);
				for (int i = 1; i < list.size(); ++i) {
					name.clear();
					name.append(list.at(i));
					currentItem->addChild(new QTreeWidgetItem(name));
					currentItem = currentItem->child(0);
				}
			}
			// otherwise we search for the first level that isn't part of the tree,
			// then add every level of the topic to the tree from that certain level
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
						// this is the level that isn't present in the tree
						break;
					}
				}

				// add every level to the tree starting with the first level that isn't part of the tree
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

	// if a subscribed topic contains the new topic, we have to update twSubscriptions
	for (int i = 0; i < m_subscriptionWidget->subscriptionCount(); ++i) {
		const QStringList subscriptionName = m_subscriptionWidget->topLevelSubscription(i)->text(0).split(sep, Qt::SkipEmptyParts);
		if (!subscriptionName.isEmpty()) {
			if (rootName == subscriptionName.first()) {
				QVector<QString> subscriptions;
				for (const auto& sub : m_mqttSubscriptions)
					subscriptions.push_back(sub->topic().filter());
				Q_EMIT updateSubscriptionTree(subscriptions);
				break;
			}
		}
	}

	// signals that a newTopic was added, in order to fill the completer of leTopics
	Q_EMIT newTopic(rootName);
}

/*!
 *\brief called when the client receives a message from a subscribed topic (that isn't the "#" wildcard)
 */
void ImportFileWidget::mqttSubscriptionMessageReceived(const QMqttMessage& msg) {
	QDEBUG("message received from: " << msg.topic().name());

	// update the last message for the topic
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
		Q_EMIT error(i18n("Wrong username or password"));
		break;
	case QMqttClient::IdRejected:
		Q_EMIT error(i18n("The client ID wasn't accepted"));
		break;
	case QMqttClient::ServerUnavailable:
	case QMqttClient::TransportInvalid:
		Q_EMIT error(i18n("The broker %1 couldn't be reached.", m_client->hostname()));
		break;
	case QMqttClient::NotAuthorized:
		Q_EMIT error(i18n("The client is not authorized to connect."));
		break;
	case QMqttClient::UnknownError:
		Q_EMIT error(i18n("An unknown error occurred."));
		break;
	case QMqttClient::NoError:
	case QMqttClient::InvalidProtocolVersion:
	case QMqttClient::ProtocolViolation:
	case QMqttClient::Mqtt5SpecificError:
		Q_EMIT error(i18n("An error occurred."));
		break;
	default:
		Q_EMIT error(i18n("An error occurred."));
		break;
	}
	m_connectTimeoutTimer->stop();
}

/*!
 *\brief called when m_connectTimeoutTimer ticks,
 *		 meaning that the client couldn't connect to the broker in 5 seconds
 *		 disconnects the client, stops the timer, and warns the user
 */
void ImportFileWidget::mqttConnectTimeout() {
	m_client->disconnectFromHost();
	m_connectTimeoutTimer->stop();
	Q_EMIT error(i18n("Connecting to '%1:%2' timed out.", m_client->hostname(), m_client->port()));
	RESET_CURSOR;
}

/*!
	Shows the MQTT connection manager where the connections are created and edited.
	The selected connection is selected in the connection combo box in this widget.
*/
void ImportFileWidget::showMQTTConnectionManager() {
	bool previousConnectionChanged = false;
	auto* dlg = new MQTTConnectionManagerDialog(this, ui.cbConnection->currentText(), previousConnectionChanged);

	if (dlg->exec() == QDialog::Accepted) {
		// re-read the available connections to be in sync with the changes in MQTTConnectionManager
		m_initialisingMQTT = true;
		const QString& prevConn = ui.cbConnection->currentText();
		ui.cbConnection->clear();
		readMQTTConnections();
		m_initialisingMQTT = false;

		// select the connection the user has selected in MQTTConnectionManager
		const QString& conn = dlg->connection();

		int index = ui.cbConnection->findText(conn);
		if (conn != prevConn) { // Current connection isn't the previous one
			if (ui.cbConnection->currentIndex() != index)
				ui.cbConnection->setCurrentIndex(index);
			else
				mqttConnectionChanged();
		} else if (dlg->initialConnectionChanged()) { // Current connection is the same with previous one but it changed
			if (ui.cbConnection->currentIndex() == index)
				mqttConnectionChanged();
			else
				ui.cbConnection->setCurrentIndex(index);
		} else { // Previous connection wasn't changed
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
	for (const auto& child : children)
		topics.append(child->text(0));

	MQTTWillSettingsWidget willSettingsWidget(&menu, m_willSettings, topics);

	connect(&willSettingsWidget, &MQTTWillSettingsWidget::applyClicked, [this, &menu, &willSettingsWidget]() {
		m_willSettings = willSettingsWidget.will();
		menu.close();
	});
	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&willSettingsWidget);
	menu.addAction(widgetAction);

	const QPoint pos(ui.bLWT->sizeHint().width(), ui.bLWT->sizeHint().height());
	menu.exec(ui.bLWT->mapToGlobal(pos));
}

void ImportFileWidget::enableWill(bool enable) {
	ui.bLWT->setEnabled(enable);
}

/*!
	saves the settings to the MQTTClient \c client.
*/
void ImportFileWidget::saveMQTTSettings(MQTTClient* client) const {
	DEBUG(Q_FUNC_INFO);
	auto updateType = static_cast<MQTTClient::UpdateType>(ui.cbUpdateType->currentIndex());
	auto readingType = static_cast<MQTTClient::ReadingType>(ui.cbReadingType->currentIndex());

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
