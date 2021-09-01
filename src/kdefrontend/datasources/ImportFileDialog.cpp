/*
    File                 : ImportDialog.cc
    Project              : LabPlot
    Description          : import file data dialog
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008-2019 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2008-2015 Stefan Gerlach (stefan.gerlach@uni.kn)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "ImportFileDialog.h"
#include "ImportFileWidget.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/filters.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include "backend/core/Workbook.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/MainWin.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif

#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageWidget>
#include <KSharedConfig>
#include <KWindowConfig>

#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QProgressBar>
#include <QTcpSocket>
#include <QLocalSocket>
#include <QUdpSocket>
#include <QStatusBar>
#include <QDir>
#include <QInputDialog>
#include <QMenu>
#include <QWindow>

/*!
	\class ImportFileDialog
	\brief Dialog for importing data from a file. Embeds \c ImportFileWidget and provides the standard buttons.

	\ingroup kdefrontend
 */

ImportFileDialog::ImportFileDialog(MainWin* parent, bool liveDataSource, const QString& fileName) : ImportDialog(parent),
	m_importFileWidget(new ImportFileWidget(this, liveDataSource, fileName)) {

	vLayout->addWidget(m_importFileWidget);

	//dialog buttons
	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Reset |QDialogButtonBox::Cancel);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	m_optionsButton = buttonBox->button(QDialogButtonBox::Reset); //we highjack the default "Reset" button and use if for showing/hiding the options
	okButton->setEnabled(false); //ok is only available if a valid container was selected
	vLayout->addWidget(buttonBox);

	//hide the data-source related widgets
	if (!liveDataSource)
		setModel();

	//Signals/Slots
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	if (!liveDataSource)
		setWindowTitle(i18nc("@title:window", "Import Data to Spreadsheet or Matrix"));
	else
		setWindowTitle(i18nc("@title:window", "Add New Live Data Source"));

	setWindowIcon(QIcon::fromTheme("document-import-database"));

	//restore saved settings if available
	create(); // ensure there's a window created

	QApplication::processEvents(QEventLoop::AllEvents, 0);
	m_importFileWidget->loadSettings();

	KConfigGroup conf(KSharedConfig::openConfig(), "ImportFileDialog");
	if (conf.exists()) {
		m_showOptions = conf.readEntry("ShowOptions", false);

		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));

	m_importFileWidget->showOptions(m_showOptions);
	//do the signal-slot connections after all settings were loaded in import file widget and check the OK button after this
	connect(m_importFileWidget, &ImportFileWidget::checkedFitsTableToMatrix, this, &ImportFileDialog::checkOnFitsTableToMatrix);
	connect(m_importFileWidget, static_cast<void (ImportFileWidget::*)()>(&ImportFileWidget::fileNameChanged), this, &ImportFileDialog::checkOkButton);
	connect(m_importFileWidget, static_cast<void (ImportFileWidget::*)()>(&ImportFileWidget::sourceTypeChanged), this, &ImportFileDialog::checkOkButton);
	connect(m_importFileWidget, &ImportFileWidget::hostChanged, this, &ImportFileDialog::checkOkButton);
	connect(m_importFileWidget, &ImportFileWidget::portChanged, this, &ImportFileDialog::checkOkButton);
	connect(m_importFileWidget, &ImportFileWidget::error, this, &ImportFileDialog::showErrorMessage);
#ifdef HAVE_MQTT
	connect(m_importFileWidget, &ImportFileWidget::subscriptionsChanged, this, &ImportFileDialog::checkOkButton);
	connect(m_importFileWidget, &ImportFileWidget::checkFileType, this, &ImportFileDialog::checkOkButton);
#endif

	m_showOptions ? m_optionsButton->setText(i18n("Hide Options")) : m_optionsButton->setText(i18n("Show Options"));
	connect(m_optionsButton, &QPushButton::clicked, this, &ImportFileDialog::toggleOptions);

	ImportFileDialog::checkOkButton();
}

ImportFileDialog::~ImportFileDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportFileDialog");
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

	//show a progress bar in the status bar
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
	statusBar->showMessage( i18n("Live data source created in %1 seconds.", (float)timer.elapsed()/1000) );

	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
}

#ifdef HAVE_MQTT
/*!
  triggers data import to the MQTTClient \c client
*/
void ImportFileDialog::importToMQTT(MQTTClient* client) const{
	m_importFileWidget->saveMQTTSettings(client);
	client->read();
	client->ready();
}
#endif

