/***************************************************************************
File                 : ImportFileWidget.cpp
Project              : LabPlot
Description          : import file data widget
--------------------------------------------------------------------
Copyright            : (C) 2009-2015 Stefan Gerlach (stefan.gerlach@uni.kn)
Copyright            : (C) 2009-2012 Alexander Semke (alexander.semke@web.de)

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
#include "backend/datasources/FileDataSource.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/filters/BinaryFilter.h"
#include "backend/datasources/filters/HDFFilter.h"
#include "backend/datasources/filters/NetCDFFilter.h"
#include "backend/datasources/filters/ImageFilter.h"

#include <QInputDialog>
#include <QDir>
#include <QFileDialog>
#include <QProcess>
#include <QTextStream>
#include <KUrlCompletion>
#include <QDebug>

#include <kfilterdev.h>

/*!
   \class ImportFileWidget
   \brief Widget for importing data from a file.

   \ingroup kdefrontend
*/

ImportFileWidget::ImportFileWidget(QWidget* parent) : QWidget(parent) {

	ui.setupUi(this);

	KUrlCompletion *comp = new KUrlCompletion();
	ui.kleFileName->setCompletionObject(comp);

	ui.cbFileType->addItems(FileDataSource::fileTypes());
	QStringList filterItems;
	filterItems<<i18n("Automatic")<<i18n("Custom");
	ui.cbFilter->addItems( filterItems );

	// file type specific option widgets
	QWidget* asciiw=new QWidget(0);
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

	QWidget* imagew=new QWidget(0);
	imageOptionsWidget.setupUi(imagew);
	imageOptionsWidget.cbImportFormat->addItems(ImageFilter::importFormats());
	ui.swOptions->insertWidget(FileDataSource::Image, imagew);

	QWidget* hdfw=new QWidget(0);
	hdfOptionsWidget.setupUi(hdfw);
	QStringList hdfheaders;
	hdfheaders<<i18n("Name")<<i18n("Link")<<i18n("Type")<<i18n("Properties")<<i18n("Attributes");
	hdfOptionsWidget.twContent->setHeaderLabels(hdfheaders);
	// link and type column are filled but we don't need to show it
	hdfOptionsWidget.twContent->hideColumn(1);
	hdfOptionsWidget.twContent->hideColumn(2);
	ui.swOptions->insertWidget(FileDataSource::HDF, hdfw);

	QWidget* netcdfw=new QWidget(0);
	netcdfOptionsWidget.setupUi(netcdfw);
	QStringList headers;
	headers<<i18n("Name")<<i18n("Type")<<i18n("Properties")<<i18n("Values");
	// type is hidden
	hdfOptionsWidget.twContent->hideColumn(1);
	netcdfOptionsWidget.twContent->setHeaderLabels(headers);
	ui.swOptions->insertWidget(FileDataSource::NETCDF, netcdfw);

	// default filter
	ui.swOptions->setCurrentIndex(FileDataSource::Ascii);
	// disable items
#ifndef HAVE_HDF5
	ui.cbFileType->setItemData(FileDataSource::HDF, 0, Qt::UserRole - 1);
#endif
#ifndef HAVE_NETCDF
	ui.cbFileType->setItemData(FileDataSource::NETCDF, 0, Qt::UserRole - 1);
#endif

	ui.gbOptions->hide();

	ui.bOpen->setIcon( KIcon("document-open") );
	ui.bFileInfo->setIcon( KIcon("help-about") );
	ui.bManageFilters->setIcon( KIcon("configure") );
	ui.bSaveFilter->setIcon( KIcon("document-save") );
	ui.bRefreshPreview->setIcon( KIcon("view-refresh") );

	connect( ui.kleFileName, SIGNAL(textChanged(QString)), SLOT(fileNameChanged(QString)) );
	connect( ui.bOpen, SIGNAL(clicked()), this, SLOT (selectFile()) );
	connect( ui.bFileInfo, SIGNAL(clicked()), this, SLOT (fileInfoDialog()) );
	connect( ui.bSaveFilter, SIGNAL(clicked()), this, SLOT (saveFilter()) );
	connect( ui.bManageFilters, SIGNAL(clicked()), this, SLOT (manageFilters()) );
	connect( ui.cbFileType, SIGNAL(currentIndexChanged(int)), SLOT(fileTypeChanged(int)) );
	connect( ui.cbFilter, SIGNAL(activated(int)), SLOT(filterChanged(int)) );
	connect( ui.bRefreshPreview, SIGNAL(clicked()), SLOT(refreshPreview()) );

	//load last used settings
	KConfigGroup conf(KSharedConfig::openConfig(),"Import");

	//settings for data type specific widgets
	// ascii data
	connect( asciiOptionsWidget.chbHeader, SIGNAL(stateChanged(int)), SLOT(headerChanged(int)) );
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

	// HDF data
	connect( hdfOptionsWidget.twContent, SIGNAL(itemActivated(QTreeWidgetItem*,int)), SLOT(hdfTreeWidgetItemSelected(QTreeWidgetItem*,int)) );

	// NetCDF data
	connect( netcdfOptionsWidget.twContent, SIGNAL(itemActivated(QTreeWidgetItem*,int)), SLOT(netcdfTreeWidgetItemSelected(QTreeWidgetItem*,int)) );

	//general settings
	ui.cbFileType->setCurrentIndex(conf.readEntry("Type", 0));
	ui.kleFileName->setText(conf.readEntry("LastImportedFile", ""));
	ui.cbFilter->setCurrentIndex(conf.readEntry("Filter", 0));
	filterChanged(ui.cbFilter->currentIndex());

	//TODO: implement save/load of user-defined settings later and activate these buttons again
	ui.bSaveFilter->hide();
	ui.bManageFilters->hide();
}

