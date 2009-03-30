#include <KDebug>
#include <KMessageBox>
#include <KFilterDev>
#include "ExportWidget.h"
#include "MainWin.h"
#include "filter/FilterOPJ.h"
#include "filter/FilterHDF5.h"
#include "filter/FilterCDF.h"
#include "filter/FilterNETCDF.h"
#include "elements/export.h"

ExportWidget::ExportWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	KConfigGroup conf(KSharedConfig::openConfig(),"Export");
	ui.leFileName->setText(conf.readEntry("Filename",""));
	updateSelectedFormat();

	ui.cbFileType->insertItems(0,exportitems);
	ui.cbFileType->setCurrentIndex(conf.readEntry("Filetype",0));

	ui.gbOptions->hide();
/*	ui.niStartRow->setValue(conf.readEntry("StartRow",1));
	ui.niEndRow->setValue(conf.readEntry("EndRow",0));
	ui.cbSimplifyWhitespaces->setChecked(conf.readEntry("SimplifyWhitespaces",true));
	ui.cbImportHeader->setChecked(conf.readEntry("ImportHeader",false));
	ui.cbEmptyLines->setChecked(conf.readEntry("EmptyLines",false));
	ui.cbSeparatingCharacter->setEditText(conf.readEntry("SeparatingCharacter","auto"));
	ui.cbSeparatingCharacter->insertItems(0,separatoritems);
	ui.cbCommentCharacter->setEditText(conf.readEntry("CommentCharacter","#"));
	ui.cbCommentCharacter->insertItems(0,commentitems);
*/
/*	// Options
	samexcb = new QCheckBox(i18n("Same first column"));
	samexcb->setChecked(conf.readEntry("SameXColumn",false));
	samexcb->setVisible(false);
*/
	ui.gbBinaryOptions->hide();
/*	binaryMode=conf.readEntry("Binary",false);
	updateBinaryMode();
	ui.niBinaryFields->setValue(conf.readEntry("BinaryFields","2").toInt());
	ui.cbBinaryFormat->insertItems(0,formatitems);
	ui.cbBinaryFormat->setCurrentIndex(conf.readEntry("BinaryFormat",0));
	ui.cbBinaryByteOrder->insertItems(0,byteorderitems);
	ui.cbBinaryByteOrder->setCurrentIndex(conf.readEntry("ByteOrder",0));
*/
}

ExportWidget::~ExportWidget() {
}

void ExportWidget::save() {
	KConfigGroup conf(KSharedConfig::openConfig(),"Export");
	
	conf.writeEntry("Filename",ui.leFileName->text());
	conf.writeEntry("Filetype",ui.cbFileType->currentIndex());

/*	// options
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
*/
	conf.config()->sync();
}


void ExportWidget::selectFile() {
	QString file = QFileDialog::getSaveFileName(this,i18n("Select one or more files to open"));
	ui.leFileName->setText(file);
}

/*update export file type*/
void ExportWidget::updateSelectedFormat() {
	//TODO
	//get ending	
	//switch()	-> set selection
}

