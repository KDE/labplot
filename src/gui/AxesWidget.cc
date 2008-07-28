#include "AxesWidget.h"
#include "LabelWidget.h"
#include <KDebug>

AxesWidget::AxesWidget(QWidget* parent):QWidget(parent){

	ui.setupUi(this);
	plotType=Plot::PLOT2D;

	//create a LabelWidget in the "Title"-tab
    QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget=new LabelWidget(ui.tabTitle);
    hboxLayout->addWidget(labelWidget);

	//Validators
	ui.leLowerLimit->setValidator( new QDoubleValidator(ui.leLowerLimit) );
	ui.leUpperLimit->setValidator( new QDoubleValidator(ui.leUpperLimit) );
	ui.leZeroOffset->setValidator( new QDoubleValidator(ui.leZeroOffset) );
	ui.leScaleFactor->setValidator( new QDoubleValidator(ui.leScaleFactor) );
	ui.leMajorTicksNumber->setValidator( new QIntValidator(ui.leMajorTicksNumber) );
	ui.leMinorTicksNumber->setValidator( new QIntValidator(ui.leMinorTicksNumber) );


	//TODO move this stuff to retranslateUI()
	ui.cbTicksPosition->addItem( i18n("in") );
	ui.cbTicksPosition->addItem( i18n("out") );
	ui.cbTicksPosition->addItem( i18n("in and out") );
	ui.cbTicksPosition->addItem( i18n("none") );

	//Tick labels format
	 ui.cbLabelsFormat->addItem( i18n("automatic") );
	 ui.cbLabelsFormat->addItem( i18n("normal") );
	 ui.cbLabelsFormat->addItem( i18n("scientific") );
	 ui.cbLabelsFormat->addItem( i18n("power of 10") );
	 ui.cbLabelsFormat->addItem( i18n("power of 2") );
	 ui.cbLabelsFormat->addItem( i18n("power of e") );
	 ui.cbLabelsFormat->addItem( i18n("sqrt") );
	 ui.cbLabelsFormat->addItem( i18n("time") );
	 ui.cbLabelsFormat->addItem( i18n("date") );
	 ui.cbLabelsFormat->addItem( i18n("datetime") );
	 ui.cbLabelsFormat->addItem( i18n("degree") );

	//Grid
// 	 ui.kcbMajorGridColor->setColor(Qt::black);
// 	 this->createMajorGridStyles();
// 	 ui.kcbMinorGridColor->setColor(Qt::black);
// 	 this->createMinorGridStyles();
//
	//Slots
	connect( ui.cbAxes, SIGNAL(stateChanged(int)), this, SLOT(currentAxisChanged(int)) );

	//"General"-tab
	//TODO add slot for position (show text filed if "custom" selected)


	//"Ticks"-tab
	connect( ui.cbTicksStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(ticksStyleChanged(int)) );

	//"Tick labels"-tab
	connect( ui.cbLabelsFormat, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(labelFormatChanged(const QString&)) );

	//"Grid"-tab
	connect( ui.sbMajorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMajorGridStyles()) );
	connect( ui.kcbMajorGridColor, SIGNAL(changed (const QColor &)), this, SLOT(createMajorGridStyles()) );
	connect( ui.sbMinorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMinorGridStyles()) );
	connect( ui.kcbMinorGridColor, SIGNAL(changed (const QColor &)), this, SLOT(createMinorGridStyles()) );
}


/*!
	sets the current plot type for the axes to be edited in this widget.
	Depending on the current type adjusts the appearance of the widget.
*/
void AxesWidget::setPlotType(const Plot::PlotType& type){
	QStringList list;
	if (type == Plot::PLOT2D || type == Plot::PLOTSURFACE) {
		list<<QString("x")<<QString("y")<<QString("y2")<<QString("x2");
	}else if (type == Plot::PLOT3D || type == Plot::PLOTQWT3D){
		list<<QString("x")<<QString("y")<<QString("z")<<QString("x2")<<QString("x3")<<QString("x4");
		list<<QString("y4")<<QString("y3")<<QString("y2")<<QString("z2")<<QString("z4")<<QString("z3");
	}else if (type == Plot::PLOTPOLAR) {
		list<<QString("phi")<<QString("r");
	}else if (type == Plot::PLOTTERNARY || type == Plot::PLOTPIE) {
		list<<QString("a");
	}

	ui.cbAxes->clear();
	ui.cbAxes->insertItems( -1, list );

	//TODO
	if(type == Plot::PLOTPIE) {
// 		ui.tab2->hide();
// 		ui.tab3->hide();
// 		ui.tab5->hide();
	}else if (type == Plot::PLOTPOLAR) {
// 		minle->setEnabled(false);
// 		maxle->setEnabled(false);
	}

	if(type != Plot::PLOTQWT3D) {
		ui.lMajorTicksLength->hide();
		ui.sbMajorTicksLength->hide();
		ui.lMinorTicksLength->hide();
		ui.sbMinorTicksLength->hide();
	}

	plotType=type;
}

