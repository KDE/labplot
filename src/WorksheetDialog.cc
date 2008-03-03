//LabPlot : WorksheetDialog.cc

#include "WorksheetDialog.h"

WorksheetDialog::WorksheetDialog(MainWin *mw)
	: Dialog(mw)
{
	kdDebug()<<"WorksheetDialog()"<<endl;
	setCaption(i18n("Worksheet Settings"));
	layout->addWidget(new QLabel(i18n("Not implemented yet!")),1,0);

	showButton(KDialog::User1,false);

/*	KConfig *config = mw->Config();
	config->setGroup( "Worksheet" );

	QHBox *hb = new QHBox(vbox);
	enabletcb = new QCheckBox(i18n("enable worksheet title :"),hb);
	enabletcb->setChecked(config->readBoolEntry("TitleEnabled",true));
	titlele = new KLineEdit(config->readEntry("Title",""),hb);

	hb = new QHBox(vbox);
	new QLabel(i18n("Background Color : "),hb);
	bgcolor = new KColorButton(config->readColorEntry("Background",&Qt::white),hb);
	QObject::connect(bgcolor,SIGNAL(changed(const QColor &)),this,SLOT(colorChanged()));

	hb = new QHBox(vbox);
	new QLabel(i18n("    Brush : "),hb);
	wbrushcb = new KComboBox(hb);
	fillBrushBox(wbrushcb,SRECT,Qt::black,FFULL,config->readColorEntry("Background",&Qt::white));
	wbrushcb->setCurrentItem(config->readNumEntry("Brush",Qt::SolidPattern));

	hb = new QHBox(vbox);
	enablecb = new QCheckBox(i18n("enable time stamp"),hb);
	enablecb->setChecked(config->readBoolEntry("TimeStampEnabled",true));
	objectscb = new QCheckBox(i18n("draw objects in background"),hb);
	objectscb->setChecked(config->readBoolEntry("DrawObjectsFirst",false));

	hb = new QHBox(vbox);
	new QLabel(i18n("Time stamp : "),hb);
	timestample = new KLineEdit(config->readEntry("TimeStamp",""),hb);

	KPushButton *update_button = new KPushButton( i18n("Update time stamp"), hb );
	QObject::connect(update_button,SIGNAL(clicked()),SLOT(updateTimeStamp()));

	hb = new QHBox(vbox);
	new QLabel(i18n("Width : "),hb);
	xni = new KIntNumInput(config->readNumEntry("Width",0),hb);
	xni->setRange(1,INF,1,false);
	hb = new QHBox(vbox);
	new QLabel(i18n("Height : "),hb);
	yni = new KIntNumInput(config->readNumEntry("Height",0),hb);
	yni->setRange(1,INF,1,false);

	updateDialog();
*/
}

/*void WorksheetDialog::updateDialog() {
	p = mw->activeWorksheet();

	if(p == 0)
		return;

	enabletcb->setChecked(p->TitleEnabled());
	titlele->setText(p->Title());
	bgcolor->setColor(p->Background().color());
	enablecb->setChecked(p->TimeStampEnabled());
	timestample->setText(p->TimeStamp().toString(Qt::TextDate));
	xni->setValue(p->size().width());
	yni->setValue(p->size().height());
}

void WorksheetDialog::colorChanged() {
	fillBrushBox(wbrushcb,SRECT,Qt::black,FFULL,bgcolor->color());
}


void WorksheetDialog::updateTimeStamp() {
#if QT_VERSION > 0x030007
	timestample->setText(QDateTime::currentDateTime(Qt::LocalTime).toString(Qt::TextDate));
#else
	timestample->setText(QDateTime::currentDateTime().toString(Qt::TextDate));
#endif
}

void WorksheetDialog::saveSettings() {
	KConfig *config = mw->Config();
	config->setGroup( "Worksheet" );
	config->writeEntry("TitleEnabled",enabletcb->isChecked());
	config->writeEntry("Title",titlele->text());
	config->writeEntry("Background",bgcolor->color());
	config->writeEntry("Brush",wbrushcb->currentItem());
	config->writeEntry("TimeStampEnabled",enablecb->isChecked());
	config->writeEntry("DrawObjectsFirst",objectscb->isChecked());
	config->writeEntry("TimeStamp",timestample->text());
	config->writeEntry("Width",xni->value());
	config->writeEntry("Height",yni->value());
}

int WorksheetDialog::apply_clicked() {
	p->setBackground(QBrush(bgcolor->color(),(BrushStyle)wbrushcb->currentItem()));

	p->setTimeStamp(QDateTime::fromString(timestample->text()));
	p->setTimeStampEnabled(enablecb->isChecked());
	p->setDrawObjectsFirst(objectscb->isChecked());

	p->setTitle(titlele->text());
	p->setCaption(p->Title());
	p->setTitleEnabled(enabletcb->isChecked());

	p->resize(xni->value(),yni->value());

	p->updatePixmap();

	return 0;
}
*/
