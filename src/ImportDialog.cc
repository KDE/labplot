//LabPlot : ImportDialog.cc

#include <KPushButton>
#include <KMessageBox>
#include <KSharedConfig>
#include <QProcess>
#include <QFile>
#include <QFileDialog>
#include <QProgressDialog>
#include <KFilterDev>

#include "ImportDialog.h"
#include "FileInfoDialog.h"
#include "FilterCDF.h"
#include "FilterHDF5.h"
#include "FilterNETCDF.h"
#include "FilterOPJ.h"
#include "Project.h"
#include "importitems.h"

// Dialog for importing data into spreadsheet
ImportDialog::ImportDialog(MainWin *mw)
	: Dialog(mw)
{
	kdDebug()<<"ImportDialog()"<<endl;
	setCaption(i18n("Import Data"));

	setupGUI();

	updateFileInfo();
	QObject::connect(this,SIGNAL(applyClicked()),SLOT(Apply()));
	QObject::connect(this,SIGNAL(okClicked()),SLOT(Apply()));
	QObject::connect(this,SIGNAL(user1Clicked()),SLOT(saveSettings()));
	QObject::connect(this,SIGNAL(user2Clicked()),SLOT(toggleOptions()));
}

void ImportDialog::setupGUI() {
	kdDebug()<<"ImportDialog::setupGUI()"<<endl;
	KConfigGroup conf(KSharedConfig::openConfig(),"Import");

	// Data Import
	QGroupBox *filegb = new QGroupBox(i18n("Select file"));
	QGridLayout *grid = new QGridLayout;
	filegb->setLayout(grid);
	layout->addWidget(filegb,0,0,1,2);

	filele = new KLineEdit(conf.readEntry("Filename",i18n("file.dat")));
	QObject::connect(filele,SIGNAL(textChanged(const QString &)),SLOT(updateFileInfo()));
	grid->addWidget(filele,0,0);
	KPushButton *filepb = new KPushButton(i18n("Browse"));
	QObject::connect(filepb,SIGNAL(clicked()),SLOT(selectFile()));
	grid->addWidget(filepb,0,1);

	filel = new QLabel;
	grid->addWidget(filel,1,0);
	KPushButton *fileinfopb = new KPushButton(i18n("File info"));
	QObject::connect(fileinfopb,SIGNAL(clicked()),SLOT(fileInfoDialog()));
	grid->addWidget(fileinfopb,1,1);
	
	// Options
	optionsgb = new QGroupBox(i18n("Options"));
	grid = new QGridLayout;
	optionsgb->setLayout(grid);
	layout->addWidget(optionsgb,1,0,1,2);
	if(conf.readEntry("HideOptions",true))
		optionsgb->hide();
	else
		setButtonText(KDialog::User2,i18n("Hide options"));

	binarycb = new QCheckBox(i18n("Binary data"));
	binarycb->setChecked(conf.readEntry("Binary",false));
	QObject::connect(binarycb,SIGNAL(toggled(bool)),SLOT(toggleBinary(bool)));
	grid->addWidget(binarycb,0,0);

	grid->addWidget(new QLabel(i18n("Start row :")),1,0);
	startle = new KLineEdit(conf.readEntry("StartRow",""));
	QIntValidator *validator = new QIntValidator(this);
	validator->setBottom(1);
	startle->setValidator(validator);
	grid->addWidget(startle,1,1);
	grid->addWidget(new QLabel("End row :"),1,2);
	endle = new KLineEdit(conf.readEntry("EndRow",""));
	endle->setValidator(validator);
	grid->addWidget(endle,1,3);
	simplifycb = new QCheckBox(i18n("Simplify whitespaces"));
	simplifycb->setChecked(conf.readEntry("SimplifyWhitespace",true));
	grid->addWidget(simplifycb,2,0,1,2);
	separatinglabel = new QLabel(i18n("Separating character :"));
	grid->addWidget(separatinglabel,2,2);
	separatingcb = new KComboBox;
	separatingcb->insertItems(0,separatoritems);
	separatingcb->setEditable(true);
	separatingcb->setEditText(conf.readEntry("SeparatingCharacter","auto"));
	grid->addWidget(separatingcb,2,3);
	emptycb = new QCheckBox(i18n("Allow empty entries"));
	emptycb->setChecked(conf.readEntry("EmptyEntries",false));
	grid->addWidget(emptycb,3,0,1,2);
	commentlabel = new QLabel(i18n("Comment character :"));
	grid->addWidget(commentlabel,3,2);
	commentcb = new KComboBox;
	commentcb->setEditable(true);
	commentcb->insertItems(0,commentitems);
	commentcb->setEditText(conf.readEntry("CommentCharacter","#"));
	grid->addWidget(commentcb,3,3);
	headercb = new QCheckBox(i18n("Import header"));
	headercb->setChecked(conf.readEntry("ImportHeader",false));
	grid->addWidget(headercb,4,0,1,2);
	samexcb = new QCheckBox(i18n("Same first column"));
	samexcb->setChecked(conf.readEntry("SameXColumn",false));
	//TODO
	samexcb->setVisible(false);
	grid->addWidget(samexcb,4,2);

	// binary options
	fieldslabel = new QLabel(i18n("Fields :"));
	grid->addWidget(fieldslabel,2,0);
	fieldsle = new KLineEdit(conf.readEntry("BinaryFields","2"));
	validator = new QIntValidator(this);
	validator->setBottom(1);
	fieldsle->setValidator(validator);
	grid->addWidget(fieldsle,2,1);
	formatlabel = new QLabel(i18n("Format :"));
	grid->addWidget(formatlabel,2,2);
	formatcb = new KComboBox;
	formatcb->insertItems(0,formatitems);
	formatcb->setCurrentIndex(conf.readEntry("BinaryFormat",0));
	grid->addWidget(formatcb,2,3);
	byteorderlabel = new QLabel(i18n("Byteorder :"));
	grid->addWidget(byteorderlabel,3,0);
	byteordercb = new KComboBox;
	byteordercb->insertItems(0,byteorderitems);
	byteordercb->setCurrentIndex(conf.readEntry("ByteOrder",0));
	grid->addWidget(byteordercb,3,1);

	toggleBinary(conf.readEntry("Binary",false));

	// global options
	newspreadcb = new QCheckBox(i18n("Create new spreadsheet"));
	newspreadcb->setChecked(conf.readEntry("CreateNewSpreadsheet",true));
	layout->addWidget(newspreadcb,2,0);
	usetitlecb = new QCheckBox(i18n("Use filename as spreadsheet title"));
	usetitlecb->setChecked(conf.readEntry("UseFilenameAsTitle",false));
	layout->addWidget(usetitlecb,2,1);
}