/*!
	sets the pointer \c axes to list containing all the axes. \c axisNumber is the number of the axis to be edited.<br>
*/
void AxesWidget::setAxesList(const QList<Axis> axes, const int axis){
	//Create a local working copy of the axes list
	listAxes = axes;

	//Fill the ComboBox
	ui.cbAxes->clear();
	for (int i=0; i<listAxes.size(); i++){
		ui.cbAxes->addItem( listAxes[i].label()->text() );
	}

	//Show the settings for the current axis
	this->currentAxisChanged(axis);
}

/*!

*/
void AxesWidget::saveAxesData(QList<Axis>* axes) const{
	//TODO

	if ( ui.chbUseAsDefault->isChecked() ){

	}
}

void AxesWidget::restoreDefaults(){
	if (QMessageBox::warning(this,
		i18n("Default axes values"), tr("All settings will be reset to the default values. Do you want to continue?"),
			QMessageBox::Yes | QMessageBox::No)==QMessageBox::Yes){

		//TODO
	}
}


//**********************************************************
//****************** SLOTS ********************************
//**********************************************************
/*!
 	Shows the data for the axis \c number.
	Is called when the user changes the current axis in the coresponding ComboBox
*/
void AxesWidget::currentAxisChanged(int index){
	if (!dataChanged)
		return;

	currentAxis=index;

	//TODO
/*
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
*/

	/*
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
	*/
//*****************

	const Axis* axis=&listAxes.at(currentAxis);

	//*************************  "General"-tab  ************************************
	ui.chbAxisEnabled->setChecked( axis->isAxisEnabled() );
	ui.cbScaleType->setCurrentIndex( axis->scaleType() );
	ui.cbPosition->setCurrentIndex( axis->position() );

	ui.leLowerLimit->setText( QString::number(axis->lowerLimit()) );
	ui.leUpperLimit->setText( QString::number(axis->upperLimit()) );
	ui.leZeroOffset->setText( QString::number(axis->offset()) );
	ui.leScaleFactor->setText( QString::number(axis->scaleFactor()) );

	ui.chbBorder->setChecked( axis->hasBorder() );
	ui.kcbBorderColor->setColor( axis->borderColor() );
	ui.sbBorderWidth->setValue( axis->borderWidth() );

	//*******************   "Title"-tab  *************************************
	//TODO !!!!!!!!!!!!!!
// 	const Label* label=axis->label();
// 	labelWidget->setLabel( label );


	//*******************   "Ticks"-tab  *************************************
	ui.cbTicksPosition->setCurrentIndex( axis->ticksPosition() );
	ui.cbTicksStyle->setCurrentIndex( axis->ticksType() );
	ui.kcbTicksColor->setColor( axis->ticksColor() );

	ui.chbMajorTicks->setChecked( axis->hasMajorTicks() );
	if (axis->majorTicksNumber()==-1){
		ui.cbMajorTicksNumber->setCurrentIndex(0);
		ui.leMajorTicksNumber->setText( "" );
	}else{
		ui.cbMajorTicksNumber->setCurrentIndex(1);
		ui.leMajorTicksNumber->setText( QString::number(axis->majorTicksNumber()) );
	}
	ui.sbMajorTicksWidth->setValue( axis->majorTicksWidth() );
	ui.sbMajorTicksLength->setValue( axis->majorTicksLength() );

	ui.chbMinorTicks->setChecked( axis->minorTicksEnabled() );
	if (axis->minorTicksNumber()==-1){
		ui.cbMinorTicksNumber->setCurrentIndex(0);
		ui.leMinorTicksNumber->setText( "" );
	}else{
		ui.cbMinorTicksNumber->setCurrentIndex(1);
		ui.leMinorTicksNumber->setText( QString::number(axis->minorTicksNumber()) );
	}
	ui.sbMinorTicksWidth->setValue( axis->minorTicksWidth() );
	ui.sbMinorTicksLength->setValue( axis->minorTicksLength() );


	//TODO ???
// 	if(plotType == PLOTQWT3D) {
// 		ui.sbMajorTicksWidth->setValue(axis->getMajorTicksWidth() );
// 		ui.sbMajorTicksWidth->setValue(axis->getMinorTicksWidth() );
// 	}


	//*******************   "Tick labels"-tab  ************************************
	ui.chbLabels->setChecked( axis->hasLabels() );
	ui.cbLabelsPosition->setCurrentIndex( axis->labelsPosition() );
	ui.leLabelsRotation->setText( QString::number(axis->labelsRotation()) );

	ui.sbLabelsPrecision->setValue( axis->labelsPrecision() );
	ui.cbLabelsFormat->setCurrentIndex( axis->labelsFormat() );
	ui.leLabelsDateFormat->setText( axis->labelsDateFormat() );

	ui.kfontrequesterLabels->setFont( axis->labelsFont() );
	ui.kcbLabelsColor->setColor( axis->labelsColor() );
	ui.leLabelsPrefix->setText( axis->labelsPrefix() );
	ui.leLabelsSuffix->setText( axis->labelsSuffix() );


	//*******************   "Grid"-tab  ************************************
	ui.chbMajorGrid->setChecked( axis->hasMajorGrid() );
	ui.cbMajorGridStyle->setCurrentIndex( axis->majorGridStyle() );
	ui.kcbMajorGridColor->setColor( axis->majorGridColor() );
	ui.sbMajorGridWidth->setValue( axis->majorGridWidth() );

	ui.chbMinorGrid->setChecked( axis->hasMinorGrid() );
	ui.cbMinorGridStyle->setCurrentIndex( axis->minorGridStyle() );
	ui.kcbMinorGridColor->setColor( axis->minorGridColor() );
	ui.sbMinorGridWidth->setValue( axis->minorGridWidth() );
}



