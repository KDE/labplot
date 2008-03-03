//LabPlot : Dialog.cc

#include <QFontComboBox>
#include <KComboBox>
#include <KColorButton>
#include "Dialog.h"

Dialog::Dialog(MainWin *mw)
	: KDialog(), mw(mw)
{
	QWidget *page = new QWidget(this);
	setMainWidget(page);
	layout = new QGridLayout(page);

	setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply| KDialog::User1 | KDialog::User2 );
	setButtonText(KDialog::User1,i18n("Save"));
	setButtonText(KDialog::User2,i18n("Show options"));
	//TODO (not working) 
	setDefaultButton(KDialog::Ok);

	QObject::connect(this,SIGNAL(cancelClicked()),SLOT(reject()));
}

void Dialog::labelWidget(QWidget *parent, Label *label) {
	QGridLayout *grid = new QGridLayout(parent);
	//grid->addWidget(new QLabel(i18n("Position :")),0,0);
	grid->addWidget(new QLabel(i18n("Under construction ...")),0,0);
	grid->addWidget(new QLabel(i18n("x :")),1,0,Qt::AlignRight);
	xni = new KDoubleNumInput(0.0,1.0,label->X());
	grid->addWidget(xni,1,1);
	grid->addWidget(new QLabel(i18n("y :")),1,2,Qt::AlignRight);
	yni = new KDoubleNumInput(0.0,1.0,label->Y());
	grid->addWidget(yni,1,3);
	// TODO :
	// tex label
	// transparent+color	-> QColorDialog
	// boxed
	// rotation
	labelte = new QTextEdit();	// needed here for the slots
	QFontComboBox *fontcb = new QFontComboBox();
	grid->addWidget(fontcb,2,0,1,2);
	QObject::connect(fontcb,SIGNAL(currentFontChanged(QFont)),SLOT(setLabelFont(QFont)));

	KComboBox *fontsizecb = new KComboBox();
	QFontDatabase db;
	foreach(int size, db.standardSizes())
		fontsizecb->addItem(QString::number(size));
	connect(fontsizecb, SIGNAL(activated(const QString &)),SLOT(setLabelSize(const QString &)));
	grid->addWidget(fontsizecb,2,2);
	KColorButton *colorb = new KColorButton(Qt::black);
	QObject::connect(colorb,SIGNAL(changed(QColor)),labelte,SLOT(setTextColor(QColor)));
	grid->addWidget(colorb,2,3);
	// TODO : Bold, Italic, Underline
	KComboBox *smallgreekcb = new KComboBox();
	QFont symbol("Symbol", 15, QFont::Bold);
 	smallgreekcb->setFont(symbol);
	for(int i=97;i<122;i++)
		smallgreekcb->addItem(QChar(i));
	connect(smallgreekcb, SIGNAL(activated(const QString &)),SLOT(insertSymbol(const QString &)));
	grid->addWidget(smallgreekcb,3,0);
	KComboBox *biggreekcb = new KComboBox();
 	biggreekcb->setFont(symbol);
	for(int i=68;i<90;i++)
		biggreekcb->addItem(QChar(i));
	connect(biggreekcb, SIGNAL(activated(const QString &)),SLOT(insertSymbol(const QString &)));
	grid->addWidget(biggreekcb,3,1);
	KComboBox *symbolcb = new KComboBox();
 	symbolcb->setFont(symbol);
	symbolcb->addItem(QChar(34));
	symbolcb->addItem(QChar(36));
	symbolcb->addItem(QChar(39));
	symbolcb->addItem(QChar(64));
	for(int i=160;i<255;i++)
		symbolcb->addItem(QChar(i));
	connect(symbolcb, SIGNAL(activated(const QString &)),SLOT(insertSymbol(const QString &)));
	grid->addWidget(symbolcb,3,2);
	// TODO : combobox : normal, x^2, x_2
	labelte->setHtml(label->Text());
	grid->addWidget(labelte,4,0,1,4);
}

void Dialog::setLabelFont(QFont font) {
	labelte->setFontFamily(font.family());
	// TODO : set font family of cursor?
	/*QTextCharFormat format = labelte->currentCharFormat();
	format.setFontFamily(font.family());
	labelte->setCurrentCharFormat(format);
	*/
}

void Dialog::setLabelSize(QString size) {
	// TODO : set font size of cursor?
	labelte->setFontPointSize(size.toInt());
}

void Dialog::insertSymbol(QString c) {
	labelte->setFontFamily("Symbol");
	labelte->insertPlainText(c);
}

void Dialog::setupLabel(Label *label) {
	label->setX(xni->value());
	label->setY(yni->value());
	// TODO
	label->setText(labelte->toHtml());
}