void ImportDialog::saveSettings() {
	kdDebug()<<"ImportDialog::saveSettings()"<<endl;
	KConfigGroup conf(KSharedConfig::openConfig(),"Import");
	
	conf.writeEntry("Filename",filele->text());
	conf.writeEntry("CreateNewSpreadsheet",newspreadcb->isChecked());
	conf.writeEntry("UseFilenameAsTitle",usetitlecb->isChecked());

	conf.writeEntry("HideOptions",optionsgb->isHidden());
	conf.writeEntry("Binary",binarycb->isChecked());
	conf.writeEntry("StartRow",startle->text());
	conf.writeEntry("EndRow",endle->text());
	conf.writeEntry("SimplifyWhitespace",simplifycb->isChecked());
	conf.writeEntry("EmptyEntries",emptycb->isChecked());
	conf.writeEntry("ImportHeader",headercb->isChecked());
	conf.writeEntry("SameXColumn",samexcb->isChecked());
	conf.writeEntry("SeparatingCharacter",separatingcb->currentText());
	conf.writeEntry("CommentCharacter",commentcb->currentText());
	
	conf.writeEntry("BinaryFields",fieldsle->text());
	conf.writeEntry("BinaryFormat",formatcb->currentIndex());
	conf.writeEntry("ByteOrder",byteordercb->currentIndex());
	
	conf.config()->sync();
}

void ImportDialog::toggleOptions() {
	if(optionsgb->isHidden()) {
		optionsgb->show();
		setButtonText(KDialog::User2,i18n("Hide options"));
	} else {
		optionsgb->hide();
		setButtonText(KDialog::User2,i18n("Show options"));
		// TODO : doesn't work
		//adjustSize();
		// updateGeometry();
		//resize(minimumSizeHint());
	}
}

void ImportDialog::toggleBinary(bool checked) {
	// not binay options
	simplifycb->setVisible(!checked);
	separatinglabel->setVisible(!checked);
	separatingcb->setVisible(!checked);
	commentlabel->setVisible(!checked);
	commentcb->setVisible(!checked);
	emptycb->setVisible(!checked);
	headercb->setVisible(!checked);
	
	// binary options
	fieldslabel->setVisible(checked);
	fieldsle->setVisible(checked);
	formatlabel->setVisible(checked);
	formatcb->setVisible(checked);
	byteorderlabel->setVisible(checked);
	byteordercb->setVisible(checked);
}

