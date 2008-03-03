//LabPlot : PlotDialog.cc

#include "PlotDialog.h"

PlotDialog::PlotDialog(MainWin *mw)
	: Dialog(mw)
{
	kdDebug()<<"LegendDialog()"<<endl;
	setCaption(i18n("Plot Dialog"));
	layout->addWidget(new QLabel(i18n("Not implemented yet!")),1,0);

	showButton(KDialog::User1,false);

/*	lv = new QListView(vbox,0);
	lv->addColumn(i18n("Number"));
	lv->addColumn(i18n("Type"));
	lv->addColumn(i18n("Position"));
	lv->addColumn(i18n("Size"));
	lv->addColumn(i18n("Transparent"));
	lv->addColumn(i18n("Background Color"));
	lv->addColumn(i18n("Graph Background Color"));

	lv->setAllColumnsShowFocus(true);
	for (int i=0;i<lv->columns();i++)
		lv->setColumnAlignment(i,Qt::AlignHCenter);
	// multi selection with CTRL and SHIFT
	lv->setSelectionMode(QListView::Extended);
	lv->setMinimumWidth(350);

	// popup menu
	if(p) {
		menu = new QPopupMenu( lv );
		menu->insertItem( i18n( "Clone" ),this,SLOT(clonePlot()) );
		menu->insertItem( i18n( "Delete" ),this,SLOT(deletePlot()) );
		connect(lv, SIGNAL( rightButtonPressed( QListViewItem *, const QPoint& , int ) ),
			this, SLOT( Menu(QListViewItem *, const QPoint& , int) ) );
	}
	updateList();
*/
}

/*
void PlotDialog::updateList() {
	lv->clear();
	lv->setSorting(-1);

	for (int i= p->NrPlots()-1;i>=0;i--) {
		QListViewItem *lvi = new QListViewItem(lv);
		QStringList sl;
		sl += QString::number(i+1);

		Plot *plot = p->getPlot(i);
		if(plot==0) continue;
		PType type = plot->Type();
		switch (type) {
		case P2D:	sl += ((Plot2DSimple *)plot)->Info();break;
		case PSURFACE:	sl += ((Plot2DSurface *)plot)->Info();break;
		case P3D:	sl += ((Plot3D *)plot)->Info();break;
		case PPIE:	sl += ((PlotPie *)plot)->Info();break;
		case PPOLAR:	sl += ((PlotPolar *)plot)->Info();break;
		case PTERNARY:	sl += ((PlotTernary *)plot)->Info();break;
		case PQWT3D:	sl += ((PlotQWT3D *)plot)->Info();break;
		default:	break;
		}

		for (unsigned int j = 0;j < sl.count();j++) {
			lvi->setText(j,sl[j]);
		}
	}
}

void PlotDialog::deletePlot(int item) {
#if QT_VERSION > 0x030102
	QListViewItemIterator it(lv,QListViewItemIterator::Selected);
#else
	QListViewItemIterator it(lv);
#endif
	for ( ; it.current(); ++it ) {
#if QT_VERSION <= 0x030102
		if(!it.current()->isSelected())
			continue;
#endif
		if(item == -1)
			item = (int) (lv->itemPos(it.current())/it.current()->height());
		p->deletePlot(item);
	}

	updateList();
}

void PlotDialog::clonePlot(int pitem) {
#if QT_VERSION > 0x030102
	QListViewItemIterator it(lv,QListViewItemIterator::Selected);
#else
	QListViewItemIterator it(lv);
#endif
	for ( ; it.current(); ++it ) {
#if QT_VERSION <= 0x030102
		if(!it.current()->isSelected())
			continue;
#endif
		if(pitem == -1)
			pitem = (int) (lv->itemPos(it.current())/it.current()->height());

		p->clonePlot(pitem);
	}

	updateList();
}
*/
