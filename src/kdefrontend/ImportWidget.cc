/***************************************************************************
    File                 : ImportWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : import data widget

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
#include <KDebug>
#include <KMessageBox>
#include <KFilterDev>
#include "ImportWidget.h"
#include "MainWin.h"
#include "core/Project.h"
#include "core/column/Column.h"
#include "table/Table.h"
#include "filter/FilterOPJ.h"
#include "filter/FilterHDF5.h"
#include "filter/FilterCDF.h"
#include "filter/FilterNETCDF.h"
#include "elements/importitems.h"
#include "Spreadsheet.h"

ImportWidget::ImportWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	KConfigGroup conf(KSharedConfig::openConfig(),"Import");
	ui.leFileName->setText(conf.readEntry("Filename",""));
	updateFileType();

	ui.gbOptions->hide();
	// TODO : file type selection
	ui.niStartRow->setValue(conf.readEntry("StartRow",1));
	ui.niEndRow->setValue(conf.readEntry("EndRow",0));
	ui.cbSimplifyWhitespaces->setChecked(conf.readEntry("SimplifyWhitespaces",true));
	ui.cbImportHeader->setChecked(conf.readEntry("ImportHeader",false));
	ui.cbEmptyLines->setChecked(conf.readEntry("EmptyLines",false));
	ui.cbSeparatingCharacter->setEditText(conf.readEntry("SeparatingCharacter","auto"));
	ui.cbSeparatingCharacter->insertItems(0,separatoritems);
	ui.cbCommentCharacter->setEditText(conf.readEntry("CommentCharacter","#"));
	ui.cbCommentCharacter->insertItems(0,commentitems);

/*	// Options
	samexcb = new QCheckBox(i18n("Same first column"));
	samexcb->setChecked(conf.readEntry("SameXColumn",false));
	samexcb->setVisible(false);
*/
	binaryMode=conf.readEntry("Binary",false);
	updateBinaryMode();
	ui.niBinaryFields->setValue(conf.readEntry("BinaryFields","2").toInt());
	ui.cbBinaryFormat->insertItems(0,formatitems);
	ui.cbBinaryFormat->setCurrentIndex(conf.readEntry("BinaryFormat",0));
	ui.cbBinaryByteOrder->insertItems(0,byteorderitems);
	ui.cbBinaryByteOrder->setCurrentIndex(conf.readEntry("ByteOrder",0));

	ui.cbCreateSpreadsheet->setChecked(conf.readEntry("CreateNewSpreadsheet",true));
	ui.cbUseFilename->setChecked(conf.readEntry("UseFilenameAsTitle",false));
}

ImportWidget::~ImportWidget() {
}

void ImportWidget::save() {
	KConfigGroup conf(KSharedConfig::openConfig(),"Import");

	conf.writeEntry("Filename",ui.leFileName->text());
	conf.writeEntry("CreateNewSpreadsheet",ui.cbCreateSpreadsheet->isChecked());
	conf.writeEntry("UseFilenameAsTitle",ui.cbUseFilename->isChecked());
	//TODO : file type selection

	// options
	conf.writeEntry("StartRow",ui.niStartRow->value());
	conf.writeEntry("EndRow",ui.niEndRow->value());
	conf.writeEntry("SimplifyWhitespace",ui.cbSimplifyWhitespaces->isChecked());
	conf.writeEntry("ImportHeader",ui.cbImportHeader->isChecked());
	conf.writeEntry("EmptyLines",ui.cbEmptyLines->isChecked());
	conf.writeEntry("SeparatingCharacter",ui.cbSeparatingCharacter->currentText());
	conf.writeEntry("CommentCharacter",ui.cbCommentCharacter->currentText());

	conf.writeEntry("HideOptions",ui.gbOptions->isHidden() && ui.gbBinaryOptions->isHidden());
	conf.writeEntry("Binary",binaryMode);
//TODO	conf.writeEntry("SameXColumn",samexcb->isChecked());

	conf.writeEntry("BinaryFields",ui.niBinaryFields->value());
	conf.writeEntry("BinaryFormat",ui.cbBinaryFormat->currentIndex());
	conf.writeEntry("ByteOrder",ui.cbBinaryByteOrder->currentIndex());

	conf.config()->sync();
}

