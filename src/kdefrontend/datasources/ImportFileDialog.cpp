/***************************************************************************
    File                 : ImportDialog.cc
    Project              : LabPlot
    Description          : import file data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2018 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008-2015 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "ImportFileDialog.h"
#include "ImportFileWidget.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/HDF5Filter.h"
#include "backend/datasources/filters/NetCDFFilter.h"
#include "backend/datasources/filters/ROOTFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include "backend/core/Workbook.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/MainWin.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif

#include <KMessageBox>
#include <KSharedConfig>
#include <KWindowConfig>
#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QProgressBar>
#include <QTcpSocket>
#include <QLocalSocket>
#include <QUdpSocket>
#include <QStatusBar>
#include <QDir>
#include <QInputDialog>
#include <QMenu>

/*!
	\class ImportFileDialog
	\brief Dialog for importing data from a file. Embeds \c ImportFileWidget and provides the standard buttons.

	\ingroup kdefrontend
 */

ImportFileDialog::ImportFileDialog(MainWin* parent, bool liveDataSource, const QString& fileName) : ImportDialog(parent),
	m_importFileWidget(new ImportFileWidget(this, fileName)),
	m_showOptions(false) {

	vLayout->addWidget(m_importFileWidget);

	//dialog buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Reset |QDialogButtonBox::Cancel);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	m_optionsButton = buttonBox->button(QDialogButtonBox::Reset); //we highjack the default "Reset" button and use if for showing/hiding the options
	okButton->setEnabled(false); //ok is only available if a valid container was selected
	vLayout->addWidget(buttonBox);

	//hide the data-source related widgets
	if (!liveDataSource) {
		setModel();
		//TODO: disable for file data sources
		m_importFileWidget->hideDataSource();
	} else
		m_importFileWidget->initializeAndFillPortsAndBaudRates();

	//Signals/Slots
#ifdef HAVE_MQTT
	connect(m_importFileWidget, &ImportFileWidget::subscriptionMade, this, &ImportFileDialog::checkOkButton);
	connect(m_importFileWidget, &ImportFileWidget::checkFileType, this, &ImportFileDialog::checkOkButton);
#endif
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	if (!liveDataSource) {
		setWindowTitle(i18nc("@title:window", "Import Data to Spreadsheet or Matrix"));
		m_importFileWidget->hideDataSource();
	} else
		setWindowTitle(i18nc("@title:window", "Add New Live Data Source"));

	setWindowIcon(QIcon::fromTheme("document-import-database"));

	QTimer::singleShot(0, this, &ImportFileDialog::loadSettings);
}

void ImportFileDialog::loadSettings() {
	//restore saved settings
	QApplication::processEvents(QEventLoop::AllEvents, 0);
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportFileDialog");
	m_showOptions = conf.readEntry("ShowOptions", false);
	m_showOptions ? m_optionsButton->setText(i18n("Hide Options")) : m_optionsButton->setText(i18n("Show Options"));

	m_importFileWidget->showOptions(m_showOptions);
	m_importFileWidget->loadSettings();

	//do the signal-slot connections after all settings where loaded in import file widget and check the OK button after this
	connect(m_importFileWidget, SIGNAL(checkedFitsTableToMatrix(bool)), this, SLOT(checkOnFitsTableToMatrix(bool)));
	connect(m_importFileWidget, SIGNAL(fileNameChanged()), this, SLOT(checkOkButton()));
	connect(m_importFileWidget, SIGNAL(sourceTypeChanged()), this, SLOT(checkOkButton()));
	connect(m_importFileWidget, SIGNAL(hostChanged()), this, SLOT(checkOkButton()));
	connect(m_importFileWidget, SIGNAL(portChanged()), this, SLOT(checkOkButton()));
	connect(m_importFileWidget, SIGNAL(previewRefreshed()), this, SLOT(checkOkButton()));
	connect(m_optionsButton, SIGNAL(clicked()), this, SLOT(toggleOptions()));

	checkOkButton();

	KWindowConfig::restoreWindowSize(windowHandle(), conf);
}

ImportFileDialog::~ImportFileDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportFileDialog");
	conf.writeEntry("ShowOptions", m_showOptions);
	if (cbPosition)
		conf.writeEntry("Position", cbPosition->currentIndex());

	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

int ImportFileDialog::sourceType() const {
	return static_cast<int>(m_importFileWidget->currentSourceType());
}

/*!
  triggers data import to the live data source \c source
*/
void ImportFileDialog::importToLiveDataSource(LiveDataSource* source, QStatusBar* statusBar) const {
	DEBUG("ImportFileDialog::importToLiveDataSource()");
	m_importFileWidget->saveSettings(source);

	//show a progress bar in the status bar
	QProgressBar* progressBar = new QProgressBar();
	progressBar->setRange(0, 100);
    connect(source->filter(), SIGNAL(completed(int)), progressBar, SLOT(setValue(int)));

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);
	WAIT_CURSOR;

	QTime timer;
	timer.start();
	DEBUG("	Inital read()");
	source->read();
	statusBar->showMessage( i18n("Live data source created in %1 seconds.", (float)timer.elapsed()/1000) );

	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
	source->ready();
}

