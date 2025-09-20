/*
	File                 : ImportDialog.cc
	Project              : LabPlot
	Description          : import file data dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2008-2015 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImportFileDialog.h"
#include "ImportFileWidget.h"
#include "ImportWarningsDialog.h"
#include "backend/core/Settings.h"
#include "backend/core/Workbook.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/filters.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/MainWin.h"
#include "frontend/widgets/TreeViewComboBox.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif

#include <KLocalizedString>
#include <KWindowConfig>

#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QInputDialog>
#include <QLocalSocket>
#include <QMenu>
#include <QProgressBar>
#include <QStatusBar>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QWindow>

/*!
	\class ImportFileDialog
	\brief Dialog for importing data from a file. Embeds \c ImportFileWidget and provides the standard buttons.

	\ingroup frontend
 */

ImportFileDialog::ImportFileDialog(MainWin* parent, bool liveDataSource, const QString& path, bool importDir)
	: ImportDialog(parent)
	, m_importFileWidget(new ImportFileWidget(this, liveDataSource, path, false /* embedded */, importDir)) {
	vLayout->addWidget(m_importFileWidget);
	m_liveDataSource = liveDataSource;
	m_importDir = importDir;

	// dialog buttons
	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Reset | QDialogButtonBox::Cancel);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	m_optionsButton = buttonBox->button(QDialogButtonBox::Reset); // we highjack the default "Reset" button and use if for showing/hiding the options
	okButton->setEnabled(false); // ok is only available if a valid container was selected
	vLayout->addWidget(buttonBox);

	if (!liveDataSource)
		setModel(); // set the model and hide the data-source related widgets
	else
		setAttribute(Qt::WA_DeleteOnClose, false); // don't delete on close for live data sources, it's done in MainWin::newLiveDataSource()

	// Signals/Slots
	connect(buttonBox, &QDialogButtonBox::accepted, this, &ImportDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	if (!liveDataSource)
		setWindowTitle(i18nc("@title:window", "Import Data to Spreadsheet or Matrix"));
	else
		setWindowTitle(i18nc("@title:window", "Add New Live Data Source"));

	setWindowIcon(QIcon::fromTheme(QStringLiteral("document-import-database")));

	// restore saved settings if available
	create(); // ensure there's a window created

	KConfigGroup conf = Settings::group(QStringLiteral("ImportFileDialog"));
	if (conf.exists()) {
		m_showOptions = conf.readEntry("ShowOptions", false);

		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));

	m_importFileWidget->showOptions(m_showOptions);
	// do the signal-slot connections after all settings were loaded in import file widget and check the OK button after this
	connect(m_importFileWidget, &ImportFileWidget::enableImportToMatrix, this, &ImportFileDialog::enableImportToMatrix);
	connect(m_importFileWidget, QOverload<>::of(&ImportFileWidget::fileNameChanged), this, &ImportFileDialog::checkOkButton);
	connect(m_importFileWidget, QOverload<>::of(&ImportFileWidget::sourceTypeChanged), this, &ImportFileDialog::checkOkButton);
	connect(m_importFileWidget, &ImportFileWidget::hostChanged, this, &ImportFileDialog::checkOkButton);
	connect(m_importFileWidget, &ImportFileWidget::portChanged, this, &ImportFileDialog::checkOkButton);
	connect(m_importFileWidget, &ImportFileWidget::error, this, &ImportFileDialog::showErrorMessage);
	connect(m_importFileWidget, &ImportFileWidget::previewReady, this, &ImportFileDialog::checkOkButton);
	connect(this, &ImportDialog::dataContainerChanged, m_importFileWidget, &ImportFileWidget::dataContainerChanged);
#ifdef HAVE_MQTT
	connect(m_importFileWidget, &ImportFileWidget::subscriptionsChanged, this, &ImportFileDialog::checkOkButton);
	connect(m_importFileWidget, &ImportFileWidget::checkFileType, this, &ImportFileDialog::checkOkButton);
#endif

	m_showOptions ? m_optionsButton->setText(i18n("Hide Options")) : m_optionsButton->setText(i18n("Show Options"));
	connect(m_optionsButton, &QPushButton::clicked, this, &ImportFileDialog::toggleOptions);

	// Must be after connect, to send an error message if loading failed
	QApplication::processEvents(QEventLoop::AllEvents, 0);
	m_importFileWidget->loadSettings();
}

