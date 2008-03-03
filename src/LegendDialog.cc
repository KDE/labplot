//LabPlot : LegendDialog.cc

#include "LegendDialog.h"

LegendDialog::LegendDialog(MainWin *mw)
	: Dialog(mw)
{
	kdDebug()<<"LegendDialog()"<<endl;
	setCaption(i18n("Legend Settings"));
	layout->addWidget(new QLabel(i18n("Not implemented yet!")),1,0);

	showButton(KDialog::User1,false);

/*	QHBox *hb = new QHBox(vbox);
	enabledcb = new QCheckBox(i18n("Legend enabled "),hb);
	bordercb = new QCheckBox(i18n("Border "),hb);

	new QLabel(i18n("Position : "),vbox);
	hb = new QHBox(vbox);
	new QLabel(i18n("x : "),hb);
	xle = new KLineEdit("0",hb);
	xle->setValidator(new QDoubleValidator(0.0,1.0,3,xle));
	new QLabel(i18n(" y : "),hb);
	yle = new KLineEdit("1",hb);
	yle->setValidator(new QDoubleValidator(0.0,1.0,3,yle));

	new QLabel(i18n("Legend Font : "),vbox);
	hb = new QHBox(vbox);
	fontle = new KLineEdit("",hb);
	KPushButton *newFont = new KPushButton(i18n("New"),hb);
	QObject::connect(newFont,SIGNAL(clicked()),SLOT(selectFont()));

	hb = new QHBox(vbox);
	transcb = new QCheckBox(i18n("Transparent"),hb);
	colorcb = new KColorButton(Qt::white,hb);

	hb = new QHBox(vbox);
	QLabel *olabel = new QLabel(i18n("Orientation :"),hb);
	orientcb = new KComboBox(hb);
	QStringList olist;
	olist << i18n("BottomTop");
	olist << i18n("LeftRight");
	orientcb->insertStringList(olist);
	PType type = p->getPlot(p->API())->Type();
	if(type != PSURFACE && type != PQWT3D) {
		olabel->hide();
		orientcb->hide();
	}

	updateDialog(p);
*/
}

/*
void LegendDialog::updateDialog(Worksheet *ws) {
	kdDebug()<<"LegendDialog::updateDialog()"<<endl;
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
