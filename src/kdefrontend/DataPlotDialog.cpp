/***************************************************************************
    File                 : DataPlotDialog.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : dialog for importing data to a Worksheet or Spreadsheet.

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
#include "DataPlotDialog.h"
#include "MainWin.h"
#include "ImportWidget.h"
#include "LabelWidget.h"
#include "PlotStyleWidget.h"
#include "ImportWidget.h"
#include "PlotSurfaceStyleWidget.h"
#include "ValuesWidget.h"

#include "../backend/widgets/TreeViewComboBox.h"
#include <KDebug>

DataPlotDialog::DataPlotDialog(MainWin *mw, const Plot::PlotType& type)
	: KDialog(mw){

	mainWin=mw;
// 	plotType=type;
// 	editMode=false;

	QWidget* mainWidget = new QWidget(this);
	QVBoxLayout* vLayout = new QVBoxLayout(mainWidget);

	tabWidget = new QTabWidget(mainWidget);

	importWidget = new ImportWidget(mainWidget);
 	importWidget->showOptions();
	tabWidget->addTab(importWidget, i18n("Data options"));

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
	frameAddTo = new QFrame(this);
	QHBoxLayout* hLayout = new QHBoxLayout(frameAddTo);
	hLayout->addWidget( new QLabel(i18n("Add function to"),  frameAddTo) );

	cbAddTo = new TreeViewComboBox(frameAddTo);
	hLayout->addWidget( cbAddTo);
	vLayout->addWidget(frameAddTo);

	this->setMainWidget( mainWidget );
	this->setButtons( KDialog::Ok | KDialog::Cancel );

	QString caption;
	if (type == Plot::PLOT2D)
		caption=i18n("New 2D Data Plot");
	else if (type == Plot::PLOTSURFACE)
		caption=i18n("New 2D Surface Data Plot");
	else if (type == Plot::PLOTPOLAR)
		caption=i18n("New 2D Polar Data Plot");
	else if (type == Plot::PLOT3D)
		caption=i18n("New 3D Data Plot");

	this->setCaption(caption);
	this->setWindowIcon( KIcon("document-import-database") );
	resize( QSize(200,400) );
	kDebug()<<"Initialization done."<<endl;
}

void DataPlotDialog::setModel(QAbstractItemModel * model){
 	cbAddTo->setModel(model);
}

void DataPlotDialog::setCurrentIndex(const QModelIndex& index){
	cbAddTo->setCurrentModelIndex(index);
}

QModelIndex DataPlotDialog::currentIndex() const{
	return cbAddTo->currentIndex();
}

void DataPlotDialog::saveSet(Set* set) const{
	//TODO
	//this->apply();
// 	importWidget->saveSet(set);
	kDebug()<<"Changes saved."<<endl;
// 	this->close();
}