//"Ticks"-tab
/*!
	called if the current style of the ticks (Number or Increment) is changed. Adjustes the names of the corresponding labels in UI.
*/
void AxesWidget::ticksStyleChanged(int index){
	if (index==0){
		ui.lMajorTicksNumber->setText( i18n("Number") );
		ui.lMinorTicksNumber->setText( i18n("Number") );
	}else{
		ui.lMajorTicksNumber->setText( i18n("Increment") );
		ui.lMinorTicksNumber->setText( i18n("Increment") );
	}
}

/*!
	called if the Number-Style (auto or custom) is changed in the corresponding ComboBox.
	Enables/disables the text field for the number of major ticks if custom/auto is selected.
*/
void AxesWidget::majorTicksChanged(int index){
	ui.leMajorTicksNumber->setEnabled((bool)index);
}


/*!
	called if the Number-Style (auto or custom) is changed in the corresponding ComboBox.
	Enables/disables the textfield for the number of minor ticks if custom/auto is selected.
*/
void AxesWidget::minorTicksChanged(int index){
	ui.leMinorTicksNumber->setEnabled((bool)index);
}

//"Tick labels"-tab
/*!
	called if the ticks format is changed in the corresponding ComboBox.
	Enables an additional textfield for "Date-Format" if time/date/datetime selected.
	Disables this field otherwise.
*/
void AxesWidget::labelFormatChanged(const QString& text){
	if ( text != i18n("time") && text != i18n("date") && text != i18n("datetime") )
		ui.leLabelsDateFormat->setEnabled(false);
	else
		ui.leLabelsDateFormat->setEnabled(true);
}


