/***************************************************************************
    File                 : RenameWindowDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Rename window dialog
                           
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
#include "RenameWindowDialog.h"
#include "ApplicationWindow.h"

#include <QPushButton>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QRadioButton>
#include <QMessageBox>
#include <QButtonGroup>
#include <QRegExp>
#include <QHBoxLayout>
#include <QGridLayout>

RenameWindowDialog::RenameWindowDialog(QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
	setWindowTitle(tr("Rename Window"));

	QGridLayout * leftLayout = new QGridLayout();
	QVBoxLayout * rightLayout = new QVBoxLayout();

	groupBox1 = new QGroupBox(tr("Window Title"));
	groupBox1->setLayout(leftLayout);

	boxName = new QRadioButton(tr("&Name (single word)"));
	leftLayout->addWidget(boxName, 0, 0);
	boxNameLine = new QLineEdit();
	leftLayout->addWidget(boxNameLine, 0, 1);
	setFocusProxy(boxNameLine);

	boxLabel = new QRadioButton(tr("&Label"));
	leftLayout->addWidget(boxLabel, 2, 0);
	boxLabelEdit = new QTextEdit();
	leftLayout->addWidget(boxLabelEdit, 1, 1, 3, 1);
	boxLabelEdit->setMaximumHeight(100);
	boxLabelEdit->setMinimumHeight(100);

	boxBoth = new QRadioButton(tr("&Both Name and Label"));
	leftLayout->addWidget(boxBoth, 4, 0);
	
	buttons = new QButtonGroup(this);
	buttons->addButton(boxName);
	buttons->addButton(boxLabel);
	buttons->addButton(boxBoth);
	
	buttonOk = new QPushButton(tr( "&OK" ));
    buttonOk->setAutoDefault( true );
    buttonOk->setDefault( true );
	rightLayout->addWidget(buttonOk);
   
    buttonCancel = new QPushButton(tr( "&Cancel" ));
    buttonCancel->setAutoDefault( true );
	rightLayout->addWidget(buttonCancel);
	rightLayout->addStretch();
	
	QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(groupBox1);
	mainLayout->addLayout(rightLayout);

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

void RenameWindowDialog::setWidget(MyWidget *w)
{
	window = w;
	boxNameLine->setText(w->name());
	boxLabelEdit->setText(w->windowLabel());
	switch (w->captionPolicy())
	{
		case MyWidget::Name:
			boxName->setChecked(true);
			break;

		case MyWidget::Label:
			boxLabel->setChecked(true);
			break;

		case MyWidget::Both:
			boxBoth->setChecked(true);
			break;
	}
}

MyWidget::CaptionPolicy RenameWindowDialog::getCaptionPolicy()
{
	MyWidget::CaptionPolicy policy = MyWidget::Name;
	if (boxLabel->isChecked())
		policy = MyWidget::Label;
	else if (boxBoth->isChecked())
		policy = MyWidget::Both;

	return policy;
}

void RenameWindowDialog::accept()
{
	QString name = window->name();
	QString text = boxNameLine->text().remove("=").remove(QRegExp("\\s"));
	QString label = boxLabelEdit->text();

	MyWidget::CaptionPolicy policy = getCaptionPolicy();
	if (text == name && label == window->windowLabel() && window->captionPolicy() == policy)
		close();

	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	if (text.contains("_")){
  		QMessageBox::warning(this, tr("Warning"),
  	    tr("For internal consistency reasons the underscore character is replaced with a minus sign."));}
  	 
  	if (text.replace("_", "-") != name){
		if(!app->renameWindow(window, text))
			return;
	}

	label.replace("\n"," ").replace("\t"," ");
	window->setWindowLabel(label);
	window->setCaptionPolicy(policy);
	app->setListViewLabel(window->name(), label);
	app->modifiedProject(window);
	close();
}
