//LabPlot : AxesDialog.cc

#include "AxesDialog.h"

AxesDialog::AxesDialog(MainWin *mw)
	: Dialog(mw)
{
	kDebug()<<"AxesDialog()"<<endl;
	setCaption(i18n("Axes Settings"));
	layout->addWidget(new QLabel(i18n("Not implemented yet!")),1,0);

	showButton(KDialog::User1,false);
//	kDebug()<<"	Axis nr = "<<axesnr<<endl;

/*	// TODO : use updateDialog()
	plot = p->getPlot(p->API());
	if(plot==0)
		return;
	type = plot->Type();
	kDebug()<<"	Plot Type : "<<type<<endl;

	if (type == P2D || type == PSURFACE) {
		for (int i=0;i<4;i++)
			axis[i] = plot->getAxis(i);
	}
	else if (type == P3D || type == PQWT3D) {
		for (int i=0;i<12;i++)
			axis[i] = plot->getAxis(i);
	}
	else if (type == PPOLAR) {
		axis[0] = plot->getAxis(0);	// phi
		axis[1] = plot->getAxis(1);	// r
	}
	else if (type == PTERNARY || type == PPIE)
		axis[0] = plot->getAxis(0);

	QHBox *hb = new QHBox(vbox);
	new QLabel(i18n("Selected Axis"),hb);

	axescb = new KComboBox(vbox);
	QStringList list;
	double rmin=0, rmax=0;	// axes range
	// TODO : switch ListBox to KComboBox
	if (type == P2D || type == PSURFACE) {
		list<<QString("x")<<QString("y")<<QString("y2")<<QString("x2");
		if(axesnr == 0 || axesnr == 3) {
			rmin = plot->ActRanges()[0].rMin();
			rmax = plot->ActRanges()[0].rMax();
		}
		else {
			rmin = plot->ActRanges()[1].rMin();
			rmax = plot->ActRanges()[1].rMax();
		}
	}
	else if (type == P3D || type == PQWT3D){
		list<<QString("x")<<QString("y")<<QString("z")<<QString("x2")<<QString("x3")<<QString("x4");
		list<<QString("y4")<<QString("y3")<<QString("y2")<<QString("z2")<<QString("z4")<<QString("z3");
		if(axesnr == 0 || axesnr == 3 || axesnr == 4 || axesnr == 5) {
			rmin = plot->ActRanges()[0].rMin();
			rmax = plot->ActRanges()[0].rMax();
		}
		else if(axesnr == 1 || axesnr == 6 || axesnr == 7 || axesnr == 8) {
			rmin = plot->ActRanges()[1].rMin();
			rmax = plot->ActRanges()[1].rMax();
		}
		else {
			rmin = plot->ActRanges()[2].rMin();
			rmax = plot->ActRanges()[2].rMax();
		}
	}
	else if (type == PPOLAR) {
		list<<QString("phi")<<QString("r");

		rmin = plot->ActRanges()[axesnr].rMin();
		rmax = plot->ActRanges()[axesnr].rMax();
	}
	else if (type == PTERNARY || type == PPIE) {
		list<<QString("a");

		rmin = plot->ActRanges()[axesnr].rMin();
		rmax = plot->ActRanges()[axesnr].rMax();
	}

	axescb->insertStringList(list);
	axescb->setCurrentItem(axesnr);
	QObject::connect(axescb,SIGNAL(activated(int)),this,SLOT(updateAxis(int)));

	QTabWidget *tw = new QTabWidget(vbox);

	QVBox *tab1 = new QVBox(tw);

	QGroupBox *generalgb = new QGroupBox(1,QGroupBox::Horizontal,i18n("General"),tab1);
	axiscb = new QCheckBox(i18n("enable Axis"),generalgb);
	axiscb->setChecked(axis[axesnr]->Enabled());
	QObject::connect(axiscb,SIGNAL(toggled(bool)),this,SLOT(axisEnabled(bool)));

	hb = new QHBox(generalgb);
	new QLabel(i18n("Position :"),hb);
	positioncb = new KComboBox(hb);
	positioncb->setMaximumWidth(200);
	positioncb->insertItem(i18n("Normal"));
	positioncb->insertItem(i18n("Center"));

	QGroupBox *rangegb = new QGroupBox(1,QGroupBox::Horizontal,i18n("Axis Range"),tab1);
	hb = new QHBox(rangegb);
	new QLabel(i18n("Scale Type :"),hb);
	ascb = new KComboBox(hb);
	ascb->setMaximumWidth(200);
	int i=0;
	while(scaleitems[i] != 0) ascb->insertItem(i18n(scaleitems[i++]));
	ascb->setCurrentItem(axis[axesnr]->Scale());
	QObject::connect(ascb,SIGNAL(activated (int)),SLOT(updateScale(int)));

	hb = new QHBox(rangegb);
	QVBox *vb = new QVBox(hb);
	new QLabel(i18n("Lower Limit"),vb);
	minle = new KLineEdit(QString::number(rmin,'g',15),vb);
	minle->setMaximumWidth(100);
	vb = new QVBox(hb);
	new QLabel(i18n("Upper Limit"),vb);
	maxle = new KLineEdit(QString::number(rmax,'g',15),vb);
	maxle->setMaximumWidth(100);

	if (type == PPOLAR) {
		minle->setEnabled(false);
		maxle->setEnabled(false);
	}

	hb = new QHBox(rangegb);
	vb = new QVBox(hb);
	new QLabel(i18n("Zero Offset"),vb);
	shiftle = new KLineEdit(QString::number(axis[axesnr]->Shift()),vb);
	shiftle->setMaximumWidth(100);
	vb = new QVBox(hb);
	new QLabel(i18n("Scaling Factor"),vb);
	scalingle = new KLineEdit(QString::number(axis[axesnr]->Scaling()),vb);
	scalingle->setMaximumWidth(100);

	QGroupBox *bordergb = new QGroupBox(1,QGroupBox::Horizontal,i18n("Border"),tab1);
	bordercb = new QCheckBox(i18n("enable Border"),bordergb);
	hb = new QHBox(bordergb);
	bordercb->setChecked(axis[axesnr]->BorderEnabled());
	new QLabel(i18n("Color : "),hb);
	bcb = new KColorButton(axis[axesnr]->BorderColor(),hb);
	hb = new QHBox(bordergb);
	new QLabel(i18n("Width : "),hb);
	borderwidth = new KIntNumInput(axis[axesnr]->borderWidth(),hb);
	borderwidth->setRange(0,20);

	QVBox *tab2 = new QVBox(tw);
	centercb = new QCheckBox(i18n("Center Label on Axis"),tab2);
	centercb->setChecked(false);
	QObject::connect(centercb,SIGNAL(toggled(bool)),this,SLOT(centerEnabled(bool)));

	label = axis[axesnr]->getLabel();
	if(mw->activeWorksheet() && mw->activeWorksheet()->getPlot(0) &&
		mw->activeWorksheet()->getPlot(0)->Type() == PQWT3D )
		rtw = new RichTextWidget((QWidget *)tab2,label,0,1);
	else
		rtw = new RichTextWidget((QWidget *)tab2,label,0);

	QVBox *tab3 = new QVBox(tw);

	QGroupBox *tickgb = new QGroupBox(1,QGroupBox::Horizontal,i18n("General"),tab3);
	hb = new QHBox(tickgb);
	new QLabel(i18n("Tick Position :"),hb);
	tickposcb = new KComboBox(hb);
	i=0;
	while(tickpositems[i] != 0) tickposcb->insertItem(i18n(tickpositems[i++]));
	tickposcb->setCurrentItem(axis[axesnr]->TickPos());

	hb = new QHBox(tickgb);
	new QLabel(i18n("Tick Style :"),hb);
	ticktypecb = new KComboBox(hb);
	ticktypecb->insertItem(i18n("Number"));
	ticktypecb->insertItem(i18n("Increment"));
	ticktypecb->setCurrentItem(axis[axesnr]->tickType());
	QObject::connect(ticktypecb,SIGNAL(activated (int)),SLOT(updateTickType(int)));

	hb = new QHBox(tickgb);
	new QLabel(i18n("Tick Color : "),hb);
	tcb = new KColorButton(axis[axesnr]->TickColor(),hb);

	QGroupBox *majortickgb = new QGroupBox(1,QGroupBox::Horizontal,i18n("Major Ticks"),tab3);
	majortickscb = new QCheckBox(i18n("enable Major Ticks"),majortickgb);
	hb = new QHBox(majortickgb);
	majorlabel = new QLabel(i18n("Number : "),hb);
	if(axis[axesnr]->tickType() == 1)
		majorlabel->setText(i18n("Increment : "));
	majortickscb->setChecked(axis[axesnr]->MajorTicksEnabled());
	if(axis[axesnr]->MajorTicks()==-1)
		majorle = new KLineEdit(i18n("auto"),hb);
	else
		majorle = new KLineEdit(QString::number(axis[axesnr]->MajorTicks()),hb);
	// allow auto
	//majorle->setValidator(new QIntValidator(majorle));
	hb = new QHBox(majortickgb);
	new QLabel(i18n("Width : "),hb);
	majortickwidth = new KIntNumInput(axis[axesnr]->majorTickWidth(),hb);
	majortickwidth->setRange(0,10);
	if(type == PQWT3D) {
		new QLabel(i18n("Length : "),hb);
		majorticklengthle = new KLineEdit(QString::number(axis[axesnr]->majorTickLength()),hb);
		majorticklengthle->setValidator(new QDoubleValidator(0.0,1.0,3,majorticklengthle));
	 }

	QGroupBox *minortickgb = new QGroupBox(1,QGroupBox::Horizontal,i18n("Minor Ticks"),tab3);
	minortickscb = new QCheckBox(i18n("enable Minor Ticks"),minortickgb);
	minortickscb->setChecked(axis[axesnr]->MinorTicksEnabled());
	hb = new QHBox(minortickgb);
	new QLabel(i18n("Number : "),hb);
	minorle = new KLineEdit(QString::number(axis[axesnr]->MinorTicks()),hb);
	minorle->setValidator(new QIntValidator(minorle));
	hb = new QHBox(minortickgb);
	new QLabel(i18n("Width : "),hb);
	minortickwidth = new KIntNumInput(axis[axesnr]->minorTickWidth(),hb);
	minortickwidth->setRange(0,10);
	if(type == PQWT3D) {
		new QLabel(i18n("Length : "),hb);
		minorticklengthle = new KLineEdit(QString::number(axis[axesnr]->minorTickLength()),hb);
		minorticklengthle->setValidator(new QDoubleValidator(0.0,1.0,3,minorticklengthle));
	}

	QVBox *tab4 = new QVBox(tw);
	hb = new QHBox(tab4);
	ticklabelcb = new QCheckBox(i18n("Tick Label Enabled"),hb);
	ticklabelcb->setChecked(axis[axesnr]->tickLabelEnabled());

	hb = new QHBox(tab4);
	new QLabel(i18n("Tick Label Font : "),hb);
	tf = axis[axesnr]->TickLabelFont();
	tickfont = new KLineEdit(tf.family() + tr(" ") +
			QString::number(tf.pointSize()),hb);
	KPushButton *newticsfont = new KPushButton(i18n("New"),hb);
	QObject::connect(newticsfont,SIGNAL(clicked()),SLOT(selectTickFont()));

	hb = new QHBox(tab4);
	new QLabel(i18n("Tick Label Color : "),hb);
	tlcb = new KColorButton(axis[axesnr]->TickLabelColor(),hb);

	hb = new QHBox(tab4);
	new QLabel(i18n("Tick Label Format : "),hb);
	atlfcb = new KComboBox(hb);
	i=0;
	while(tickformatitems[i] != 0) atlfcb->insertItem(i18n(tickformatitems[i++]));
	atlfcb->setCurrentItem(axis[axesnr]->TickLabelFormat());
	QObject::connect(atlfcb,SIGNAL(activated(int)),this,SLOT(update_timeformat()));

	hb = new QHBox(tab4);
	new QLabel(i18n("date/time/datetime format (dd.MM.yyyy hh:mm:ss.zzz) : "),hb);
	timeformat = new KLineEdit(axis[axesnr]->DateTimeFormat(),hb);
	if (atlfcb->currentText() != i18n("time") && atlfcb->currentText() != i18n("date")
		&& atlfcb->currentText() != i18n("datetime"))
		timeformat->setDisabled("true");

	hb = new QHBox(tab4);
	new QLabel(i18n("Tick Label Precision : "),hb);
	tlpni = new KIntNumInput(axis[axesnr]->TickLabelPrecision(),hb);
	tlpni->setRange(0,10,1,false);
	hb = new QHBox(tab4);
	new QLabel(i18n("Tick Label Position : "),hb);
	tlgni = new KIntNumInput(axis[axesnr]->TickLabelPosition(),hb);
	tlgni->setRange(0,1000,1,false);

	hb = new QHBox(tab4);
	new QLabel(i18n("Prefix : "),hb);
	tlprefix = new KLineEdit(axis[axesnr]->TickLabelPrefix(),hb);
	new QLabel(i18n("Suffix : "),hb);
	tlsuffix = new KLineEdit(axis[axesnr]->TickLabelSuffix(),hb);

	hb = new QHBox(tab4);
	new QLabel(i18n("Tick Label Rotation : "),hb);
	tlrotation = new KLineEdit(QString::number(axis[axesnr]->TickLabelRotation()),hb);
	tlrotation->setValidator(new QDoubleValidator(tlrotation));

	// grid & border
	QVBox *tab5 = new QVBox(tw);

	QGroupBox *gb = new QGroupBox(1,QGroupBox::Horizontal,i18n("Major Grid"),tab5);
	hb = new QHBox(gb);
	majorgridcb = new QCheckBox(i18n("Enabled"),hb);
	majorgridcb->setChecked(axis[axesnr]->MajorGridEnabled());
	hb = new QHBox(gb);
	new QLabel(i18n("Style : "),hb);
	majorgridstylecb = new KComboBox(hb);
	hb = new QHBox(gb);
	new QLabel(i18n("Color : "),hb);
	majorgridcolorcb = new KColorButton(axis[axesnr]->majorGridColor(),hb);
	hb = new QHBox(gb);
	new QLabel(i18n("Width : "),hb);
	majorgridwidth = new KIntNumInput(axis[axesnr]->majorGridWidth(),hb);
	majorgridwidth->setRange(0,10);

	gb = new QGroupBox(1,QGroupBox::Horizontal,i18n("Minor Grid"),tab5);
	hb = new QHBox(gb);
	minorgridcb = new QCheckBox(i18n("Enabled"),hb);
	minorgridcb->setChecked(axis[axesnr]->MinorGridEnabled());
	hb = new QHBox(gb);
	new QLabel(i18n("Style : "),hb);
	minorgridstylecb = new KComboBox(hb);
	hb = new QHBox(gb);
	new QLabel(i18n("Color : "),hb);
	minorgridcolorcb = new KColorButton(axis[axesnr]->minorGridColor(),hb);
	hb = new QHBox(gb);
	new QLabel(i18n("Width : "),hb);
	minorgridwidth = new KIntNumInput(axis[axesnr]->minorGridWidth(),hb);
	minorgridwidth->setRange(0,10);

	majorgridstylecb->clear();
	minorgridstylecb->clear();
	for (int i=0;i<6;i++) {	// major grid
		QPainter pa;
		QPixmap pm( 100, 30 );
		pm.fill(Qt::white);
		pa.begin( &pm );

		pa.setPen(QPen(axis[axesnr]->majorGridColor(),axis[axesnr]->majorGridWidth(),(PenStyle)i));
		pa.drawLine(5,15,95,15);
		pa.end();

		majorgridstylecb->insertItem(pm);
	}
	for (int i=0;i<6;i++) {	// minor grid
		QPainter pa;
		QPixmap pm( 100, 30 );
		pm.fill(Qt::white);
		pa.begin( &pm );

		pa.setPen(QPen(axis[axesnr]->minorGridColor(),axis[axesnr]->minorGridWidth(),(PenStyle)i));
		pa.drawLine(5,15,95,15);
		pa.end();

		minorgridstylecb->insertItem(pm);
	}
	majorgridstylecb->setCurrentItem(axis[axesnr]->MajorGridType());
	minorgridstylecb->setCurrentItem(axis[axesnr]->MinorGridType());

	if(type == PPIE) {
		tab2->hide();
		tab3->hide();
		tab5->hide();
		tw->addTab(tab1,i18n("Scale"));
		tw->addTab(tab4,i18n("Tick Label"));
	}
	else {
		tw->addTab(tab1,i18n("Scale"));
		tw->addTab(tab2,i18n("Axis Label"));
		tw->addTab(tab3,i18n("Ticks"));
		tw->addTab(tab4,i18n("Tick Label"));
		tw->addTab(tab5,i18n("Grid"));
	}

	update_timeformat();
*/
}

