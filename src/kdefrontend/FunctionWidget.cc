/***************************************************************************
    File                 : FunctionWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : widget for creating a data set from function

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
#include "FunctionWidget.h"
#include "LabelWidget.h"
#include "../elements/Set.h"
#include "../elements/Point3D.h"

#include <QProgressDialog>
#include <KDebug>
#include <KMessageBox>
#include <cmath>

#ifdef HAVE_GSL
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#endif

#include "../parser/parser_struct.h"
extern "C" con constants[];
extern "C" init arith_fncts[];
#include "../parser/parser_extern.h"

 /*!
	\class FunctionWidget
	\brief Widget for creating a data set from a function.

	\ingroup kdefrontend
 */

FunctionWidget::FunctionWidget(QWidget* parent, const Plot::PlotType& type)
	:QWidget(parent){

	ui.setupUi(this);
	ui.bClear->setIcon( KIcon("edit-clear-locationbar-rtl") );
	//ui.bSync->setIcon( KIcon("view-refresh") );

	//Validators
	ui.leAxis1Start->setValidator( new QDoubleValidator(ui.leAxis1Start) );
	ui.leAxis1End->setValidator( new QDoubleValidator(ui.leAxis1End) );
	ui.leAxis2Start->setValidator( new QDoubleValidator(ui.leAxis2Start) );
	ui.leAxis2End->setValidator( new QDoubleValidator(ui.leAxis2End) );

	int f=0;
	QString tmp;
	while(true){
		tmp = arith_fncts[f++].fname;
		if(tmp.isEmpty())
			break;
		ui.cbFunctions->addItem( tmp+"()" );
	}

	f=0;
	while(true){
		QString tmp = constants[f++].name;
		if(tmp.isEmpty())
			break;
		ui.cbConstants->addItem( tmp );
	}


	if (type == Plot::Plot::PLOT2D || type == Plot::PLOTPOLAR){
		ui.leFunction->setText( "sin(x)" );
		ui.leAxis1Start->setText( "0" );
		ui.leAxis1End->setText( "1" );
		ui.frameAxis2->hide();
	}else if (type == Plot::PLOT3D || type == Plot::PLOTSURFACE){
		ui.leFunction->setText( "sin(x+y)" );
		ui.leAxis1Start->setText( "0" );
		ui.leAxis1End->setText( "1" );
		ui.leAxis2Start->setText( "0" );
		ui.leAxis2End->setText( "1" );
		ui.frameAxis2->show();
	}
	plotType=type;


	//SLOTS
	connect( ui.bClear, SIGNAL(clicked()), ui.leFunction, SLOT(clear()) );
	//connect( ui.bSync, SIGNAL(toggled(bool)),  SLOT(syncStatusChangedSlot(bool)) );
 	connect( ui.cbFunctions,SIGNAL(activated(const QString&)),SLOT(insertSlot(const QString&)) );
	connect( ui.cbConstants,SIGNAL(activated(const QString&)),SLOT(insertSlot(const QString&)) );
	connect( ui.leFunction, SIGNAL(textChanged (const QString&)), SLOT(functionChangedSlot(const QString&)) );

	emit functionChanged(ui.leFunction->text());
}

/*!
	emits \a functionChanged().
	Is only used once in the constructor of FunctionPlotDialog,  in order to update of the Label text.
*/
void FunctionWidget::init(){
	//TODO better solution?
	emit functionChanged(ui.leFunction->text());
}

FunctionWidget::~FunctionWidget(){
}


/*!
	sets the Set-object \a set to be displayed/edited in this widget
*/
void FunctionWidget::setSet(Set* set){
	ui.leFunction->setText( set->functionName() );

	ui.leAxis1Start->setText( QString::number(set->list_ranges.at(0).min()) );
	ui.leAxis1End->setText( QString::number(set->list_ranges.at(0).max()) );
	ui.sbAxis1Number->setValue( set->list_numbers.at(0) );

	if (plotType == Plot::PLOT3D || plotType == Plot::PLOTSURFACE || plotType == Plot::PLOTQWT3D){
		ui.leAxis2Start->setText( QString::number(set->list_ranges.at(2).min()) );
		ui.leAxis2End->setText( QString::number(set->list_ranges.at(2).max()) );
		ui.sbAxis2Number->setValue( set->list_numbers.at(1) );
	}
}

/*!
	saves the function parameters and create a new data set to \a Set.
*/
void FunctionWidget::saveSet(Set* set){
	set->setFunctionName( ui.leFunction->text() );

	//parameters for the x-variable
	set->list_ranges[0].setMin( ui.leAxis1Start->text().toFloat() );
	set->list_ranges[0].setMax( ui.leAxis1End->text().toFloat() );
	set->list_numbers[0] = ui.sbAxis1Number->value();

	//parameters for the y-variable, if available
	if (plotType == Plot::PLOT3D || plotType == Plot::PLOTSURFACE){
		set->list_ranges[1].setMin( ui.leAxis2Start->text().toFloat() );
		set->list_ranges[1].setMax( ui.leAxis2End->text().toFloat() );
		set->list_numbers[1] = ui.sbAxis2Number->value();
	}

	//TODO call this only if the function was changed.
 	this->createSetData(set);
	kDebug()<<"Set saved"<<endl;
}