void ImportWidget::selectFile() {
	QStringList filelist = QFileDialog::getOpenFileNames(this,i18n("Select one or more files to open"));
	if (! filelist.isEmpty() )
		ui.leFileName->setText(filelist.join(";"));
}

void ImportWidget::updateFileType() {
	QProcess *proc = new QProcess(this);
	QString program= "file";
	QStringList args;
	args<<"-b"<<ui.leFileName->text();
	proc->start(program,args);
	if(proc->waitForReadyRead(1000) == false)
		kDebug()<<"ERROR: reading file type of file"<<ui.leFileName->text()<<endl;
	else {
		QString info = proc->readLine().left(60);
		if(info.contains(i18n("cannot open")))
			ui.lFileType->setText("");
		else {
			info.remove(info.length()-1,1);	// remove '\n'
			kDebug()<<info<<endl;
			if(info == "data")
				binaryMode=true;
			else
				binaryMode=false;
			updateBinaryMode();
			ui.lFileType->setText(info);
		}
	}
}

void ImportWidget::fileInfoDialog() {
	kDebug()<<"OK"<<endl;
	QStringList files = ui.leFileName->text().split(";");
	 for ( int i=0; i<files.size(); i++ ) {
		QString filename = files.at(i);
		if(filename.isEmpty())
			continue;

		QIODevice *file = KFilterDev::deviceForFile(filename,QString::null,true);
		if(file==0)
			file = new QFile(filename);

		if (file->open(QIODevice::ReadOnly))
			;//TODO : (new FileInfoDialog(mw,filename))->show();
		else {
			kDebug()<<"WARNING: Could not open file"<<filename<<endl;
			// TODO : hangs here
			KMessageBox::sorry(this, i18n("Sorry. Could not open file %1 for reading!").arg(filename));
		}
	}
}

void ImportWidget::toggleOptions() {
	if(ui.gbOptions->isHidden() && ui.gbBinaryOptions->isHidden()) {
		if(binaryMode)
			ui.gbBinaryOptions->show();
		else
			ui.gbOptions->show();
		((KDialog *)parent())->setButtonText(KDialog::User2,i18n("Hide Options"));
	} else {
		if(binaryMode)
			ui.gbBinaryOptions->hide();
		else
			ui.gbOptions->hide();
		((KDialog *)parent())->setButtonText(KDialog::User2,i18n("Show Options"));
		// TODO : doesn't work
		//adjustSize();
		//updateGeometry();
		//resize(QSize(500,200));
	}
}

void ImportWidget::updateBinaryMode() {
	kDebug()<<"OK"<<endl;
	if (binaryMode) {
		if( ! ui.gbOptions->isHidden()) {
			ui.gbOptions->hide();
			ui.gbBinaryOptions->show();
		}
	}
	else {
		if( ! ui.gbBinaryOptions->isHidden()) {
			ui.gbOptions->show();
			ui.gbBinaryOptions->hide();
		}
	 }
}

int ImportWidget::startRow() const {
	return ui.niStartRow->value()-1;
}

int ImportWidget::endRow() const {
	int row = ui.niEndRow->value()-1;
	if(row == -1)
		return INT_MAX;
	return row;
}

