#include "AxesWidget.h"

AxesWidget::AxesWidget(QWidget* parent):QWidget(parent){

	ui.setupUi(this);
	plotType=PLOT2D;

	//Populate the comboboxes
	QFont symbol("Symbol", 15, QFont::Bold);
 	ui.cbSmallGreekLetters->setFont(symbol);
	for(int i=97;i<122;i++)
		ui.cbSmallGreekLetters->addItem(QChar(i));

	ui.cbBigGreekLetters->setFont(symbol);
	for(int i=68;i<90;i++)
		ui.cbBigGreekLetters->addItem(QChar(i));

	ui.cbSymbolLetters->setFont(symbol);
 	ui.cbSymbolLetters->addItem(QChar(34));
	ui.cbSymbolLetters->addItem(QChar(36));
	ui.cbSymbolLetters->addItem(QChar(39));
	ui.cbSymbolLetters->addItem(QChar(64));
	for(int i=160;i<255;i++)
		ui.cbSymbolLetters->addItem(QChar(i));

	//Validators
	ui.leLowerLimit->setValidator( new QDoubleValidator(ui.leLowerLimit) );
	ui.leUpperLimit->setValidator( new QDoubleValidator(ui.leUpperLimit) );
	ui.leZeroOffset->setValidator( new QDoubleValidator(ui.leZeroOffset) );
	ui.leScaleFactor->setValidator( new QDoubleValidator(ui.leScaleFactor) );
	ui.leTitlePositionX->setValidator( new QDoubleValidator(ui.leTitlePositionX) );
	ui.leTitlePositionY->setValidator( new QDoubleValidator(ui.leTitlePositionY) );
	ui.leTitleRotation->setValidator( new QDoubleValidator(ui.leTitleRotation) );
	ui.leMajorTicksNumber->setValidator( new QIntValidator(ui.leMajorTicksNumber) );
	ui.leMinorTicksNumber->setValidator( new QIntValidator(ui.leMinorTicksNumber) );
	ui.leTitleRotation->setValidator( new QDoubleValidator(ui.leTitleRotation) );

	//Slots
	connect( ui.cbAxes, SIGNAL(stateChanged(int)), this, SLOT(currentAxisChanged(int)) );

	//"Title"-tab
	connect( ui.cbTitlePosition, SIGNAL(currentIndexChanged(int)), this, SLOT(titlePositionChanged(int)) );
	connect( ui.rbTitleFilling0, SIGNAL(toggled(bool)), this, SLOT( titleFillingChanged(bool)) );
	connect( ui.rbTitleFilling1, SIGNAL(toggled(bool)), this, SLOT( titleFillingChanged(bool)) );
//TODO ??? 	connect( ui.bTitleFillingColour, SIGNAL(clicked()), this, SLOT( titleFillingColourClicked()) );
	connect( ui.bTitleFont, SIGNAL(clicked()), this, SLOT( titleFontClicked()) );
	connect( ui.bTitleFontColour, SIGNAL(clicked()), this, SLOT( titleFontColourClicked()) );
	connect( ui.chbTitleUseTex, SIGNAL(stateChanged(int)), this, SLOT(titleUseTexChanged(int)) );
	connect( ui.bTitleTextBold, SIGNAL(clicked()), this, SLOT( titleFontBoldClicked()) );
	connect( ui.bTitleTextItalic, SIGNAL(clicked()), this, SLOT( titleFontItalicClicked()) );
	connect( ui.bTitleTextUnderline, SIGNAL(clicked()), this, SLOT( titleFontUnderlineClicked()) );
	connect( ui.bTitleTextUp, SIGNAL(clicked()), this, SLOT( titleFontUpClicked()) );
	connect( ui.bTitleTextDown, SIGNAL(clicked()), this, SLOT( titleFontDownClicked()) );
	connect( ui.cbSmallGreekLetters, SIGNAL(activated(const QString &)), this, SLOT(insertSymbol(const QString &)));
	connect( ui.cbBigGreekLetters, SIGNAL(activated(const QString &)), this, SLOT(insertSymbol(const QString &)));
	connect( ui.cbSymbolLetters, SIGNAL(activated(const QString &)), this, SLOT(insertSymbol(const QString &)));

	//"Ticks"-tab
	connect( ui.cbTicksStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(ticksStyleChanged(int)) );
	connect( ui.bTicksColour, SIGNAL(clicked()), this, SLOT( ticksColourClicked()) );

	//"Tick labels"-tab
	connect( ui.cbLabelsFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(labelFormatChanged(int)) );
	connect( ui.bLabelsFont, SIGNAL(clicked()), this, SLOT( labelFontClicked()) );
	connect( ui.bLabelsColour, SIGNAL(clicked()), this, SLOT( labelColourClicked()) );

	//"Grid"-tab
	connect( ui.sbMajorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMajorGridStyles()) );
	//TODO colour
	connect( ui.sbMinorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMinorGridStyles()) );
	//TODO colour
}


