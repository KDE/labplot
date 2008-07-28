#include "FunctionPlotDialog.h"
#include "../MainWin.h"
#include "FunctionPlotWidget.h"
#include <KDebug>

FunctionPlotDialog::FunctionPlotDialog(MainWin *mw, const Plot::PlotType& type)
	: KDialog(mw){

	plotType=type;
	editMode=false;

	QWidget* mainWidget = new QWidget(this);
	QVBoxLayout* vLayout = new QVBoxLayout(mainWidget);

	functionPlotWidget = new FunctionPlotWidget( mainWidget );
	functionPlotWidget->setPlotType(type);
	vLayout->addWidget(functionPlotWidget);

	//Frame for the "Add To"-Stuff
	frameAddTo = new QFrame(mainWidget);
	QHBoxLayout* hLayout = new QHBoxLayout(frameAddTo);
	hLayout->addWidget( new QLabel(i18n("Add function to"),  frameAddTo) );
	hLayout->addItem( new QSpacerItem(10, 20, QSizePolicy::Expanding) );

	cbAddTo = new QComboBox(frameAddTo);
	hLayout->addWidget( cbAddTo);

	mainWin=mw;
	QList<QMdiSubWindow *> wlist = mw->getMdi()->subWindowList();
 	qSort(wlist);

	for (int i=0; i<wlist.size(); i++)
     	cbAddTo->addItem( wlist.at(i)->windowTitle() );

	cbAddTo->addItem( i18n("New Worksheet") );
	cbAddTo->addItem( i18n("New Spreadsheet") );

	vLayout->addWidget(frameAddTo);

	this->setMainWidget( mainWidget );
	this->setButtons( KDialog::Ok | KDialog::Cancel );

	//TODO
// 	Plot *plot=0;
// 	if(p != 0)
// 	 	plot = p->getPlot(p->API());

	QString caption;
	if (type == Plot::PLOT2D)
		caption=i18n("Add New 2D Function Plot");
	else if (type == Plot::PLOTSURFACE)
		caption=i18n("Add New 2D Surface Function Plot");
	else if (type == Plot::PLOTPOLAR)
		caption=i18n("Add New 2D Polar Function");
	else if (type == Plot::PLOT3D)
		caption=i18n("Add New 3D Function");
	else if (type == Plot::PLOTQWT3D)
		caption=i18n("Add New QWT Function");

	this->setCaption(caption);

	//SLOTs
	connect( this, SIGNAL( applyClicked() ), this, SLOT( apply() ) );
// 	connect( this, SIGNAL( okClicked() ), this, SLOT( save() ) );
	connect( this, SIGNAL( changed( bool ) ), this, SLOT( enableButtonApply( bool ) ) ); //TODO

	resize( QSize(300,400) );
	kDebug()<<"Initialization done."<<endl;
}

/*!
	displays the data set \c s to be edited.
*/
void FunctionPlotDialog::setSet(Set* s){
	set=s;
	functionPlotWidget->setSet(s);

	//in the edit mode. There should be no possibility to add the changed set
	//to a new worksheet/spreadsheet.
	editMode=true;
	frameAddTo->hide();

	//Add apply-Button
	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
	this->enableButtonApply( false );
}

void FunctionPlotDialog::saveSet(Set* set) const{
	//TODO
	//this->apply();
	functionPlotWidget->saveSet(set);
	kDebug()<<"Changes saved."<<endl;
// 	this->close();
}

int FunctionPlotDialog::currentSheetIndex() const{
	return cbAddTo->currentIndex();
}

void FunctionPlotDialog::apply() const{
	functionPlotWidget->saveSet(set);
/*
	if (editMode){
		//we're in the edit mode, there is an edit-object:
		// -> save the changes and trigger the update in MainWin
		functionPlotWidget->saveSet(set);
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

		functionPlotWidget->saveSet(&newSet);
   		mainWin->addSet(newSet, cbAddTo->currentIndex(), plotType);
	}
	*/
	kDebug()<<"Changes applied."<<endl;
}