void ImportWidget::apply(MainWin *mainWin) {
	kDebug()<<"ImportWidget::apply()"<<endl;

	QStringList files = fileNames();
	 for ( int i=0; i<files.size(); i++ ) {
		QString filename = files.at(i);
		if(filename.isEmpty())
			continue;

		// TODO : if file type selected ...
		// exit

		// automatic
		if (filename.endsWith(".opj",Qt::CaseInsensitive)) {
			importOPJ(mainWin, filename);
			return;
		}

		// open a spreadsheet
		Table *table=0;
		if( ! ui.cbCreateSpreadsheet->isChecked() && i == 0 ) {
			table=mainWin->activeTable();
		}
		if(!table)
			table=mainWin->newSpreadsheet();
		if(!table) {
			kDebug()<<"ERROR : Couldn't create spreadsheet!"<<endl;
			continue;
		}

		if(ui.cbUseFilename->isChecked())
			table->setName(filename);

		// filter using file ending
		if(filename.endsWith(".hdf",Qt::CaseInsensitive) || filename.endsWith(".h5",Qt::CaseInsensitive))
			importHDF5(mainWin,filename,table);
		else if (filename.endsWith(".nc",Qt::CaseInsensitive))
			importNETCDF(filename,table);
		else if (filename.endsWith(".cdf",Qt::CaseInsensitive))
			importCDF(filename,table);
		else {
			kDebug()<<"	Opening file"<<filename<<endl;
			QIODevice *file = KFilterDev::deviceForFile(filename,QString::null,true);
			if(file==0) {
				kDebug()<<"No device for file found. Opening as normal file."<<endl;
				file = new QFile(filename);
			}
			if (!file->open(QIODevice::ReadOnly)) {
				kDebug()<<"ERROR : Unable to open file "<<filename<<" for reading!"<<endl;
				continue;
			}

			if(binaryMode)
				importBinary(file,table);
			else
				importASCII(file,table);

			delete file;
		}
	}
}

void ImportWidget::importOPJ(MainWin *mainWin, QString filename) {
	FilterOPJ importer(mainWin, filename);
	importer.import();
}

int ImportWidget::importHDF5(MainWin *mainWin, QString filename, Table *table) {
	Q_UNUSED(mainWin);
	Q_UNUSED(table);
	kDebug()<<"ImportDialog::importHDF5("<<filename<<")"<<endl;
#ifdef HAVE_HDF5
	FilterHDF5 hdf5 = FilterHDF5(filename);
	if(!hdf5.fileOK()) {
		kDebug()<<"ERROR : unable to read HDF5 file "<<filename<<endl;
		 return -1;
	}

	hdf5.importFile();
	kDebug()<<"Reading HDF5 data"<<endl;
	kDebug()<<"number of attributes = "<<hdf5.numAttributes()<<endl;
	kDebug()<<"number of datasets = "<<hdf5.numSets()<<endl;

	// add attributes as project comments
	Project* project=0;
	QString notes;
	if(hdf5.numAttributes()>0) {
		project = mainWin->getProject();
		notes = project->comment();
	}
	for (int i=0;i<hdf5.numAttributes();i++) {
		notes.append(hdf5.getAttribute(i)+"\n");
		// kDebug()<<"	ATTRIBUTE "<<i+1<<" : "<<hdf5.getAttribute(i)<<endl;
	}
	if(hdf5.numAttributes()>0)
		project->setComment(notes);

	// read data
	for (int i=0;i<hdf5.numSets();i++) {
		if(i>0) table = mainWin->newSpreadsheet();

		int rows = hdf5.Rows(i);
		int cols = hdf5.Cols(i);
		if(rows==0) cols=0;
		kDebug()<<"Dataset "<<i+1<<" ("<<hdf5.datasetName(i)<<") has "<< rows<<" rows and "<<cols<<" cols"<<endl;
		table->setName(hdf5.datasetName(i));

		QString setnotes;
		int nrsetattr = hdf5.numSetAttributes(i);
		if(nrsetattr>0) {
			setnotes = table->comment();
			for(int j=0;j<nrsetattr;j++)
				setnotes.append(hdf5.getSetAttribute(i,j)+"\n");
			table->setComment(setnotes);
		}

		table->setRowCount(rows);
		table->setColumnCount(cols);
		for (int col=0;col<cols;col++) {
			QString colname = hdf5.columnName(i,col);
			if(colname.length()<1)
				colname = QChar(col+65);
			table->column(col)->setName(colname);

		 	for ( int j=0; j<rows; j++ )
				table->column(col)->setValueAt(j,hdf5.Data(i,j,col));
		}
	}

	return 0;
#else
	kDebug()<<"Not compiled with HDF5 support!"<<endl;
	return -2;
#endif
}