ImportFileDialog::~ImportFileDialog() {
	// save current settings
	KConfigGroup conf = Settings::group(QStringLiteral("ImportFileDialog"));
	conf.writeEntry("ShowOptions", m_showOptions);
	if (cbPosition)
		conf.writeEntry("Position", cbPosition->currentIndex());

	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

LiveDataSource::SourceType ImportFileDialog::sourceType() const {
	return m_importFileWidget->currentSourceType();
}

/*!
  triggers data import to the live data source \c source
*/
void ImportFileDialog::importToLiveDataSource(LiveDataSource* source, QStatusBar* statusBar) const {
	DEBUG(Q_FUNC_INFO);
	m_importFileWidget->saveSettings(source);

	// show a progress bar in the status bar
	auto* progressBar = new QProgressBar();
	progressBar->setRange(0, 100);
	connect(source->filter(), &AbstractFileFilter::completed, progressBar, &QProgressBar::setValue);

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);
	WAIT_CURSOR;

	QElapsedTimer timer;
	timer.start();
	DEBUG("	Initial read()");
	source->read();
	statusBar->showMessage(i18n("Live data source created in %1 seconds.", (float)timer.elapsed() / 1000));

	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
}

#ifdef HAVE_MQTT
/*!
  triggers data import to the MQTTClient \c client
*/
void ImportFileDialog::importToMQTT(MQTTClient* client) const {
	m_importFileWidget->saveMQTTSettings(client);
	client->read();
	client->ready();
}
#endif

/*!
  triggers data import to the currently selected data container
*/
bool ImportFileDialog::importTo(QStatusBar* statusBar) const {
	// a target aspect is required, do this check first
	auto* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
	if (!aspect) {
		DEBUG(Q_FUNC_INFO << ", ERROR: No aspect available");
		DEBUG("	cbAddTo->currentModelIndex().isValid() = " << cbAddTo->currentModelIndex().isValid());
		DEBUG("	cbAddTo->currentModelIndex() row/column = " << cbAddTo->currentModelIndex().row() << ' ' << cbAddTo->currentModelIndex().column());
		const_cast<ImportFileDialog*>(this)->showErrorMessage(i18n("No target data container selected"));
		return false;
	}

	// show a progress bar in the status bar
	auto* progressBar = new QProgressBar();
	progressBar->setRange(0, 100);
	const auto& path = m_importFileWidget->path();
	auto* filter = m_importFileWidget->currentFileFilter();
	filter->setLastError(QString()); // clear the previos error, if any available
	filter->clearLastWarnings(); // clear the previos warnings, if any available
	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);

	WAIT_CURSOR;
	QApplication::processEvents(QEventLoop::AllEvents, 100);
	QElapsedTimer timer;
	timer.start();

	if (!m_importDir) { // import a single file
		connect(filter, &AbstractFileFilter::completed, progressBar, &QProgressBar::setValue);
		const auto mode = AbstractFileFilter::ImportMode(cbPosition->currentIndex());
		importFile(path, aspect, filter, mode);
		statusBar->showMessage(i18n("File %1 imported in %2 seconds.", path, (float)timer.elapsed() / 1000));
	} else { // import all files from a directory
		QDir dir(path);
		if (!dir.exists()) {
			const_cast<ImportFileDialog*>(this)->showErrorMessage(i18n("The directory %1 doesn't exist.", path));
			RESET_CURSOR;
			statusBar->removeWidget(progressBar);
			return false;
		}

		const auto files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
		const int totalCount = files.count();
		int count = 0;

		// iterate over all files in the directory and import them one by one.
		// we import into spreadsheets only at the moment and re-use existing sheets if the name matches
		const auto& sheets = aspect->children<Spreadsheet>();
		for (const auto& fileName : files) {
			// if there's already a sheet with the same name, use it
			Spreadsheet* sheet = nullptr;
			for (const auto& s : sheets) {
				if (s->name() == fileName) {
					sheet = s;
					break;
				}
			}
			if (!sheet) {
				sheet = new Spreadsheet(fileName);
				aspect->addChild(sheet);
			}
			importFile(dir.absoluteFilePath(fileName), sheet, filter);
			++count;
			progressBar->setValue(count/totalCount * 100);
		}

		statusBar->showMessage(i18n("%1 files imported in %2 seconds.", count, (float)timer.elapsed() / 1000));
	}

	RESET_CURSOR;

	// handle errors
	if (!filter->lastError().isEmpty()) {
		const_cast<ImportFileDialog*>(this)->showErrorMessage(filter->lastError());
		return false;
	}

	// show warnings, if available
	const auto& warnings = filter->lastWarnings();
	if (!warnings.isEmpty()) {
		auto* d = new ImportWarningsDialog(warnings, m_mainWin);
		d->show();
	}

	statusBar->removeWidget(progressBar);
	DEBUG(Q_FUNC_INFO << ", DONE")
	return true;
}