/*
void ExportWidget::toggleOptions() {
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

void ExportWidget::updateBinaryMode() {
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

int ExportWidget::startRow() const {
	return ui.niStartRow->value()-1;
}

int ExportWidget::endRow() const {
	int row = ui.niEndRow->value()-1;
	if(row == -1)
		return INT_MAX;
	return row;
}
*/
void ExportWidget::apply() {
	kDebug()<<endl;

	QString filename = ui.leFileName->text();
	if(filename.isEmpty())
		return;
	
	// check if file exists
	if ( QFile::exists(filename) ) {
		int answer = KMessageBox::warningYesNoCancel( this,
			i18n( "Overwrite\n\'%1\'?" ).arg( filename ), i18n("Export data"));
		if (answer != KMessageBox::Yes)
			return;
		else {
			// delete it (needed for CDF)
			QFile::remove(filename);
		}
	}

	QIODevice *file=0;
	QTextStream t;
	QDataStream d;
	int format = ui.cbFileType->currentIndex();
	if(format == EASCII || format == EBINARY) {
		file = KFilterDev::deviceForFile(filename,QString::null,true);
		if(file == 0) file = new QFile(filename);

		if (! file->open( QIODevice::WriteOnly )) {
			KMessageBox::error(this, i18n("Sorry. Could not open file for writing!"));
			return;
		}

		if(format == EASCII)
			t.setDevice(file);
		else if (format == EBINARY)
			d.setDevice(file);
	}

/*	switch (exportcb->currentItem()) {
	case EASCII: 	dumpASCII(&t, sep); break;
	case ECDF: dumpCDF(filename); break;
	case ENETCDF: dumpNETCDF(filename); break;
	case EAUDIO: 	dumpAUDIOFILE(filename); break;
	case EIMAGE: dumpIMAGE(filename); break;
	case EBINARY: dumpBINARY(&d); break;
	case EKEXIDB: dumpKexiDB(); break;
	case EHDF5: dumpHDF5(filename); break;
	}
*/

	// TODO
	if ( ui.cbFileType->currentIndex() == EAUTO) {
		if (filename.endsWith(".dat",Qt::CaseInsensitive) ||
			filename.endsWith(".txt",Qt::CaseInsensitive) )
				exportASCII(&t);
		// TODO : check for other file ending
	}
	else {
		// TODO : implement all formats
		switch(ui.cbFileType->currentIndex()) {
		case EASCII: 
			kDebug()<<"ASCII selected"<<endl;
			exportASCII(&t);
			break;
		case ECDF: break;
		case EAUDIO: break;
		case EIMAGE: break;
		case EBINARY: break;
		case EDATABASE: break;
		case EHDF5: break;
		default: break;
		}
	}
	
	if(file) file->close();
}
/*
void ExportWidget::importOPJ(MainWin *mainWin, QString filename) {
	FilterOPJ importer(mainWin, filename);
	importer.import();
}

int ExportWidget::importHDF5(MainWin *mainWin, QString filename, Spreadsheet *s) {
	Q_UNUSED(mainWin);
	Q_UNUSED(s);
	kDebug()<<"ExportDialog::importHDF5("<<filename<<")"<<endl;
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
		notes = project->notes();
	}
	for (int i=0;i<hdf5.numAttributes();i++) {
		notes.append(hdf5.getAttribute(i)+"\n");
		// kDebug()<<"	ATTRIBUTE "<<i+1<<" : "<<hdf5.getAttribute(i)<<endl;
	}
	if(hdf5.numAttributes()>0)
		project->setNotes(notes);

	// read data
	for (int i=0;i<hdf5.numSets();i++) {
		if(i>0) s = mainWin->newSpreadsheet();

		int rows = hdf5.Rows(i);
		int cols = hdf5.Cols(i);
		if(rows==0) cols=0;
		kDebug()<<"Dataset "<<i+1<<" ("<<hdf5.datasetName(i)<<") has "<< rows<<" rows and "<<cols<<" cols"<<endl;
		s->setTitle(hdf5.datasetName(i));

		// spreadsheet notes
		QString setnotes;
		int nrsetattr = hdf5.numSetAttributes(i);
		if(nrsetattr>0) {
			setnotes = s->Notes();
			for(int j=0;j<nrsetattr;j++)
				setnotes.append(hdf5.getSetAttribute(i,j)+"\n");
			s->setNotes(setnotes);
		}

		s->setRowCount(rows);
		s->setColumnCount(cols);
		s->resetHeader();
		for (int col=0;col<cols;col++) {
			QString colname = hdf5.columnName(i,col);
			if(colname.length()<1)
				colname = QChar(col+65);
			s->setColumnName(col,colname);

		 	for ( int j=0; j<rows; j++ )
				s->setText(j,col,QString::number(hdf5.Data(i,j,col)));
		}
	}

	return 0;
#else
	kDebug()<<"Not compiled with HDF5 support!"<<endl;
	return -2;
#endif	
}

int ExportWidget::importNETCDF(QString filename, Spreadsheet *s) {
	Q_UNUSED(s);
	kDebug()<<"ExportDialog::importNETCDF("<<filename<<")"<<endl;
#ifdef HAVE_NETCDF
	FilterNETCDF ncf = FilterNETCDF(filename);
	if(!ncf.fileOK()) return -1;
	kDebug()<<"Reading NETCDF data"<<endl;
	s->setColumnCount(ncf.NVars());
	s->resetHeader();

	kDebug()<<" nvars = "<<ncf.NVars()<<endl;
	QProgressDialog progress( i18n("Reading NETCDF data ..."), i18n("Cancel"), 0, ncf.NVars()-startRow());
	progress.setWindowModality(Qt::WindowModal);
	s->setUpdatesEnabled(false);
	for (int j=startRow();j<ncf.NVars();j++) {
		progress.setValue(j-startRow());

		QString name = ncf.VarName(j);
		int rows = ncf.VarLen(name);
		kDebug()<<" var / varid / len = "<<name<<' '<<j<<' '<<rows<<endl;
		s->setColumnName(j-startRow(),name);

		if (j==startRow() || s->rowCount()<rows)
			s->setRowCount(rows);
		for ( int i=0; i<rows; i++ )
			s->setText(i,j,QString::number(ncf.Data(name,i),'g',10));

		if (j>endRow())
			break;
	}
	s->setUpdatesEnabled(true);
#else
	kDebug()<<"Not compiled with NETCDF support!"<<endl;
	return 1;
#endif
	return 0;
}

int ExportWidget::importCDF(QString filename, Spreadsheet *s) {
	Q_UNUSED(s);
	kDebug()<<"ExportDialog::importCDF("<<filename<<")"<<endl;
#ifdef HAVE_CDF
	FilterCDF cdf = FilterCDF(filename);
	if(!cdf.fileOK()) return -1;
	kDebug()<<"Reading CDF data"<<endl;

	s->setColumnCount(cdf.NVars());
	s->setRowCount(cdf.MaxRec());
	s->resetHeader();

	kDebug()<<" nvars = "<<cdf.NVars()<<endl;
	QProgressDialog progress( i18n("Reading CDF data ..."), i18n("Cancel"), 0, cdf.NVars()-startRow());
	progress.setWindowModality(Qt::WindowModal);
	s->setUpdatesEnabled(false);
	for (int j=startRow();j<cdf.NVars();j++) {
		progress.setValue(j-startRow());

		QString name = cdf.VarName(j);
		int rows = cdf.VarLen(name);
		kDebug()<<" var / varid / len = "<<name<<' '<<j<<' '<<rows<<endl;

		s->setColumnName(j-startRow(),name);

		for (int i=0;i<rows;i++)
			s->setText(i,j,QString::number(cdf.Data(name,i),'g',10));

		if (j>endRow())
			break;
	}
	s->setUpdatesEnabled(true);
#else
	kDebug()<<"Not compiled with CDF support!"<<endl;
	return 1;
#endif
	return 0;
}


void ExportWidget::importBinary(QIODevice *file, Spreadsheet *s) {
	kDebug()<<"ExportDialog::importBinary()"<<endl;
// TODO : use samex

	QDataStream ds(file);
	int byteorder=ui.cbBinaryByteOrder->currentIndex();
	ds.setByteOrder((QDataStream::ByteOrder)byteorder);
	kDebug()<<"	byte order : "<<byteorder<<" (big/little endian : "<<QDataStream::BigEndian<<"/"<<QDataStream::LittleEndian<<")"<<endl;

	int fields = ui.niBinaryFields->value();
	if(fields > s->columnCount())
		s->setColumnCount(fields);

	QProgressDialog progress( i18n("Reading binary data ..."), i18n("Cancel"), 0, file->size());
	progress.setWindowModality(Qt::WindowModal);
	s->setUpdatesEnabled(false);
	int row=0;
	BinaryFormat format = (BinaryFormat) ui.cbBinaryFormat->currentIndex();
	for (int j=0;j<fields*startRow();j++)
		getBinaryValue(&ds,format);
	while(!ds.atEnd()) {
		if (row%50000==0) {
			progress.setValue(file->pos());
			s->setRowCount(row+50000);
			if (progress.wasCanceled()) {
				kDebug()<<"WARNING: Export canceled()"<<endl;
             			break;
			}
		}
		 for ( int i=0; i<fields; i++ )
			s->setText(row, i, QString::number(getBinaryValue(&ds,format),'g',10));

		row++;
	}
	s->setRowCount(row);
	s->setUpdatesEnabled(true);
}

double ExportWidget::getBinaryValue(QDataStream *ds, BinaryFormat type) const {
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
*/

void ExportWidget::exportASCII(QTextStream *t) {
	kDebug()<<"not implemented yet!"<<endl;

	//TODO
}