int ImportWidget::importNETCDF(QString filename, Table *table) {
	Q_UNUSED(table);
	kDebug()<<"ImportDialog::importNETCDF("<<filename<<")"<<endl;
#ifdef HAVE_NETCDF
	FilterNETCDF ncf = FilterNETCDF(filename);
	if(!ncf.fileOK()) return -1;
	kDebug()<<"Reading NETCDF data"<<endl;
	table->setColumnCount(ncf.NVars());

	kDebug()<<" nvars = "<<ncf.NVars()<<endl;
	QProgressDialog progress( i18n("Reading NETCDF data ..."), i18n("Cancel"), 0, ncf.NVars()-startRow());
	progress.setWindowModality(Qt::WindowModal);
//	s->setUpdatesEnabled(false);
	for (int j=startRow();j<ncf.NVars();j++) {
		progress.setValue(j-startRow());

		QString name = ncf.VarName(j);
		int rows = ncf.VarLen(name);
		kDebug()<<" var / varid / len = "<<name<<' '<<j<<' '<<rows<<endl;
		table->column(j-startRow())->setName(name);

		if (j==startRow() || table->rowCount()<rows)
			table->setRowCount(rows);
		for ( int i=0; i<rows; i++ )
			table->column(j)->setValueAt(i,ncf.Data(name,i));

		if (j>endRow())
			break;
	}
//	s->setUpdatesEnabled(true);
#else
	kDebug()<<"Not compiled with NETCDF support!"<<endl;
	return 1;
#endif
	return 0;
}

int ImportWidget::importCDF(QString filename, Table *table) {
	Q_UNUSED(table);
	kDebug()<<"ImportDialog::importCDF("<<filename<<")"<<endl;
#ifdef HAVE_CDF
	FilterCDF cdf = FilterCDF(filename);
	if(!cdf.fileOK()) return -1;
	kDebug()<<"Reading CDF data"<<endl;

	s->setColumnCount(cdf.NVars());
	s->setRowCount(cdf.MaxRec());

	kDebug()<<" nvars = "<<cdf.NVars()<<endl;
	QProgressDialog progress( i18n("Reading CDF data ..."), i18n("Cancel"), 0, cdf.NVars()-startRow());
	progress.setWindowModality(Qt::WindowModal);
//	s->setUpdatesEnabled(false);
	for (int j=startRow();j<cdf.NVars();j++) {
		progress.setValue(j-startRow());

		QString name = cdf.VarName(j);
		int rows = cdf.VarLen(name);
		kDebug()<<" var / varid / len = "<<name<<' '<<j<<' '<<rows<<endl;

		table->setColumnName(j-startRow(),name);

		for (int i=0;i<rows;i++)
			table->setText(i,j,QString::number(cdf.Data(name,i),'g',10));

		if (j>endRow())
			break;
	}
//	s->setUpdatesEnabled(true);
#else
	kDebug()<<"Not compiled with CDF support!"<<endl;
	return 1;
#endif
	return 0;
}


void ImportWidget::importBinary(QIODevice *file, Table *table) {
	kDebug()<<"ImportDialog::importBinary()"<<endl;
// TODO : use samex

	QDataStream ds(file);
	int byteorder=ui.cbBinaryByteOrder->currentIndex();
	ds.setByteOrder((QDataStream::ByteOrder)byteorder);
	kDebug()<<"	byte order : "<<byteorder<<" (big/little endian : "<<QDataStream::BigEndian<<"/"<<QDataStream::LittleEndian<<")"<<endl;

	int fields = ui.niBinaryFields->value();
	if(fields > table->columnCount())
		table->setColumnCount(fields);

	QProgressDialog progress( i18n("Reading binary data ..."), i18n("Cancel"), 0, file->size());
	progress.setWindowModality(Qt::WindowModal);
//	s->setUpdatesEnabled(false);
	int row=0;
	BinaryFormat format = (BinaryFormat) ui.cbBinaryFormat->currentIndex();
	for (int j=0;j<fields*startRow();j++)
		getBinaryValue(&ds,format);
	while(!ds.atEnd()) {
		if (row%50000==0) {
			progress.setValue(file->pos());
			table->setRowCount(row+50000);
			if (progress.wasCanceled()) {
				kDebug()<<"WARNING: Import canceled()"<<endl;
             			break;
			}
		}
		 for ( int i=0; i<fields; i++ )
			table->column(i)->setValueAt(row,getBinaryValue(&ds,format));

		row++;
	}
	table->setRowCount(row);
//	s->setUpdatesEnabled(true);
}