void ImportFileDialog::importFile(const QString& fileName, AbstractAspect* aspect, AbstractFileFilter* filter, AbstractFileFilter::ImportMode mode) const {
	DEBUG(Q_FUNC_INFO << ", file name: " << fileName.toStdString());
	if (aspect->inherits(AspectType::Matrix)) {
		DEBUG(Q_FUNC_INFO << ", to Matrix");
		auto* matrix = qobject_cast<Matrix*>(aspect);
		filter->readDataFromFile(fileName, matrix, mode);
	} else if (aspect->inherits(AspectType::Spreadsheet)) {
		DEBUG(Q_FUNC_INFO << ", to Spreadsheet");
		auto* spreadsheet = qobject_cast<Spreadsheet*>(aspect);
		DEBUG("CALLING filter->readDataFromFile()")
		// TODO: which extension (table) is imported?
		filter->readDataFromFile(fileName, spreadsheet, mode);
	} else if (aspect->inherits(AspectType::Workbook)) {
		DEBUG(Q_FUNC_INFO << ", to Workbook");
		auto* workbook = static_cast<Workbook*>(aspect);
		workbook->setUndoAware(false);
		auto sheets = workbook->children<AbstractAspect>();

		auto fileType = m_importFileWidget->currentFileType();
		// types supporting multiple data sets/variables
		if (fileType == AbstractFileFilter::FileType::HDF5 || fileType == AbstractFileFilter::FileType::NETCDF || fileType == AbstractFileFilter::FileType::ROOT
			|| fileType == AbstractFileFilter::FileType::MATIO || fileType == AbstractFileFilter::FileType::XLSX
			|| fileType == AbstractFileFilter::FileType::Ods || fileType == AbstractFileFilter::FileType::VECTOR_BLF) {
			QStringList names;
			if (fileType == AbstractFileFilter::FileType::HDF5)
				names = m_importFileWidget->selectedHDF5Names();
			else if (fileType == AbstractFileFilter::FileType::VECTOR_BLF)
				names = QStringList({QStringLiteral("TODO")}); // m_importFileWidget->selectedVectorBLFNames();
			else if (fileType == AbstractFileFilter::FileType::NETCDF)
				names = m_importFileWidget->selectedNetCDFNames();
			else if (fileType == AbstractFileFilter::FileType::ROOT)
				names = m_importFileWidget->selectedROOTNames();
			else if (fileType == AbstractFileFilter::FileType::MATIO)
				names = m_importFileWidget->selectedMatioNames();
			else if (fileType == AbstractFileFilter::FileType::XLSX)
				names = m_importFileWidget->selectedXLSXRegionNames();
			else if (fileType == AbstractFileFilter::FileType::Ods)
				names = m_importFileWidget->selectedOdsSheetNames();

			int nrNames = names.size(), offset = sheets.size();
			QDEBUG(Q_FUNC_INFO << ", selected names: " << names)

			// TODO: think about importing multiple sets into one sheet

			int start = 0; // add nrNames sheets (0 to nrNames)

			// in replace mode add only missing sheets (from offset to nrNames)
			// and rename the already available sheets
			if (mode == AbstractFileFilter::ImportMode::Replace) {
				start = offset;

				// if there are more available spreadsheets, than needed,
				// delete the unneeded spreadsheets
				if (offset > nrNames) {
					for (int i = nrNames; i < offset; i++)
						sheets.at(i)->remove();
					offset = nrNames;
				}

				// rename the available sheets
				for (int i = 0; i < offset; ++i) {
					// HDF5 and Ods names contain the whole path, remove it and keep the name only
					QString sheetName = names.at(i);
					if (fileType == AbstractFileFilter::FileType::HDF5 || fileType == AbstractFileFilter::FileType::Ods)
						sheetName = sheetName.split(QLatin1Char('/')).last();

					auto* sheet = sheets.at(i);
					sheet->setUndoAware(false);
					sheet->setName(sheetName);
					sheet->setUndoAware(true);
				}
			}

			// add additional spreadsheets
			for (int i = start; i < nrNames; ++i) {
				// HDF5 and Ods names contain the whole path, remove it and keep the name only
				QString sheetName = names.at(i);
				if (fileType == AbstractFileFilter::FileType::HDF5 || fileType == AbstractFileFilter::FileType::Ods)
					sheetName = sheetName.split(QLatin1Char('/')).last();

				auto* spreadsheet = new Spreadsheet(sheetName);
				if (mode == AbstractFileFilter::ImportMode::Prepend && !sheets.isEmpty())
					workbook->insertChildBefore(spreadsheet, sheets.at(0));
				else
					workbook->addChildFast(spreadsheet);
			}

			// start at offset for append, else at 0
			if (mode != AbstractFileFilter::ImportMode::Append)
				offset = 0;

			// import every set to a different sheet
			sheets = workbook->children<AbstractAspect>();
			for (int i = 0; i < nrNames; ++i) {
				if (fileType == AbstractFileFilter::FileType::HDF5)
					static_cast<HDF5Filter*>(filter)->setCurrentDataSetName(names.at(i));
				else if (fileType == AbstractFileFilter::FileType::NETCDF)
					static_cast<NetCDFFilter*>(filter)->setCurrentVarName(names.at(i));
				else if (fileType == AbstractFileFilter::FileType::MATIO)
					static_cast<MatioFilter*>(filter)->setCurrentVarName(names.at(i));
				else if (fileType == AbstractFileFilter::FileType::Ods) // all selected sheets are imported
					static_cast<OdsFilter*>(filter)->setSelectedSheetNames(QStringList() << names.at(i));
				else if (fileType == AbstractFileFilter::FileType::XLSX) {
					const auto& nameSplit = names.at(i).split(QLatin1Char('!'));
					const auto& sheet = nameSplit.at(0);
					const auto& range = nameSplit.at(1);
					static_cast<XLSXFilter*>(filter)->setCurrentSheet(sheet);
					static_cast<XLSXFilter*>(filter)->setCurrentRange(range);
				} else
					static_cast<ROOTFilter*>(filter)->setCurrentObject(names.at(i));

				int index = i + offset;
				filter->readDataFromFile(fileName, qobject_cast<Spreadsheet*>(sheets.at(index)));
			}

			workbook->setUndoAware(true);
		} else { // single import file types
			// workbook selected -> create a new spreadsheet in the workbook
			workbook->setUndoAware(true);
			auto* spreadsheet = new Spreadsheet(fileName);
			workbook->addChild(spreadsheet);
			workbook->setUndoAware(false);
			filter->readDataFromFile(fileName, spreadsheet, mode);
		}
	}
}

