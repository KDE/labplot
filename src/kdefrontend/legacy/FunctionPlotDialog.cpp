/***************************************************************************
    File                 : FunctionPlotDialog.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2009 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : dialog for plotting function

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
#include "FunctionPlotDialog.h"
#include "MainWin.h"
#include "FunctionWidget.h"
#include "LabelWidget.h"
#include "PlotStyleWidget.h"
#include "ImportWidget.h"
#include "PlotSurfaceStyleWidget.h"
#include "ValuesWidget.h"
#include "../backend/widgets/TreeViewComboBox.h"
#include "pixmaps/pixmap.h" //TODO remove this. Use Qt's resource system instead.
#include <KDebug>
#include <KMessageBox>

FunctionPlotDialog::FunctionPlotDialog(MainWin *mw, const Plot::PlotType& type)
	: KDialog(mw){

	mainWin=mw;
	plotType=type;
	editMode=false;

	QWidget* mainWidget = new QWidget(this);
	QVBoxLayout* vLayout = new QVBoxLayout(mainWidget);
	tabWidget = new QTabWidget(mainWidget);

	functionWidget = new FunctionWidget( mainWidget, type );
 	tabWidget->addTab(functionWidget, i18n("Function"));

 	labelWidget = new LabelWidget(mainWidget);
 	tabWidget->addTab(labelWidget, i18n("Title"));

	if (type==Plot::PLOTSURFACE)
		plotStyleWidget=new PlotSurfaceStyleWidget(mainWidget);
	else
		plotStyleWidget=new PlotStyleWidget(mainWidget);

	tabWidget->addTab(dynamic_cast<QWidget*>(plotStyleWidget), i18n("Style"));

	valuesWidget = new ValuesWidget(mainWidget);
	tabWidget->addTab(valuesWidget, i18n("Values"));

	vLayout->addWidget(tabWidget);


	//Frame for the "Add To"-Stuff
	frameAddTo = new QFrame(mainWidget);
	QHBoxLayout* hLayout = new QHBoxLayout(frameAddTo);
	hLayout->addWidget( new QLabel(i18n("Add function to"),  frameAddTo) );
// 	hLayout->addItem( new QSpacerItem(10, 20, QSizePolicy::Expanding) );

	cbAddTo = new TreeViewComboBox(frameAddTo);
	hLayout->addWidget( cbAddTo);
	vLayout->addWidget(frameAddTo);

	this->setMainWidget( mainWidget );
	this->setButtons( KDialog::Ok | KDialog::Cancel );

	QString caption;
	if (type == Plot::PLOT2D)
		caption=i18n("Add New 2D Function Plot");
	else if (type == Plot::PLOTSURFACE)
		caption=i18n("Add New 2D Surface Function Plot");
	else if (type == Plot::PLOTPOLAR)
		caption=i18n("Add New 2D Polar Function Plot");
	else if (type == Plot::PLOT3D)
		caption=i18n("Add New 3D Function Plot");

	this->setCaption(caption);
	this->setWindowIcon(QIcon(newFunction_xpm));

	//SLOTs
	connect( this, SIGNAL(applyClicked()), this, SLOT(apply()) );
 	connect( this, SIGNAL(okClicked()), this, SLOT(save()) );
	connect( this, SIGNAL(changed(bool)), this, SLOT(enableButtonApply(bool)) ); //TODO
	connect(functionWidget, SIGNAL(functionChanged(QString)), labelWidget, SLOT(setText(QString)) );

	functionWidget->init(); //call this to trigger the signal in FunctionWidget in order to update of the Label text
	resize( QSize(200,400) );
	kDebug()<<"Initialization done."<<endl;
}

void FunctionPlotDialog::setModel(QAbstractItemModel * model){
 	cbAddTo->setModel(model);
	//TODO select the first non-folder item in the TreeView
}

/*!
	displays the data set \c s to be edited.
*/
void FunctionPlotDialog::setSet(Set* s){
	set=s;
	functionWidget->setSet(s);
	labelWidget->setLabel(set->label());
	plotStyleWidget->setStyle(set->style());
	//TODO valuesWidget->setStyle(set->style());

	//in the edit mode. There should be no possibility to add the changed set
	//to a new worksheet/spreadsheet.
	editMode=true;
	frameAddTo->hide();

	//Add apply-Button
	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
	this->enableButtonApply( false );
}

void FunctionPlotDialog::saveSet(Set* set) const{
	functionWidget->saveSet(set);
	labelWidget->saveLabel(set->label());
	plotStyleWidget->saveStyle(set->style());
	//TODO valuesWidget->saveStyle(set->style());

	kDebug()<<"Changes saved."<<endl;
}

void FunctionPlotDialog::setCurrentIndex(const QModelIndex& index){
	cbAddTo->setCurrentModelIndex(index);
}

QModelIndex FunctionPlotDialog::currentIndex() const{
	return cbAddTo->currentIndex();
}

void FunctionPlotDialog::save(){
// 	AbstractAspect* aspect =  static_cast<AbstractAspect*>(currentIndex().internalPointer());
// 	if (aspect->inherits("Worksheet")==false){
// 		KMessageBox::error(this, i18n("No worksheet selected"), i18n("Please select a worksheet.") );
// 		return;
// 	}
// 	close();
}

void FunctionPlotDialog::apply() const{
	functionWidget->saveSet(set);
/*
	if (editMode){
		//we're in the edit mode, there is an edit-object:
		// -> save the changes and trigger the update in MainWin
		functionWidget->saveSet(set);
		//TODO
	}else{
		//there is no set-object:
		// -> create a new one and add it in MainWin.
		kDebug()<<"Not in the edit-mode. Create new data set and apply the changes."<<endl;
		Set newSet;
		if (plotType==PLOT2D || plotType==PLOTPOLAR)
			newSet.setType(Set::SET2D);
		else
			newSet.setType(Set::SET3D);

		functionWidget->saveSet(&newSet);
   		mainWin->addSet(newSet, cbAddTo->currentIndex(), plotType);
	}
	*/
	kDebug()<<"Changes applied."<<endl;
}