double ImportWidget::getBinaryValue(QDataStream *ds, BinaryFormat type) const {
//	kDebug()<<"Dialog::getBinaryValue() : Type = "<<type<<endl;
	double value=0;

	switch(type) {
	case BDOUBLE: {
		double var;
		*ds>>var;
		value=var;
		} break;
	case BFLOAT: {
		float var;
		*ds>>var;
		value=var;
		} break;
	case BINT8: {
		qint8 var;
		*ds>>var;
		value=var;
		} break;
	case BINT16: {
		qint16 var;
		*ds>>var;
		value=var;
		} break;
	case BINT32: {
		qint32 var;
		*ds>>var;
		value=var;
		} break;
	case BINT64: {
		qint64 var;
		*ds>>var;
		value=var;
		} break;
	}
	// kDebug()<<"	value = "<<value<<endl;

	return value;
}

void ImportWidget::importASCII(QIODevice *file, Table *table) {
	kDebug()<<"ImportDialog::importASCII()"<<endl;
	// TODO : use samex

	int row=0,actrow=0;
	QTextStream in(file);
	QProgressDialog progress( i18n("Reading ASCII data ..."), i18n("Cancel"), 0, file->size());
	progress.setWindowModality(Qt::WindowModal);
//	s->setUpdatesEnabled(false);
	while (!in.atEnd()) {
		while(row<startRow()) {
			in.readLine();
			row++;
		}
		actrow = row-startRow();
		if (ui.cbImportHeader->isChecked()) actrow--;

		if (row%100==0) {
			progress.setValue(file->pos());
			table->setRowCount(actrow+100);
			if (progress.wasCanceled()) {
				kDebug()<<"WARNING: Import canceled()"<<endl;
             			break;
			}
		}

		QString line = in.readLine();

		if(ui.cbSimplifyWhitespaces->isChecked())
			line = line.simplified();
		if(line.startsWith(ui.cbCommentCharacter->currentText()) == true)
			continue;

		QStringList oneline;
		QString sep=ui.cbSeparatingCharacter->currentText();
		if(sep == "auto")
			oneline = line.split(QRegExp("\\s+"),(QString::SplitBehavior)ui.cbEmptyLines->isChecked());
		else
			oneline = line.split(sep,(QString::SplitBehavior)ui.cbEmptyLines->isChecked());

		// handle empty lines correct
		if(oneline.count() == 0)
			continue;
		// kDebug()<<"cols="<<oneline.size()<<endl;
		if(oneline.size() > table->columnCount())
			table->setColumnCount(oneline.size());

		// import header
		if(row==startRow() && ui.cbImportHeader->isChecked()) {
	 		for ( int i=0; i<oneline.size(); i++ )
				table->column(i)->setName(oneline.at(i));
		}
		else {
// TODO : read strings (comments) or datetime too
		 	for ( int i=0; i<oneline.size(); i++ )
				//table->column(i)->setTextAt(actrow, oneline.at(i));
				table->column(i)->setValueAt(actrow, oneline.at(i).toDouble());
		}
		if(row>endRow())
			break;
		row++;
	}
	table->setRowCount(actrow+1);
//	if( ! ui.cbImportHeader->isChecked())
//		s->setUpdatesEnabled(true);
}
