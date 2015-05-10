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
#include "backend/datasources/filters/AsciiMatrixFilter.h"
#include "backend/datasources/filters/BinaryFilter.h"
//TODO #include "backend/datasources/filters/BinaryMatrixFilter.h"
#include "backend/datasources/filters/HDFFilter.h"
#include "backend/datasources/filters/ImageFilter.h"

#include <QInputDialog>
#include <QDir>
#include <QFileDialog>
#include <QProcess>
#include <QTextStream>
#include <KUrlCompletion>
#include <QDebug>

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

	// file type specific option widgets
	QWidget* asciiw=new QWidget(0);
	asciiOptionsWidget.setupUi(asciiw);
	asciiOptionsWidget.cbSeparatingCharacter->addItems(AsciiFilter::separatorCharacters());
	asciiOptionsWidget.cbCommentCharacter->addItems(AsciiFilter::commentCharacters());
	asciiOptionsWidget.chbTranspose->hide(); //TODO: enable later
	ui.swOptions->insertWidget(FileDataSource::AsciiVector, asciiw);

	QWidget* asciiMatrixw=new QWidget(0);
	asciiMatrixOptionsWidget.setupUi(asciiMatrixw);
	ui.swOptions->insertWidget(FileDataSource::AsciiMatrix, asciiMatrixw);

	QWidget* binaryw=new QWidget(0);
	binaryOptionsWidget.setupUi(binaryw);
	binaryOptionsWidget.cbDataType->addItems(BinaryFilter::dataTypes());
	binaryOptionsWidget.cbByteOrder->addItems(BinaryFilter::byteOrders());
	ui.swOptions->insertWidget(FileDataSource::BinaryVector, binaryw);

	QWidget* binaryMatrixw=new QWidget(0);
	ui.swOptions->insertWidget(FileDataSource::BinaryMatrix, binaryMatrixw);

	QWidget* imagew=new QWidget(0);
	imageOptionsWidget.setupUi(imagew);
	imageOptionsWidget.cbImportFormat->addItems(ImageFilter::importFormats());
	ui.swOptions->insertWidget(FileDataSource::Image, imagew);

	QWidget* hdfw=new QWidget(0);
	hdfOptionsWidget.setupUi(hdfw);
	QStringList headers;
	headers<<i18n("Name")<<i18n("Link")<<i18n("Type")<<i18n("Properties")<<i18n("Attributes");
	hdfOptionsWidget.twContent->setHeaderLabels(headers);
	ui.swOptions->insertWidget(FileDataSource::HDF, hdfw);

	//TODO: add widgets for other file types

	// default filter
	ui.swOptions->setCurrentIndex(FileDataSource::AsciiVector);
	// disabled for the moment
	ui.cbFileType->setItemData(FileDataSource::AsciiMatrix, 0, Qt::UserRole - 1);
	ui.cbFileType->setItemData(FileDataSource::BinaryMatrix, 0, Qt::UserRole - 1);
	//ui.cbFileType->setItemData(FileDataSource::Image, 0, Qt::UserRole - 1);

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

	//general settings
	ui.kleFileName->setText(conf.readEntry("LastImportedFile", ""));
	ui.cbFileType->setCurrentIndex(conf.readEntry("Type", 0));
	ui.cbFilter->setCurrentIndex(conf.readEntry("Filter", 0));

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

	//TODO: other file types

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

	//TODO: HDF data
	//TODO: other file types
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

	if ( fileType==FileDataSource::AsciiVector ) {
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
		filter->setStartRow( ui.sbStartRow->value()-1 );
		if (ui.sbEndRow->value()==-1)
			filter->setEndRow(-1);
		else
			filter->setEndRow( ui.sbEndRow->value() );

		filter->setStartColumn( ui.sbStartColumn->value()-1 );
		if (ui.sbEndColumn->value() == -1)
			filter->setEndColumn(-1);
		else
			filter->setEndColumn( ui.sbEndColumn->value()-1 );

		return filter;
//		source->setFilter(filter);
	} else if ( fileType==FileDataSource::AsciiMatrix ) {
		AsciiMatrixFilter* filter = new AsciiMatrixFilter();
		return filter;
	} else if ( fileType==FileDataSource::BinaryVector ) {
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
	} else if ( fileType==FileDataSource::BinaryMatrix ) {
		//TODO
		//BinaryMatrixFilter* filter = new BinaryMatrixFilter();
		//return filter;
	} else if ( fileType==FileDataSource::Image ) {
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
	} else if ( fileType==FileDataSource::HDF ) {
		HDFFilter* filter = new HDFFilter();
 		if ( ui.cbFilter->currentIndex()==0 ){	//"automatic"
			filter->setAutoModeEnabled(true);
 		}else if ( ui.cbFilter->currentIndex()==1 ){ //"custom"
			filter->setAutoModeEnabled(false);
		} else {
			//TODO
		}

		//TODO
		return filter;
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

	if ( proc->waitForReadyRead(1000) == false ) {
	    // 		kDebug()<<"ERROR: reading file type of file"<<ui.kleFileName->text()<<endl;
	} else {
		QString info = proc->readLine();
		if (info.contains("image") || info.contains("bitmap" )) {
#ifdef QT_DEBUG
			qDebug()<<"detected IMAGE file";
#endif
			ui.cbFileType->setCurrentIndex(FileDataSource::Image);

			//TODO: update image preview
		} else if ( info.contains( ("ASCII") ) ) {
#ifdef QT_DEBUG
			qDebug()<<"detected ASCII file";
#endif
			ui.cbFileType->setCurrentIndex(FileDataSource::AsciiVector);
		} else if (info.contains(("Hierarchical Data Format"))) {
#ifdef QT_DEBUG
			qDebug()<<"detected HDF file";
#endif
			ui.cbFileType->setCurrentIndex(FileDataSource::HDF);

			// update HDF tree widget using current selected file
			hdfOptionsWidget.twContent->clear();
			QString fileName = ui.kleFileName->text();
			QFileInfo fileInfo(fileName);
			QTreeWidgetItem *rootItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<fileInfo.baseName());
			HDFFilter *filter = (HDFFilter *)this->currentFileFilter();
			filter->parse(fileName, rootItem);
			hdfOptionsWidget.twContent->insertTopLevelItem(0,rootItem);
			hdfOptionsWidget.twContent->expandAll();
			hdfOptionsWidget.twContent->resizeColumnToContents(0);
			// link and type column is filled but we dont need to show it
			//hdfOptionsWidget.twContent->resizeColumnToContents(1);
			//hdfOptionsWidget.twContent->resizeColumnToContents(2);
			hdfOptionsWidget.twContent->hideColumn(1);
			hdfOptionsWidget.twContent->hideColumn(2);
			hdfOptionsWidget.twContent->resizeColumnToContents(3);
		} else {
#ifdef QT_DEBUG
			qDebug()<<"probably BINARY file";
#endif
			ui.cbFileType->setCurrentIndex(FileDataSource::BinaryVector);
		}
	}
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

	//FileDataSource::FileType fileType = (FileDataSource::FileType)ui.cbFileType->currentIndex();
	if (fileType == FileDataSource::AsciiVector || fileType == FileDataSource::AsciiMatrix) {
		ui.lStartColumn->show();
		ui.sbStartColumn->show();
		ui.lEndColumn->show();
		ui.sbEndColumn->show();
	}
	else if (fileType == FileDataSource::BinaryVector || fileType == FileDataSource::BinaryMatrix) {
		ui.lStartColumn->hide();
		ui.sbStartColumn->hide();
		ui.lEndColumn->hide();
		ui.sbEndColumn->hide();
	}
	else if (fileType == FileDataSource::HDF) {
		ui.lStartColumn->show();
		ui.sbStartColumn->show();
		ui.lEndColumn->show();
		ui.sbEndColumn->show();

	}	

	int lastUsedFilterIndex = ui.cbFilter->currentIndex();
	ui.cbFilter->clear();
	ui.cbFilter->addItem( i18n("Automatic") );
	ui.cbFilter->addItem( i18n("Custom") );

	//TODO: populate the combobox with the available pre-defined filter settings for the selected type

	ui.cbFilter->setCurrentIndex(lastUsedFilterIndex);
}