/*void AxesDialog::updateDialog(int i) {
	if(i != -1 )
		axescb->setCurrentItem(i);

	p = mw->activeWorksheet();
	if(p == 0)	// active spreadsheet
		return;

	plot = p->getPlot(p->API());
	if(plot==0)
		return;
	type = plot->Type();

	if (type == P2D || type == PSURFACE) {
		for (int i=0;i<4;i++)
			axis[i] = plot->getAxis(i);
	}
	else if (type == P3D || type == PQWT3D) {
		for (int i=0;i<12;i++)
			axis[i] = plot->getAxis(i);
	}
	else if (type == PPOLAR) {
		axis[0] = plot->getAxis(0);	// phi
		axis[1] = plot->getAxis(1);	// r
	}
	else if (type == PTERNARY || type == PPIE)
		axis[0] = plot->getAxis(0);

	updateAxis();
}

//! update values if scale changed
void AxesDialog::updateScale(int i) {
	if(i==LINEAR)
		minorle->setText(QString("3"));
	else if (i==LOG10)
		minorle->setText(QString("8"));
	else
		minorle->setText(QString("0"));
}

void AxesDialog::updateTickType(int tt) {
	if(tt)
		majorlabel->setText(i18n("Increment : "));
	else
		majorlabel->setText(i18n("Number : "));
}

//! called when axis tic format is changed : update format line edit & ranges
void AxesDialog::update_timeformat() {
	int item = axescb->currentItem();
	kDebug()<<"ATLF = "<<atlfcb->currentText()<<endl;
	if (atlfcb->currentText() == i18n("time") || atlfcb->currentText() == i18n("date")
		|| atlfcb->currentText() == i18n("datetime") )
		timeformat->setEnabled("true");
	else
		timeformat->setDisabled("true");

	// update range here
	TFormat atlf = (TFormat) atlfcb->currentItem();
	// dont use richtext formats here
	if(atlf==POWER10 || atlf == POWER2 || atlf == POWERE || atlf == FSQRT)
		atlf = AUTO;
	QString dtf;
	if(atlf == TIME)
		dtf = QString("hh:mm:ss");
	else if (atlf == DATE)
		dtf = QString("yyyy-MM-dd");
	else if (atlf == DATETIME)
		dtf = QString("yyyy-MM-ddThh:mm:ss");
	int prec = tlpni->value();

	double rmin=0, rmax=1;
	if (type == P2D || type == PSURFACE || type == PPOLAR) {
		if(item == 0 || item == 3) {
			rmin = plot->ActRanges()[0].rMin();
			rmax = plot->ActRanges()[0].rMax();
		}
		else if (item == 1 || item == 2) {
			rmin = plot->ActRanges()[1].rMin();
			rmax = plot->ActRanges()[1].rMax();
		}
	}
	else if (type == P3D || type == PQWT3D){
		if(item == 0 || item == 3 || item == 6 || item == 9) {
			rmin = plot->ActRanges()[0].rMin();
			rmax = plot->ActRanges()[0].rMax();
		}
		else if (item == 1 || item == 4 || item == 7 || item == 10) {
			rmin = plot->ActRanges()[1].rMin();
			rmax = plot->ActRanges()[1].rMax();
		}
		else if (item == 2 || item == 5 || item == 8 || item == 11) {
			rmin = plot->ActRanges()[2].rMin();
			rmax = plot->ActRanges()[2].rMax();
		}
	}

	QString mintext = plot->TicLabel(atlf,prec,dtf,rmin);
	QString maxtext = plot->TicLabel(atlf,prec,dtf,rmax);

	minle->setText(mintext);
	maxle->setText(maxtext);
}

void AxesDialog::axisEnabled(bool on) {
	kDebug()<<"AxesDialog::axisEnabled() : "<<on<<endl;
	int item = axescb->currentItem();
	if (on) {	// axis should be enabled
		axis[item]->enableBorder();
		bordercb->setChecked(true);
		// TODO : show the label position in richtext widget (rtw)
		rtw->setLabel(axis[item]->getLabel());
	}
}

void AxesDialog::centerEnabled(bool on) {
	// set position xle,yle readonly
	rtw->setPostionReadOnly(on);
}

//! update the selected axis
void AxesDialog::updateAxis(int i) {
	kDebug()<<"AxesDialog::updateAxis()"<<endl;
	int item;
	i==-1?item = axescb->currentItem():item=i;
	axiscb->setChecked(axis[item]->Enabled());

	KConfig *config = mw->Config();
	config->setGroup( "Axes" );
	QString entry = QString("PlotType %1 Axis %2 ").arg(type).arg(item);

	centercb->setChecked(config->readBoolEntry(entry+"CenterLabel",false));
*/
/*	TODO : update if plot type changes
	if(type == PPIE) {
		tab2->hide();
		tab3->hide();
		tab5->hide();
		tab6->hide();
		tw->addTab(tab1,i18n("Main"));
		tw->addTab(tab4,i18n("Tick Label"));
	}
	else {
		tw->addTab(tab1,i18n("Main"));
		tw->addTab(tab2,i18n("Axis Label"));
		tw->addTab(tab3,i18n("Ticks"));
		tw->addTab(tab4,i18n("Tick Label"));
		tw->addTab(tab5,i18n("Grid"));
		tw->addTab(tab6,i18n("Border"));
	}
*/
/*
	label = axis[item]->getLabel();
	rtw->setLabel(label);

	positioncb->setCurrentItem(axis[item]->Position());
	scalingle->setText(QString::number(axis[item]->Scaling()));
	shiftle->setText(QString::number(axis[item]->Shift()));

	double rmin=0, rmax=1;
	if (type == P2D || type == PSURFACE || type == PPOLAR) {
		if(item == 0 || item == 3) {
			rmin = plot->ActRanges()[0].rMin();
			rmax = plot->ActRanges()[0].rMax();

			ascb->setCurrentItem(axis[0]->Scale());
			if (type == PPOLAR) {
				maxle->setEnabled(false);
			}
		}
		else if (item == 1 || item == 2) {
			rmin = plot->ActRanges()[1].rMin();
			rmax = plot->ActRanges()[1].rMax();

			ascb->setCurrentItem(axis[1]->Scale());
			if (type == PPOLAR) {
				maxle->setEnabled(true);
			}
		}
	}
	else if (type == P3D || type == PQWT3D){
		if(item == 0 || item == 3 || item == 6 || item == 9) {
			rmin = plot->ActRanges()[0].rMin();
			rmax = plot->ActRanges()[0].rMax();
			ascb->setCurrentItem(axis[0]->Scale());
		}
		else if (item == 1 || item == 4 || item == 7 || item == 10) {
			rmin = plot->ActRanges()[1].rMin();
			rmax = plot->ActRanges()[1].rMax();
			ascb->setCurrentItem(axis[1]->Scale());
		}
		else if (item == 2 || item == 5 || item == 8 || item == 11) {
			rmin = plot->ActRanges()[2].rMin();
			rmax = plot->ActRanges()[2].rMax();
			ascb->setCurrentItem(axis[2]->Scale());
		}
	}
	// set text of range line edits
	TFormat atlf=axis[item]->TickLabelFormat();
	// dont use richtext formats here
	if(atlf==POWER10 || atlf == POWER2 || atlf == POWERE || atlf == FSQRT)
		atlf = AUTO;
	QString dtf;
	if(atlf == TIME)
		dtf = QString("hh:mm:ss");
	else if (atlf == DATE)
		dtf = QString("yyyy-MM-dd");
	else if (atlf == DATETIME)
		dtf = QString("yyyy-MM-ddThh:mm:ss");
	QString mintext = plot->TicLabel(atlf,axis[item]->TickLabelPrecision(),dtf,rmin);
	QString maxtext = plot->TicLabel(atlf,axis[item]->TickLabelPrecision(),dtf,rmax);

	minle->setText(mintext);
	maxle->setText(maxtext);

	// ticks
	bool tt = axis[item]->tickType();
	ticktypecb->setCurrentItem(tt);
	if(tt)
		majorlabel->setText(i18n("Increment : "));
	else
		majorlabel->setText(i18n("Number : "));
	majortickscb->setChecked(axis[item]->MajorTicksEnabled());
	minortickscb->setChecked(axis[item]->MinorTicksEnabled());
	if(axis[item]->MajorTicks()==-1)
		majorle->setText(i18n("auto"));
	else
		majorle->setText(QString::number(axis[item]->MajorTicks()));
	minorle->setText(QString::number(axis[item]->MinorTicks()));
	majortickwidth->setValue(axis[item]->majorTickWidth());
	minortickwidth->setValue(axis[item]->minorTickWidth());
	if(type == PQWT3D) {
		majorticklengthle->setText(QString::number(axis[item]->majorTickLength()));
		minorticklengthle->setText(QString::number(axis[item]->majorTickLength()));
	}
	majorgridwidth->setValue(axis[item]->majorGridWidth());
	minorgridwidth->setValue(axis[item]->minorGridWidth());

	ticklabelcb->setChecked(axis[item]->tickLabelEnabled());
	tf = axis[item]->TickLabelFont();
	tickfont->setText(tf.family() + tr(" ") + QString::number(tf.pointSize()));

	tlprefix->setText(axis[item]->TickLabelPrefix());
	tlsuffix->setText(axis[item]->TickLabelSuffix());

	tcb->setColor(axis[item]->TickColor());
	tlcb->setColor(axis[item]->TickLabelColor());

	tickposcb->setCurrentItem(axis[item]->TickPos());
	atlfcb->setCurrentItem(axis[item]->TickLabelFormat());
	tlpni->setValue(axis[item]->TickLabelPrecision());
	tlgni->setValue(axis[item]->TickLabelPosition());
	timeformat->setText(axis[item]->DateTimeFormat());
	if (atlfcb->currentText() == i18n("time"))
		timeformat->setEnabled("true");
	tlrotation->setText(QString::number(axis[item]->TickLabelRotation()));

	// grid
	majorgridcb->setChecked(axis[item]->MajorGridEnabled());
	minorgridcb->setChecked(axis[item]->MinorGridEnabled());
	majorgridcolorcb->setColor(axis[item]->majorGridColor());
	minorgridcolorcb->setColor(axis[item]->minorGridColor());
	bcb->setColor(axis[item]->BorderColor());
	majorgridstylecb->setCurrentItem(axis[item]->MajorGridType());
	minorgridstylecb->setCurrentItem(axis[item]->MinorGridType());

	bordercb->setChecked(axis[item]->BorderEnabled());
	borderwidth->setValue(axis[item]->borderWidth());

	majorgridstylecb->clear();
	minorgridstylecb->clear();
	for (int i=0;i<6;i++) {	// major grid
		QPainter pa;
		QPixmap pm( 100, 30 );
		pm.fill(Qt::white);
		pa.begin( &pm );

		pa.setPen(QPen(axis[item]->majorGridColor(),axis[item]->majorGridWidth(),(PenStyle)i));
		pa.drawLine(5,15,95,15);
		pa.end();

		majorgridstylecb->insertItem(pm);
	}
	for (int i=0;i<6;i++) {	// minor grid
		QPainter pa;
		QPixmap pm( 100, 30 );
		pm.fill(Qt::white);
		pa.begin( &pm );

		pa.setPen(QPen(axis[item]->minorGridColor(),axis[item]->minorGridWidth(),(PenStyle)i));
		pa.drawLine(5,15,95,15);
		pa.end();

		minorgridstylecb->insertItem(pm);
	}
	majorgridstylecb->setCurrentItem(axis[item]->MajorGridType());
	minorgridstylecb->setCurrentItem(axis[item]->MinorGridType());
}

void AxesDialog::selectTickFont() {
    bool res;
    QFont font = QFontDialog::getFont( &res,axis[axescb->currentItem()]->TickLabelFont(), this );
    if(res) {
	tf = font;
	tickfont->setText(tf.family() + tr(" ") + QString::number(tf.pointSize()));
    }
}

void AxesDialog::saveSettings() {
	int item = axescb->currentItem();

	KConfig *config = mw->Config();
	config->setGroup( "Axes" );

	QString entry = QString("PlotType %1 Axis %2 ").arg(type).arg(item);

	config->writeEntry(entry+"Enabled",axiscb->isChecked());
	config->writeEntry(entry+"Position",positioncb->currentItem());
	config->writeEntry(entry+"Scale",ascb->currentItem());
	config->writeEntry(entry+"Scaling",scalingle->text().toDouble());
	config->writeEntry(entry+"Shift",shiftle->text().toDouble());
	config->writeEntry(entry+"RangeMin",minle->text().toDouble());
	config->writeEntry(entry+"RangeMax",maxle->text().toDouble());

	config->writeEntry(entry+"CenterLabel",centercb->isChecked());
	rtw->getLabel()->saveSettings(config,entry);

	config->writeEntry(entry+"TickPosition",tickposcb->currentItem());
	config->writeEntry(entry+"TickStyle",ticktypecb->currentItem());
	config->writeEntry(entry+"MajorTicksEnabled",majortickscb->isChecked());
	if(majorle->text() == i18n("auto"))
		config->writeEntry(entry+"MajorTicks",-1);
	else
		config->writeEntry(entry+"MajorTicks",majorle->text().toDouble());
	config->writeEntry(entry+"MajorTicksWidth",majortickwidth->value());
	if(type == PQWT3D)
		config->writeEntry(entry+"MajorTicksLength",majorticklengthle->text().toInt());
	config->writeEntry(entry+"MinorTicksEnabled",minortickscb->isChecked());
	config->writeEntry(entry+"MinorTicks",minorle->text().toInt());
	config->writeEntry(entry+"MinorTicksWidth",minortickwidth->value());
	if(type == PQWT3D)
		config->writeEntry(entry+"MinorTicksLength",minorticklengthle->text().toInt());
	config->writeEntry(entry+"TickColor",tcb->color());

	config->writeEntry(entry+"TickLabelEnabled",ticklabelcb->isChecked());
	config->writeEntry(entry+"TickLabelFont",tf);
	config->writeEntry(entry+"TickLabelColor",tlcb->color());
	config->writeEntry(entry+"TickLabelFormat",atlfcb->currentItem());
	config->writeEntry(entry+"DateTimeFormat",timeformat->text());
	config->writeEntry(entry+"TickLabelPrecision",tlpni->value());
	config->writeEntry(entry+"TickLabelPosition",tlgni->value());
	config->writeEntry(entry+"TickLabelPrefix",tlprefix->text());
	config->writeEntry(entry+"TickLabelSuffix",tlsuffix->text());
	config->writeEntry(entry+"TickLabelRotation",tlrotation->text().toInt());

	config->writeEntry(entry+"MajorGridEnabled",majorgridcb->isChecked());
	config->writeEntry(entry+"MajorGridWidth",majorgridwidth->value());
	config->writeEntry(entry+"MajorGridStyle",majorgridstylecb->currentItem());
	config->writeEntry(entry+"MajorGridColor",majorgridcolorcb->color());
	config->writeEntry(entry+"MinorGridEnabled",minorgridcb->isChecked());
	config->writeEntry(entry+"MinorGridWidth",minorgridwidth->value());
	config->writeEntry(entry+"MinorGridStyle",minorgridstylecb->currentItem());
	config->writeEntry(entry+"MinorGridColor",minorgridcolorcb->color());

	config->writeEntry(entry+"BorderEnabled",bordercb->isChecked());
	config->writeEntry(entry+"BorderColor",bcb->color());
	config->writeEntry(entry+"BorderWidth",borderwidth->value());
}

int AxesDialog::apply_clicked() {
	kDebug()<<"AxesDialog::apply_clicked()"<<endl;
	int item = axescb->currentItem();

	// change xmin,etc. if somethings changed
	if(axiscb->isChecked() != axis[item]->Enabled())
		axis[item]->enableBorder(axiscb->isChecked());
	axis[item]->Enable(axiscb->isChecked());

	// DONT DO THIS !!!
	label = rtw->label();
	//axis[item]->setLabel(rtw->label());

	if(centercb->isChecked()) {	// center if selected
		switch(type) {
		case P2D: case PSURFACE :
			if(item == 0 || item == 3)
				axis[item]->centerX((int)(plot->Size().X()*p->getX()),0.5*(plot->P2().X()+plot->P1().X()));
			else
				axis[item]->centerY((int)(plot->Size().Y()*p->getY()),0.5*(plot->P2().Y()+plot->P1().Y()));
			break;
		default: break;
		}
		// update shown position when center label
		rtw->setLabel(axis[item]->getLabel());
	}

	axis[item]->setPosition(positioncb->currentItem());
	axis[item]->setScaling(scalingle->text().toDouble());
	axis[item]->setShift(shiftle->text().toDouble());

	axis[item]->setMajorTickWidth(majortickwidth->value());
	axis[item]->setMinorTickWidth(minortickwidth->value());
	if(type == PQWT3D) {
		axis[item]->setMajorTickLength(majorticklengthle->text().toDouble());
		axis[item]->setMinorTickLength(minorticklengthle->text().toDouble());
	}
	axis[item]->setTickLabelPrefix(tlprefix->text());
	axis[item]->setTickLabelSuffix(tlsuffix->text());

	axis[item]->enableTickLabel(ticklabelcb->isChecked());
	axis[item]->setTickLabelFont(tf);
	axis[item]->setTickColor(tcb->color());
	axis[item]->setTickLabelColor(tlcb->color());
	axis[item]->setMajorGridColor(majorgridcolorcb->color());
	axis[item]->setMinorGridColor(minorgridcolorcb->color());
	axis[item]->setBorderColor(bcb->color());
	axis[item]->setMajorGridType((Qt::PenStyle) majorgridstylecb->currentItem());
	axis[item]->setMinorGridType((Qt::PenStyle) minorgridstylecb->currentItem());
	axis[item]->setMajorGridWidth(majorgridwidth->value());
	axis[item]->setMinorGridWidth(minorgridwidth->value());

	axis[item]->setTickPos(tickposcb->currentItem());
	axis[item]->setTickLabelFormat((TFormat)atlfcb->currentItem());
	axis[item]->setTickLabelPrecision(tlpni->value());
	axis[item]->setTickLabelPosition(tlgni->value());
	axis[item]->setDateTimeFormat(timeformat->text());
	axis[item]->setTickLabelRotation(tlrotation->text().toDouble());
	axis[item]->setBorderWidth(borderwidth->value());

	double ret = parse((char *) (minle->text()).latin1());
	switch((TScale)(ascb->currentItem()) ) {
	case LOG10: case LOG2: case LN:
		if(ret <= 0) {
			KMessageBox::warningContinueCancel(this,
				i18n("The axes range has negative values!\nResetting minimum value."));
			minle->setText(QString("0.01"));
		}
		break;
	case SQRT:
		if(ret < 0) {
			KMessageBox::warningContinueCancel(this,
				i18n("The axes range has negative values!\nResetting minimum value."));
			minle->setText(QString("0"));
		}
		break;
	default: break;
	}

	// TODO : read range line edits with format !
	TFormat atlf = axis[item]->TickLabelFormat();
	double rmin = plot->TicLabelValue(atlf,minle->text());
	double rmax = plot->TicLabelValue(atlf,maxle->text());
	kDebug()<<"	TYPE = "<<type<<endl;
	if (type == P2D || type == PSURFACE || type == PPOLAR) {
		bool tt = ticktypecb->currentItem();
		axis[item]->setTickType(tt);
		axis[item]->enableMajorTicks(majortickscb->isChecked());
		axis[item]->enableMinorTicks(minortickscb->isChecked());
		if(majorle->text()==i18n("auto"))
			axis[item]->setMajorTicks(-1);
		else
			axis[item]->setMajorTicks(majorle->text().toDouble());
		axis[item]->setMinorTicks(minorle->text().toInt());

		if (item == 0 || item == 3 ) {
			plot->setXRange(rmin, rmax);
			axis[0]->setScale((TScale)ascb->currentItem());
			if (type != PPOLAR) {
				if (item == 0) {
					axis[3]->setMajorGridColor(axis[0]->majorGridColor());
					axis[3]->setMinorGridColor(axis[0]->minorGridColor());
				}
				else {
					axis[0]->setMajorGridColor(axis[3]->majorGridColor());
					axis[0]->setMinorGridColor(axis[3]->minorGridColor());
				}
				axis[3]->setScale((TScale)ascb->currentItem());
			}
		}
		else if (item == 1 || item == 2){
			plot->setYRange(rmin, rmax);
			axis[1]->setScale((TScale)ascb->currentItem());

			if (type != PPOLAR) {
				if (item == 1) {
					axis[2]->setMajorGridColor(axis[1]->majorGridColor());
					axis[2]->setMinorGridColor(axis[1]->minorGridColor());
				}
				else {
					axis[1]->setMajorGridColor(axis[2]->majorGridColor());
					axis[1]->setMinorGridColor(axis[2]->minorGridColor());
				}
				axis[2]->setScale((TScale)ascb->currentItem());
			}
		}
	}
	else if (type == P3D || type == PQWT3D) {
		kDebug()<<"	P3D or PQWT3D"<<endl;

		if (item == 0 || item == 3 || item == 6 || item == 9) {
			kDebug()<<"	X"<<endl;
			plot->setXRange(rmin,rmax);

			axis[0]->setScale((TScale)ascb->currentItem());

			axis[item]->enableMajorTicks(majortickscb->isChecked());
			axis[item]->enableMinorTicks(minortickscb->isChecked());
			if(majorle->text()==i18n("auto"))
				axis[item]->setMajorTicks(-1);
			else
				axis[item]->setMajorTicks(majorle->text().toDouble());
			axis[item]->setMinorTicks(minorle->text().toInt());
		}
		else if (item == 1 || item == 4 || item == 7 || item == 10) {
			kDebug()<<"	Y"<<endl;
			plot->setYRange(rmin,rmax);

			axis[1]->setScale((TScale)ascb->currentItem());

			axis[item]->enableMajorTicks(majortickscb->isChecked());
			axis[item]->enableMinorTicks(minortickscb->isChecked());
			if(majorle->text()==i18n("auto"))
				axis[item]->setMajorTicks(-1);
			else
				axis[item]->setMajorTicks(majorle->text().toDouble());
			axis[item]->setMinorTicks(minorle->text().toInt());
		}
		else if (item == 2 || item == 5 || item == 8 || item == 11) {
			kDebug()<<"	Z"<<endl;
			plot->setZRange(rmin,rmax);

			axis[2]->setScale((TScale)ascb->currentItem());

			axis[item]->enableMajorTicks(majortickscb->isChecked());
			axis[item]->enableMinorTicks(minortickscb->isChecked());
			if(majorle->text()==i18n("auto"))
				axis[item]->setMajorTicks(-1);
			else
				axis[item]->setMajorTicks(majorle->text().toDouble());
			axis[item]->setMinorTicks(minorle->text().toInt());
		}
	}

	axis[item]->enableMajorGrid(majorgridcb->isChecked());
	axis[item]->enableMinorGrid(minorgridcb->isChecked());
	axis[item]->enableBorder(bordercb->isChecked());
	p->updatePixmap();

	return 0;
}
*/
