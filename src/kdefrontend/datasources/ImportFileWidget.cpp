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

#include <QTableWidget>
#include <QInputDialog>
#include <QDir>
#include <QFileDialog>
#include <QProcess>
#include <KUrlCompletion>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDebug>
#include <QTimer>
#include <QStandardItemModel>
#include <QImageReader>

/*!
   \class ImportFileWidget
   \brief Widget for importing data from a file.

   \ingroup kdefrontend
*/

ImportFileWidget::ImportFileWidget(QWidget* parent, const QString& fileName) : QWidget(parent), m_fileName(fileName) {
	ui.setupUi(this);

	KUrlCompletion *comp = new KUrlCompletion();
	ui.kleFileName->setCompletionObject(comp);

	ui.cbFileType->addItems(FileDataSource::fileTypes());
	QStringList filterItems;
	filterItems << i18n("Automatic") << i18n("Custom");
	ui.cbFilter->addItems( filterItems );

	// file type specific option widgets
	QWidget* asciiw = new QWidget(0);
	asciiOptionsWidget.setupUi(asciiw);
	asciiOptionsWidget.cbSeparatingCharacter->addItems(AsciiFilter::separatorCharacters());
	asciiOptionsWidget.cbCommentCharacter->addItems(AsciiFilter::commentCharacters());
	asciiOptionsWidget.chbTranspose->hide(); //TODO: enable later
	ui.swOptions->insertWidget(FileDataSource::Ascii, asciiw);

	QWidget* binaryw=new QWidget(0);
	binaryOptionsWidget.setupUi(binaryw);
	binaryOptionsWidget.cbDataType->addItems(BinaryFilter::dataTypes());
	binaryOptionsWidget.cbByteOrder->addItems(BinaryFilter::byteOrders());
	ui.swOptions->insertWidget(FileDataSource::Binary, binaryw);

	QWidget* imagew = new QWidget(0);
	imageOptionsWidget.setupUi(imagew);
	imageOptionsWidget.cbImportFormat->addItems(ImageFilter::importFormats());
	ui.swOptions->insertWidget(FileDataSource::Image, imagew);

	QWidget* hdfw = new QWidget(0);
	hdfOptionsWidget.setupUi(hdfw);
	QStringList hdfheaders;
	hdfheaders << i18n("Name") << i18n("Link") << i18n("Type") << i18n("Properties") << i18n("Attributes");
	hdfOptionsWidget.twContent->setHeaderLabels(hdfheaders);
	hdfOptionsWidget.twContent->setAlternatingRowColors(true);
	// link and type column are hidden
	hdfOptionsWidget.twContent->hideColumn(1);
	hdfOptionsWidget.twContent->hideColumn(2);
	hdfOptionsWidget.twContent->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.swOptions->insertWidget(FileDataSource::HDF, hdfw);

	QWidget* netcdfw = new QWidget(0);
	netcdfOptionsWidget.setupUi(netcdfw);
	QStringList headers;
	headers << i18n("Name") << i18n("Type") << i18n("Properties") << i18n("Values");
	netcdfOptionsWidget.twContent->setHeaderLabels(headers);
	// type column is hidden
	netcdfOptionsWidget.twContent->hideColumn(1);
	netcdfOptionsWidget.twContent->setSelectionMode(QAbstractItemView::ExtendedSelection);
	netcdfOptionsWidget.twContent->setAlternatingRowColors(true);
	ui.swOptions->insertWidget(FileDataSource::NETCDF, netcdfw);

	QWidget* fitsw = new QWidget(0);
	fitsOptionsWidget.setupUi(fitsw);
	fitsOptionsWidget.twExtensions->headerItem()->setText(0, i18n("Extensions"));
	fitsOptionsWidget.twExtensions->setSelectionMode(QAbstractItemView::SingleSelection);
	fitsOptionsWidget.twExtensions->setAlternatingRowColors(true);
	ui.swOptions->insertWidget(FileDataSource::FITS, fitsw);

	// the table widget for preview
	twPreview = new QTableWidget(ui.tePreview);
	twPreview->horizontalHeader()->hide();
	twPreview->verticalHeader()->hide();
	twPreview->setEditTriggers(QTableWidget::NoEditTriggers);
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(twPreview);
	ui.tePreview->setLayout(layout);
	twPreview->hide();

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

	ui.bOpen->setIcon( QIcon::fromTheme("document-open") );
	ui.bFileInfo->setIcon( QIcon::fromTheme("help-about") );
	ui.bManageFilters->setIcon( QIcon::fromTheme("configure") );
	ui.bSaveFilter->setIcon( QIcon::fromTheme("document-save") );
	ui.bRefreshPreview->setIcon( QIcon::fromTheme("view-refresh") );
	hdfOptionsWidget.bRefreshPreview->setIcon( QIcon::fromTheme("view-refresh") );
	netcdfOptionsWidget.bRefreshPreview->setIcon( QIcon::fromTheme("view-refresh") );

	connect( ui.kleFileName, SIGNAL(textChanged(QString)), SLOT(fileNameChanged(QString)) );
	connect( ui.bOpen, SIGNAL(clicked()), this, SLOT (selectFile()) );
	connect( ui.bFileInfo, SIGNAL(clicked()), this, SLOT (fileInfoDialog()) );
	connect( ui.bSaveFilter, SIGNAL(clicked()), this, SLOT (saveFilter()) );
	connect( ui.bManageFilters, SIGNAL(clicked()), this, SLOT (manageFilters()) );
	connect( ui.cbFileType, SIGNAL(currentIndexChanged(int)), SLOT(fileTypeChanged(int)) );
	connect( ui.cbFilter, SIGNAL(activated(int)), SLOT(filterChanged(int)) );
	connect( ui.bRefreshPreview, SIGNAL(clicked()), SLOT(refreshPreview()) );

	connect( asciiOptionsWidget.chbHeader, SIGNAL(stateChanged(int)), SLOT(headerChanged(int)) );
	connect( hdfOptionsWidget.twContent, SIGNAL(itemSelectionChanged()), SLOT(hdfTreeWidgetSelectionChanged()) );
	connect( hdfOptionsWidget.bRefreshPreview, SIGNAL(clicked()), SLOT(refreshPreview()) );
	connect( netcdfOptionsWidget.twContent, SIGNAL(itemSelectionChanged()), SLOT(netcdfTreeWidgetSelectionChanged()) );
	connect( netcdfOptionsWidget.bRefreshPreview, SIGNAL(clicked()), SLOT(refreshPreview()) );
	connect( fitsOptionsWidget.twExtensions, SIGNAL(itemSelectionChanged()), SLOT(fitsTreeWidgetSelectionChanged()));
	connect( fitsOptionsWidget.bRefreshPreview, SIGNAL(clicked()), SLOT(refreshPreview()) );

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
	// ascii data
	//TODO: check if this works (character gets currentItem?)
	asciiOptionsWidget.cbCommentCharacter->setCurrentItem(conf.readEntry("CommentCharacter", "#"));
	asciiOptionsWidget.cbSeparatingCharacter->setCurrentItem(conf.readEntry("SeparatingCharacter", "auto"));
	asciiOptionsWidget.chbSimplifyWhitespaces->setChecked(conf.readEntry("SimplifyWhitespaces", true));
	asciiOptionsWidget.chbSkipEmptyParts->setChecked(conf.readEntry("SkipEmptyParts", false));
	asciiOptionsWidget.chbHeader->setChecked(conf.readEntry("UseFirstRow", true));
	asciiOptionsWidget.kleVectorNames->setText(conf.readEntry("Names", ""));

	// binary data
	binaryOptionsWidget.niVectors->setValue(conf.readEntry("Vectors", "2").toInt());
	binaryOptionsWidget.cbDataType->setCurrentIndex(conf.readEntry("DataType", 0));
	binaryOptionsWidget.cbByteOrder->setCurrentIndex(conf.readEntry("ByteOrder", 0));
	binaryOptionsWidget.sbSkipStartBytes->setValue(conf.readEntry("SkipStartBytes", 0));
	binaryOptionsWidget.sbSkipBytes->setValue(conf.readEntry("SkipBytes", 0));

	// image data
	imageOptionsWidget.cbImportFormat->setCurrentIndex(conf.readEntry("ImportFormat", 0));

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
	// ascii data
	conf.writeEntry("CommentCharacter", asciiOptionsWidget.cbCommentCharacter->currentText());
	conf.writeEntry("SeparatingCharacter", asciiOptionsWidget.cbSeparatingCharacter->currentText());
	conf.writeEntry("SimplifyWhitespaces", asciiOptionsWidget.chbSimplifyWhitespaces->isChecked());
	conf.writeEntry("SkipEmptyParts", asciiOptionsWidget.chbSkipEmptyParts->isChecked());
	conf.writeEntry("UseFirstRow", asciiOptionsWidget.chbHeader->isChecked());
	conf.writeEntry("Names", asciiOptionsWidget.kleVectorNames->text());

	// binary data
	conf.writeEntry("Vectors", binaryOptionsWidget.niVectors->value());
	conf.writeEntry("ByteOrder", binaryOptionsWidget.cbByteOrder->currentIndex());
	conf.writeEntry("DataType", binaryOptionsWidget.cbDataType->currentIndex());
	conf.writeEntry("SkipStartBytes", binaryOptionsWidget.sbSkipStartBytes->value());
	conf.writeEntry("SkipBytes", binaryOptionsWidget.sbSkipBytes->value());

	// image data
	conf.writeEntry("ImportFormat", imageOptionsWidget.cbImportFormat->currentIndex());

	//HDF/NetCDF data
	// nothing
}