/*!
	updates the data set of a HDF file when the tree widget item is selected
*/
void ImportFileWidget::hdfTreeWidgetItemSelected(QTreeWidgetItem* item,int column) {
	Q_UNUSED(column);
	if( item->data(2,Qt::DisplayRole).toString() == i18n("data set") ) {
		// the data link is saved in the second column
		QString dataSetLink = item->data(1,Qt::DisplayRole).toString();
		hdfOptionsWidget.leDataSet->setText(dataSetLink);
	} else
		qDebug()<<"non data set selected in HDF tree widget";
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
	QString fileName = ui.kleFileName->text();
	if ( fileName.left(1) != QDir::separator() )
	    fileName = QDir::homePath() + QDir::separator() + fileName;

	QFile file(fileName);
	QString importedText;
	if ( file.open(QFile::ReadOnly)){
		FileDataSource::FileType fileType = (FileDataSource::FileType)ui.cbFileType->currentIndex();
		int lines = ui.sbPreviewLines->value();

		if (fileType == FileDataSource::AsciiVector || fileType == FileDataSource::AsciiMatrix) {
			QTextStream in(&file);
			for (int i=0; i<lines; ++i){
				if( in.atEnd() )
					break;

				importedText += in.readLine();
				importedText += '\n';
			}
		}
		else if (fileType == FileDataSource::BinaryVector || fileType == FileDataSource::BinaryMatrix) {
			QDataStream in(&file);

			BinaryFilter::ByteOrder byteOrder = (BinaryFilter::ByteOrder) binaryOptionsWidget.cbByteOrder->currentIndex();
			if (byteOrder == BinaryFilter::BigEndian)
				in.setByteOrder(QDataStream::BigEndian);
			else if (byteOrder == BinaryFilter::LittleEndian)
				in.setByteOrder(QDataStream::LittleEndian);

			int vectors = binaryOptionsWidget.niVectors->value();
			BinaryFilter::DataType type = (BinaryFilter::DataType) binaryOptionsWidget.cbDataType->currentIndex();
			//qDebug() <<" vectors ="<<vectors<<"  type ="<<type;

			// TODO: this is also done in BinaryFilter: reuse it

			// skip at start
			for (int i=0; i<binaryOptionsWidget.sbSkipStartBytes->value(); ++i){
				qint8 tmp;
				in >> tmp;
			}

			// skip until start row
			for (int i=0; i<(ui.sbStartRow->value()-1)*vectors; ++i){
				for(int j=0;j<BinaryFilter::dataSize(type);++j) {
					qint8 tmp;
					in >> tmp;
				}
			}

			for (int i=0; i<lines; ++i){
				if( in.atEnd() )
					break;

				for(int j=0;j < vectors; ++j) {
					switch(type) {
					case BinaryFilter::INT8: {
						qint8 tmp;
						in >> tmp;
						importedText += QString::number(tmp);
						break;
					}
					case BinaryFilter::INT16: {
						qint16 tmp;
						in >> tmp;
						importedText += QString::number(tmp);
						break;
					}
					case BinaryFilter::INT32: {
						qint32 tmp;
						in >> tmp;
						importedText += QString::number(tmp);
						break;
					}
					case BinaryFilter::INT64: {
						qint64 tmp;
						in >> tmp;
						importedText += QString::number(tmp);
						break;
					}
					case BinaryFilter::UINT8: {
						quint8 tmp;
						in >> tmp;
						importedText += QString::number(tmp);
						break;
					}
					case BinaryFilter::UINT16: {
						quint16 tmp;
						in >> tmp;
						importedText += QString::number(tmp);
						break;
					}
					case BinaryFilter::UINT32: {
						quint32 tmp;
						in >> tmp;
						importedText += QString::number(tmp);
						break;
					}
					case BinaryFilter::UINT64: {
						quint64 tmp;
						in >> tmp;
						importedText += QString::number(tmp);
						break;
					}
					case BinaryFilter::REAL32: {
						float tmp;
						in >> tmp;
						importedText += QString::number(tmp);
						break;
					}
					case BinaryFilter::REAL64: {
						double tmp;
						in >> tmp;
						importedText += QString::number(tmp);
						break;
					}
					}
					importedText += ' ';
					
					// skip after each value
					for (int i=0; i<binaryOptionsWidget.sbSkipBytes->value(); ++i){
						qint8 tmp;
						in >> tmp;
					}
				}
				importedText += '\n';
			}
		}
		else if (fileType == FileDataSource::Image) {
			//TODO
		}
		else if (fileType == FileDataSource::HDF) {
			// read data from selected data set
			QString dataSetName = hdfOptionsWidget.leDataSet->text();
			HDFFilter *filter = (HDFFilter *)this->currentFileFilter();
			importedText = filter->readDataSet(fileName,dataSetName);
		}
	}

	ui.tePreview->setPlainText(importedText);
}
