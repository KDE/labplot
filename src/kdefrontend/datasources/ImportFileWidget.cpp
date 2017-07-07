/***************************************************************************
File                 : ImportFileWidget.cpp
Project              : LabPlot
Description          : import file data widget
--------------------------------------------------------------------
Copyright            : (C) 2009-2017 Stefan Gerlach (stefan.gerlach@uni.kn)
Copyright            : (C) 2009-2017 Alexander Semke (alexander.semke@web.de)

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
#include "backend/datasources/filters/HDFFilter.h"
#include "backend/datasources/filters/NetCDFFilter.h"
#include "backend/datasources/filters/ImageFilter.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "AsciiOptionsWidget.h"
#include "BinaryOptionsWidget.h"
#include "HDFOptionsWidget.h"
#include "ImageOptionsWidget.h"
#include "NetCDFOptionsWidget.h"
#include "FITSOptionsWidget.h"

#include <QTableWidget>
#include <QInputDialog>
#include <QDir>
#include <QFileDialog>
#include <QProcess>
#include <KUrlCompletion>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QTimer>
#include <QStandardItemModel>
#include <QImageReader>
#include <KUrlCompletion>

/*!
   \class ImportFileWidget
   \brief Widget for importing data from a file.

   \ingroup kdefrontend
*/
ImportFileWidget::ImportFileWidget(QWidget* parent, const QString& fileName) : QWidget(parent), m_fileName(fileName),
    m_fileDataSource(true) {
	ui.setupUi(this);

	KUrlCompletion *comp = new KUrlCompletion();
	ui.kleFileName->setCompletionObject(comp);

	ui.cbFileType->addItems(FileDataSource::fileTypes());
	QStringList filterItems;
	filterItems << i18n("Automatic") << i18n("Custom");
	ui.cbFilter->addItems(filterItems);

	// file type specific option widgets
	QWidget* asciiw = new QWidget();
	m_asciiOptionsWidget = std::unique_ptr<AsciiOptionsWidget>(new AsciiOptionsWidget(asciiw));
	ui.swOptions->insertWidget(FileDataSource::Ascii, asciiw);

	QWidget* binaryw = new QWidget();
	m_binaryOptionsWidget = std::unique_ptr<BinaryOptionsWidget>(new BinaryOptionsWidget(binaryw));
	ui.swOptions->insertWidget(FileDataSource::Binary, binaryw);

	QWidget* imagew = new QWidget();
	m_imageOptionsWidget = std::unique_ptr<ImageOptionsWidget>(new ImageOptionsWidget(imagew));
	ui.swOptions->insertWidget(FileDataSource::Image, imagew);

	QWidget* hdfw = new QWidget();
	m_hdfOptionsWidget = std::unique_ptr<HDFOptionsWidget>(new HDFOptionsWidget(hdfw, this));
	ui.swOptions->insertWidget(FileDataSource::HDF, hdfw);

	QWidget* netcdfw = new QWidget(0);
	m_netcdfOptionsWidget.setupUi(netcdfw);
	QStringList headers;
	headers << i18n("Name") << i18n("Type") << i18n("Properties") << i18n("Values");
	m_netcdfOptionsWidget.twContent->setHeaderLabels(headers);
	// type column is hidden
	m_netcdfOptionsWidget.twContent->hideColumn(1);
	m_netcdfOptionsWidget.twContent->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_netcdfOptionsWidget.twContent->setAlternatingRowColors(true);
	m_netcdfOptionsWidget.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.swOptions->insertWidget(FileDataSource::NETCDF, netcdfw);

	QWidget* fitsw = new QWidget(0);
	m_fitsOptionsWidget.setupUi(fitsw);
	m_fitsOptionsWidget.twExtensions->headerItem()->setText(0, i18n("Content"));
	m_fitsOptionsWidget.twExtensions->setSelectionMode(QAbstractItemView::SingleSelection);
	m_fitsOptionsWidget.twExtensions->setAlternatingRowColors(true);
	m_fitsOptionsWidget.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.swOptions->insertWidget(FileDataSource::FITS, fitsw);

	// the table widget for preview
	m_twPreview = new QTableWidget(ui.tePreview);
	m_twPreview->verticalHeader()->hide();
	m_twPreview->setEditTriggers(QTableWidget::NoEditTriggers);
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(m_twPreview);
	ui.tePreview->setLayout(layout);
	m_twPreview->hide();

	// default filter
	ui.swOptions->setCurrentIndex(FileDataSource::Ascii);
#if !defined(HAVE_HDF5) || !defined(HAVE_NETCDF) || !defined(HAVE_FITS)
	const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(ui.cbFileType->model());
#endif
#ifndef HAVE_HDF5
	// disable HDF5 item
	QStandardItem* item = model->item(FileDataSource::HDF);
	item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
#endif
#ifndef HAVE_NETCDF
	// disable NETCDF item
	QStandardItem* item2 = model->item(FileDataSource::NETCDF);
	item2->setFlags(item2->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
#endif
#ifndef HAVE_FITS
	// disable FITS item
	QStandardItem* item3 = model->item(FileDataSource::FITS);
	item3->setFlags(item3->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
#endif

	ui.gbOptions->hide();
    ui.gbUpdateOptions->hide();

	ui.bOpen->setIcon( QIcon::fromTheme("document-open") );
	ui.bFileInfo->setIcon( QIcon::fromTheme("help-about") );
	ui.bManageFilters->setIcon( QIcon::fromTheme("configure") );
	ui.bSaveFilter->setIcon( QIcon::fromTheme("document-save") );
	ui.bRefreshPreview->setIcon( QIcon::fromTheme("view-refresh") );
	m_netcdfOptionsWidget.bRefreshPreview->setIcon( QIcon::fromTheme("view-refresh") );

	connect( ui.kleFileName, SIGNAL(textChanged(QString)), SLOT(fileNameChanged(QString)) );
	connect( ui.bOpen, SIGNAL(clicked()), this, SLOT (selectFile()) );
	connect( ui.bFileInfo, SIGNAL(clicked()), this, SLOT (fileInfoDialog()) );
	connect( ui.bSaveFilter, SIGNAL(clicked()), this, SLOT (saveFilter()) );
	connect( ui.bManageFilters, SIGNAL(clicked()), this, SLOT (manageFilters()) );
	connect( ui.cbFileType, SIGNAL(currentIndexChanged(int)), SLOT(fileTypeChanged(int)) );
	connect( ui.cbFilter, SIGNAL(activated(int)), SLOT(filterChanged(int)) );
	connect( ui.bRefreshPreview, SIGNAL(clicked()), SLOT(refreshPreview()) );

	connect( m_netcdfOptionsWidget.twContent, SIGNAL(itemSelectionChanged()), SLOT(netcdfTreeWidgetSelectionChanged()) );
	connect( m_netcdfOptionsWidget.bRefreshPreview, SIGNAL(clicked()), SLOT(refreshPreview()) );
	connect( m_fitsOptionsWidget.twExtensions, SIGNAL(itemSelectionChanged()), SLOT(fitsTreeWidgetSelectionChanged()));
	connect( m_fitsOptionsWidget.bRefreshPreview, SIGNAL(clicked()), SLOT(refreshPreview()) );

    connect( ui.cbSourceType, SIGNAL(currentIndexChanged(int)), this, SLOT(sourceTypeChanged(int)));

	//TODO: implement save/load of user-defined settings later and activate these buttons again
	ui.bSaveFilter->hide();
	ui.bManageFilters->hide();

	//defer the loading of settings a bit in order to show the dialog prior to blocking the GUI in refreshPreview()
	QTimer::singleShot( 100, this, SLOT(loadSettings()) );
}

void ImportFileWidget::loadSettings() {
	//load last used settings
	KConfigGroup conf(KSharedConfig::openConfig(), "Import");

	//settings for data type specific widgets
	m_asciiOptionsWidget->loadSettings();
	m_binaryOptionsWidget->loadSettings();
	m_imageOptionsWidget->loadSettings();

	//general settings
	ui.cbFileType->setCurrentIndex(conf.readEntry("Type", 0));
	ui.cbFilter->setCurrentIndex(conf.readEntry("Filter", 0));
	filterChanged(ui.cbFilter->currentIndex());	// needed if filter is not changed
	if (m_fileName.isEmpty())
		ui.kleFileName->setText(conf.readEntry("LastImportedFile", ""));
	else
		ui.kleFileName->setText(m_fileName);
}

ImportFileWidget::~ImportFileWidget() {
	// save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "Import");

	// general settings
	conf.writeEntry("LastImportedFile", ui.kleFileName->text());
	conf.writeEntry("Type", ui.cbFileType->currentIndex());
	conf.writeEntry("Filter", ui.cbFilter->currentIndex());

	// data type specific settings
	m_asciiOptionsWidget->saveSettings();
	m_binaryOptionsWidget->saveSettings();
	m_imageOptionsWidget->saveSettings();
}

void ImportFileWidget::hideDataSource() {

    m_fileDataSource = false;
    ui.gbUpdateOptions->hide();

    ui.chbWatchFile->hide();
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

    ui.cbUpdateOn->hide();
    ui.lUpdateOn->hide();

    ui.sbUpdateFrequency->hide();
    ui.lUpdateFrequency->hide();
}

void ImportFileWidget::showAsciiHeaderOptions(bool b) {
	m_asciiOptionsWidget->showAsciiHeaderOptions(b);
}

void ImportFileWidget::showOptions(bool b) {
	ui.gbOptions->setVisible(b);

    if (m_fileDataSource)
        ui.gbUpdateOptions->setVisible(b);

	resize(layout()->minimumSize());
}

QString ImportFileWidget::fileName() const {
	if (currentFileType() == FileDataSource::FITS) {
		if (m_fitsOptionsWidget.twExtensions->currentItem() != 0) {
			if (m_fitsOptionsWidget.twExtensions->currentItem()->text(0) != i18n("Primary header")) {
				return ui.kleFileName->text() + QLatin1String("[") +
					m_fitsOptionsWidget.twExtensions->currentItem()->text(m_fitsOptionsWidget.twExtensions->currentColumn()) + QLatin1String("]");
			}
		}

	}
	return ui.kleFileName->text();
}

/*!
	saves the settings to the data source \c source.
*/
void ImportFileWidget::saveSettings(FileDataSource* source) const {
	//save the data source information
    FileDataSource::FileType fileType = static_cast<FileDataSource::FileType>(ui.cbFileType->currentIndex());
    FileDataSource::UpdateType updateType = static_cast<FileDataSource::UpdateType>(ui.cbUpdateOn->currentIndex());
    FileDataSource::SourceType sourceType = static_cast<FileDataSource::SourceType>(ui.cbSourceType->currentIndex());

	source->setComment( ui.kleFileName->text() );
    source->setFileType(fileType);
    source->setFilter(this->currentFileFilter());

    source->setUpdateType(updateType);
    source->setUpdateFrequency(ui.sbUpdateFrequency->value());
    source->setSourceType(sourceType);
    source->setSampleRate(ui.sbKeepValues->value());

    if ((sourceType == FileDataSource::SourceType::FileOrPipe) || (sourceType == FileDataSource::SourceType::LocalSocket)) {
        source->setFileName( ui.kleFileName->text() );
        source->setFileWatched( ui.chbWatchFile->isChecked() );
        source->setFileLinked( ui.chbLinkFile->isChecked() );
    } else if (sourceType == FileDataSource::SourceType::NetworkSocket) {
        source->setHost(ui.leHost->text());
        source->setPort(ui.lePort->text().toInt());
    } else if (sourceType == FileDataSource::SourceType::SerialPort) {
        source->setBaudRate(ui.cbBaudRate->currentText().toInt());
        source->setSerialPort(ui.cbSerialPort->currentText());
    }
}

/*!
	returns the currently used file type.
*/
FileDataSource::FileType ImportFileWidget::currentFileType() const {
    return static_cast<FileDataSource::FileType>(ui.cbFileType->currentIndex());
}

/*!
	returns the currently used filter.
*/
AbstractFileFilter* ImportFileWidget::currentFileFilter() const {
	DEBUG("currentFileFilter()");
    FileDataSource::FileType fileType = static_cast<FileDataSource::FileType>(ui.cbFileType->currentIndex());

	switch (fileType) {
	case FileDataSource::Ascii: {
//TODO			std::unique_ptr<AsciiFilter> filter(new AsciiFilter());
			AsciiFilter* filter = new AsciiFilter();

			if (ui.cbFilter->currentIndex() == 0) {   //"automatic"
				filter->setAutoModeEnabled(true);
			} else if (ui.cbFilter->currentIndex() == 1) { //"custom"
				filter->setAutoModeEnabled(false);
				m_asciiOptionsWidget->applyFilterSettings(filter);
			} else {
				filter->loadFilterSettings( ui.cbFilter->currentText() );
			}

			//save the data portion to import
			filter->setStartRow( ui.sbStartRow->value());
			filter->setEndRow( ui.sbEndRow->value() );
			filter->setStartColumn( ui.sbStartColumn->value());
			filter->setEndColumn( ui.sbEndColumn->value());

			return filter;
		}
	case FileDataSource::Binary: {
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
	case FileDataSource::Image: {
			ImageFilter* filter = new ImageFilter();

			filter->setImportFormat(m_imageOptionsWidget->currentFormat());
			filter->setStartRow( ui.sbStartRow->value() );
			filter->setEndRow( ui.sbEndRow->value() );
			filter->setStartColumn( ui.sbStartColumn->value() );
			filter->setEndColumn( ui.sbEndColumn->value() );

			return filter;
		}
	case FileDataSource::HDF: {
			HDFFilter* filter = new HDFFilter();
			QStringList names = selectedHDFNames();
			if (!names.isEmpty())
				filter->setCurrentDataSetName(names[0]);
			filter->setStartRow( ui.sbStartRow->value() );
			filter->setEndRow( ui.sbEndRow->value() );
			filter->setStartColumn( ui.sbStartColumn->value() );
			filter->setEndColumn( ui.sbEndColumn->value() );

			return filter;
		}
	case FileDataSource::NETCDF: {
			NetCDFFilter* filter = new NetCDFFilter();

			if (!selectedNetCDFNames().isEmpty())
				filter->setCurrentVarName(selectedNetCDFNames()[0]);
			filter->setStartRow( ui.sbStartRow->value() );
			filter->setEndRow( ui.sbEndRow->value() );
			filter->setStartColumn( ui.sbStartColumn->value() );
			filter->setEndColumn( ui.sbEndColumn->value() );

			return filter;
		}
	case FileDataSource::FITS: {
			FITSFilter* filter = new FITSFilter();
			filter->setStartRow( ui.sbStartRow->value());
			filter->setEndRow( ui.sbEndRow->value() );
			filter->setStartColumn( ui.sbStartColumn->value());
			filter->setEndColumn( ui.sbEndColumn->value());
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
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastDir", newDir);
	}

	ui.kleFileName->setText(path);

	//TODO: decide whether the selection of several files should be possible
// 	QStringList filelist = QFileDialog::getOpenFileNames(this,i18n("Select one or more files to open"));
// 	if (! filelist.isEmpty() )
// 		ui.kleFileName->setText(filelist.join(";"));
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
	if ( !fileName.isEmpty() && fileName.left(1) != QDir::separator())
		fileName = QDir::homePath() + QDir::separator() + fileName;
#endif

	bool fileExists = QFile::exists(fileName);
	if (fileExists)
		ui.kleFileName->setStyleSheet("");
	else
		ui.kleFileName->setStyleSheet("QLineEdit{background:red;}");

	ui.gbOptions->setEnabled(fileExists);
	ui.bFileInfo->setEnabled(fileExists);
	ui.cbFileType->setEnabled(fileExists);
	ui.cbFilter->setEnabled(fileExists);
	ui.bManageFilters->setEnabled(fileExists);
	ui.chbWatchFile->setEnabled(fileExists);
	ui.chbLinkFile->setEnabled(fileExists);
	if (!fileExists) {
		//file doesn't exist -> delete the content preview that is still potentially
		//available from the previously selected file
		ui.tePreview->clear();
		m_twPreview->clear();
		m_hdfOptionsWidget->clear();
		m_netcdfOptionsWidget.twContent->clear();
		m_netcdfOptionsWidget.twPreview->clear();
		m_fitsOptionsWidget.twExtensions->clear();
		m_fitsOptionsWidget.twPreview->clear();

		emit fileNameChanged();
		return;
	}

	QString fileInfo;
#ifndef HAVE_WINDOWS
	//check, if we can guess the file type by content
	QProcess *proc = new QProcess(this);
	QStringList args;
	args << "-b" << ui.kleFileName->text();
	proc->start("file", args);
	if (proc->waitForReadyRead(1000) == false) {
		QDEBUG("ERROR: reading file type of file" << fileName);
		return;
	}
	fileInfo = proc->readLine();
#endif

	QByteArray imageFormat = QImageReader::imageFormat(fileName);
	if (fileInfo.contains(QLatin1String("compressed data")) || fileInfo.contains(QLatin1String("ASCII")) ||
	        fileName.endsWith(QLatin1String("dat"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String("txt"), Qt::CaseInsensitive)) {
		//probably ascii data
		ui.cbFileType->setCurrentIndex(FileDataSource::Ascii);
	} else if (fileInfo.contains(QLatin1String("Hierarchical Data Format")) || fileName.endsWith(QLatin1String("h5"), Qt::CaseInsensitive) ||
	           fileName.endsWith(QLatin1String("hdf"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String("hdf5"), Qt::CaseInsensitive) ) {
		ui.cbFileType->setCurrentIndex(FileDataSource::HDF);

		// update HDF tree widget using current selected file
		m_hdfOptionsWidget->updateContent((HDFFilter*)this->currentFileFilter(), fileName);
	} else if (fileInfo.contains(QLatin1String("NetCDF Data Format")) || fileName.endsWith(QLatin1String("nc"), Qt::CaseInsensitive) ||
	           fileName.endsWith(QLatin1String("netcdf"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String("cdf"), Qt::CaseInsensitive)) {
		ui.cbFileType->setCurrentIndex(FileDataSource::NETCDF);

		// update NetCDF tree widget using current selected file
		m_netcdfOptionsWidget.twContent->clear();

		QTreeWidgetItem *rootItem = m_netcdfOptionsWidget.twContent->invisibleRootItem();
		NetCDFFilter *filter = (NetCDFFilter *)this->currentFileFilter();
		filter->parse(fileName, rootItem);
		m_netcdfOptionsWidget.twContent->insertTopLevelItem(0, rootItem);
		m_netcdfOptionsWidget.twContent->expandAll();
		m_netcdfOptionsWidget.twContent->resizeColumnToContents(0);
		m_netcdfOptionsWidget.twContent->resizeColumnToContents(2);
	} else if (fileInfo.contains(QLatin1String("FITS image data")) || fileName.endsWith(QLatin1String("fits"), Qt::CaseInsensitive) ||
	           fileName.endsWith(QLatin1String("fit"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String("fts"), Qt::CaseInsensitive)) {
#ifdef HAVE_FITS
		ui.cbFileType->setCurrentIndex(FileDataSource::FITS);
#endif
		m_fitsOptionsWidget.twExtensions->clear();
		QString fileName = ui.kleFileName->text();
		FITSFilter *filter = (FITSFilter *)this->currentFileFilter();
		filter->parseExtensions(fileName, m_fitsOptionsWidget.twExtensions, true);
	} else if (fileInfo.contains("image") || fileInfo.contains("bitmap") || !imageFormat.isEmpty())
		ui.cbFileType->setCurrentIndex(FileDataSource::Image);
	else
		ui.cbFileType->setCurrentIndex(FileDataSource::Binary);

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

	//if we switch from netCDF-format (only two tabs available), add the data preview-tab again
	if (ui.tabWidget->count() == 2) {
		ui.tabWidget->setTabText(0, i18n("Data format"));
		ui.tabWidget->insertTab(1, ui.tabDataPreview, i18n("Preview"));
	}
	ui.lPreviewLines->show();
	ui.sbPreviewLines->show();
	ui.lStartColumn->show();
	ui.sbStartColumn->show();
	ui.lEndColumn->show();
	ui.sbEndColumn->show();

	switch (fileType) {
	case FileDataSource::Ascii:
		break;
	case FileDataSource::Binary:
		ui.lStartColumn->hide();
		ui.sbStartColumn->hide();
		ui.lEndColumn->hide();
		ui.sbEndColumn->hide();
		break;
	case FileDataSource::HDF:
	case FileDataSource::NETCDF:
		ui.lFilter->hide();
		ui.cbFilter->hide();
		// hide global preview tab. we have our own
		ui.tabWidget->setTabText(0, i18n("Data format && preview"));
		ui.tabWidget->removeTab(1);
		ui.tabWidget->setCurrentIndex(0);
		break;
	case FileDataSource::Image:
		ui.lPreviewLines->hide();
		ui.sbPreviewLines->hide();
		ui.lFilter->hide();
		ui.cbFilter->hide();
		break;
	case FileDataSource::FITS:
		ui.lFilter->hide();
		ui.cbFilter->hide();
		ui.tabWidget->setTabText(0, i18n("Data format && preview"));
		ui.tabWidget->removeTab(1);
		ui.tabWidget->setCurrentIndex(0);
		break;
	default:
		DEBUG("unknown file type");
	}

	m_hdfOptionsWidget->clear();
	m_netcdfOptionsWidget.twContent->clear();

	int lastUsedFilterIndex = ui.cbFilter->currentIndex();
	ui.cbFilter->clear();
	ui.cbFilter->addItem( i18n("Automatic") );
	ui.cbFilter->addItem( i18n("Custom") );

	//TODO: populate the combobox with the available pre-defined filter settings for the selected type
	ui.cbFilter->setCurrentIndex(lastUsedFilterIndex);
	filterChanged(lastUsedFilterIndex);

	refreshPreview();
}

//TODO
void ImportFileWidget::fitsTreeWidgetSelectionChanged() {
	DEBUG("fitsTreeWidgetItemSelected()");
	QDEBUG("SELECTED ITEMS =" << m_fitsOptionsWidget.twExtensions->selectedItems());

	if (m_fitsOptionsWidget.twExtensions->selectedItems().isEmpty())
		return;

	QTreeWidgetItem* item = m_fitsOptionsWidget.twExtensions->selectedItems().first();
	int column = m_fitsOptionsWidget.twExtensions->currentColumn();

	WAIT_CURSOR;
	const QString& itemText = item->text(column);
	QString selectedExtension;
	int extType = 0;
	if (itemText.contains(QLatin1String("IMAGE #")) ||
	        itemText.contains(QLatin1String("ASCII_TBL #")) ||
	        itemText.contains(QLatin1String("BINARY_TBL #")))
		extType = 1;
	else if (!itemText.compare(i18n("Primary header")))
		extType = 2;
	if (extType == 0) {
		if (item->parent() != 0) {
			if (item->parent()->parent() != 0)
				selectedExtension = item->parent()->parent()->text(0) + QLatin1String("[") + item->text(column) + QLatin1String("]");
		}
	} else if (extType == 1) {
		if (item->parent() != 0) {
			if (item->parent()->parent() != 0) {
				bool ok;
				int hduNum = itemText.right(1).toInt(&ok);
				selectedExtension = item->parent()->parent()->text(0) + QLatin1String("[") + QString::number(hduNum-1) + QLatin1String("]");
			}
		}
	} else {
		if (item->parent()->parent() != 0)
			selectedExtension = item->parent()->parent()->text(column);
	}

	if (!selectedExtension.isEmpty()) {
		FITSFilter* filter = (FITSFilter*)this->currentFileFilter();
		bool readFitsTableToMatrix;
		QVector<QStringList> importedStrings = filter->readChdu(selectedExtension, &readFitsTableToMatrix, ui.sbPreviewLines->value());
		emit checkedFitsTableToMatrix(readFitsTableToMatrix);

		const int rows = importedStrings.size();
		m_fitsOptionsWidget.twPreview->clear();

		m_fitsOptionsWidget.twPreview->setRowCount(rows);
		int colCount = 0;
		const int maxColumns = 300;
		for (int i = 0; i < rows; i++) {
			QStringList lineString = importedStrings[i];
			if (i == 0) {
				colCount = lineString.size() > maxColumns ? maxColumns : lineString.size();
				m_fitsOptionsWidget.twPreview->setColumnCount(colCount);
			}
			colCount = lineString.size() > maxColumns ? maxColumns : lineString.size();

			for (int j = 0; j < colCount; j++) {
				QTableWidgetItem* item = new QTableWidgetItem(lineString[j]);
				m_fitsOptionsWidget.twPreview->setItem(i, j, item);
			}
		}
		m_fitsOptionsWidget.twPreview->resizeColumnsToContents();
	}
	RESET_CURSOR;
}

/*!
	updates the selected var name of a NetCDF file when the tree widget item is selected
*/
void ImportFileWidget::netcdfTreeWidgetSelectionChanged() {
	DEBUG("netcdfTreeWidgetItemSelected()");
	QDEBUG("SELECTED ITEMS =" << m_netcdfOptionsWidget.twContent->selectedItems());

	if (m_netcdfOptionsWidget.twContent->selectedItems().isEmpty())
		return;

	QTreeWidgetItem* item = m_netcdfOptionsWidget.twContent->selectedItems().first();
	if (item->data(1, Qt::DisplayRole).toString() == "variable")
		refreshPreview();
	else if (item->data(1, Qt::DisplayRole).toString().contains("attribute")) {
		// reads attributes (only for preview)
		NetCDFFilter *filter = (NetCDFFilter *)this->currentFileFilter();
		QString fileName = ui.kleFileName->text();
		QString name = item->data(0, Qt::DisplayRole).toString();
		QString varName = item->data(1, Qt::DisplayRole).toString().split(' ')[0];
		QDEBUG("name =" << name << "varName =" << varName);

		QString importedText = filter->readAttribute(fileName, name, varName);
		QDEBUG("importedText =" << importedText);

		QStringList lineStrings = importedText.split('\n');
		int rows = lineStrings.size();
		m_netcdfOptionsWidget.twPreview->setRowCount(rows);
		m_netcdfOptionsWidget.twPreview->setColumnCount(0);
		for (int i = 0; i < rows; i++) {
			QStringList lineString = lineStrings[i].split(' ');
			int cols = lineString.size();
			if (m_netcdfOptionsWidget.twPreview->columnCount() < cols)
				m_netcdfOptionsWidget.twPreview->setColumnCount(cols);

			for (int j = 0; j < cols; j++) {
				QTableWidgetItem* item = new QTableWidgetItem();
				item->setText(lineString[j]);
				m_netcdfOptionsWidget.twPreview->setItem(i, j, item);
			}
		}
	} else
		DEBUG("non showable object selected in NetCDF tree widget");
}

const QStringList ImportFileWidget::selectedHDFNames() const {
	return m_hdfOptionsWidget->selectedHDFNames();
}

/*!
	return list of selected NetCDF item names
*/
const QStringList ImportFileWidget::selectedNetCDFNames() const {
	QStringList names;
	for (auto* item: m_netcdfOptionsWidget.twContent->selectedItems())
		names << item->text(0);

	return names;
}

const QStringList ImportFileWidget::selectedFITSExtensions() const {
	QStringList extensionNames;
	//TODO
	for (auto* item: m_fitsOptionsWidget.twExtensions->selectedItems())
		extensionNames << item->text(0);
	return extensionNames;
}

/*!
	shows the dialog with the information about the file(s) to be imported.
*/
void ImportFileWidget::fileInfoDialog() {
	QStringList files = ui.kleFileName->text().split(';');
	FileInfoDialog* dlg = new FileInfoDialog(this);
	dlg->setFiles(files);
	dlg->exec();
}

/*!
	enables the options if the filter "custom" was chosen. Disables the options otherwise.
*/
void ImportFileWidget::filterChanged(int index) {
	// ignore filter for these formats
	if (ui.cbFileType->currentIndex() == FileDataSource::HDF || ui.cbFileType->currentIndex() == FileDataSource::NETCDF
	        || ui.cbFileType->currentIndex() == FileDataSource::Image || ui.cbFileType->currentIndex() == FileDataSource::FITS) {
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
	DEBUG("refreshPreview()");
	WAIT_CURSOR;

	QString fileName = ui.kleFileName->text();
#ifndef HAVE_WINDOWS
	if (fileName.left(1) != QDir::separator())
		fileName = QDir::homePath() + QDir::separator() + fileName;
#endif

	QVector<QStringList> importedStrings;
	FileDataSource::FileType fileType = (FileDataSource::FileType)ui.cbFileType->currentIndex();

	// generic table widget
	if (fileType == FileDataSource::Ascii || fileType == FileDataSource::Binary)
		m_twPreview->show();
	else
		m_twPreview->hide();

	int lines = ui.sbPreviewLines->value();

	bool ok = true;
	QTableWidget *tmpTableWidget = nullptr;
	QStringList vectorNameList;
	QVector<AbstractColumn::ColumnMode> columnModes;
	switch (fileType) {
	case FileDataSource::Ascii: {
		ui.tePreview->clear();

		AsciiFilter *filter = (AsciiFilter *)this->currentFileFilter();
		importedStrings = filter->readDataFromFile(fileName, nullptr, AbstractFileFilter::Replace, lines);
		tmpTableWidget = m_twPreview;
		vectorNameList = filter->vectorNames();
		columnModes = filter->columnModes();
		break;
	}
	case FileDataSource::Binary: {
		ui.tePreview->clear();

		BinaryFilter *filter = (BinaryFilter *)this->currentFileFilter();
		importedStrings = filter->readDataFromFile(fileName, nullptr, AbstractFileFilter::Replace, lines);
		tmpTableWidget = m_twPreview;
		break;
	}
	case FileDataSource::Image: {
		ui.tePreview->clear();

		QImage image(fileName);
		QTextCursor cursor = ui.tePreview->textCursor();
		cursor.insertImage(image);
		RESET_CURSOR;
		return;
	}
	case FileDataSource::HDF: {
		HDFFilter *filter = (HDFFilter *)this->currentFileFilter();
		lines = m_hdfOptionsWidget->lines();
		importedStrings = filter->readCurrentDataSet(fileName, NULL, ok, AbstractFileFilter::Replace, lines);
		tmpTableWidget = m_hdfOptionsWidget->previewWidget();
		break;
	}
	case FileDataSource::NETCDF: {
		NetCDFFilter *filter = (NetCDFFilter *)this->currentFileFilter();
		lines = m_netcdfOptionsWidget.sbPreviewLines->value();
		importedStrings = filter->readCurrentVar(fileName, NULL, AbstractFileFilter::Replace, lines);
		tmpTableWidget = m_netcdfOptionsWidget.twPreview;
		break;
	}
	case FileDataSource::FITS: {
		FITSFilter* filter = (FITSFilter*)this->currentFileFilter();
		lines = m_fitsOptionsWidget.sbPreviewLines->value();
		if (m_fitsOptionsWidget.twExtensions->currentItem() != 0) {
			const QTreeWidgetItem* item = m_fitsOptionsWidget.twExtensions->currentItem();
			const int currentColumn = m_fitsOptionsWidget.twExtensions->currentColumn();
			QString itemText = item->text(currentColumn);
			int extType = 0;
			if (itemText.contains(QLatin1String("IMAGE #")) ||
				itemText.contains(QLatin1String("ASCII_TBL #")) ||
				itemText.contains(QLatin1String("BINARY_TBL #")))
				extType = 1;
			else if (!itemText.compare(i18n("Primary header")))
				extType = 2;
			if (extType == 0) {
				if (item->parent() != 0) {
					if (item->parent()->parent() != 0)
						fileName = item->parent()->parent()->text(0) + QLatin1String("[")+ item->text(currentColumn) + QLatin1String("]");
				}
			} else if (extType == 1) {
				if (item->parent() != 0) {
					if (item->parent()->parent() != 0) {
						int hduNum = itemText.right(1).toInt(&ok);
						fileName = item->parent()->parent()->text(0) + QLatin1String("[") + QString::number(hduNum-1) + QLatin1String("]");
					}
				}
			} else {
				if (item->parent()->parent() != 0)
					fileName = item->parent()->parent()->text(currentColumn);
			}
		}
		bool readFitsTableToMatrix;
		importedStrings = filter->readChdu(fileName, &readFitsTableToMatrix, lines);
		emit checkedFitsTableToMatrix(readFitsTableToMatrix);

		tmpTableWidget = m_fitsOptionsWidget.twPreview;
		break;
	}
	}

	// fill the table widget
	tmpTableWidget->setRowCount(0);
	tmpTableWidget->setColumnCount(0);
	if( !importedStrings.isEmpty() ) {
		QDEBUG("importedStrings =" << importedStrings);
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

			for (int i = 0; i < rows; i++) {
				QDEBUG(importedStrings[i]);

				int cols = importedStrings[i].size() > maxColumns ? maxColumns : importedStrings[i].size();	// new
				if (cols > tmpTableWidget->columnCount())
					tmpTableWidget->setColumnCount(cols);

				for (int j = 0; j < cols; j++) {
					QTableWidgetItem* item = new QTableWidgetItem(importedStrings[i][j]);
					tmpTableWidget->setItem(i, j, item);
				}
			}

			// set header if columnMode available
			for (int i = 0; i < qMin(tmpTableWidget->columnCount(), columnModes.size()); i++) {
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
	}
	RESET_CURSOR;
}

void ImportFileWidget::updateTypeChanged(int idx) {
    FileDataSource::UpdateType type = static_cast<FileDataSource::UpdateType>(idx);

    if (type == FileDataSource::UpdateType::TimeInterval) {
        ui.lUpdateFrequency->show();
        ui.sbUpdateFrequency->show();
    } else if (type == FileDataSource::UpdateType::NewData) {
        ui.lUpdateFrequency->hide();
        ui.sbUpdateFrequency->hide();
    }
}

void ImportFileWidget::sourceTypeChanged(int idx) {
    FileDataSource::SourceType type = static_cast<FileDataSource::SourceType>(idx);

    if ((type == FileDataSource::SourceType::FileOrPipe) || (type == FileDataSource::SourceType::LocalSocket)) {
        ui.lFileName->show();
        ui.kleFileName->show();
        ui.bFileInfo->show();
        ui.bOpen->show();

        ui.cbBaudRate->hide();
        ui.lBaudRate->hide();
        ui.lHost->hide();
        ui.leHost->hide();
        ui.lPort->hide();
        ui.lePort->hide();
        ui.cbSerialPort->hide();
        ui.lSerialPort->hide();
    } else if (type == FileDataSource::SourceType::NetworkSocket) {
        ui.lHost->show();
        ui.leHost->show();
        ui.lePort->show();
        ui.lPort->show();

        ui.lBaudRate->hide();
        ui.cbBaudRate->hide();
        ui.lSerialPort->hide();
        ui.cbSerialPort->hide();

        ui.lFileName->hide();
        ui.kleFileName->hide();
        ui.bFileInfo->hide();
        ui.bOpen->hide();

    } else if (type == FileDataSource::SourceType::SerialPort) {
        ui.lBaudRate->show();
        ui.cbBaudRate->show();
        ui.lSerialPort->show();
        ui.cbSerialPort->show();

        ui.lHost->hide();
        ui.leHost->hide();
        ui.lePort->hide();
        ui.lPort->hide();
        ui.lFileName->hide();
        ui.kleFileName->hide();
        ui.bFileInfo->hide();
        ui.bOpen->hide();
    }
}

void ImportFileWidget::initializeAndFillPortsAndBaudRates() {

    for (int i = 2; i < ui.swOptions->count(); ++i) {
        ui.swOptions->removeWidget(ui.swOptions->widget(i));
    }

    const int size = ui.cbFileType->count();
    for (int i = 2; i < size; ++i) {
        ui.cbFileType->removeItem(2);
    }

    ui.cbBaudRate->hide();
    ui.lBaudRate->hide();

    ui.lHost->hide();
    ui.leHost->hide();

    ui.lPort->hide();
    ui.lePort->hide();

    ui.cbSerialPort->hide();
    ui.lSerialPort->hide();

    ui.cbBaudRate->addItems(FileDataSource::supportedBaudRates());
    ui.cbSerialPort->addItems(FileDataSource::availablePorts());

    ui.tabDataPortion->hide();
}