//"Grid"-tab
/*!
	inserts five possible Qt pen styles for the major grid.<br>
	The pixmaps where the styles are painted on have the current width of the coresponding ComboBox.
	//TODO call this function in the resize-event to adjust the width of the pixmaps.
*/
void AxesWidget::createMajorGridStyles(){
	int index=ui.cbMajorGridStyle->currentIndex();
	ui.cbMajorGridStyle->clear();

	//TODO
	//It's not possible to insert a QPixmap-Object with the true pixmap size.
	//using QIcon scales down the size of the pixmap. So playing around with setIconSize is needed
	//It' not so nice. Look for another possiblity (rewrite combobox and use QIconView to show the pixmaps?)
	QPainter painter;
	int w=ui.cbMajorGridStyle->width()-50;
	int h=10;
	int offset=5;
	QPixmap pm( w, h );
	ui.cbMajorGridStyle->setIconSize( QSize(w,h) );
 	QColor penColor = ui.kcbMajorGridColor->color();
	int penWidth = ui.sbMajorGridWidth->value();

	//loop over six possible PenStyles, draw on the pixmap and insert it
	for (int i=1;i<6;i++) {
		pm.fill(Qt::transparent);
		painter.begin( &pm );
		painter.setPen( QPen( penColor, penWidth, (Qt::PenStyle)i ) );
		painter.drawLine( offset, h/2, w-offset, h/2);
		painter.end();
		ui.cbMajorGridStyle->addItem( QIcon(pm), "" );
	}
	ui.cbMajorGridStyle->setCurrentIndex(index);

	//TODO add the possibility to add/create custom penstyles?
	kDebug()<<"major grid styles updated"<<endl;
}

/*!
	inserts five possible Qt pen styles for the minor grid.<br>
	The pixmaps where the styles are painted on have the current width of the coresponding ComboBox.
	//TODO call this function in the resize-event to adjust the width of the pixmaps.
*/
void AxesWidget::createMinorGridStyles(){
	int index=ui.cbMajorGridStyle->currentIndex();
	ui.cbMinorGridStyle->clear();

	QPainter painter;
	int w=ui.cbMinorGridStyle->width()-50;
	int h=10;
	int offset=5;
	QPixmap pm( w, h );
	ui.cbMinorGridStyle->setIconSize( QSize(w,h) );

	QColor penColor = ui.kcbMinorGridColor->color();
 	int penWidth = ui.sbMinorGridWidth->value();

	//loop over six possible PenStyles, draw on the pixmap and insert it
	for (int i=1;i<6;i++) {
		pm.fill(Qt::transparent);
		painter.begin( &pm );
		painter.setPen( QPen( penColor, penWidth, (Qt::PenStyle)i ) );
		painter.drawLine( offset, h/2, w-offset, h/2);
		painter.end();
		ui.cbMinorGridStyle->addItem( QIcon(pm), "" );
	}
	ui.cbMajorGridStyle->setCurrentIndex(index);

	kDebug()<<"minor grid styles updated"<<endl;
}

//! update values if scale changed
/*
void AxesDialog::updateScale(int i) {

	if(i==LINEAR)
		minorle->setText(QString("3"));
	else if (i==LOG10)
		minorle->setText(QString("8"));
	else
		minorle->setText(QString("0"));
}
*/


//! called when axis tic format is changed : update format line edit & ranges
/*
void AxesDialog::update_timeformat() {
	int item = axescb->currentItem();
	kdDebug()<<"ATLF = "<<atlfcb->currentText()<<endl;
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
*/

/*
void AxesDialog::axisEnabled(bool on) {

	kdDebug()<<"AxesDialog::axisEnabled() : "<<on<<endl;
	int item = axescb->currentItem();
	if (on) {	// axis should be enabled
		axis[item]->enableBorder();
		bordercb->setChecked(true);
		// TODO : show the label position in richtext widget (rtw)
		rtw->setLabel(axis[item]->getLabel());
	}

}

*/

//! update the selected axis
/*
void AxesDialog::updateAxis(int i) {
	/*
	kdDebug()<<"AxesDialog::updateAxis()"<<endl;
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
*/

/*
void AxesWidget::selectTickFont() {

    bool res;
    QFont font = QFontDialog::getFont( &res,axis[axescb->currentItem()]->TickLabelFont(), this );
    if(res) {
	tf = font;
	tickfont->setText(tf.family() + tr(" ") + QString::number(tf.pointSize()));
    }

}
*/

/*
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
*/

void AxesWidget::apply(QList<Axis>*) const{
	/*
	kdDebug()<<"AxesDialog::apply_clicked()"<<endl;
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
	kdDebug()<<"	TYPE = "<<type<<endl;
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
		kdDebug()<<"	P3D or PQWT3D"<<endl;

		if (item == 0 || item == 3 || item == 6 || item == 9) {
			kdDebug()<<"	X"<<endl;
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
			kdDebug()<<"	Y"<<endl;
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
			kdDebug()<<"	Z"<<endl;
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
	*/
}

AxesWidget::~AxesWidget(){
}