void ImportFileWidget::hideDataSource() const {
	ui.lSourceName->hide();
	ui.kleSourceName->hide();
	ui.chbWatchFile->hide();
	ui.chbLinkFile->hide();
}

void ImportFileWidget::showAsciiHeaderOptions(bool b) {
	asciiOptionsWidget.chbHeader->setVisible(b);
	asciiOptionsWidget.lVectorNames->setVisible(b);
	asciiOptionsWidget.kleVectorNames->setVisible(b);
}

void ImportFileWidget::showOptions(bool b) {
	ui.gbOptions->setVisible(b);
	resize(layout()->minimumSize());
}

QString ImportFileWidget::fileName() const {
	if (currentFileType() == FileDataSource::FITS) {
		if (fitsOptionsWidget.twExtensions->currentItem() != 0) {
			if (fitsOptionsWidget.twExtensions->currentItem()->text(0) != i18n("Primary header")) {
				return ui.kleFileName->text() + QLatin1String("[") +
				       fitsOptionsWidget.twExtensions->currentItem()->text(fitsOptionsWidget.twExtensions->currentColumn()) + QLatin1String("]");
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
	source->setFileName( ui.kleFileName->text() );
	source->setName( ui.kleSourceName->text() );
	source->setComment( ui.kleFileName->text() );
	source->setFileWatched( ui.chbWatchFile->isChecked() );
	source->setFileLinked( ui.chbLinkFile->isChecked() );

	FileDataSource::FileType fileType = (FileDataSource::FileType)ui.cbFileType->currentIndex();
	source->setFileType(fileType);
	source->setFilter(this->currentFileFilter());
}

/*!
	returns the currently used file type.
*/
FileDataSource::FileType ImportFileWidget::currentFileType() const {
	return (FileDataSource::FileType)ui.cbFileType->currentIndex();
}

/*!
	returns the currently used filter.
*/
AbstractFileFilter* ImportFileWidget::currentFileFilter() const {
	DEBUG_LOG("currentFileFilter()");
	FileDataSource::FileType fileType = (FileDataSource::FileType)ui.cbFileType->currentIndex();

	switch (fileType) {
	case FileDataSource::Ascii: {
			//TODO use auto_ptr
			AsciiFilter* filter = new AsciiFilter();

			if (ui.cbFilter->currentIndex() == 0)   //"automatic"
				filter->setAutoModeEnabled(true);
			else if (ui.cbFilter->currentIndex() == 1) { //"custom"
				filter->setAutoModeEnabled(false);
				filter->setCommentCharacter( asciiOptionsWidget.cbCommentCharacter->currentText() );
				filter->setSeparatingCharacter( asciiOptionsWidget.cbSeparatingCharacter->currentText() );
				filter->setSimplifyWhitespacesEnabled( asciiOptionsWidget.chbSimplifyWhitespaces->isChecked() );
				filter->setSkipEmptyParts( asciiOptionsWidget.chbSkipEmptyParts->isChecked() );
				filter->setTransposed( asciiOptionsWidget.chbTranspose->isChecked() );
				filter->setVectorNames( asciiOptionsWidget.kleVectorNames->text() );
				filter->setHeaderEnabled( asciiOptionsWidget.chbHeader->isChecked() );
			} else
				filter->loadFilterSettings( ui.cbFilter->currentText() );

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
				filter->setVectors( binaryOptionsWidget.niVectors->value() );
				filter->setDataType( (BinaryFilter::DataType) binaryOptionsWidget.cbDataType->currentIndex() );
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

			filter->setImportFormat((ImageFilter::ImportFormat)imageOptionsWidget.cbImportFormat->currentIndex());
			filter->setStartRow( ui.sbStartRow->value() );
			filter->setEndRow( ui.sbEndRow->value() );
			filter->setStartColumn( ui.sbStartColumn->value() );
			filter->setEndColumn( ui.sbEndColumn->value() );

			return filter;
		}
	case FileDataSource::HDF: {
			HDFFilter* filter = new HDFFilter();

			if (!selectedHDFNames().isEmpty())
				filter->setCurrentDataSetName(selectedHDFNames()[0]);
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

	//use the file name as the name of the data source,
	//if there is no data source name provided yet
	if (ui.kleSourceName->text().isEmpty()) {
		QString fileName = QFileInfo(path).fileName();
		ui.kleSourceName->setText(fileName);
	}

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
#ifndef _WIN32
	// make relative path
	if ( !fileName.isEmpty() && fileName.left(1) != QDir::separator())
		fileName = QDir::homePath() + QDir::separator() + fileName;
#endif

	bool fileExists = QFile::exists(fileName);
	ui.gbOptions->setEnabled(fileExists);
	ui.bFileInfo->setEnabled(fileExists);
	ui.cbFileType->setEnabled(fileExists);
	ui.cbFilter->setEnabled(fileExists);
	ui.bManageFilters->setEnabled(fileExists);
	ui.kleSourceName->setEnabled(fileExists);
	ui.chbWatchFile->setEnabled(fileExists);
	ui.chbLinkFile->setEnabled(fileExists);
	if (!fileExists) {
		refreshPreview();
		emit fileNameChanged();
		return;
	}

	QString fileInfo;
#ifndef _WIN32
	//check, if we can guess the file type by content
	QProcess *proc = new QProcess(this);
	QStringList args;
	args << "-b" << ui.kleFileName->text();
	proc->start("file", args);
	if (proc->waitForReadyRead(1000) == false) {
		qDebug() << "ERROR: reading file type of file" << fileName;
		return;
	}
	fileInfo = proc->readLine();
#endif

	QByteArray imageFormat = QImageReader::imageFormat(fileName);
	if (fileInfo.contains("compressed data") || fileInfo.contains("ASCII") ||
	        fileName.endsWith("dat", Qt::CaseInsensitive) || fileName.endsWith("txt", Qt::CaseInsensitive)) {
		//probably ascii data
		ui.cbFileType->setCurrentIndex(FileDataSource::Ascii);
	} else if (fileInfo.contains("Hierarchical Data Format") || fileName.endsWith("h5", Qt::CaseInsensitive) ||
	           fileName.endsWith("hdf", Qt::CaseInsensitive) || fileName.endsWith("hdf5", Qt::CaseInsensitive) ) {
		ui.cbFileType->setCurrentIndex(FileDataSource::HDF);

		// update HDF tree widget using current selected file
		hdfOptionsWidget.twContent->clear();

		QTreeWidgetItem *rootItem = hdfOptionsWidget.twContent->invisibleRootItem();
		HDFFilter *filter = (HDFFilter *)this->currentFileFilter();
		filter->parse(fileName, rootItem);
		hdfOptionsWidget.twContent->insertTopLevelItem(0, rootItem);
		hdfOptionsWidget.twContent->expandAll();
		hdfOptionsWidget.twContent->resizeColumnToContents(0);
		hdfOptionsWidget.twContent->resizeColumnToContents(3);
	} else if (fileInfo.contains("NetCDF Data Format") || fileName.endsWith("nc", Qt::CaseInsensitive) ||
	           fileName.endsWith("netcdf", Qt::CaseInsensitive) || fileName.endsWith("cdf", Qt::CaseInsensitive)) {
		ui.cbFileType->setCurrentIndex(FileDataSource::NETCDF);

		// update NetCDF tree widget using current selected file
		netcdfOptionsWidget.twContent->clear();

		QTreeWidgetItem *rootItem = netcdfOptionsWidget.twContent->invisibleRootItem();
		NetCDFFilter *filter = (NetCDFFilter *)this->currentFileFilter();
		filter->parse(fileName, rootItem);
		netcdfOptionsWidget.twContent->insertTopLevelItem(0, rootItem);
		netcdfOptionsWidget.twContent->expandAll();
		netcdfOptionsWidget.twContent->resizeColumnToContents(0);
		netcdfOptionsWidget.twContent->resizeColumnToContents(2);
	} else if (fileInfo.contains("FITS image data") || fileName.endsWith("fits", Qt::CaseInsensitive) ||
	           fileName.endsWith("fit", Qt::CaseInsensitive) || fileName.endsWith("fts", Qt::CaseInsensitive)) {
#ifdef HAVE_FITS
		ui.cbFileType->setCurrentIndex(FileDataSource::FITS);
#endif
		fitsOptionsWidget.twExtensions->clear();
		QString fileName = ui.kleFileName->text();
		FITSFilter *filter = (FITSFilter *)this->currentFileFilter();
		filter->parseExtensions(fileName, fitsOptionsWidget.twExtensions, true);
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
		qDebug()<<"unknown file type";
	}

	hdfOptionsWidget.twContent->clear();
	netcdfOptionsWidget.twContent->clear();

	int lastUsedFilterIndex = ui.cbFilter->currentIndex();
	ui.cbFilter->clear();
	ui.cbFilter->addItem( i18n("Automatic") );
	ui.cbFilter->addItem( i18n("Custom") );

	//TODO: populate the combobox with the available pre-defined filter settings for the selected type
	ui.cbFilter->setCurrentIndex(lastUsedFilterIndex);
	filterChanged(lastUsedFilterIndex);

	refreshPreview();
}

/*!
	updates the selected data set of a HDF file when a new tree widget item is selected
*/
void ImportFileWidget::hdfTreeWidgetSelectionChanged() {
	DEBUG_LOG("hdfTreeWidgetItemSelected()");
	DEBUG_LOG("SELECTED ITEMS =" << hdfOptionsWidget.twContent->selectedItems());

	if (hdfOptionsWidget.twContent->selectedItems().isEmpty())
		return;

	QTreeWidgetItem* item = hdfOptionsWidget.twContent->selectedItems().first();
	if (item->data(2, Qt::DisplayRole).toString() == i18n("data set"))
		refreshPreview();
	else
		qDebug()<<"non data set selected in HDF tree widget";
}

/*!
	return list of selected HDF item names
*/
const QStringList ImportFileWidget::selectedHDFNames() const {
	QStringList names;
	QList<QTreeWidgetItem*> items = hdfOptionsWidget.twContent->selectedItems();

	// the data link is saved in the second column
	foreach (QTreeWidgetItem* item, items)
		names << item->text(1);

	return names;
}

//TODO
void ImportFileWidget::fitsTreeWidgetSelectionChanged() {
	DEBUG_LOG("fitsTreeWidgetItemSelected()");
	DEBUG_LOG("SELECTED ITEMS =" << fitsOptionsWidget.twExtensions->selectedItems());

	if (fitsOptionsWidget.twExtensions->selectedItems().isEmpty())
		return;

	QTreeWidgetItem* item = fitsOptionsWidget.twExtensions->selectedItems().first();
	int column = fitsOptionsWidget.twExtensions->currentColumn();

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
		QList<QStringList> importedStrings = filter->readChdu(selectedExtension, &readFitsTableToMatrix, ui.sbPreviewLines->value());
		emit checkedFitsTableToMatrix(readFitsTableToMatrix);

		const int rows = importedStrings.size();
		fitsOptionsWidget.twPreview->clear();

		fitsOptionsWidget.twPreview->setRowCount(rows);
		int colCount = 0;
		const int maxColumns = 300;
		for (int i = 0; i < rows; i++) {
			QStringList lineString = importedStrings[i];
			if (i == 0) {
				colCount = lineString.size() > maxColumns ? maxColumns : lineString.size();
				fitsOptionsWidget.twPreview->setColumnCount(colCount);
			}
			colCount = lineString.size() > maxColumns ? maxColumns : lineString.size();

			for (int j = 0; j < colCount; j++) {
				QTableWidgetItem* item = new QTableWidgetItem(lineString[j]);
				fitsOptionsWidget.twPreview->setItem(i, j, item);
			}
		}
		fitsOptionsWidget.twPreview->resizeColumnsToContents();
	}
	RESET_CURSOR;
}

/*!
	updates the selected var name of a NetCDF file when the tree widget item is selected
*/
void ImportFileWidget::netcdfTreeWidgetSelectionChanged() {
	DEBUG_LOG("netcdfTreeWidgetItemSelected()");
	DEBUG_LOG("SELECTED ITEMS =" << netcdfOptionsWidget.twContent->selectedItems());

	if (netcdfOptionsWidget.twContent->selectedItems().isEmpty())
		return;

	QTreeWidgetItem* item = netcdfOptionsWidget.twContent->selectedItems().first();
	if (item->data(1, Qt::DisplayRole).toString() == "variable")
		refreshPreview();
	else if (item->data(1, Qt::DisplayRole).toString().contains("attribute")) {
		// reads attributes (only for preview)
		NetCDFFilter *filter = (NetCDFFilter *)this->currentFileFilter();
		QString fileName = ui.kleFileName->text();
		QString name = item->data(0, Qt::DisplayRole).toString();
		QString varName = item->data(1, Qt::DisplayRole).toString().split(" ")[0];
		DEBUG_LOG("name =" << name << "varName =" << varName);

		QString importedText = filter->readAttribute(fileName, name, varName);
		DEBUG_LOG("importedText =" << importedText);

		QStringList lineStrings = importedText.split("\n");
		int rows = lineStrings.size();
		netcdfOptionsWidget.twPreview->setRowCount(rows);
		netcdfOptionsWidget.twPreview->setColumnCount(0);
		for (int i = 0; i < rows; i++) {
			QStringList lineString = lineStrings[i].split(" ");
			int cols = lineString.size();
			if (netcdfOptionsWidget.twPreview->columnCount() < cols)
				netcdfOptionsWidget.twPreview->setColumnCount(cols);

			for (int j = 0; j < cols; j++) {
				QTableWidgetItem* item = new QTableWidgetItem();
				item->setText(lineString[j]);
				netcdfOptionsWidget.twPreview->setItem(i, j, item);
			}
		}
	} else
		qDebug()<<"non showable object selected in NetCDF tree widget";
}

/*!
	return list of selected NetCDF item names
*/
const QStringList ImportFileWidget::selectedNetCDFNames() const {
	QStringList names;
	QList<QTreeWidgetItem *> items = netcdfOptionsWidget.twContent->selectedItems();

	foreach (QTreeWidgetItem* item, items)
		names << item->text(0);

	return names;
}

const QStringList ImportFileWidget::selectedFITSExtensions() const {
	QStringList extensionNames;
	//TODO
	QList<QTreeWidgetItem* > items = fitsOptionsWidget.twExtensions->selectedItems();
	foreach (QTreeWidgetItem* item, items)
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

/*!
  enables a text field for the vector names if the option "Use the first row..." was not selected.
  Disables it otherwise.
*/
void ImportFileWidget::headerChanged(int state) {
	if (state == Qt::Checked) {
		asciiOptionsWidget.kleVectorNames->setEnabled(false);
		asciiOptionsWidget.lVectorNames->setEnabled(false);
	} else {
		asciiOptionsWidget.kleVectorNames->setEnabled(true);
		asciiOptionsWidget.lVectorNames->setEnabled(true);
	}
}

void ImportFileWidget::refreshPreview() {
	DEBUG_LOG("refreshPreview()");
	WAIT_CURSOR;

	QString fileName = ui.kleFileName->text();
#ifndef _WIN32
	if (fileName.left(1) != QDir::separator())
		fileName = QDir::homePath() + QDir::separator() + fileName;
#endif

	QList<QStringList> importedStrings;
	FileDataSource::FileType fileType = (FileDataSource::FileType)ui.cbFileType->currentIndex();

	// generic table widget
	if (fileType == FileDataSource::Ascii || fileType == FileDataSource::Binary)
		twPreview->show();
	else
		twPreview->hide();

	int lines = ui.sbPreviewLines->value();

	bool ok = true;
	QTableWidget *tmpTableWidget = 0;
	switch (fileType) {
	case FileDataSource::Ascii: {
			ui.tePreview->clear();

			AsciiFilter *filter = (AsciiFilter *)this->currentFileFilter();
			importedStrings = filter->readData(fileName, NULL, AbstractFileFilter::Replace, lines);
			tmpTableWidget = twPreview;
			break;
		}
	case FileDataSource::Binary: {
			ui.tePreview->clear();

			BinaryFilter *filter = (BinaryFilter *)this->currentFileFilter();
			importedStrings = filter->readData(fileName, NULL, AbstractFileFilter::Replace, lines);
			tmpTableWidget = twPreview;
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
			lines = hdfOptionsWidget.sbPreviewLines->value();
			importedStrings = filter->readCurrentDataSet(fileName, NULL, ok, AbstractFileFilter::Replace, lines);
			tmpTableWidget = hdfOptionsWidget.twPreview;
			break;
		}
	case FileDataSource::NETCDF: {
			NetCDFFilter *filter = (NetCDFFilter *)this->currentFileFilter();
			lines = netcdfOptionsWidget.sbPreviewLines->value();
			importedStrings = filter->readCurrentVar(fileName, NULL, AbstractFileFilter::Replace, lines);
			tmpTableWidget = netcdfOptionsWidget.twPreview;
			break;
		}
	case FileDataSource::FITS: {
			FITSFilter* filter = (FITSFilter*)this->currentFileFilter();
			lines = fitsOptionsWidget.sbPreviewLines->value();
			if (fitsOptionsWidget.twExtensions->currentItem() != 0) {
				const QTreeWidgetItem* item = fitsOptionsWidget.twExtensions->currentItem();
				const int currentColumn = fitsOptionsWidget.twExtensions->currentColumn();
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
							bool ok;
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

			tmpTableWidget = fitsOptionsWidget.twPreview;
			break;
		}
	}

	// fill the table widget
	tmpTableWidget->setRowCount(0);
	tmpTableWidget->setColumnCount(0);
	if( !importedStrings.isEmpty() ) {
		DEBUG_LOG("importedStrings =" << importedStrings);	// new
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
			tmpTableWidget->setRowCount(rows);	// new
			for (int i = 0; i < rows; i++) {
				DEBUG_LOG(importedStrings[i]);

				int cols = importedStrings[i].size() > maxColumns ? maxColumns : importedStrings[i].size();	// new
				if (cols > tmpTableWidget->columnCount())
					tmpTableWidget->setColumnCount(cols);

				for (int j = 0; j < cols; j++) {
					QTableWidgetItem* item = new QTableWidgetItem(importedStrings[i][j]);
					tmpTableWidget->setItem(i, j, item);
				}
			}
		}

		tmpTableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	}
	RESET_CURSOR;
}