int ImportDialog::startRow() {
	int row=0;
	if(!startle->text().isEmpty())
		row = startle->text().toInt()-1;
	return row;
}

int ImportDialog::endRow() {
	int row=INT_MAX;
	if(!endle->text().isEmpty())
		row = endle->text().toInt()-1;
	return row;
}

void ImportDialog::selectFile() {
	QStringList f = QFileDialog::getOpenFileNames(this,i18n("Select one or more files to open"));
	if (! f.isEmpty() )
		filele->setText(f.join(";"));
}

void ImportDialog::fileInfoDialog() {
	QStringList files = filele->text().split(";");
	 for ( int i=0; i<files.size(); i++ ) {
		QString filename = files.at(i);
		if(filename.isEmpty())
			continue;

		QIODevice *file = KFilterDev::deviceForFile(filename,QString::null,true);
		if(file==0) 
			file = new QFile(filename);

		if (file->open(QIODevice::ReadOnly))
			(new FileInfoDialog(mw,filename))->show();
		else {
			kdDebug()<<"WARNING: Could not open file"<<filename<<endl;
			// TODO : hangs here
			KMessageBox::sorry(this, i18n("Sorry. Could not open file %1 for reading!").arg(filename));
		}
	}
}

void ImportDialog::updateFileInfo() {
	QProcess *proc = new QProcess(this);
	QString program= "file";
	QStringList args;
	args<<"-b"<<filele->text();
	proc->start(program,args);
	if(proc->waitForReadyRead(1000) == false)
		kdDebug()<<"ERROR: reading file info of file"<<filele->text()<<endl;
	else {
		QString info = proc->readLine().left(60);
		if(info.contains(i18n("cannot open")))
			filel->setText("");
		else
			filel->setText(info);
	}
}

void ImportDialog::Apply() {
	kdDebug()<<"ImportDialog::Apply()"<<endl;

	QStringList files = filele->text().split(";");
	 for ( int i=0; i<files.size(); i++ ) {
		QString filename = files.at(i);
		if(filename.isEmpty())
			continue;

		if (filename.endsWith(".opj",Qt::CaseInsensitive)) {
			importOPJ(filename);
			return;
		}

		// open a spreadsheet
		Spreadsheet *s=0;
		if(!newspreadcb->isChecked() && i==0 )
			s=mw->activeSpreadsheet();
		if(!s)
			s=mw->newSpreadsheet();

		if(!s) {
			kdDebug()<<"ERROR : Couldn't create spreadsheet!"<<endl;
			continue;
		}

		if(usetitlecb->isChecked())
			s->setTitle(filename);
	
		if(filename.endsWith(".hdf",Qt::CaseInsensitive) || filename.endsWith(".h5",Qt::CaseInsensitive))
			importHDF5(filename,s);
		else if (filename.endsWith(".nc",Qt::CaseInsensitive))
			importNETCDF(filename,s);
		else if (filename.endsWith(".cdf",Qt::CaseInsensitive))
			importCDF(filename,s);
		else {
			kdDebug()<<"	Opening file"<<filename<<endl;
			QIODevice *file = KFilterDev::deviceForFile(filename,QString::null,true);
			if(file==0) {
				kdDebug()<<"No device for file found. Opening as normal file."<<endl;
				file = new QFile(filename);
			}
			if (!file->open(QIODevice::ReadOnly)) {
				kdDebug()<<"ERROR : Unable to open file "<<filename<<" for reading!"<<endl;
				continue;
			}
	
			if(binarycb->isChecked())
				importBinary(file,s);
			else
				importASCII(file,s);

			delete file;
		}
	}
}

void ImportDialog::importOPJ(QString filename) {
	FilterOPJ importer(mw, filename);
	importer.import();
}


