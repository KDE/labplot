#include "AxesWidget.h"
#include "LabelWidget.h"
#include <KDebug>

AxesWidget::AxesWidget(QWidget* parent):QWidget(parent){

	ui.setupUi(this);
	plotType=Plot::PLOT2D;
	initializing=false;
	m_dataChanged=false;

	//create a LabelWidget in the "Title"-tab
    QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget=new LabelWidget(ui.tabTitle);
    hboxLayout->addWidget(labelWidget);

	//Validators
	ui.lePositionOffset->setValidator( new QDoubleValidator(ui.lePositionOffset) );
	ui.leLowerLimit->setValidator( new QDoubleValidator(ui.leLowerLimit) );
	ui.leUpperLimit->setValidator( new QDoubleValidator(ui.leUpperLimit) );
	ui.leZeroOffset->setValidator( new QDoubleValidator(ui.leZeroOffset) );
	ui.leScaleFactor->setValidator( new QDoubleValidator(ui.leScaleFactor) );
	ui.leMajorTicksNumber->setValidator( new QIntValidator(ui.leMajorTicksNumber) );
	ui.leMajorTicksLength->setValidator( new QDoubleValidator(ui.leMajorTicksLength) );
	ui.leMinorTicksNumber->setValidator( new QIntValidator(ui.leMinorTicksNumber) );
	ui.leMinorTicksLength->setValidator( new QDoubleValidator(ui.leMinorTicksLength) );
	ui.leLabelsRotation->setValidator( new QIntValidator(ui.leLabelsRotation) );

	//TODO move this stuff to retranslateUI()
	ui.cbScaleType->addItem( i18n("linear") );
	ui.cbScaleType->addItem( i18n("log 10") );
	ui.cbScaleType->addItem( i18n("log 2") );
	ui.cbScaleType->addItem( i18n("log e") );
	ui.cbScaleType->addItem( i18n("sqrt") );
	ui.cbScaleType->addItem( i18n("x^2") );

	ui.cbPosition->addItem( i18n("normal") );
	ui.cbPosition->addItem( i18n("center") );
	ui.cbPosition->addItem( i18n("custom") );

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
	 ui.kcbMajorGridColor->setColor(Qt::black);
	 this->createMajorGridStyles();
	 ui.kcbMinorGridColor->setColor(Qt::black);
	 this->createMinorGridStyles();

	//**********************************  Slots **********************************************
	connect( ui.cbAxes, SIGNAL(currentIndexChanged(int)), this, SLOT(currentAxisChanged(int)) );

	//"General"-tab
	connect( ui.chbAxis, SIGNAL(stateChanged(int)), this, SLOT(axisStateChanged(int)) );
	connect( ui.cbScaleType, SIGNAL(currentIndexChanged(int)), this, SLOT(scaleTypeChanged(int)) );
	connect( ui.cbPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(positionChanged(int)) );
	connect( ui.lePositionOffset, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	connect( ui.leLowerLimit, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
	connect( ui.leUpperLimit, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
	connect( ui.leZeroOffset, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
	connect( ui.leScaleFactor, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	connect( ui.chbBorder, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.kcbBorderColor, SIGNAL(changed(const QColor& )), this, SLOT(slotDataChanged()) );
	connect( ui.sbBorderWidth, SIGNAL(valueChanged(int)), this, SLOT(slotDataChanged()) );

	//"Title"-tab
	connect( labelWidget, SIGNAL(dataChanged(bool)), this, SLOT(slotDataChanged()) );

	//"Ticks"-tab
	connect( ui.cbTicksPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbTicksType, SIGNAL(currentIndexChanged(int)), this, SLOT(ticksTypeChanged(int)) );
	connect( ui.kcbTicksColor, SIGNAL(changed(const QColor& )), this, SLOT(slotDataChanged()) );

	connect( ui.chbMajorTicks, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbMajorTicksNumberType, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksNumberTypeChanged(int)) );
	connect( ui.leMajorTicksNumber, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
	connect( ui.sbMajorTicksWidth, SIGNAL(valueChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.leMajorTicksLength, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	connect( ui.chbMinorTicks, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbMinorTicksNumberType, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksNumberTypeChanged(int)) );
	connect( ui.leMinorTicksNumber, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
	connect( ui.sbMinorTicksWidth, SIGNAL(valueChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.leMinorTicksLength, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	//"Tick labels"-tab
	connect( ui.chbLabels, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.sbLabelsPosition, SIGNAL(valueChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.leLabelsRotation, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	connect( ui.sbLabelsPrecision, SIGNAL(valueChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbLabelsFormat, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(labelFormatChanged(const QString&)) );
	connect( ui.leLabelsDateFormat, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	connect( ui.kfrLabelsFont, SIGNAL(fontSelected(const QFont& )), this, SLOT(slotDataChanged()) );
	connect( ui.kcbLabelsColor, SIGNAL(changed(const QColor& )), this, SLOT(slotDataChanged()) );
	connect( ui.leLabelsPrefix, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
	connect( ui.leLabelsSuffix, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	//"Grid"-tab
	connect( ui.chbMajorGrid, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbMajorGridStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.kcbMajorGridColor, SIGNAL(changed (const QColor &)), this, SLOT(createMajorGridStyles()) );
	connect( ui.sbMajorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMajorGridStyles()) );

	connect( ui.chbMinorGrid, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbMinorGridStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.kcbMinorGridColor, SIGNAL(changed (const QColor &)), this, SLOT(createMinorGridStyles()) );
	connect( ui.sbMinorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMinorGridStyles()) );
}

AxesWidget::~AxesWidget(){}

void AxesWidget::resizeEvent(QResizeEvent * event){
	//TODO
// 	this->createMajorGridStyles();
// 	this->createMinorGridStyles();
}

/*!
	sets the current plot type for the axes to be edited in this widget.
	Adjusts the appearance of the widget depending on the current plot type.
*/
void AxesWidget::setPlotType(const Plot::PlotType& type){
	kDebug()<<"PlotType "<<type<<endl;
	QStringList list;
	if (type == Plot::PLOT2D || type == Plot::PLOTSURFACE) {
		list<<QString("x")<<QString("y")<<QString("x2")<<QString("y2");
	}else if (type == Plot::PLOT3D || type == Plot::PLOTQWT3D){
		//TODO change the ordering
		list<<QString("x")<<QString("y")<<QString("z")<<QString("x2")<<QString("x3")<<QString("x4");
		list<<QString("y4")<<QString("y3")<<QString("y2")<<QString("z2")<<QString("z4")<<QString("z3");
	}else if (type == Plot::PLOTPOLAR) {
		list<<QString("phi")<<QString("r");
	}else if (type == Plot::PLOTTERNARY || type == Plot::PLOTPIE) {
		list<<QString("a");
	}

	initializing=true;
	ui.cbAxes->clear();
  	ui.cbAxes->insertItems( -1, list );
	initializing=false;

	//TODO
	if(type == Plot::PLOTPIE) {
// 		ui.tab2->hide();
// 		ui.tab3->hide();
// 		ui.tab5->hide();
	}else if (type == Plot::PLOTPOLAR) {
// 		minle->setEnabled(false);
// 		maxle->setEnabled(false);
	}

	//Enable tick length for all plots
// 	if(type != Plot::PLOTQWT3D) {
// 		ui.lMajorTicksLength->hide();
// 		ui.sbMajorTicksLength->hide();
// 		ui.lMinorTicksLength->hide();
// 		ui.sbMinorTicksLength->hide();
// 	}

	plotType=type;
}

/*!
	sets the pointer \c axes to list containing all the axes. \c axisNumber is the number of the axis to be edited.<br>
*/
void AxesWidget::setAxes(QList<Axis>* axes, const int axis){
	kDebug()<<"";
	//Create a local working copy of the axes list
	for (int i = 0; i < axes->size(); ++i)
  		listAxes<<axes->at(i);

	//Show the settings for the current axis
	currentAxis=axis;
	initializing=true;
	this->showAxis(axis);
	initializing=false;
}

/*!
	shows the properties of the axis with index \c axisIndex in the UI.
*/
void AxesWidget::showAxis(const short axisIndex){
	const Axis* axis=&listAxes.at(axisIndex);

	//*************************  "General"-tab  ************************************
	ui.chbAxis->setChecked( axis->isEnabled() );
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
 	labelWidget->setLabel( const_cast<Axis*>(axis)->label() );


	//*******************   "Ticks"-tab  *************************************
	ui.cbTicksPosition->setCurrentIndex( axis->ticksPosition() );
	ui.cbTicksType->setCurrentIndex( axis->ticksType() );
	ui.kcbTicksColor->setColor( axis->ticksColor() );

	ui.chbMajorTicks->setChecked( axis->hasMajorTicks() );
	ui.leMajorTicksNumber->setText( QString::number(axis->majorTicksNumber()) );
	if (axis->majorTicksNumberType()==Axis::TICKSNUMBERTYPE_AUTO)
		ui.cbMajorTicksNumberType->setCurrentIndex(0);
	else
		ui.cbMajorTicksNumberType->setCurrentIndex(1);

	ui.sbMajorTicksWidth->setValue( axis->majorTicksWidth() );
	ui.leMajorTicksLength->setText( QString().setNum(axis->majorTicksLength()) );

	ui.chbMinorTicks->setChecked( axis->hasMinorTicks() );
	ui.leMinorTicksNumber->setText( QString::number(axis->minorTicksNumber()) );
	if (axis->minorTicksNumberType()==Axis::TICKSNUMBERTYPE_AUTO)
		ui.cbMinorTicksNumberType->setCurrentIndex(0);
	else
		ui.cbMinorTicksNumberType->setCurrentIndex(1);

	ui.sbMinorTicksWidth->setValue( axis->minorTicksWidth() );
	ui.leMinorTicksLength->setText( QString().setNum(axis->minorTicksLength()) );


	//*******************   "Tick labels"-tab  ************************************
	ui.chbLabels->setChecked( axis->hasLabels() );
	ui.sbLabelsPosition->setValue( axis->labelsPosition() );
	ui.leLabelsRotation->setText( QString::number(axis->labelsRotation()) );

	ui.sbLabelsPrecision->setValue( axis->labelsPrecision() );
	ui.cbLabelsFormat->setCurrentIndex( axis->labelsFormat() );
	ui.leLabelsDateFormat->setText( axis->labelsDateFormat() );

	ui.kfrLabelsFont->setFont( axis->labelsFont() );
	ui.kcbLabelsColor->setColor( axis->labelsColor() );
	ui.leLabelsPrefix->setText( axis->labelsPrefix() );
	ui.leLabelsSuffix->setText( axis->labelsSuffix() );


	//*******************   "Grid"-tab  ************************************
	ui.chbMajorGrid->setChecked( axis->hasMajorGrid() );
	ui.cbMajorGridStyle->setCurrentIndex( axis->majorGridStyle()-1 );
	ui.kcbMajorGridColor->setColor( axis->majorGridColor() );
	ui.sbMajorGridWidth->setValue( axis->majorGridWidth() );

	ui.chbMinorGrid->setChecked( axis->hasMinorGrid() );
	ui.cbMinorGridStyle->setCurrentIndex( axis->minorGridStyle()-1 );
	ui.kcbMinorGridColor->setColor( axis->minorGridColor() );
	ui.sbMinorGridWidth->setValue( axis->minorGridWidth() );

	//TODO old code:
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

	//TODO following lines were previously in updateAxis(). looks very similar to the above code. combine/port/remove.
	/*
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
	*/
}

/*!
	saves the localy/temporaly used axes to the QList* \c axes.
*/
void AxesWidget::saveAxes(QList<Axis>* axes){
	kDebug()<<"";
	//save the data, if changed in the UI
	if (m_dataChanged){
		this->saveAxis(currentAxis);
	}

	//copy all axes from the own local axes list to the axis list of the plot.
	//TODO Copy only if there were changes!!!
	axes->clear();
	for (int i = 0; i < listAxes.size(); ++i)
		axes->append( listAxes[i] );

	if ( ui.chbUseAsDefault->isChecked() ){
		if (QMessageBox::warning(this,
			i18n("Save default axes values"), tr("Default settings will be overwritten by the current values. Do you want to continue?"),
				QMessageBox::Yes | QMessageBox::No)==QMessageBox::Yes){

			//TODO
			QMessageBox::warning(this, i18n("Save default axes values"), i18n("Not yet implemented. Sorry.") );
		}
	}
}

/*!
	saves the data in the UI to the axis with the index \c axisIndex.
*/
void AxesWidget::saveAxis(const short axisIndex){
	Axis* axisSave=&listAxes[axisIndex];

	//*************************  "General"-tab  ************************************
	axisSave->setEnabled( ui.chbAxis->isChecked()  );
	axisSave->setScaleType( (Axis::ScaleType)ui.cbScaleType->currentIndex()  );
	axisSave->setPosition( (Axis::Position)ui.cbPosition->currentIndex()  );
	axisSave->setPositionOffset( ui.lePositionOffset->text().toDouble() );

	axisSave->setLowerLimit( ui.leLowerLimit->text().toDouble() );
	axisSave->setUpperLimit( ui.leUpperLimit->text().toDouble() );
	axisSave->setOffset( ui.leZeroOffset->text().toDouble() );
	axisSave->setScaleFactor( ui.leScaleFactor->text().toDouble() );

	axisSave->enableBorder( ui.chbBorder->isChecked()  );
	axisSave->setBorderColor( ui.kcbBorderColor->color()  );
	axisSave->setBorderWidth( ui.sbBorderWidth->value()  );


	//*******************   "Title"-tab  *************************************
	labelWidget->saveLabel( axisSave->label() );


	//*******************   "Ticks"-tab  *************************************
	axisSave->setTicksPosition( (Axis::TicksPosition)ui.cbTicksPosition->currentIndex()  );
	axisSave->setTicksType( (Axis::TicksType)ui.cbTicksType->currentIndex()  );
	axisSave->setTicksColor( ui.kcbTicksColor->color()  );

	axisSave->enableMajorTicks( ui.chbMajorTicks->isChecked() );
	axisSave->setMajorTicksNumberType( (Axis::TicksNumberType)ui.cbMajorTicksNumberType->currentIndex() );
	axisSave->setMajorTicksNumber( ui.leMajorTicksNumber->text().toInt() );
	axisSave->setMajorTicksWidth( ui.sbMajorTicksWidth->value() );
	axisSave->setMajorTicksLength( ui.leMajorTicksLength->text().toDouble() );

	axisSave->enableMinorTicks( ui.chbMinorTicks->isChecked() );
	axisSave->setMinorTicksNumberType( (Axis::TicksNumberType)ui.cbMinorTicksNumberType->currentIndex() );
	axisSave->setMinorTicksNumber( ui.leMinorTicksNumber->text().toInt() );
	axisSave->setMinorTicksWidth( ui.sbMinorTicksWidth->value() );
	axisSave->setMinorTicksLength( ui.leMinorTicksLength->text().toDouble() );


	//*******************   "Tick labels"-tab  ************************************
	axisSave->enableLabels( ui.chbLabels->isChecked() );
	axisSave->setLabelsPosition( ui.sbLabelsPosition->value() );
	axisSave->setLabelsRotation( ui.leLabelsRotation->text().toDouble() );

	axisSave->setLabelsPrecision( ui.sbLabelsPrecision->value()  );
	axisSave->setLabelsFormat( (Axis::LabelsFormat)ui.cbLabelsFormat->currentIndex()  );
	axisSave->setLabelsDateFormat( ui.leLabelsDateFormat->text() );

	axisSave->setLabelsFont( ui.kfrLabelsFont->font() );
	axisSave->setLabelsColor( ui.kcbLabelsColor->color() );
	axisSave->setLabelsPrefix( ui.leLabelsPrefix->text() );
	axisSave->setLabelsSuffix( ui.leLabelsSuffix->text() );


	//*******************   "Grid"-tab  ************************************
	axisSave->enableMajorGrid( ui.chbMajorGrid->isChecked() );
	axisSave->setMajorGridStyle( Qt::PenStyle(ui.cbMajorGridStyle->currentIndex()+1) );
	axisSave->setMajorGridColor( ui.kcbMajorGridColor->color() );
	axisSave->setMajorGridWidth( ui.sbMajorGridWidth->value() );

	axisSave->enableMinorGrid( ui.chbMinorGrid->isChecked() );
	axisSave->setMinorGridStyle( Qt::PenStyle(ui.cbMinorGridStyle->currentIndex()+1) );
	axisSave->setMinorGridColor( ui.kcbMinorGridColor->color() );
	axisSave->setMinorGridWidth( ui.sbMinorGridWidth->value() );
}


/*!
	restores/shows default axis properties in the UI.
*/
void AxesWidget::restoreDefaults(){
	if (QMessageBox::warning(this,
		i18n("Restore default axes values"), i18n("All settings will be reset to the default values. Do you want to continue?"),
			QMessageBox::Yes | QMessageBox::No)==QMessageBox::Yes){

		//TODO
		QMessageBox::warning(this, i18n("Restore default axes values"), i18n("Not yet implemented. Sorry.") );
	}
}

//TODO
/*
void AxesWidget::apply(QList<Axis>*) const{
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
}
*/


//**********************************************************
//****************** SLOTS *********************************
//**********************************************************
/*!
 	Is called when the user changes the current axis in the coresponding ComboBox.
	Triggers the functions to save the properties of the previosly shown axis (if needed)
	and to show the properties of the newly selected axis with the index \c index.
*/
void AxesWidget::currentAxisChanged(int index){
	if (initializing)
		return;

	//current axis was changed
	//->save the data of the previos selected axis if changed
	if (m_dataChanged){
		this->saveAxis(currentAxis);
	}

	//make the selected axis to the new current axis
	//and show the data of the new current axis
	currentAxis=index;
	initializing=true;
	this->showAxis(currentAxis);
	initializing=false;
}


void AxesWidget::slotDataChanged(){
	if (initializing)
		return;

	m_dataChanged=true;
	emit dataChanged(true);
}

//"General"-tab
void AxesWidget::axisStateChanged(int state){
	if (state==Qt::Checked)
		ui.chbBorder->setChecked(true);

	this->slotDataChanged();
}

/*!
	called if the scale type of the current axis is changed.
	Updates the number of minor ticks.
*/
void AxesWidget::scaleTypeChanged(int index) {
	if( index == Axis::SCALETYPE_LINEAR)
		ui.leMinorTicksNumber->setText( "3" );
	else if (index == Axis::SCALETYPE_LOG10)
		ui.leMinorTicksNumber->setText( "8");
	else
		ui.leMinorTicksNumber->setText( "0" );

	this->slotDataChanged();
}

void AxesWidget::positionChanged(int index){
	if (index==Axis::POSITION_CUSTOM){
		ui.lPositionOffset->setEnabled(true);
		ui.lePositionOffset->setEnabled(true);
	}else{
		ui.lPositionOffset->setEnabled(false);
		ui.lePositionOffset->setEnabled(false);
	}

	this->slotDataChanged();
}

//"Ticks"-tab
/*!
	called if the current style of the ticks (Number or Increment) is changed.
	Adjusts the names of the corresponding labels in UI.
*/
void AxesWidget::ticksTypeChanged(int index){
	if (index==0){
		ui.lMajorTicksNumber->setText( i18n("Number") );
		ui.lMinorTicksNumber->setText( i18n("Number") );
	}else{
		ui.lMajorTicksNumber->setText( i18n("Increment") );
		ui.lMinorTicksNumber->setText( i18n("Increment") );
	}

	this->slotDataChanged();
}

/*!
	called if the Number-Style (auto or custom) for the major ticks  is changed in the corresponding ComboBox.
	Enables/disables the text field for the number of major ticks if custom/auto is selected.
*/
void AxesWidget::majorTicksNumberTypeChanged(int index){
	ui.leMajorTicksNumber->setEnabled((bool)index);
	this->slotDataChanged();
}


/*!
	called if the Number-Style (auto or custom) for the minor ticks is changed in the corresponding ComboBox.
	Enables/disables the textfield for the number of minor ticks if custom/auto is selected.
*/
void AxesWidget::minorTicksNumberTypeChanged(int index){
	ui.leMinorTicksNumber->setEnabled((bool)index);
	this->slotDataChanged();
}

//"Tick labels"-tab
/*!
	called if the ticks format is changed in the corresponding ComboBox.
	Enables an additional text field for "Date-Format" if time/date/datetime selected,
	disables this field otherwise.
*/
void AxesWidget::labelFormatChanged(const QString& text){
	if ( text != i18n("time") && text != i18n("date") && text != i18n("datetime") )
		ui.leLabelsDateFormat->setEnabled(false);
	else
		ui.leLabelsDateFormat->setEnabled(true);

	this->slotDataChanged();
}


//TODO was macht diese Funktion?
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


//TODO port/remove this function
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
