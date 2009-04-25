/***************************************************************************
    File                 : ExportSpreadsheetDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Export ASCII dialog
                           
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
#include "ExportSpreadsheetDialog.h"

#include <QLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>

ExportSpreadsheetDialog::ExportSpreadsheetDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
	setWindowTitle( tr( "Export ASCII" ) );
	setSizeGripEnabled( true );
	
	QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel(tr("Spreadsheet")), 0, 0);
	boxSpreadsheet = new QComboBox();
	boxSpreadsheet->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	gl1->addWidget(boxSpreadsheet, 0, 1);

	boxAllSpreadsheets = new QCheckBox(tr( "&All" ));
    boxAllSpreadsheets->setChecked(false);
	gl1->addWidget(boxAllSpreadsheets, 0, 2);
	
    QLabel *sepText = new QLabel( tr( "Separator" ) );
	gl1->addWidget(sepText, 1, 0);

    boxSeparator = new QComboBox();
	boxSeparator->addItem(tr("TAB"));
    boxSeparator->addItem(tr("SPACE"));
	boxSeparator->addItem(";" + tr("TAB"));
	boxSeparator->addItem("," + tr("TAB"));
	boxSeparator->addItem(";" + tr("SPACE"));
	boxSeparator->addItem("," + tr("SPACE"));
    boxSeparator->addItem(";");
    boxSeparator->addItem(",");
	boxSeparator->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	boxSeparator->setEditable( true );
	gl1->addWidget(boxSeparator, 1, 1);

	QString help = tr("The column separator can be customized. The following special codes can be used:\n\\t for a TAB character \n\\s for a SPACE");
	help += "\n"+tr("The separator must not contain the following characters: 0-9eE.+-");

	boxSeparator->setWhatsThis(help);
	sepText->setWhatsThis(help);
	boxSeparator->setToolTip(help);
	sepText->setToolTip(help);

	boxNames = new QCheckBox(tr( "Include Column &Names" ));
    boxNames->setChecked( true );
	
    boxSelection = new QCheckBox(tr( "Export &Selection" ));
    boxSelection->setChecked( false );

	QVBoxLayout *vl1 = new QVBoxLayout();
	vl1->addLayout( gl1 );
	vl1->addWidget( boxNames );
	vl1->addWidget( boxSelection );
	
	QHBoxLayout *hbox3 = new QHBoxLayout();	
	buttonOk = new QPushButton(tr( "&OK" ));
    buttonOk->setDefault( true );
	hbox3->addWidget( buttonOk );
    buttonCancel = new QPushButton(tr( "&Cancel" ));
	hbox3->addWidget( buttonCancel );
	buttonHelp = new QPushButton(tr( "&Help" ));
	hbox3->addWidget( buttonHelp );
	hbox3->addStretch();
	
	QVBoxLayout *vl = new QVBoxLayout( this );
    vl->addLayout(vl1);
	vl->addStretch();
	vl->addLayout(hbox3);
	
	resize(minimumSize());
   
    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( buttonHelp, SIGNAL( clicked() ), this, SLOT( help() ) );
	connect( boxAllSpreadsheets, SIGNAL( toggled(bool) ), this, SLOT( enableSpreadsheetName(bool) ) );
}

void ExportSpreadsheetDialog::help()
{
	QString s = tr("The column separator can be customized. The following special codes can be used:\n\\t for a TAB character \n\\s for a SPACE");
	s += "\n"+tr("The separator must not contain the following characters: 0-9eE.+-");
	QMessageBox::about(0, tr("Help"),s);
}

void ExportSpreadsheetDialog::setSpreadsheetNames(const QStringList& names)
{
	boxSpreadsheet->addItems(names);
}

void ExportSpreadsheetDialog::setActiveSpreadsheetName(const QString& name)
{
	boxSpreadsheet->setCurrentIndex(boxSpreadsheet->findText(name));
}

void ExportSpreadsheetDialog::enableSpreadsheetName(bool ok)
{
	boxSpreadsheet->setEnabled(!ok);
}

void ExportSpreadsheetDialog::accept()
{
	QString sep = boxSeparator->currentText();
	sep.replace(tr("TAB"), "\t", Qt::CaseInsensitive);
	sep.replace(tr("SPACE"), " ");
	sep.replace("\\s", " ");
	sep.replace("\\t", "\t");

	if (sep.contains(QRegExp("[0-9.eE+-]")))
	{
		QMessageBox::warning(0, tr("Import options error"),
				tr("The separator must not contain the following characters: 0-9eE.+-"));
		return;
	}

	hide();
	if (boxAllSpreadsheets->isChecked())
		emit exportAllSpreadsheets(sep, boxNames->isChecked(), boxSelection->isChecked());
	else
		emit exportSpreadsheet(boxSpreadsheet->currentText(), sep, 
				boxNames->isChecked(), boxSelection->isChecked());
	close();
}

void ExportSpreadsheetDialog::setColumnSeparator(const QString& sep)
{
	if (sep=="\t")
		boxSeparator->setCurrentIndex(0);
	else if (sep==" ")
		boxSeparator->setCurrentIndex(1);
	else if (sep==";\t")
		boxSeparator->setCurrentIndex(2);
	else if (sep==",\t")
		boxSeparator->setCurrentIndex(3);
	else if (sep=="; ")
		boxSeparator->setCurrentIndex(4);
	else if (sep==", ")
		boxSeparator->setCurrentIndex(5);
	else if (sep==";")
		boxSeparator->setCurrentIndex(6);
	else if (sep==",")
		boxSeparator->setCurrentIndex(7);
	else
	{
		QString separator = sep;
		boxSeparator->setEditText(separator.replace(" ","\\s").replace("\t","\\t"));
	}
}

ExportSpreadsheetDialog::~ExportSpreadsheetDialog()
{
}