int ImportDialog::importHDF5(QString filename, Spreadsheet *s) {
	kdDebug()<<"ImportDialog::importHDF5() filename="<<filename<<endl;
#ifdef HAVE_HDF5
	FilterHDF5 hdf5 = FilterHDF5(filename);
	if(!hdf5.fileOK()) {
		kdDebug()<<"ERROR : unable to read HDF5 file "<<filename<<endl;
		 return -1;
	}
	
	hdf5.importFile();
	kdDebug()<<"Reading HDF5 data"<<endl;
	kdDebug()<<"number of attributes = "<<hdf5.numAttributes()<<endl;
	kdDebug()<<"number of datasets = "<<hdf5.numSets()<<endl;

	// add attributes as project comments
	Project* project=0;
	QString notes;
	if(hdf5.numAttributes()>0) {
		project = mw->getProject();
		notes = project->Notes();
	}
	for (int i=0;i<hdf5.numAttributes();i++) {
		notes.append(hdf5.getAttribute(i)+"\n");
		// kdDebug()<<"	ATTRIBUTE "<<i+1<<" : "<<hdf5.getAttribute(i)<<endl;
	}
	if(hdf5.numAttributes()>0)
		project->setNotes(notes);

	// read data
	for (int i=0;i<hdf5.numSets();i++) {
		if(i>0) s = mw->newSpreadsheet();

		int rows = hdf5.Rows(i);
		int cols = hdf5.Cols(i);
		if(rows==0) cols=0;
		kdDebug()<<"Dataset "<<i+1<<" ("<<hdf5.datasetName(i)<<") has "<< rows<<" rows and "<<cols<<" cols"<<endl;
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
		for (int c=0;c<cols;c++) {
			QString colname = hdf5.columnName(i,c);
			if(colname.length()<1)
				colname = QChar(c+65);
			s->setColumnName(c,colname);

		 	for ( int j=0; j<rows; j++ ) {
				QTableWidgetItem *item = s->item(j,c);
				if(item==0) {
					item = new QTableWidgetItem();
     					s->setItem(j,c, item);
				}
				item->setText(QString::number(hdf5.Data(i,j,c),'g',10));
			}
		}
	}

	return 0;
#else
	kdDebug()<<"Not compiled with HDF5 support!"<<endl;
	return -2;
#endif	
}

int ImportDialog::importNETCDF(QString filename, Spreadsheet *s) {
	kdDebug()<<"ImportDialog::importNETCDF()"<<endl;
#ifdef HAVE_NETCDF
	FilterNETCDF ncf = FilterNETCDF(filename);
	if(!ncf.fileOK()) return -1;
	kdDebug()<<"Reading NETCDF data"<<endl;
	s->setColumnCount(ncf.NVars());
	s->resetHeader();

	kdDebug()<<" nvars = "<<ncf.NVars()<<endl;
	QProgressDialog progress( i18n("Reading NETCDF data ..."), i18n("Cancel"), 0, ncf.NVars()-startRow());
	progress.setWindowModality(Qt::WindowModal);
	s->setUpdatesEnabled(false);
	for (int j=startRow();j<ncf.NVars();j++) {
		progress.setValue(j-startRow());

		QString name = ncf.VarName(j);
		int rows = ncf.VarLen(name);
		kdDebug()<<" var / varid / len = "<<name<<' '<<j<<' '<<rows<<endl;
		s->setColumnName(j-startRow(),name);

		if (j==startRow() || s->rowCount()<rows)
			s->setRowCount(rows);
		for ( int i=0; i<rows; i++ ) {
			QTableWidgetItem *item = s->item(i,j);
			if(item==0) {
				item = new QTableWidgetItem();
     				s->setItem(i,j, item);
			}
			item->setText(QString::number(ncf.Data(name,i),'g',10));
		}
		if (j>endRow())
			break;
	}
	s->setUpdatesEnabled(true);
#else
	kdDebug()<<"Not compiled with NETCDF support!"<<endl;
	return 1;
#endif
	return 0;
}

int ImportDialog::importCDF(QString filename, Spreadsheet *s) {
	kdDebug()<<"ImportDialog::importCDF()"<<endl;
#ifdef HAVE_CDF
	FilterCDF cdf = FilterCDF(filename);
	if(!cdf.fileOK()) return -1;
	kdDebug()<<"Reading CDF data"<<endl;

	s->setColumnCount(cdf.NVars());
	s->setRowCount(cdf.MaxRec());
	s->resetHeader();

	kdDebug()<<" nvars = "<<cdf.NVars()<<endl;
	QProgressDialog progress( i18n("Reading CDF data ..."), i18n("Cancel"), 0, cdf.NVars()-startRow());
	progress.setWindowModality(Qt::WindowModal);
	s->setUpdatesEnabled(false);
	for (int j=startRow();j<cdf.NVars();j++) {
		progress.setValue(j-startRow());

		QString name = cdf.VarName(j);
		int rows = cdf.VarLen(name);
		kdDebug()<<" var / varid / len = "<<name<<' '<<j<<' '<<rows<<endl;

		s->setColumnName(j-startRow(),name);

		for (int i=0;i<rows;i++) {
			QTableWidgetItem *item = s->item(i,j);
			if(item==0) {
				item = new QTableWidgetItem();
     				s->setItem(i,j, item);
			}
			item->setText(QString::number(cdf.Data(name,i),'g',10));
		}
		if (j>endRow())
			break;
	}
	s->setUpdatesEnabled(true);
#else
	kdDebug()<<"Not compiled with CDF support!"<<endl;
	return 1;
#endif
	return 0;
}

