/***************************************************************************
    File                 : ImportFileWidget.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2009 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    Copyright            : (C) 2009-2012 Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses)
    Description          : import file data widget

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
//#include "backend/datasources/filters/BinaryFilter.h"

#include <QtGui>
#include <KUrlCompletion>

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

    QWidget* w1=new QWidget(0);
    asciiOptionsWidget.setupUi(w1);
    asciiOptionsWidget.cbSeparatingCharacter->addItems(AsciiFilter::separatorCharacters());
    asciiOptionsWidget.cbCommentCharacter->addItems(AsciiFilter::commentCharacters());
	asciiOptionsWidget.chbTranspose->hide(); //TODO: enable later
    ui.swOptions->insertWidget(0, w1);

    QWidget* w2=new QWidget(0);
    binaryOptionsWidget.setupUi(w2);
    ui.swOptions->insertWidget(1, w2);

    ui.swOptions->setCurrentIndex(0);

	ui.gbOptions->hide();
	
    ui.bOpen->setIcon( KIcon("document-open") );
    ui.bFileInfo->setIcon( KIcon("help-about") );
    ui.bManageFilters->setIcon( KIcon("configure") );
	ui.bSaveFilter->setIcon( KIcon("document-save") );
	ui.bRefreshPreview->setIcon( KIcon("view-refresh") );

    connect( ui.kleFileName, SIGNAL(textChanged (const QString&)), SLOT(fileNameChanged(const QString&)) );
    connect( ui.bOpen, SIGNAL(clicked()), this, SLOT (selectFile()) );
    connect( ui.bFileInfo, SIGNAL(clicked()), this, SLOT (fileInfoDialog()) );
	connect( ui.bSaveFilter, SIGNAL(clicked()), this, SLOT (saveFilter()) );
	connect( ui.bManageFilters, SIGNAL(clicked()), this, SLOT (manageFilters()) );
    connect( ui.cbFileType, SIGNAL(activated(int)), SLOT(fileTypeChanged(int)) );
    connect( ui.cbFilter, SIGNAL(activated(int)), SLOT(filterChanged(int)) );
	connect( ui.bRefreshPreview, SIGNAL(clicked()), SLOT(refreshPreview()) );
    connect( asciiOptionsWidget.chbHeader, SIGNAL(stateChanged(int)), SLOT(headerChanged(int)) );

	KConfigGroup conf(KSharedConfig::openConfig(),"Import");
	ui.kleFileName->setText(conf.readEntry("LastImportedFile", ""));
	ui.cbFileType->setCurrentIndex(conf.readEntry("Type", 0));
	ui.cbFilter->setCurrentIndex(conf.readEntry("Filter", 0));

	filterChanged(ui.cbFilter->currentIndex());
}

ImportFileWidget::~ImportFileWidget() {
    //save the last selected file to KConfigGroup
    KConfigGroup conf(KSharedConfig::openConfig(),"Import");
    conf.writeEntry("LastImportedFile", ui.kleFileName->text());
	conf.writeEntry("Type", ui.cbFileType->currentIndex());
	conf.writeEntry("Filter", ui.cbFilter->currentIndex());
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
//         source->setFilter(filter);
	}else if ( fileType==FileDataSource::BinaryVector ) {
	  //TODO
// 		BinaryFilter filter;
// 		if ( ui.cbFilter->currentIndex()==0 ){	//"automatic"
// 			filter.setAutoMode(true);
// 		}else if ( ui.cbFilter->currentIndex()==1 ){ //"custom"
// 			filter.setNumberOfVectors( binaryOptionsWidget.niVectors->value() );
// 			filter.setFormat( binaryOptionsWidget.cbFormat->currentText() );
// 		}else{
// 			filter.setFilterName( ui.cbFilter->currentText() );
// 		}
// 		source->setFilter(filter);
    }
	return 0;
}


/*!
	opens a file dialog and lets the user select the file data source.
*/
void ImportFileWidget::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportFileWidget");
	QString dir = conf.readEntry("LastDir", "");
    QString path = QFileDialog::getOpenFileName(this, i18n("Select the file data source"), dir);
    if (path=="")
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
    if (ui.kleSourceName->text()=="") {
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

	//check, whether the file has ascii or binary contant
    QProcess *proc = new QProcess(this);
    QStringList args;
    args<<"-b"<<ui.kleFileName->text();
    proc->start("file", args);

    if ( proc->waitForReadyRead(1000) == false ) {
        // 		kDebug()<<"ERROR: reading file type of file"<<ui.kleFileName->text()<<endl;
    } else {
        QString info = proc->readLine();
        //TODO
        if ( info.contains( ("ASCII") ) ) {
            //select "ASCII vector data"
            this->fileTypeChanged(0);
        } else {
            this->fileTypeChanged(2);
        }
    }
}

/*!
  saves the current filter settings
*/
void ImportFileWidget::saveFilter(){
  bool ok;
  QString text = QInputDialog::getText(this, i18n("Save filter settings as"),
												i18n("Filter name"), QLineEdit::Normal,
												i18n("new filter"), &ok);
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
	Depending on the selected file type, activates the corresponding options
	and populates the combobox with the available pre-defined fllter settings for the selected type.
*/
void ImportFileWidget::fileTypeChanged(int id) {
    ui.swOptions->setCurrentIndex(id);

    ui.cbFilter->clear();
    ui.cbFilter->addItem( i18n("Automatic") );
    ui.cbFilter->addItem( i18n("Custom") );

    //TODO: populate the combobox with the available pre-defined flter settings for the selected type
    FileDataSource::FileType fileType = (FileDataSource::FileType)ui.cbFileType->currentIndex();
	if (fileType==FileDataSource::AsciiVector || fileType==FileDataSource::AsciiMatrix){
	  ui.swOptions->setCurrentIndex(0);

	}else if (fileType==FileDataSource::BinaryVector || fileType==FileDataSource::BinaryMatrix){
	  ui.swOptions->setCurrentIndex(1);
    }
}

/*!
	shows the dialog with the information about the file(s) to be imported.
*/
void ImportFileWidget::fileInfoDialog() {
    QStringList files = ui.kleFileName->text().split(";");
    FileInfoDialog* dlg = new FileInfoDialog(this);
    dlg->setFiles(files);
    dlg->exec();
}

/*!
	enables the options if the filter "custom" was choosen. Disables the options otherwise.
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
		QTextStream stream(&file);
		int lines = ui.sbPreviewLines->value();
		for (int i=0; i<lines; ++i){
			if( stream.atEnd() )
				break;
			
			importedText += stream.readLine();
			importedText += "\n";
		}
	}

	ui.tePreview->setPlainText(importedText);
}