ImportFileWidget::~ImportFileWidget() {
	// save current settings
	KConfigGroup conf(KSharedConfig::openConfig(),"Import");

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

void ImportFileWidget::hideDataSource() const{
	ui.lSourceName->hide();
	ui.kleSourceName->hide();
	ui.chbWatchFile->hide();
	ui.chbLinkFile->hide();
}

void ImportFileWidget::showOptions(bool b) {
	ui.gbOptions->setVisible(b);
	resize(layout()->minimumSize());
}

QString ImportFileWidget::fileName() const{
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
	returns the currently used filter.
*/
AbstractFileFilter* ImportFileWidget::currentFileFilter() const{
	FileDataSource::FileType fileType = (FileDataSource::FileType)ui.cbFileType->currentIndex();

	//qDebug()<<"	current filter ="<<ui.cbFilter->currentIndex();

	switch(fileType) {
	case FileDataSource::Ascii: {
		 //TODO use auto_ptr
		AsciiFilter* filter = new AsciiFilter();
	
		if ( ui.cbFilter->currentIndex()==0 ) { //"automatic"
			filter->setAutoModeEnabled(true);
		} else if ( ui.cbFilter->currentIndex()==1 ) { //"custom"
			filter->setAutoModeEnabled(false);
			filter->setCommentCharacter( asciiOptionsWidget.cbCommentCharacter->currentText() );
			filter->setSeparatingCharacter( asciiOptionsWidget.cbSeparatingCharacter->currentText() );
			filter->setSimplifyWhitespacesEnabled( asciiOptionsWidget.chbSimplifyWhitespaces->isChecked() );
			filter->setSkipEmptyParts( asciiOptionsWidget.chbSkipEmptyParts->isChecked() );
			filter->setTransposed( asciiOptionsWidget.chbTranspose->isChecked() );
			filter->setVectorNames( asciiOptionsWidget.kleVectorNames->text() );
			filter->setHeaderEnabled( asciiOptionsWidget.chbHeader->isChecked() );
		} else {
			filter->loadFilterSettings( ui.cbFilter->currentText() );
		}

		//save the data portion to import
		filter->setStartRow( ui.sbStartRow->value());
		filter->setEndRow( ui.sbEndRow->value() );
		filter->setStartColumn( ui.sbStartColumn->value());
		filter->setEndColumn( ui.sbEndColumn->value());

		return filter;
//		source->setFilter(filter);
		break;
	}
	case FileDataSource::Binary: {
		BinaryFilter* filter = new BinaryFilter();
 		if ( ui.cbFilter->currentIndex()==0 ){	//"automatic"
			filter->setAutoModeEnabled(true);
 		}else if ( ui.cbFilter->currentIndex()==1 ){ //"custom"
			filter->setAutoModeEnabled(false);
 			filter->setVectors( binaryOptionsWidget.niVectors->value() );
 			filter->setDataType( (BinaryFilter::DataType) binaryOptionsWidget.cbDataType->currentIndex() );
 		}else{
// 			filter->setFilterName( ui.cbFilter->currentText() );
 		}

		filter->setStartRow( ui.sbStartRow->value() );
		filter->setEndRow( ui.sbEndRow->value() );

//		source->setFilter(filter);
		return filter;
		break;
	}
	case FileDataSource::Image: {
		ImageFilter* filter = new ImageFilter();
 		if ( ui.cbFilter->currentIndex()==0 ){	//"automatic"
			filter->setAutoModeEnabled(true);
 		}else if ( ui.cbFilter->currentIndex()==1 ){ //"custom"
			filter->setAutoModeEnabled(false);
 		}else{
// 			filter->setFilterName( ui.cbFilter->currentText() );
		}
		filter->setImportFormat((ImageFilter::ImportFormat)imageOptionsWidget.cbImportFormat->currentIndex());
		filter->setStartRow( ui.sbStartRow->value() );
		filter->setEndRow( ui.sbEndRow->value() );
		filter->setStartColumn( ui.sbStartColumn->value() );
		filter->setEndColumn( ui.sbEndColumn->value() );
		
		return filter;
		break;
	}
	case FileDataSource::HDF: {
		HDFFilter* filter = new HDFFilter();
 		if ( ui.cbFilter->currentIndex()==0 ){	//"automatic"
			filter->setAutoModeEnabled(true);
 		}else if ( ui.cbFilter->currentIndex()==1 ){ //"custom"
			filter->setAutoModeEnabled(false);
		} else {
// 			filter->setFilterName( ui.cbFilter->currentText() );
		}
		filter->setCurrentDataSet(hdfOptionsWidget.leDataSet->text());
		filter->setStartRow( ui.sbStartRow->value() );
		filter->setEndRow( ui.sbEndRow->value() );
		filter->setStartColumn( ui.sbStartColumn->value() );
		filter->setEndColumn( ui.sbEndColumn->value() );

		return filter;
		break;
	}
	case FileDataSource::NETCDF: {
		NetCDFFilter* filter = new NetCDFFilter();
 		if ( ui.cbFilter->currentIndex()==0 ){	//"automatic"
			filter->setAutoModeEnabled(true);
 		}else if ( ui.cbFilter->currentIndex()==1 ){ //"custom"
			filter->setAutoModeEnabled(false);
		} else {
// 			filter->setFilterName( ui.cbFilter->currentText() );
		}
		filter->setCurrentVarName(netcdfOptionsWidget.leVarName->text());
		filter->setStartRow( ui.sbStartRow->value() );
		filter->setEndRow( ui.sbEndRow->value() );
		filter->setStartColumn( ui.sbStartColumn->value() );
		filter->setEndColumn( ui.sbEndColumn->value() );

		return filter;
		break;
	}
	default: 
		qDebug()<<"Unknown file type!";	
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
	if (pos!=-1) {
		QString newDir = path.left(pos);
		if (newDir!=dir)
			conf.writeEntry("LastDir", newDir);
	}

	ui.kleFileName->setText( path );

	//use the file name as the name of the data source,
	//if there is no data source name provided yet
	if (ui.kleSourceName->text().isEmpty()) {
		QString fileName=path.right( path.length()-path.lastIndexOf(QDir::separator())-1 );
		ui.kleSourceName->setText(fileName);
	}

	//TODO: decide whether the selection of several files should be possible
// 	QStringList filelist = QFileDialog::getOpenFileNames(this,i18n("Select one or more files to open"));
// 	if (! filelist.isEmpty() )
// 		ui.kleFileName->setText(filelist.join(";"));
}



//SLOTS

/*!
	called on file name changes.
	Determines the file format (ASCII, binary etc.), if the file exists,
	and activates the corresponding options.
*/
void ImportFileWidget::fileNameChanged(const QString& name) {
	QString fileName=name;
	if ( fileName.left(1)!=QDir::separator()) {
		fileName=QDir::homePath() + QDir::separator() + fileName;
	}

	bool fileExists = QFile::exists(fileName);
	ui.gbOptions->setEnabled(fileExists);
	ui.bFileInfo->setEnabled(fileExists);
	ui.cbFileType->setEnabled(fileExists);
	ui.cbFilter->setEnabled(fileExists);
	ui.bManageFilters->setEnabled(fileExists);
	ui.kleSourceName->setEnabled(fileExists);
	ui.chbWatchFile->setEnabled(fileExists);
	ui.chbLinkFile->setEnabled(fileExists);

	if ( !fileExists )
		return;

	//check, if we can guess the file type by content
	QProcess *proc = new QProcess(this);
	QStringList args;
	args<<"-b"<<ui.kleFileName->text();
	proc->start("file", args);

	QString debug;
	if ( proc->waitForReadyRead(1000) == false ) {
	    // 		kDebug()<<"ERROR: reading file type of file"<<ui.kleFileName->text()<<endl;
	} else {
		QString info = proc->readLine();
		if (info.contains("compressed data")) {
			debug="detected compressed data";
			//probably ascii data
			ui.cbFileType->setCurrentIndex(FileDataSource::Ascii);
		}
		else if (info.contains("image") || info.contains("bitmap" )) {
			debug="detected IMAGE file";
			ui.cbFileType->setCurrentIndex(FileDataSource::Image);
		} else if ( info.contains( ("ASCII") ) ) {
			debug="detected ASCII file";
			ui.cbFileType->setCurrentIndex(FileDataSource::Ascii);
		} else if (info.contains(("Hierarchical Data Format"))) {
			debug="detected HDF file";
			ui.cbFileType->setCurrentIndex(FileDataSource::HDF);

			// update HDF tree widget using current selected file
			hdfOptionsWidget.twContent->clear();
			hdfOptionsWidget.leDataSet->clear();

			QString fileName = ui.kleFileName->text();
			QFileInfo fileInfo(fileName);
			QTreeWidgetItem *rootItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<fileInfo.baseName());
			HDFFilter *filter = (HDFFilter *)this->currentFileFilter();
			filter->parse(fileName, rootItem);
			hdfOptionsWidget.twContent->insertTopLevelItem(0,rootItem);
			hdfOptionsWidget.twContent->expandAll();
			hdfOptionsWidget.twContent->resizeColumnToContents(0);
			hdfOptionsWidget.twContent->resizeColumnToContents(3);
		} else if (info.contains(("NetCDF Data Format"))) {
			debug="detected NetCDF file";
			ui.cbFileType->setCurrentIndex(FileDataSource::NETCDF);

			// update NetCDF tree widget using current selected file
			netcdfOptionsWidget.twContent->clear();
			netcdfOptionsWidget.leVarName->clear();

			QString fileName = ui.kleFileName->text();
			QFileInfo fileInfo(fileName);
			QTreeWidgetItem *rootItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<fileInfo.baseName());
			NetCDFFilter *filter = (NetCDFFilter *)this->currentFileFilter();
			filter->parse(fileName, rootItem);
			netcdfOptionsWidget.twContent->insertTopLevelItem(0,rootItem);
			netcdfOptionsWidget.twContent->expandAll();
			netcdfOptionsWidget.twContent->resizeColumnToContents(0);
			netcdfOptionsWidget.twContent->resizeColumnToContents(2);
			
		} else {
			debug="probably BINARY file";
			ui.cbFileType->setCurrentIndex(FileDataSource::Binary);
		}
	}
#ifdef QT_DEBUG
	qDebug()<<debug;
#endif

	refreshPreview();
}

/*!
  saves the current filter settings
*/
void ImportFileWidget::saveFilter(){
	bool ok;
	QString text = QInputDialog::getText(this, i18n("Save Filter Settings as"),
		i18n("Filter name:"), QLineEdit::Normal, i18n("new filter"), &ok);
	if (ok && !text.isEmpty()){
		//TODO
		//AsciiFilter::saveFilter()
	}
}

/*!
  opens a dialog for managing all available predefined filters.
*/
void ImportFileWidget::manageFilters(){
  //TODO
}

/*!
	Depending on the selected file type, activates the corresponding options in the data portion tab
	and populates the combobox with the available pre-defined fllter settings for the selected type.
*/
void ImportFileWidget::fileTypeChanged(int fileType) {
	ui.swOptions->setCurrentIndex(fileType);

	//default
	ui.lStartColumn->show();
	ui.sbStartColumn->show();
	ui.lEndColumn->show();
	ui.sbEndColumn->show();
	ui.tePreview->show();
	ui.lPreviewLines->show();
	ui.sbPreviewLines->show();

	switch (fileType) {
	case FileDataSource::Ascii: {
		break;
	}
	case FileDataSource::Binary: {
		ui.lStartColumn->hide();
		ui.sbStartColumn->hide();
		ui.lEndColumn->hide();
		ui.sbEndColumn->hide();
		break;
	}
	case FileDataSource::HDF:
	case FileDataSource::NETCDF: {
		break;
	}	
	case FileDataSource::Image: {
		ui.lPreviewLines->hide();
		ui.sbPreviewLines->hide();
		break;
	}
	default:
		qDebug()<<"unknown file type!";
	}

	int lastUsedFilterIndex = ui.cbFilter->currentIndex();
	ui.cbFilter->clear();
	ui.cbFilter->addItem( i18n("Automatic") );
	ui.cbFilter->addItem( i18n("Custom") );

	//TODO: populate the combobox with the available pre-defined filter settings for the selected type

	ui.cbFilter->setCurrentIndex(lastUsedFilterIndex);
}

/*!
	updates the selected data set of a HDF file when the tree widget item is selected
*/
void ImportFileWidget::hdfTreeWidgetItemSelected(QTreeWidgetItem* item, int column) {
	Q_UNUSED(column);
	if( item->data(2,Qt::DisplayRole).toString() == i18n("data set") ) {
		// the data link is saved in the second column
		QString dataSetLink = item->data(1,Qt::DisplayRole).toString();
		hdfOptionsWidget.leDataSet->setText(dataSetLink);
		refreshPreview();
	}
	else
		qDebug()<<"non data set selected in HDF tree widget";
}

/*!
	updates the selected var name of a NetCDF file when the tree widget item is selected
*/
void ImportFileWidget::netcdfTreeWidgetItemSelected(QTreeWidgetItem* item, int column) {
	Q_UNUSED(column);
	if( item->data(1,Qt::DisplayRole).toString() == i18n("variable") ) {
		// the data link is saved in the second column
		QString varName = item->data(0,Qt::DisplayRole).toString();
		netcdfOptionsWidget.leVarName->setText(varName);
		refreshPreview();
	}
	else
		qDebug()<<"non variable selected in NetCDF tree widget";
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
	if (index==0){// "automatic"
		ui.swOptions->setEnabled(false);
		ui.bSaveFilter->setEnabled(false);
	}else if (index==1) { //custom
		ui.swOptions->setEnabled(true);
		ui.bSaveFilter->setEnabled(true);
	}else{
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
	if (state==Qt::Checked){
	    asciiOptionsWidget.kleVectorNames->setEnabled(false);
		asciiOptionsWidget.lVectorNames->setEnabled(false);
	}else{
	    asciiOptionsWidget.kleVectorNames->setEnabled(true);
		asciiOptionsWidget.lVectorNames->setEnabled(true);
	}
}

void ImportFileWidget::refreshPreview(){
	// reset preview text edit palette if there was an image before
	ui.tePreview->setPalette(this->palette());

	QString fileName = ui.kleFileName->text();
	if ( fileName.left(1) != QDir::separator() )
	    fileName = QDir::homePath() + QDir::separator() + fileName;

	QString importedText;
	FileDataSource::FileType fileType = (FileDataSource::FileType)ui.cbFileType->currentIndex();
	int lines = ui.sbPreviewLines->value();

	switch (fileType) {
	case FileDataSource::Ascii: {
		AsciiFilter *filter = (AsciiFilter *)this->currentFileFilter();
		importedText = filter->readData(fileName,NULL,AbstractFileFilter::Replace,lines);
		break;
	}
	case FileDataSource::Binary: {
		BinaryFilter *filter = (BinaryFilter *)this->currentFileFilter();
		importedText = filter->readData(fileName,NULL,AbstractFileFilter::Replace,lines);
		break;
	}
	case FileDataSource::Image: {
		QImage image(fileName);
		QImage scaledImage = image.scaled(ui.tePreview->size());

		QPalette palette;
		palette.setBrush(QPalette::Base, QBrush(scaledImage));
		ui.tePreview->setPalette(palette);
		break;
	}
	case FileDataSource::HDF: {
		HDFFilter *filter = (HDFFilter *)this->currentFileFilter();
		importedText = filter->readCurrentDataSet(fileName,NULL,AbstractFileFilter::Replace,lines);
		break;
	}
	case FileDataSource::NETCDF: {
		NetCDFFilter *filter = (NetCDFFilter *)this->currentFileFilter();
		importedText = filter->readCurrentVar(fileName,NULL,AbstractFileFilter::Replace,lines);
		break;
	}
	default:
		importedText += "Unknown file type";
	}

	ui.tePreview->setPlainText(importedText);
}