#ifdef HAVE_MQTT
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
	DEBUG("ImportFileDialog::importTo()");
	QDEBUG("	cbAddTo->currentModelIndex() =" << cbAddTo->currentModelIndex());
	AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
	if (!aspect) {
		DEBUG("ERROR in importTo(): No aspect available");
		DEBUG("	cbAddTo->currentModelIndex().isValid() = " << cbAddTo->currentModelIndex().isValid());
		DEBUG("	cbAddTo->currentModelIndex() row/column = " << cbAddTo->currentModelIndex().row() << ' ' << cbAddTo->currentModelIndex().column());
		return;
	}

	if (m_importFileWidget->isFileEmpty()) {
		KMessageBox::information(0, i18n("No data to import."), i18n("No Data"));
		return;
	}

	QString fileName = m_importFileWidget->fileName();
	AbstractFileFilter* filter = m_importFileWidget->currentFileFilter();
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode(cbPosition->currentIndex());

	//show a progress bar in the status bar
	QProgressBar* progressBar = new QProgressBar();
	progressBar->setRange(0, 100);
	connect(filter, SIGNAL(completed(int)), progressBar, SLOT(setValue(int)));

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);

	WAIT_CURSOR;
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	QTime timer;
	timer.start();
	if (aspect->inherits("Matrix")) {
		DEBUG("	to Matrix");
		Matrix* matrix = qobject_cast<Matrix*>(aspect);
		filter->readDataFromFile(fileName, matrix, mode);
	} else if (aspect->inherits("Spreadsheet")) {
		DEBUG("	to Spreadsheet");
		Spreadsheet* spreadsheet = qobject_cast<Spreadsheet*>(aspect);
		DEBUG(" Calling readDataFromFile()");
		filter->readDataFromFile(fileName, spreadsheet, mode);
	} else if (aspect->inherits("Workbook")) {
		DEBUG("	to Workbook");
		Workbook* workbook = qobject_cast<Workbook*>(aspect);
		QVector<AbstractAspect*> sheets = workbook->children<AbstractAspect>();

		AbstractFileFilter::FileType fileType = m_importFileWidget->currentFileType();
		// multiple data sets/variables for HDF5, NetCDF and ROOT
		if (fileType == AbstractFileFilter::HDF5 ||
			fileType == AbstractFileFilter::NETCDF ||
			fileType == AbstractFileFilter::ROOT) {
			QStringList names;
			switch (fileType) {
				case AbstractFileFilter::HDF5:
					names = m_importFileWidget->selectedHDF5Names();
					break;
				case AbstractFileFilter::NETCDF:
					names = m_importFileWidget->selectedNetCDFNames();
					break;
				case AbstractFileFilter::ROOT:
					names = m_importFileWidget->selectedROOTNames();
					break;
				case AbstractFileFilter::Ascii:
				case AbstractFileFilter::Binary:
				case AbstractFileFilter::Image:
				case AbstractFileFilter::FITS:
				case AbstractFileFilter::NgspiceRawAscii:
					break; // never reached, omit warning
			}

			int nrNames = names.size(), offset = sheets.size();

			int start=0;
			if (mode == AbstractFileFilter::Replace)
				start=offset;

			// add additional sheets
			for (int i = start; i < nrNames; ++i) {
				Spreadsheet *spreadsheet = new Spreadsheet(0, i18n("Spreadsheet"));
				if (mode == AbstractFileFilter::Prepend)
					workbook->insertChildBefore(spreadsheet,sheets[0]);
				else
					workbook->addChild(spreadsheet);
			}

			if (mode != AbstractFileFilter::Append)
				offset = 0;

			// import to sheets
			sheets = workbook->children<AbstractAspect>();
			for (int i = 0; i < nrNames; ++i) {
				switch (fileType) {
					case AbstractFileFilter::HDF5:
						((HDF5Filter*) filter)->setCurrentDataSetName(names[i]);
						break;
					case AbstractFileFilter::NETCDF:
						((NetCDFFilter*) filter)->setCurrentVarName(names[i]);
						break;
					case AbstractFileFilter::ROOT:
						((ROOTFilter*) filter)->setCurrentHistogram(names[i]);
						break;
					case AbstractFileFilter::Ascii:
					case AbstractFileFilter::Binary:
					case AbstractFileFilter::Image:
					case AbstractFileFilter::FITS:
					case AbstractFileFilter::NgspiceRawAscii:
						break; // never reached, omit warning
				}

				if (sheets[i+offset]->inherits("Matrix"))
					filter->readDataFromFile(fileName, qobject_cast<Matrix*>(sheets[i+offset]));
				else if (sheets[i+offset]->inherits("Spreadsheet"))
					filter->readDataFromFile(fileName, qobject_cast<Spreadsheet*>(sheets[i+offset]));
			}
		} else { // single import file types
			// use active spreadsheet/matrix if present, else new spreadsheet
			Spreadsheet* spreadsheet = workbook->currentSpreadsheet();
			Matrix* matrix = workbook->currentMatrix();
			if (spreadsheet)
				filter->readDataFromFile(fileName, spreadsheet, mode);
			else if (matrix)
				filter->readDataFromFile(fileName, matrix, mode);
			else {
				spreadsheet = new Spreadsheet(0, i18n("Spreadsheet"));
				workbook->addChild(spreadsheet);
				filter->readDataFromFile(fileName, spreadsheet, mode);
			}
		}
	}
	statusBar->showMessage( i18n("File %1 imported in %2 seconds.", fileName, (float)timer.elapsed()/1000) );

	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
	delete filter;
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

		if(aspect->inherits("Matrix")) {
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
			return;
		} else {
			lPosition->setEnabled(true);
			cbPosition->setEnabled(true);

			//when doing ASCII import to a matrix, hide the options for using the file header (first line)
			//to name the columns since the column names are fixed in a matrix
			const Matrix* matrix = dynamic_cast<const Matrix*>(aspect);
			m_importFileWidget->showAsciiHeaderOptions(matrix == NULL);
		}
	}

	QString fileName = m_importFileWidget->fileName();