/*!
  triggers data import to the currently selected data container
*/
void ImportFileDialog::importTo(QStatusBar* statusBar) const {
	DEBUG(Q_FUNC_INFO);
	QDEBUG("	cbAddTo->currentModelIndex() =" << cbAddTo->currentModelIndex());
	AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
	if (!aspect) {
		DEBUG("ERROR in importTo(): No aspect available");
		DEBUG("	cbAddTo->currentModelIndex().isValid() = " << cbAddTo->currentModelIndex().isValid());
		DEBUG("	cbAddTo->currentModelIndex() row/column = " << cbAddTo->currentModelIndex().row() << ' ' << cbAddTo->currentModelIndex().column());
		return;
	}

	if (m_importFileWidget->isFileEmpty()) {
		KMessageBox::information(nullptr, i18n("No data to import."), i18n("No Data"));
		return;
	}

	QString fileName = m_importFileWidget->fileName();
	auto filter = m_importFileWidget->currentFileFilter();
	auto mode = AbstractFileFilter::ImportMode(cbPosition->currentIndex());

	//show a progress bar in the status bar
	auto* progressBar = new QProgressBar();
	progressBar->setRange(0, 100);
	connect(filter, &AbstractFileFilter::completed, progressBar, &QProgressBar::setValue);

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);

	WAIT_CURSOR;
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	QElapsedTimer timer;
	timer.start();

	if (aspect->inherits(AspectType::Matrix)) {
		DEBUG(Q_FUNC_INFO <<", to Matrix");
		auto* matrix = qobject_cast<Matrix*>(aspect);
		filter->readDataFromFile(fileName, matrix, mode);
	} else if (aspect->inherits(AspectType::Spreadsheet)) {
		DEBUG(Q_FUNC_INFO << ", to Spreadsheet");
		auto* spreadsheet = qobject_cast<Spreadsheet*>(aspect);
		DEBUG(" Calling filter->readDataFromFile() with spreadsheet " << spreadsheet);
		filter->readDataFromFile(fileName, spreadsheet, mode);
	} else if (aspect->inherits(AspectType::Workbook)) {
		DEBUG(Q_FUNC_INFO << ", to Workbook");
		auto* workbook = static_cast<Workbook*>(aspect);
		workbook->setUndoAware(false);
		auto sheets = workbook->children<AbstractAspect>();

		AbstractFileFilter::FileType fileType = m_importFileWidget->currentFileType();
		// multiple data sets/variables for HDF5, NetCDF and ROOT
		// TODO: Matio
		if (fileType == AbstractFileFilter::FileType::HDF5 || fileType == AbstractFileFilter::FileType::NETCDF ||
			fileType == AbstractFileFilter::FileType::ROOT) {
			QStringList names;
			if (fileType == AbstractFileFilter::FileType::HDF5)
				names = m_importFileWidget->selectedHDF5Names();
			else if (fileType == AbstractFileFilter::FileType::NETCDF)
				names = m_importFileWidget->selectedNetCDFNames();
			else
				names = m_importFileWidget->selectedROOTNames();

			int nrNames = names.size(), offset = sheets.size();

			//TODO: think about importing multiple sets into one sheet

			int start = 0;	// add nrNames sheets (0 to nrNames)

			//in replace mode add only missing sheets (from offset to nrNames)
			//and rename the already available sheets
			if (mode == AbstractFileFilter::ImportMode::Replace) {
				start = offset;

				// if there are more available spreadsheets, than needed,
				// delete the unneeded spreadsheets
				if (offset > nrNames) {
					for (int i = nrNames; i < offset; i++)
						sheets[i]->remove();
					offset = nrNames;
				}

				//rename the available sheets
				for (int i = 0; i < offset; ++i) {
					//HDF5 variable names contain the whole path, remove it and keep the name only
					QString sheetName = names.at(i);
					if (fileType == AbstractFileFilter::FileType::HDF5)
						sheetName = names[i].mid(names[i].lastIndexOf("/") + 1);

					auto* sheet = sheets.at(i);
					sheet->setUndoAware(false);
					sheet->setName(sheetName);
					sheet->setUndoAware(true);
				}
			}

			// add additional spreadsheets
			for (int i = start; i < nrNames; ++i) {
				//HDF5 variable names contain the whole path, remove it and keep the name only
				QString sheetName = names.at(i);
				if (fileType == AbstractFileFilter::FileType::HDF5)
					sheetName = names[i].mid(names[i].lastIndexOf("/") + 1);

				auto* spreadsheet = new Spreadsheet(sheetName);
				if (mode == AbstractFileFilter::ImportMode::Prepend && !sheets.isEmpty())
					workbook->insertChildBefore(spreadsheet, sheets[0]);
				else
					workbook->addChildFast(spreadsheet);
			}

			// start at offset for append, else at 0
			if (mode != AbstractFileFilter::ImportMode::Append)
				offset = 0;

			// import all sets to a different sheet
			sheets = workbook->children<AbstractAspect>();
			for (int i = 0; i < nrNames; ++i) {
				if (fileType == AbstractFileFilter::FileType::HDF5)
					static_cast<HDF5Filter*>(filter)->setCurrentDataSetName(names[i]);
				else if (fileType == AbstractFileFilter::FileType::NETCDF)
					static_cast<NetCDFFilter*>(filter)->setCurrentVarName(names[i]);
				else if (fileType == AbstractFileFilter::FileType::MATIO)
					static_cast<MatioFilter*>(filter)->setCurrentVarName(names[i]);
				else
					static_cast<ROOTFilter*>(filter)->setCurrentObject(names[i]);

				int index = i + offset;
				filter->readDataFromFile(fileName, qobject_cast<Spreadsheet*>(sheets[index]));
			}

			workbook->setUndoAware(true);
		} else { // single import file types
			// use active spreadsheet/matrix if present, else new spreadsheet
			auto* sheet = workbook->currentSpreadsheet();
			if (sheet)
				filter->readDataFromFile(fileName, sheet, mode);
			else {
				workbook->setUndoAware(true);
				auto* spreadsheet = new Spreadsheet(fileName);
				workbook->addChild(spreadsheet);
				workbook->setUndoAware(false);
				filter->readDataFromFile(fileName, spreadsheet, mode);
			}
		}
	}
	statusBar->showMessage(i18n("File %1 imported in %2 seconds.", fileName, (float)timer.elapsed()/1000));

	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
}