int FunctionWidget::createSetData(Set* set) {
	set->list_data.clear(); //clear the old stuff. a new data set is going to be created now.
	int NX=set->list_numbers[0];

	QProgressDialog progress( i18n("Creating function ..."), i18n("Cancel"), 0, NX, this );
	progress.setMinimumDuration(2000);
	progress.setWindowModality(Qt::WindowModal);
	bool nanvalue;
	double x, y;
	if (plotType == Plot::PLOT2D || plotType == Plot::PLOTPOLAR){
		kDebug()<<"	\"2d\" or \" polar \" selected"<<endl;
		double xmin = parse( QString::number(set->list_ranges[0].min()).toLatin1().data() );
		double xmax = parse( QString::number(set->list_ranges[0].max()).toLatin1().data() );
		double ymin=0, ymax=1;

		kDebug()<<"xmi="<<xmin<<"	xma="<<xmax<<endl;
		kDebug()<<"NX="<<NX<<endl;
		kDebug()<<"	parsing "<<set->functionName()<<endl;

		init_table();
		for(int i = 0;i < NX;i++) {
			if(i%100 == 0) progress.setValue( i );
    			qApp->processEvents();

			x = (xmax-xmin)*i/(double)(NX-1)+xmin;
			char var[]="x";
			assign_variable(var,x);
#ifdef HAVE_GSL
			gsl_set_error_handler_off();
#endif
   			y = parse( (char *) set->functionName().toLatin1().data() );

			if(parse_errors()>0) {
				progress.cancel();
				KMessageBox::error(this, i18n("Parse Error!\n Please check the given function."));
				delete_table();
				return 1;
			}

			nanvalue=false;
			if (!finite(x))	{ x=0; nanvalue=true; }
			if (!finite(y))	{ y=0; nanvalue=true; }

			if (i == 0) ymin=ymax=y;
			y<ymin?ymin=y:0;
			y>ymax?ymax=y:0;

			Point point(x, y);
			if(nanvalue)
				point.setMasked();

			if ( progress.wasCanceled() ) {
				delete_table();
				return 1;
			}

			set->list_data<<point;
// 			kDebug()<<"Point created:   x="<<point.x()<<",	 y="<<point.y()<<endl;
		}
		delete_table();

		if(ymax-ymin == 0) {
			ymin -= 1;
			ymax += 1;
		}

		if(plotType == Plot::PLOTPOLAR) {
			xmin = 0;
			xmax = 2*M_PI;
		}

		set->list_ranges.clear();
 		set->list_ranges<<Range(xmin, xmax);
 		set->list_ranges<< Range(ymin, ymax);
		kDebug()<<"	\"2D\" or \" polar \" data set created"<<endl;
	}else if( plotType == Plot::PLOT3D || plotType == Plot::PLOTSURFACE || plotType == Plot::PLOTQWT3D ){
 		int NY = set->list_numbers[1];

		kDebug()<<"	\"surface\" or \" qwt 3d \" selected"<<endl;
		kDebug()<<"	NX = "<<NX<<"/ NY = "<<NY<<endl;
		kDebug()<<"	parsing "<<set->functionName()<<endl;

		double xmin = parse( QString::number(set->list_ranges[0].min()).toLatin1().data() );
		double xmax = parse( QString::number(set->list_ranges[0].max()).toLatin1().data() );
		double ymin = parse( QString::number(set->list_ranges[1].min()).toLatin1().data() );
		double ymax = parse( QString::number(set->list_ranges[1].max()).toLatin1().data() );
		double zmin=0,  zmax=1;

		init_table();
		double z;
		for (int i=0;i<NY;i++) {
			if(i%10==0)
				progress.setValue( i );

    		qApp->processEvents();
			y = (ymax-ymin)*i/(double)(NY-1)+ymin;
			char var[]="y";
			assign_variable(var, y);

			for (int j=0;j<NX;j++) {
				x = (xmax-xmin)*j/(double)(NX-1)+xmin;
 				var[0]='x';
				assign_variable(var, x);
#ifdef HAVE_GSL
			gsl_set_error_handler_off();
#endif
				z = parse( (char *) set->functionName().toLatin1().data() );
				if(parse_errors()>0) {
					progress.cancel();
					KMessageBox::error(this, i18n("Parse Error!\n Please check the given function."));
					delete_table();
					return 1;
				}

				nanvalue=false;
				if (!finite(z)){
					z=0;
					nanvalue=true;
				}

				if (i == 0 && j == 0) {
					zmin=z;
					zmax=z;
				}

				z<zmin?zmin=z:0;
				z>zmax?zmax=z:0;

				Point3D point(x, y, z);
				if(nanvalue)
					point.setMasked();

				if ( progress.wasCanceled() ) {
					delete_table();
					return 1;
				}
 				set->list_data<<point;
				kDebug()<<"Point created:   x="<<x<<",	 y="<<y<<",	 z="<<z<<endl;
			}
		}
		delete_table();

		if(zmax-zmin == 0) {
			zmin -= 1;
			zmax += 1;
		}

		set->list_ranges.clear();
 		set->list_ranges<<Range(xmin,xmax);
 		set->list_ranges<< Range(ymin,ymax);
		set->list_ranges<< Range(zmin,zmax);
		kDebug()<<"	\"3D\"  data set created"<<endl;
	}

	return 0;
}

//**********************************************************
//****************** SLOTS ********************************
//**********************************************************
/*!
	ss called when a new item in the ComboBox with the pre-defined functions or constants was selected.
*/
void FunctionWidget::insertSlot(const QString& s){
	ui.leFunction->insert(s);
	ui.leFunction->setFocus();
}

/*!
	called if the function was changed. If sync-button is toggled, emits \a functionChanged().
*/
void FunctionWidget::functionChangedSlot(const QString& s){
	kDebug()<<"";
	/*
 	if (ui.bSync->isChecked())
		emit functionChanged(s);
		*/
}

/*!
	called it the checked-status of the sync-button was changed.
	 Emits \a functionChanged() if the \a state is \a true (button was toggled).
*/
void FunctionWidget::syncStatusChangedSlot(bool state){
	if (state)
		emit functionChanged(ui.leFunction->text());
}