#ifndef HAVE_WINDOWS
	if (!fileName.isEmpty() && fileName.at(0) != QDir::separator())
		fileName = QDir::homePath() + QDir::separator() + fileName;
#endif

	DEBUG("Data Source Type: " << ENUM_TO_STRING(LiveDataSource, SourceType, m_importFileWidget->currentSourceType()));
	switch (m_importFileWidget->currentSourceType()) {
	case LiveDataSource::SourceType::FileOrPipe: {
		DEBUG("fileName = " << fileName.toUtf8().constData());
		const bool enable = QFile::exists(fileName);
		okButton->setEnabled(enable);
		if (enable)
			okButton->setToolTip(i18n("Close the dialog and import the data."));
		else
			okButton->setToolTip(i18n("Provide an existing file."));

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
			} else {
				DEBUG("failed connect to local socket - " << lsocket.errorString().toStdString());
				okButton->setEnabled(false);
				okButton->setToolTip(i18n("Could not connect to the provided local socket."));
			}
		} else {
			okButton->setEnabled(false);
			okButton->setToolTip(i18n("Selected local socket does not exist."));
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
				socket.disconnectFromHost();
			} else {
				DEBUG("failed to connect to TCP socket - " << socket.errorString().toStdString());
				okButton->setEnabled(false);
				okButton->setToolTip(i18n("Could not connect to the provided TCP socket."));
			}
		} else {
			okButton->setEnabled(false);
			okButton->setToolTip(i18n("Either the host name or the port number is missing."));
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
				socket.disconnectFromHost();
				// read-only socket is disconnected immediately (no waitForDisconnected())
			} else {
				DEBUG("failed to connect to UDP socket - " << socket.errorString().toStdString());
				okButton->setEnabled(false);
				okButton->setToolTip(i18n("Could not connect to the provided UDP socket."));
			}
		} else {
			okButton->setEnabled(false);
			okButton->setToolTip(i18n("Either the host name or the port number is missing."));
		}

		break;
	}
	case LiveDataSource::SourceType::SerialPort: {
		const bool enable = !m_importFileWidget->serialPort().isEmpty();
		if (enable) {
			QSerialPort* serialPort = new QSerialPort(this);

			serialPort->setBaudRate(m_importFileWidget->baudRate());
			serialPort->setPortName(m_importFileWidget->serialPort());

			bool serialPortOpened = serialPort->open(QIODevice::ReadOnly);
			okButton->setEnabled(serialPortOpened);
			if (serialPortOpened)
				okButton->setToolTip(i18n("Close the dialog and import the data."));
			else
				okButton->setToolTip(i18n("Could not connect to the provided serial port."));
		} else {
			okButton->setEnabled(false);
			okButton->setToolTip(i18n("Serial port number is missing."));
		}
    }
	case LiveDataSource::SourceType::MQTT: {
#ifdef HAVE_MQTT
		const bool enable = m_importFileWidget->isMqttValid();
		if (enable) {
			okButton->setEnabled(true);
			okButton->setToolTip(i18n("Close the dialog and import the data."));
		}
		else {
			okButton->setEnabled(false);
			okButton->setToolTip(i18n("Either there is no connection, or no subscriptions were made, or the file filter isn't Ascii."));
		}
#endif
		break;
	}
	}
}

QString ImportFileDialog::selectedObject() const {
	return m_importFileWidget->selectedObject();
}