void ImportFileDialog::toggleOptions() {
	m_importFileWidget->showOptions(!m_showOptions);
	m_showOptions = !m_showOptions;
	m_showOptions ? m_optionsButton->setText(i18n("Hide Options")) : m_optionsButton->setText(i18n("Show Options"));

	// resize the dialog
	layout()->activate();
	resize(QSize(this->width(), 0).expandedTo(minimumSize()));
}

void ImportFileDialog::enableImportToMatrix(const bool enable) {
	if (cbAddTo) {
		QDEBUG("cbAddTo->currentModelIndex() = " << cbAddTo->currentModelIndex());
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
		if (!aspect) {
			DEBUG("ERROR: no aspect available.");
			return;
		}

		if (aspect->inherits(AspectType::Matrix)) {
			okButton->setEnabled(enable);
			if (enable)
				okButton->setToolTip(i18n("Close the dialog and import the data."));
			else
				okButton->setToolTip(i18n("Cannot import into a matrix since the data contains non-numerical data."));
		}
	}
}

void ImportFileDialog::checkOkButton() {
	DEBUG(Q_FUNC_INFO);
	if (cbAddTo) { // only check for the target container when no file data source is being added
		QDEBUG(" cbAddTo->currentModelIndex() = " << cbAddTo->currentModelIndex());
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
		if (!aspect) {
			okButton->setEnabled(false);
			okButton->setToolTip(i18n("Select a data container where the data has to be imported into."));
			cbAddTo->setFocus(); // set the focus to make the user aware about the fact that a data container needs to be provided
			return;
		}
	}

	QString fileName = ImportFileWidget::absolutePath(m_importFileWidget->path());
	const auto sourceType = m_importFileWidget->currentSourceType();
	switch (sourceType) {
		case LiveDataSource::SourceType::FileOrPipe: // fall through
		case LiveDataSource::SourceType::LocalSocket: {
			if (fileName.isEmpty()) {
				okButton->setEnabled(false);
				okButton->setToolTip(i18n("No file provided for import."));
				return;
			}
			break;
		}
		case LiveDataSource::SourceType::NetworkTCPSocket: // fall through
		case LiveDataSource::SourceType::NetworkUDPSocket: // fall through
		case LiveDataSource::SourceType::SerialPort: // fall through
		case LiveDataSource::SourceType::MQTT: // fall through
			break;
	}

	// process all events first so the dialog is completely drawn before we wait for the socket connect below
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	DEBUG(Q_FUNC_INFO << ", Data Source Type: " << ENUM_TO_STRING(LiveDataSource, SourceType, sourceType));
	switch (sourceType) {
	case LiveDataSource::SourceType::FileOrPipe: {
		DEBUG(Q_FUNC_INFO << ", fileName = " << qPrintable(fileName));
		const bool enable = QFile::exists(fileName);
		if (enable)
			showErrorMessage(QString());
		else {
			// suppress the error widget when the dialog is opened the first time.
			// show only the error widget if the file was really a non-existing file.
			showErrorMessage(i18n("The provided file doesn't exist."));
		}

		break;
	}
	case LiveDataSource::SourceType::LocalSocket: {
		const bool enable = QFile::exists(fileName);
		if (enable) {
			QLocalSocket lsocket{this};
			DEBUG("CONNECT");
			lsocket.connectToServer(fileName, QLocalSocket::ReadOnly);
			if (lsocket.waitForConnected()) {
				// this is required for server that send data as soon as connected
				lsocket.waitForReadyRead();

				DEBUG("DISCONNECT");
				lsocket.disconnectFromServer();
				// read-only socket is disconnected immediately (no waitForDisconnected())
				showErrorMessage(QString());
			} else {
				QString msg = i18n("Could not connect to the provided local socket. Error: %1.", lsocket.errorString());
				showErrorMessage(msg);
			}
		} else {
			QString msg = i18n("Could not connect to the provided local socket. The socket does not exist.");
			showErrorMessage(msg);
		}

		break;
	}
	case LiveDataSource::SourceType::NetworkTCPSocket: {
		const bool enable = !m_importFileWidget->host().isEmpty() && !m_importFileWidget->port().isEmpty();
		if (enable) {
			QTcpSocket socket(this);
			socket.connectToHost(m_importFileWidget->host(), m_importFileWidget->port().toUShort(), QTcpSocket::ReadOnly);
			if (socket.waitForConnected()) {
				showErrorMessage(QString());
				socket.disconnectFromHost();
			} else {
				QString msg = i18n("Could not connect to the provided TCP socket. Error: %1.", socket.errorString());
				showErrorMessage(msg);
			}
		} else {
			QString msg = i18n("Either the host name or the port number is missing.");
			showErrorMessage(msg);
		}
		break;
	}
	case LiveDataSource::SourceType::NetworkUDPSocket: {
		const bool enable = !m_importFileWidget->host().isEmpty() && !m_importFileWidget->port().isEmpty();
		if (enable) {
			QUdpSocket socket(this);
			socket.bind(QHostAddress(m_importFileWidget->host()), m_importFileWidget->port().toUShort());
			socket.connectToHost(m_importFileWidget->host(), 0, QUdpSocket::ReadOnly);
			if (socket.waitForConnected()) {
				showErrorMessage(QString());
				socket.disconnectFromHost();
				// read-only socket is disconnected immediately (no waitForDisconnected())
			} else {
				QString msg = i18n("Could not connect to the provided UDP socket. Error: %1.", socket.errorString());
				showErrorMessage(msg);
			}
		} else {
			QString msg = i18n("Either the host name or the port number is missing.");
			showErrorMessage(msg);
		}

		break;
	}
	case LiveDataSource::SourceType::SerialPort: {
#ifdef HAVE_QTSERIALPORT
		const QString sPort = m_importFileWidget->serialPort();

		if (!sPort.isEmpty()) {
			QSerialPort serialPort{this};
			const int baudRate = m_importFileWidget->baudRate();

			DEBUG("	Port: " << STDSTRING(sPort) << ", Settings: " << baudRate << ',' << serialPort.dataBits() << ',' << serialPort.parity() << ','
							<< serialPort.stopBits());
			serialPort.setPortName(sPort);
			serialPort.setBaudRate(baudRate);

			const bool serialPortOpened = serialPort.open(QIODevice::ReadOnly);
			if (serialPortOpened) {
				showErrorMessage(QString());
				serialPort.close();
			} else {
				QString msg = LiveDataSource::serialPortErrorEnumToString(serialPort.error(), serialPort.errorString());
				showErrorMessage(msg);
			}
		} else {
			QString msg = i18n("Serial port number is missing.");
			showErrorMessage(msg);
		}
#endif
		break;
	}
	case LiveDataSource::SourceType::MQTT: {
#ifdef HAVE_MQTT
		const bool enable = m_importFileWidget->isMqttValid();
		if (enable)
			showErrorMessage(QString());
		else {
			QString msg = i18n("Either there is no connection, or no subscriptions were made, or the file filter is not ASCII.");
			showErrorMessage(msg);
		}
#endif
		break;
	}
	}
}

QString ImportFileDialog::selectedObject() const {
	return m_importFileWidget->selectedObject();
}