/*!
	sets the current plot type for the axes to be edited in this widget.
	Depending on the current type adjusts the appearance of the widget.
*/
void AxesWidget::setPlotType(const PlotType& type){
	//TODO
	if(type == PLOTPIE) {
// 		ui.tab2->hide();
// 		ui.tab3->hide();
// 		ui.tab5->hide();
	}else if (type == PLOTPOLAR) {
// 		minle->setEnabled(false);
// 		maxle->setEnabled(false);
	}

	if(type != PLOTQWT3D) {
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
void AxesWidget::setAxesData(const QList<Axis>& axes, const int axisNumber){
	//Create a local working copy of the axes list
	listAxes = axes;

	//Fill the ComboBox
	ui.cbAxes->clear();
	for (int i=0; i<listAxes.size(); i++){
		ui.cbAxes->addItem( listAxes[i].getLabel()->Text() );
	}

	//Show the settings for the current axis
	currentAxis=axisNumber;
	this->currentAxisChanged(currentAxis);
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
	Is called when the user changes the current axis in the corepsonding ComboBox
*/
void AxesWidget::currentAxisChanged(int const number){
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
//*****************

	Axis* axis=&listAxes.at(currentAxis);

	//*************************  "General"-tab  ************************************
	ui.chbAxisEnabled->setChecked( axis->isEnabled() );
	ui.cbStyle->setCurrentItem( axis->getStyle() );
	ui.cbPosition->setCurrentItem( axis->getPosition() );

	double rmin, rmax; //TODO here the Plot-object is needed!!!
	ui.leLowerLimit->setText( QString::number(rmin) );
	ui.leUpperLimit->setText( QString::number(rmax) );
	ui.leZeroOffset->setText( QString::number(axis->getShift()) );
	ui.leScaleFactor->setText( QString::number(axis->getScaleFactor()) );

	ui.chbBorderEnabled->setChecked( axis->isBorder() );
	//TODO border colour
	ui.sbBorderWidth->setValue( axis->getBorderWidth() );

	//*******************   "Title"-tab  *************************************
	ui.cbTitlePosition->setCurrentItem( axis->getTitlePosition() );
	ui.leTitlePositionX->setText( axis->getTitlePositionX() );
	ui.leTitlePositionY->setText( axis->getTitlePositionY() );
	if ( axis->getTitleFilling() == 0 )
		ui.rbTitleFilling0->setChecked(true);
	else
		ui.rbTitleFilling1->setChecked(true);

	//TODO fiiling colour
	ui.chbTitleBox->setChecked( axis->isTitleBoxEnabled() );
	ui.chbTitleShadow->setChecked( axis->isTitleShadowEnabled() );
	//TODO text colour and font
	ui.chbTitleShadow->setChecked( axis->isTitleTexUsed() );
	ui.teTitleLabel->setText( axis->getLabel() );


	//*******************   "Ticks"-tab  *************************************
	//TODO
	//i=0;
	//while(tickpositems[i] != 0) tickposcb->insertItem(i18n(tickpositems[i++]));

	ui.cbTicksPosition->setCurrentItem( axis->getTicksPosition() );
	ui.cbTicksStyle->setCurrentItem( axis->getTicksStyle() );
	//TODO ticks colour
	//tcb = new KColorButton(axis[axesnr]->TickColor(),hb);

	ui.chbMajorTicks->setChecked( axis->majorTicksEnabled() );
	//TODO ???
// 	majorlabel = new QLabel(i18n("Number : "),hb);
// 	if(axis[axesnr]->tickType() == 1)
// 		majorlabel->setText(i18n("Increment : "));
// 	majortickscb->setChecked(axis[axesnr]->MajorTicksEnabled());
// 	if(axis[axesnr]->MajorTicks()==-1)
// 		majorle = new KLineEdit(i18n("auto"),hb);
// 	else
// 		majorle = new KLineEdit(QString::number(axis[axesnr]->MajorTicks()),hb);

	ui.sbMajorTicksWidth->setValue(axis->getMajorTicksWidth() );
	ui.chbMinorTicks->setChecked( axis->minorTicksEnabled() );

// 	new QLabel(i18n("Number : "),hb);
// 	minorle = new KLineEdit(QString::number(axis[axesnr]->MinorTicks()),hb);
// 	minorle->setValidator(new QIntValidator(minorle));
// 	hb = new QHBox(minortickgb);
	ui.sbMajorTicksWidth->setValue(axis->getMajorTicksWidth() );

	if(plotType == PLOTQWT3D) {
		ui.sbMajorTicksWidth->setValue(axis->getMajorTicksWidth() );
		ui.sbMajorTicksWidth->setValue(axis->getMinorTicksWidth() );
	}


	//*******************   "Tick labels"-tab  ************************************
	ui.chbLabels->setChecked( axis->labelsEnabled() );
	ui.cbLabelsPosition->setCurrentItem( axis->getLabelsPosition() );
	ui.leLabelsRotation->setText( QString::number(axis->getLabelsRotation()) );

	ui.cbLabelsPrecision->setValue( axis->getLabelsPrecision() );
	ui.cbLabelsFormat->setCurrentItem( axis->getLabelsFormat() );

	//TODO
// 	i=0;
// 	while(tickformatitems[i] != 0) atlfcb->insertItem(i18n(tickformatitems[i++]));
// 	atlfcb->setCurrentItem(axis[axesnr]->TickLabelFormat());
	ui.leLabelsDateFormat->setText( axis->getLabelsDateFormat() );
	if ( ui.cbLabelsDateFormat->currentText() != i18n("time") && ui.cbLabelsDateFormat->currentText() != i18n("date")
		&& ui.cbLabelsDateFormat->currentText() != i18n("datetime"))
		ui.leLabelsDateFormat->setDisabled("true");


	//TODO font and colour
	ui.leLabelsPrefix->setText( axis->getLabelsPrefix() );
	ui.leLabelsSuffix->setText( axis->getLabelsSuffix() );


	//*******************   "Grid"-tab  ************************************
	ui.chbMajorGrid->setChecked( axis->isMajorGridEnabled() );
	ui.cbMajorGridStyle->setCurrentItem( axis->getMajorGridStyle() );
	//TODO colour
	ui.sbMajorGridWidth->setValue( axis->getMajorGridWidth() );

	ui.chbMajorGrid->setChecked( axis->isMajorGridEnabled() );
	ui.cbMajorGridStyle->setCurrentItem( axis->getMajorGridStyle() );
	//TODO colour
	ui.sbMajorGridWidth->setValue( axis->getMajorGridWidth() );
	*/
}

//"General"-tab

//"Title"-tab
/*!
	called if the current position of the title is changed in the combobox.
	Enables/disables the lineedits for x- and y-coordinates if the "custom"-item is selected/deselected
*/
void AxesWidget::titlePositionChanged(int index){
	if (index == ui.cbTitlePosition->count()-1 ){
		ui.leTitlePositionX->setEnabled(true);
		ui.leTitlePositionY->setEnabled(true);
	}else{
		ui.leTitlePositionX->setEnabled(false);
		ui.leTitlePositionY->setEnabled(false);
	}
}

/*!

*/
void AxesWidget::titleFillingChanged(bool){
	if (ui.rbTitleFilling0->isChecked())
		ui.bTitleFillingColour->setEnabled(false);
	else
		ui.bTitleFillingColour->setEnabled(true);
}

void AxesWidget::titleFillingColourClicked(){

}

void AxesWidget::titleFontClicked(){

}

void AxesWidget::titleFontColourClicked(){

}

void AxesWidget::titleUseTexChanged(int state){
	if (state==Qt::Checked){
		//use Tex-syntax
		ui.frameTitleTextOptions->setEnabled(false);
		//TODO remove bold-, italic- etc. properties in texteditwidget
	}else{
		ui.frameTitleTextOptions->setEnabled(true);
	}
}

void AxesWidget::titleFontBoldClicked(){
// 	if (ui.bTitleTextBold->isChecked())
// 		ui.teTitleLabel->setFontBold(true);
// 	else
// 		ui.teTitleLabel->setFontBold(false);
}


void AxesWidget::titleFontItalicClicked(){
// 	if (ui.bTitleTextItalic->isChecked())
// 		ui.teTitleLabel->setFontItalic(true);
// 	else
// 		ui.teTitleLabel->setFontItalic(false);
}

void AxesWidget::titleFontUnderlineClicked(){
	if (ui.bTitleTextUnderline->isChecked())
		ui.teTitleLabel->setFontUnderline(true);
	else
		ui.teTitleLabel->setFontUnderline(false);
}

void AxesWidget::titleFontUpClicked(){
//TODO
}


void AxesWidget::titleFontDownClicked(){
//TODO
}

void AxesWidget::insertSymbol(const QString& string) {
	ui.teTitleLabel->setFontFamily("Symbol");
	ui.teTitleLabel->insertPlainText(string);
}


//"Ticks"-tab
void AxesWidget::ticksColourClicked(){

}

/*!
	called if the current style of the ticks is changed. Adjustes the names of the corresponding labels in UI.
*/
void AxesWidget::ticksStyleChanged(int index) {
	if (index==0){
		ui.lMajorTicksNumber->setText( i18n("Number") );
		ui.lMinorTicksNumber->setText( i18n("Number") );
	}else{
		ui.lMajorTicksNumber->setText( i18n("Increment") );
		ui.lMinorTicksNumber->setText( i18n("Increment") );
	}
}


//"Tick labels"-tab
void AxesWidget::labelFontClicked(){

}

void AxesWidget::labelColourClicked(){

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
// 	int h=ui.cbMajorGridStyle->height();
	int h=10;
	int offset=5;
	QPixmap pm( w, h );
// 	ui.cbMajorGridStyle->setIconSize( QSize(w, ui.cbMajorGridStyle->iconSize().height() ) );
	ui.cbMajorGridStyle->setIconSize( QSize(w,h) );

// 	QColor penColor = listAxes.at(currentAxis).MajorGridColor();
// 	int penWidth = listAxes.at(currentAxis).MajorGridWidth();
 	QColor penColor = Qt::black;
	int penWidth = ui.sbMajorGridWidth->value();

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
// 	int h=ui.cbMinorGridStyle->height();
	int h=10;
	int offset=5;
	QPixmap pm( w, h );
// 	ui.cbMajorGridStyle->setIconSize( QSize(w, ui.cbMajorGridStyle->iconSize().height() ) );
	ui.cbMinorGridStyle->setIconSize( QSize(w,h) );

// 	QColor penColor = listAxes.at(currentAxis).MinorGridColor();
// 	int penWidth = listAxes.at(currentAxis).MinorGridWidth();
 	QColor penColor = Qt::black;
 	int penWidth = ui.sbMinorGridWidth->value();

	for (int i=1;i<6;i++) {
		pm.fill(Qt::transparent);
		painter.begin( &pm );
		painter.setPen( QPen( penColor, penWidth, (Qt::PenStyle)i ) );
		painter.drawLine( offset, h/2, w-offset, h/2);
		painter.end();
		ui.cbMinorGridStyle->addItem( QIcon(pm), "" );
	}
	ui.cbMajorGridStyle->setCurrentIndex(index);

	//TODO add the possibility to add/create custom penstyles?
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

void AxesDialog::centerEnabled(bool on) {
	// set position xle,yle readonly
	rtw->setPostionReadOnly(on);
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
