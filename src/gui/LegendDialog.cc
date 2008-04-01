#include "LegendDialog.h"
#include "LegendWidget.h"
#include "../elements/Legend.h"
#include "../MainWin.h"
#include <KDebug>

LegendDialog::LegendDialog(MainWin* parent, Legend* legend) : KDialog(parent){
	kDebug()<<"LegendDialog()"<<endl;

	setCaption(i18n("Legend Settings"));
	mainWin=parent;

	legendWidget = new LegendWidget( this );
	legendWidget->setLegend(legend);
	this->setMainWidget( legendWidget );
	this->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

	connect( this, SIGNAL( applyClicked() ), this, SLOT( apply() ) );
	connect( this, SIGNAL( okClicked() ), this, SLOT( ok() ) );
	connect( this, SIGNAL( changed( bool ) ), this, SLOT( enableButtonApply( bool ) ) );

	this->enableButtonApply( false );
	resize( QSize(100,200) );
}

void LegendDialog::apply(){
	legendWidget->save();

	//TODO
	//mainWin->updateLegend();
}

void LegendDialog::ok(){
	legendWidget->save();

}
/*
void LegendDialog::updateDialog(Worksheet *ws) {
	kDebug()<<"LegendDialog::updateDialog()"<<endl;
	if(ws == 0) {
		p = mw->activeWorksheet();
		s = mw->activeSpreadsheet();
	}
	else
		p = ws;

	if(p != 0) {
		Plot *plot = p->getPlot(p->API());
		if(plot == 0)
			return;
		legend = plot->getLegend();
		if(legend == 0)
			return;
		lf = legend->Font();

		xle->setText(QString::number(legend->X()));
		yle->setText(QString::number(legend->Y()));

		fontle->setText(lf.family() + " " + QString::number(lf.pointSize()));
		colorcb->setColor(legend->Color());

		enabledcb->setChecked(legend->Enabled());
		bordercb->setChecked(legend->BorderEnabled());
		transcb->setChecked(legend->Transparent());
		orientcb->setCurrentItem(legend->getOrientation());
	}
}

void LegendDialog::selectFont() {
    bool res;
    QFont f = QFontDialog::getFont( &res, legend->Font(), this );
    if(res) {
	lf = f;
	fontle->setText(f.family());
    }
}

int LegendDialog::apply_clicked() {
	legend->enable(enabledcb->isChecked());
	legend->enableBorder(bordercb->isChecked());
	legend->setFont(lf);
	legend->setTransparent(transcb->isChecked());
	legend->setColor(colorcb->color());
	legend->setPosition(xle->text().toDouble(),yle->text().toDouble());
	legend->setOrientation(orientcb->currentItem());
	QFont tmp(legend->Font());
	fontle->setText(tmp.family() + tr(" ") + QString::number(tmp.pointSize()));
	p->updatePixmap();

	return 0;
}
*/