void ImportDialog::importBinary(QIODevice *file, Spreadsheet *s) {
	kdDebug()<<"ImportDialog::importBinary()"<<endl;
	// TODO : use samex

	QDataStream ds(file);
	ds.setByteOrder((QDataStream::ByteOrder)byteordercb->currentIndex());
	kdDebug()<<"	byte order : "<<byteordercb->currentIndex()<<" (big/little endian : "<<QDataStream::BigEndian<<"/"<<QDataStream::LittleEndian<<")"<<endl;

	int fields = fieldsle->text().toInt();
	if(fields > s->columnCount())
		s->setColumnCount(fields);

	QProgressDialog progress( i18n("Reading binary data ..."), i18n("Cancel"), 0, file->size());
	progress.setWindowModality(Qt::WindowModal);
	s->setUpdatesEnabled(false);
	int row=0;
	for (int j=0;j<fields*startRow();j++)
		getBinaryValue(&ds,(BinaryFormat) formatcb->currentIndex());
	while(!ds.atEnd()) {
		if (row%5000==0) {
			progress.setValue(file->pos());
			s->setRowCount(row+5000);
			if (progress.wasCanceled()) {
				kdDebug()<<"WARNING: Import canceled()"<<endl;
             			break;
			}
		}
#ifndef TABLEVIEW
		 for ( int i=0; i<fields; i++ ) {
			QTableWidgetItem *item = s->item(row,i);
			if(item==0) {
				item = new QTableWidgetItem();
     				s->setItem(row,i, item);
			}
			item->setText(QString::number(getBinaryValue(&ds,(BinaryFormat) formatcb->currentIndex()),'g',10));
		}
#endif
		row++;
	}
	s->setRowCount(row);
	s->setUpdatesEnabled(true);
}

double ImportDialog::getBinaryValue(QDataStream *ds, BinaryFormat type) {
	//kdDebug()<<"Dialog::getBinaryValue() : Type = "<<type<<endl;
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
	// kdDebug()<<"	value = "<<value<<endl;

	return value;
}

void ImportDialog::importASCII(QIODevice *file, Spreadsheet *s) {
	kdDebug()<<"ImportDialog::importASCII()"<<endl;
	// TODO : use samex

	int row=0,actrow=0;
	QTextStream in(file);
	QProgressDialog progress( i18n("Reading ASCII data ..."), i18n("Cancel"), 0, file->size());
	progress.setWindowModality(Qt::WindowModal);
	s->setUpdatesEnabled(false);
	while (!in.atEnd()) {
		while(row<startRow()) {
			in.readLine();
			row++;
		}
		actrow = row-startRow();
		if (headercb->isChecked()) actrow--;

		if (row%5000==0) {
			progress.setValue(file->pos());
			s->setRowCount(actrow+5000);
			if (progress.wasCanceled()) {
				kdDebug()<<"WARNING: Import canceled()"<<endl;
             			break;
			}
		}

		QString line = in.readLine();
		
		if(simplifycb->isChecked())
			line = line.simplified();
		if(line.startsWith(commentcb->currentText()) == true)
			continue;

		QStringList oneline;
		QString sep=separatingcb->currentText();
		if(sep == "auto")
			oneline = line.split(QRegExp("\\s+"),(QString::SplitBehavior)emptycb->isChecked());
		else
			oneline = line.split(sep,(QString::SplitBehavior)emptycb->isChecked());

		// handle empty lines correct
		if(oneline.count() == 0)
			continue;
		// kdDebug()<<"cols="<<oneline.size()<<endl;
		if(oneline.size() > s->columnCount()) {
			s->setColumnCount(oneline.size());
			s->resetHeader();
		}

		// import header
		if(row==startRow() && headercb->isChecked()) {
	 		for ( int i=0; i<oneline.size(); i++ )
				s->setColumnName(i,oneline.at(i));
		}
		else {
#ifndef TABLEVIEW
		 	for ( int i=0; i<oneline.size(); i++ ) {
				QTableWidgetItem *item = s->item(actrow,i);
				if(item==0) {
					item = new QTableWidgetItem();
     					s->setItem(actrow,i, item);
				}
				item->setText(oneline.at(i));
			}
#endif
		}
		if(row>endRow())
			break;
		row++;
	}
	s->setRowCount(actrow+1);
	if(!headercb->isChecked())
		s->resetHeader();
	s->setUpdatesEnabled(true);
}
