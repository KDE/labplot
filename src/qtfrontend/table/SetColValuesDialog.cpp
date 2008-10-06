/***************************************************************************
    File                 : SetColValuesDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, 
                           Tilman Benkert,
                           Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
                           knut.franke*gmx.de
    Description          : Set column values dialog
                           
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
#include "SetColValuesDialog.h"
#include "Table.h"
#include "core/ScriptEdit.h"

#include <QTableWidget>
#include <QTableWidgetSelectionRange>
#include <QList>
#include <QLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QTextCursor>

SetColValuesDialog::SetColValuesDialog(AbstractScriptingEngine *engine, QWidget* parent, Qt::WFlags fl)
    : QDialog( parent, fl ), scripted(engine)
{
	setWindowTitle( tr( "Set column values" ) );
	setSizeGripEnabled(true);

	QHBoxLayout *hbox1 = new QHBoxLayout(); 
	hbox1->addWidget(new QLabel(tr("For row (i)")));
	start = new QSpinBox();
	start->setMinimum(1);
	hbox1->addWidget(start);

	hbox1->addWidget(new QLabel(tr("to")));

	end = new QSpinBox();
	end->setMinimum(1);
	hbox1->addWidget(end);

	if (sizeof(int)==2)
	{ // 16 bit signed integer
		start->setMaxValue(0x7fff);
		end->setMaxValue(0x7fff);
	}
	else
	{ // 32 bit signed integer
		start->setMaxValue(0x7fffffff);
		end->setMaxValue(0x7fffffff);
	}

	QGridLayout *gl1 = new QGridLayout();
	functions = new QComboBox();
	gl1->addWidget(functions, 0, 0);
	btnAddFunction = new QPushButton(tr( "Add function" ));
	gl1->addWidget(btnAddFunction, 0, 1);
	boxColumn = new QComboBox();
	gl1->addWidget(boxColumn, 1, 0);
	btnAddCol = new QPushButton(tr( "Add column" ));
	gl1->addWidget(btnAddCol, 1, 1);

	addCellButton = new QPushButton(tr( "Add cell" ));
	gl1->addWidget(addCellButton, 2, 1);

	QHBoxLayout *hbox3 = new QHBoxLayout(); 
	buttonPrev = new QPushButton(tr("&<< Prev.","previous column"));
	hbox3->addWidget(buttonPrev);
	boxSelectColumn = new QComboBox();
	hbox3->addWidget(boxSelectColumn);
	hbox3->setStretchFactor(boxSelectColumn, 1);
	buttonNext = new QPushButton(tr("Next &>>","next column"));
	hbox3->addWidget(buttonNext);

	QGroupBox *gb = new QGroupBox();
	QVBoxLayout *vbox1 = new QVBoxLayout(); 
	vbox1->addLayout(hbox1);
	vbox1->addLayout(gl1);
	gb->setLayout(vbox1);
	gb->setSizePolicy(QSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred));

	explain = new QTextEdit();
	explain->setReadOnly (true);
	explain->setSizePolicy(QSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred));
	QPalette palette = explain->palette();
	palette.setColor(QPalette::Active, QPalette::Base, Qt::lightGray);
	explain->setPalette(palette);

	QGridLayout *gl2 = new QGridLayout(); 
	gl2->addWidget(explain, 0, 0, 2, 1);
	gl2->addLayout(hbox3, 0, 1);
	gl2->addWidget(gb, 1, 1);

	commands = new ScriptEdit( m_scripting_engine);

	QVBoxLayout *vbox2 = new QVBoxLayout(); 
	btnOk = new QPushButton(tr( "&OK" ));
	vbox2->addWidget(btnOk);
	btnApply = new QPushButton(tr( "&Apply" ));

	vbox2->addWidget(btnApply);
	btnCancel = new QPushButton(tr( "Cancel" ));
	vbox2->addWidget(btnCancel);
	vbox2->addStretch();

	QHBoxLayout *hbox4 = new QHBoxLayout(); 
	hbox4->addWidget(commands);
	hbox4->addLayout(vbox2);

	QVBoxLayout* vbox3 = new QVBoxLayout();
	vbox3->addLayout(gl2);
	colNameLabel = new QLabel();
	vbox3->addWidget(colNameLabel);
	vbox3->addLayout(hbox4);

	setLayout(vbox3);
	setFocusProxy (commands);
	commands->setFocus();

	setFunctions();
	if (functions->count() > 0)
		insertExplain(0);

	connect(btnAddFunction, SIGNAL(clicked()),this, SLOT(insertFunction()));
	connect(btnAddCol, SIGNAL(clicked()),this, SLOT(insertCol()));
	connect(addCellButton, SIGNAL(clicked()),this, SLOT(insertCell()));
	connect(btnOk, SIGNAL(clicked()),this, SLOT(accept()));
	connect(btnApply, SIGNAL(clicked()),this, SLOT(apply()));
	connect(btnCancel, SIGNAL(clicked()),this, SLOT(close()));
	connect(functions, SIGNAL(activated(int)),this, SLOT(insertExplain(int)));
	connect(buttonPrev, SIGNAL(clicked()), this, SLOT(prevColumn()));
	connect(buttonNext, SIGNAL(clicked()), this, SLOT(nextColumn()));
}

void SetColValuesDialog::prevColumn()
{
	int sc = m_table->selectedColumn();
	updateColumn(--sc);
}

void SetColValuesDialog::nextColumn()
{
	int sc = m_table->selectedColumn();
	updateColumn(++sc);
}

void SetColValuesDialog::updateColumn(int sc)
{
	if(sc <0) sc = m_table->columnCount() -1;
	if(sc >= m_table->columnCount()) sc = 0;

	boxSelectColumn->setCurrentIndex(sc);

	m_table->setSelectedCol(sc);
	// TODO replace the commented out lines (###)
	// acessing TableView from outside Table is absolutely forbidden
	//### m_table->table()->clearSelection();
	//### m_table->table()->selectColumn(sc);
	colNameLabel->setText("col(\""+m_table->columnLabel(sc)+"\")= ");

	// TODO
	//### QStringList com = m_table->getCommands();
	QStringList com;
	commands->setText(com[sc]);
	QTextCursor cursor = commands->textCursor();
	cursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
}

QSize SetColValuesDialog::sizeHint() const 
{
	return QSize( 400, 190 );
}

void SetColValuesDialog::customEvent(QEvent *e)
{
	if (e->type() == SCRIPTING_CHANGE_EVENT)
		scriptingChangeEvent((ScriptingChangeEvent*)e);
}

void SetColValuesDialog::accept()
{
	if (apply())
		close();
}

bool SetColValuesDialog::apply()
{
	// TODO replace the commented out lines (###)
	int col = m_table->selectedColumn();
	QString formula = commands->text();
	QString oldFormula; //### = m_table->getCommands()[col];

	//### m_table->setCommand(col,formula);
	//### if(m_table->calculate(col,start->value()-1,end->value()-1))
	//### 	return true;
	//### m_table->setCommand(col,oldFormula);
	return false;
}

void SetColValuesDialog::setFunctions()
{
	functions->insertStringList(m_scripting_engine->mathFunctions(), -1);
}

void SetColValuesDialog::insertExplain(int index)
{
	explain->setText(m_scripting_engine->mathFunctionDoc(functions->text(index)));
}

void SetColValuesDialog::insertFunction()
{
	commands->insertFunction(functions->currentText());
}

void SetColValuesDialog::insertCol()
{
	commands->insert(boxColumn->currentText());
}

void SetColValuesDialog::insertCell()
{
	QString f=boxColumn->currentText().remove(")")+", i)";
	commands->insert(f);
}

void SetColValuesDialog::setTable(Table* w)
{
	m_table=w;
	QStringList colNames=w->colNames();
	int cols = w->columnCount();
	for (int i=0; i<cols; i++)
		boxColumn->addItem("col(\""+colNames[i]+"\")",i); 
	for (int i=0; i<cols; i++)
		boxSelectColumn->addItem(colNames[i]); 

	// TODO
	/*
	int s = w->table()->currentSelection();
	if (s >= 0)
	{
		Q3TableSelection sel = w->table()->selection(s);
		w->setSelectedCol(sel.leftCol());

		start->setValue(sel.topRow() + 1);
		end->setValue(sel.bottomRow() + 1);
	}
	else
	{
		start->setValue(1);
		end->setValue(w->rowCount());
	}

	boxSelectColumn->setCurrentIndex(w->selectedColumn());
	updateColumn(w->selectedColumn());
	commands->setContext(w);
	connect(boxSelectColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(updateColumn(int)));
	*/
}

SetColValuesDialog::~SetColValuesDialog()
{
}