void ImportFileDialog::toggleOptions() {
	m_importFileWidget->showOptions(!m_showOptions);
	m_showOptions = !m_showOptions;
	m_showOptions ? m_optionsButton->setText(i18n("Hide Options")) : m_optionsButton->setText(i18n("Show Options"));

	//resize the dialog
	layout()->activate();
	resize( QSize(this->width(), 0).expandedTo(minimumSize()) );
}

void ImportFileDialog::checkOnFitsTableToMatrix(const bool enable) {
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
	DEBUG("ImportFileDialog::checkOkButton()");
	if (cbAddTo) { //only check for the target container when no file data source is being added
		QDEBUG(" cbAddTo->currentModelIndex() = " << cbAddTo->currentModelIndex());
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
		if (!aspect) {
			okButton->setEnabled(false);
			okButton->setToolTip(i18n("Select a data container where the data has to be imported into."));
			lPosition->setEnabled(false);
			cbPosition->setEnabled(false);
			cbAddTo->setFocus(); //set the focus to make the user aware about the fact that a data container needs to be provided
			return;
		} else {
			lPosition->setEnabled(true);
			cbPosition->setEnabled(true);

			//when doing ASCII import to a matrix, hide the options for using the file header (first line)
			//to name the columns since the column names are fixed in a matrix
			const auto* matrix = dynamic_cast<const Matrix*>(aspect);
			m_importFileWidget->showAsciiHeaderOptions(matrix == nullptr);
		}
	}

	QString fileName = m_importFileWidget->fileName();
#ifdef HAVE_WINDOWS
	if (!fileName.isEmpty() && fileName.at(1) != QLatin1String(":"))
#else
	if (!fileName.isEmpty() && fileName.at(0) != QLatin1String("/"))
#endif
		fileName = QDir::homePath() + QLatin1String("/") + fileName;

	DEBUG("Data Source Type: " << ENUM_TO_STRING(LiveDataSource, SourceType, m_importFileWidget->currentSourceType()));
	switch (m_importFileWidget->currentSourceType()) {
	case LiveDataSource::SourceType::FileOrPipe: {
		DEBUG("	fileName = " << fileName.toUtf8().constData());
		const bool enable = QFile::exists(fileName);
		okButton->setEnabled(enable);
		if (enable) {
			okButton->setToolTip(i18n("Close the dialog and import the data."));
			showErrorMessage(QString());
		} else {
			QString msg = i18n("The provided file doesn't exist.");
			okButton->setToolTip(msg);

			//suppress the error widget when the dialog is opened the first time.
			//show only the error widget if the file was really a non-existing file was provided.
			if (!fileName.isEmpty())
				showErrorMessage(msg);
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
				okButton->setEnabled(true);
				okButton->setToolTip(i18n("Close the dialog and import the data."));
				showErrorMessage(QString());
			} else {
				okButton->setEnabled(false);
				QString msg = i18n("Could not connect to the provided local socket. Error: %1.", lsocket.errorString());
				okButton->setToolTip(msg);
				showErrorMessage(msg);
			}
		} else {
			okButton->setEnabled(false);
			QString msg = i18n("Could not connect to the provided local socket. The socket does not exist.");
			okButton->setToolTip(msg);
			if (!fileName.isEmpty())
				showErrorMessage(msg);
		}

		break;
	}
	case LiveDataSource::SourceType::NetworkTcpSocket: {
		const bool enable = !m_importFileWidget->host().isEmpty() && !m_importFileWidget->port().isEmpty();
		if (enable) {
			QTcpSocket socket(this);
			socket.connectToHost(m_importFileWidget->host(), m_importFileWidget->port().toUShort(), QTcpSocket::ReadOnly);
			if (socket.waitForConnected()) {
				okButton->setEnabled(true);
				okButton->setToolTip(i18n("Close the dialog and import the data."));
				showErrorMessage(QString());
				socket.disconnectFromHost();
			} else {
				okButton->setEnabled(false);
				QString msg = i18n("Could not connect to the provided TCP socket. Error: %1.", socket.errorString());
				okButton->setToolTip(msg);
				showErrorMessage(msg);
			}
		} else {
			okButton->setEnabled(false);
			QString msg = i18n("Either the host name or the port number is missing.");
			okButton->setToolTip(msg);
			showErrorMessage(msg);
		}
		break;
	}
	case LiveDataSource::SourceType::NetworkUdpSocket: {
		const bool enable = !m_importFileWidget->host().isEmpty() && !m_importFileWidget->port().isEmpty();
		if (enable) {
			QUdpSocket socket(this);
			socket.bind(QHostAddress(m_importFileWidget->host()), m_importFileWidget->port().toUShort());
			socket.connectToHost(m_importFileWidget->host(), 0, QUdpSocket::ReadOnly);
			if (socket.waitForConnected()) {
				okButton->setEnabled(true);
				okButton->setToolTip(i18n("Close the dialog and import the data."));
				showErrorMessage(QString());
				socket.disconnectFromHost();
				// read-only socket is disconnected immediately (no waitForDisconnected())
			} else {
				okButton->setEnabled(false);
				QString msg = i18n("Could not connect to the provided UDP socket. Error: %1.", socket.errorString());
				okButton->setToolTip(msg);
				showErrorMessage(msg);
			}
		} else {
			okButton->setEnabled(false);
			okButton->setToolTip(i18n("Either the host name or the port number is missing."));
		}

		break;
	}
	case LiveDataSource::SourceType::SerialPort: {
#ifdef HAVE_QTSERIALPORT
		const QString sPort = m_importFileWidget->serialPort();
		const int baudRate = m_importFileWidget->baudRate();

		if (!sPort.isEmpty()) {
			QSerialPort serialPort{this};

			DEBUG("	Port: " << STDSTRING(sPort) << ", Settings: " << baudRate << ',' << serialPort.dataBits()
					<< ',' << serialPort.parity() << ',' << serialPort.stopBits());
			serialPort.setPortName(sPort);
			serialPort.setBaudRate(baudRate);

			const bool serialPortOpened = serialPort.open(QIODevice::ReadOnly);
			okButton->setEnabled(serialPortOpened);
			if (serialPortOpened) {
				okButton->setToolTip(i18n("Close the dialog and import the data."));
				showErrorMessage(QString());
				serialPort.close();
			} else {
				QString msg = i18n("Could not connect to the provided serial port.");
				okButton->setToolTip(msg);
				showErrorMessage(msg);
			}
		} else {
			okButton->setEnabled(false);
			QString msg = i18n("Serial port number is missing.");
			okButton->setToolTip(msg);
			showErrorMessage(msg);
		}
#endif
		break;
	}
	case LiveDataSource::SourceType::MQTT: {
#ifdef HAVE_MQTT
		const bool enable = m_importFileWidget->isMqttValid();
		showErrorMessage(QString());
		if (enable) {
			okButton->setEnabled(true);
			okButton->setToolTip(i18n("Close the dialog and import the data."));
		}
		else {
			okButton->setEnabled(false);
			okButton->setToolTip(i18n("Either there is no connection, or no subscriptions were made, or the file filter is not ASCII."));
		}
#endif
		break;
	}
	}
}

QString ImportFileDialog::selectedObject() const {
	return m_importFileWidget->selectedObject();
}

void ImportFileDialog::showErrorMessage(const QString& message) {
	if (message.isEmpty()) {
		if (m_messageWidget && m_messageWidget->isVisible())
			m_messageWidget->close();
	} else {
		if (!m_messageWidget) {
			m_messageWidget = new KMessageWidget(this);
			m_messageWidget->setMessageType(KMessageWidget::Error);
			vLayout->insertWidget(vLayout->count() - 1, m_messageWidget);
		}
		m_messageWidget->setText(message);
        m_messageWidget->animatedShow();
	}
}
