/***************************************************************************
    File                 : TableDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Column options dialog

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
#include "TableDialog.h"
#include "Table.h"

#include <QApplication>
#include <QMessageBox>
#include <QLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QRegExp>
#include <QDate>

TableDialog::TableDialog(Table *t, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
    m_table(t)
{
    setWindowTitle( tr( "Column options" ) );
    setSizeGripEnabled(true);

	QHBoxLayout *hboxa = new QHBoxLayout();
	hboxa->addWidget(new QLabel(tr( "Column Name:" )));
    colName = new QLineEdit();
    hboxa->addWidget(colName);

	enumerateAllBox = new QCheckBox(tr("Enumerate all to the right" ));

	buttonPrev = new QPushButton(tr("&<< Prev.","previous column"));
	buttonPrev->setAutoDefault(false);

	boxSelectColumn = new QComboBox();
	QStringList colNames = m_table->colNames();
	for (int i=0; i<colNames.size(); i++)
		boxSelectColumn->addItem(colNames[i]); 
	boxSelectColumn->setCurrentIndex(m_table->selectedColumn());

	buttonNext = new QPushButton(tr("Next &>>","next column"));
	buttonNext->setAutoDefault(false);

	QHBoxLayout *hboxb = new QHBoxLayout();
	hboxb->addWidget(buttonPrev);
	hboxb->addWidget(boxSelectColumn);
	hboxb->addWidget(buttonNext);
	hboxb->setStretchFactor(boxSelectColumn, 1);

	QVBoxLayout *vbox1 = new QVBoxLayout();
	vbox1->addLayout(hboxa);
	vbox1->addWidget(enumerateAllBox);
	vbox1->addStretch();

	buttonOk = new QPushButton(tr( "&OK" ));
	buttonOk->setDefault(true);

	buttonApply = new QPushButton(tr("&Apply"));
	buttonApply->setAutoDefault(false);

	buttonCancel = new QPushButton(tr( "&Cancel" ));
	buttonCancel->setAutoDefault(false);

	QVBoxLayout  *vbox2 = new QVBoxLayout();
	vbox2->setSpacing(5);
	vbox2->setMargin(5);
	vbox2->addWidget(buttonOk);
	vbox2->addWidget(buttonApply);
	vbox2->addWidget(buttonCancel);

	QGridLayout  *hbox1 = new QGridLayout();
	hbox1->setSpacing(5);
	hbox1->addLayout(hboxb, 0, 0, 1, 2);
	hbox1->addLayout(vbox1, 1, 0);
	hbox1->addLayout(vbox2, 1, 1);

	QGridLayout *gl1 = new QGridLayout();
	gl1->addWidget(new QLabel( tr("Plot Designation:")), 0, 0);

	columnsBox = new QComboBox();
	columnsBox->addItem(tr("None"));
	columnsBox->addItem(tr("X (abscissae)"));
	columnsBox->addItem(tr("Y (ordinates)"));
	columnsBox->addItem(tr("Z (height)"));
	columnsBox->addItem(tr("X Error"));
	columnsBox->addItem(tr("Y Error"));
	gl1->addWidget(columnsBox, 0, 1);

	gl1->addWidget(new QLabel(tr("Display")), 1, 0);

	displayBox = new QComboBox();
	displayBox->addItem(tr("Numeric"));
	displayBox->addItem(tr("Text"));
	displayBox->addItem(tr("Date"));
	displayBox->addItem(tr("Time"));
	displayBox->addItem(tr("Month"));
	displayBox->addItem(tr("Day of Week"));
	gl1->addWidget(displayBox, 1, 1);

	labelFormat = new QLabel(tr( "Format:" ));
	gl1->addWidget(labelFormat, 2, 0);

	formatBox = new QComboBox(false);
	gl1->addWidget(formatBox, 2, 1);

	labelNumeric = new QLabel(tr( "Precision:" ));
	gl1->addWidget(labelNumeric, 3, 0);

	precisionBox = new QSpinBox();
	gl1->addWidget(precisionBox, 3, 1);

	applyToRightCols = new QCheckBox(tr( "Apply to all columns to the right" ));

	QVBoxLayout *vbox3 = new QVBoxLayout();
	vbox3->addLayout(gl1);
	vbox3->addWidget(applyToRightCols);

	QGroupBox *gb = new QGroupBox(tr("Options"));
	gb->setLayout(vbox3);

	QHBoxLayout  *hbox2 = new QHBoxLayout();
	hbox2->addWidget(new QLabel(tr( "Column Width:" )));

	colWidth = new QSpinBox();
	colWidth->setRange(0, 1000);
	colWidth->setSingleStep(10);

	hbox2->addWidget(colWidth);

	applyToAllBox = new QCheckBox(tr( "Apply to all" ));
	hbox2->addWidget(applyToAllBox);

	comments = new QTextEdit();
	boxShowTableComments = new QCheckBox(tr("&Display Comments in Header"));
	// TODO
	//boxShowTableComments->setChecked(m_table->commentsEnabled());

	QVBoxLayout* vbox4 = new QVBoxLayout();
	vbox4->addLayout(hbox1);
	vbox4->addWidget(gb);
	vbox4->addLayout(hbox2);
	vbox4->addWidget(new QLabel(tr( "Comment:" )));
	vbox4->addWidget(comments);
	vbox4->addWidget(boxShowTableComments);

	setLayout(vbox4);
	setFocusProxy (colName);

	updateColumn(m_table->selectedColumn());
	resize(minimumSize());

	// signals and slots connections
	connect(colWidth, SIGNAL(valueChanged(int)), this, SLOT(changeColWidth(int)));
	connect(buttonApply, SIGNAL(clicked()), this, SLOT(apply()));
	connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
	connect(buttonCancel, SIGNAL( clicked() ), this, SLOT( close() ) );
	connect(columnsBox, SIGNAL(activated(int)), this, SLOT(setPlotDesignation(int)) );
	connect(displayBox, SIGNAL(activated(int)), this, SLOT(updateDisplay(int)));
	connect(buttonPrev, SIGNAL(clicked()), this, SLOT(prevColumn()));
	connect(boxSelectColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(selectColumn(int)));
	connect(buttonNext, SIGNAL(clicked()), this, SLOT(nextColumn()));
	connect(formatBox, SIGNAL(activated(int)), this, SLOT(enablePrecision(int)) );
	connect(precisionBox, SIGNAL(valueChanged(int)), this, SLOT(updatePrecision(int)));
	connect(boxShowTableComments, SIGNAL(toggled(bool)), m_table, SLOT(showComments(bool)));
}

void TableDialog::enablePrecision(int f)
{
	if(displayBox->currentIndex())
		return;//the col type != "Numeric"

	precisionBox->setEnabled(f > 0);
}

void TableDialog::accept()
{
	apply();
	close();
}

void TableDialog::prevColumn()
{
	int sc = m_table->selectedColumn();
	apply();
	updateColumn(--sc);
}

void TableDialog::nextColumn()
{
	int sc = m_table->selectedColumn();
	apply();
	updateColumn(++sc);
}

void TableDialog::selectColumn(int sc)
{
	apply();
	updateColumn(sc);
}

void TableDialog::updateColumn(int sc)
{
	if(sc <0) sc = m_table->columnCount() -1;
	if(sc >= m_table->columnCount()) sc = 0;

	int colType = m_table->columnType(sc);

	// TODO replace the commented out lines (###)
	// acessing TableView from outside Table is absolutely forbidden
	m_table->setSelectedCol(sc);
	//### m_table->table()->clearSelection ();
	//### m_table->table()->selectColumn(sc);
	columnsBox->setCurrentIndex(m_table->colPlotDesignation(sc));

	QString colLabel = m_table->colLabel(sc);
	colName->setText(colLabel);
	colName->selectAll();

	comments->setText(m_table->colComment(sc));
	colWidth->setValue(m_table->columnWidth(sc));

	displayBox->setCurrentIndex(colType);
	updateDisplay(colType);

	// TODO not implemented yet in the new Table
	//### m_table->saveToMemory();

	if (colType == SciDAVis::Numeric)
	{
		int f, prec;
		m_table->columnNumericFormat(sc, &f, &prec);

		formatBox->setCurrentIndex(f);
		precisionBox->setValue(prec);
		enablePrecision(f);
	}
	else if (colType == SciDAVis::Time || colType == SciDAVis::Date)
	{
		QString format = m_table->columnFormat(sc);
		if (formatBox->findText(format) < 0)
			formatBox->insertItem(0, format);

		formatBox->setCurrentText(format);
	}
	else if (colType == SciDAVis::Day)
	{
		QString format = m_table->columnFormat(sc);
		if (format == "ddd")
			formatBox->setCurrentIndex(0);
		else if (format == "dddd")
			formatBox->setCurrentIndex(1);
		else if (format == "d")
			formatBox->setCurrentIndex(2);
	}
	else if (colType == SciDAVis::Month)
	{
		QString format = m_table->columnFormat(sc);
		if (format == "MMM")
			formatBox->setCurrentIndex(0);
		else if (format == "MMMM")
			formatBox->setCurrentIndex(1);
		else if (format == "M")
			formatBox->setCurrentIndex(2);
	}
}

void TableDialog::changeColWidth(int width)
{
	m_table->changeColWidth(width, applyToAllBox->isChecked());
	// TODO
	//m_table->setHeaderColType();
}

void TableDialog::apply()
{
	if (colName->text().contains("_")){
		QMessageBox::warning(this, tr("Warning"),
				tr("For internal consistency reasons the underscore character is replaced with a minus sign."));}

	QString name=colName->text().replace("-", "_");
	if (name.contains(QRegExp("\\W")))
	{
		QMessageBox::warning(this,tr("Error"),
				tr("The column names must only contain letters and digits!"));
		name.remove(QRegExp("\\W"));
	}
	m_table->changeColWidth(colWidth->value(), applyToAllBox->isChecked());
	// FIXME: allowing line breaks in comments may brake the file format
	// old code: m_table->setColComment(m_table->selectedColumn(), comments->text().replace("\n", " ").replace("\t", " "));
	m_table->setColComment(m_table->selectedColumn(), comments->text().replace("\t", " "));

	m_table->changeColName(name.replace("_", "-"));
	m_table->enumerateRightCols(enumerateAllBox->isChecked());
	enumerateAllBox->setChecked(false);
	colName->setText(m_table->colNames().at(m_table->selectedColumn()));

	int format = formatBox->currentIndex();
	int colType = displayBox->currentIndex();
	switch(colType)
	{
		case 0:
			setNumericFormat(formatBox->currentIndex(), precisionBox->value(), applyToRightCols->isChecked());
			break;

		case 1:
			setTextFormat(applyToRightCols->isChecked());
			break;

		case 2:
			setDateTimeFormat(colType, formatBox->currentText(), applyToRightCols->isChecked());
			break;

		case 3:
			setDateTimeFormat(colType, formatBox->currentText(), applyToRightCols->isChecked());
			break;

		case 4:
			if(format == 0)
				setMonthFormat("MMM", applyToRightCols->isChecked());
			else if(format == 1)
				setMonthFormat("MMMM", applyToRightCols->isChecked());
			else if(format == 2)
				setMonthFormat("M", applyToRightCols->isChecked());
			break;

		case 5:
			if(format == 0)
				setDayFormat("ddd", applyToRightCols->isChecked());
			else if(format == 1)
				setDayFormat("dddd", applyToRightCols->isChecked());
			else if(format == 2)
				setDayFormat("d", applyToRightCols->isChecked());
			break;
	}

	boxSelectColumn->disconnect();
	QStringList colNames = m_table->colNames();
	boxSelectColumn->clear();
	for (int i=0; i<colNames.size(); i++)
		boxSelectColumn->addItem(colNames[i]); 
	boxSelectColumn->setCurrentIndex(m_table->selectedColumn());
	connect(boxSelectColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(selectColumn(int)));
}

void TableDialog::closeEvent( QCloseEvent* ce )
{
	// TODO not implemented yet in the new Table
	//m_table->freeMemory();
	ce->accept();
}

void TableDialog::setPlotDesignation(int i)
{
	switch(i)
	{
		case 0:
			m_table->setPlotDesignation(SciDAVis::noDesignation);
			break;

		case 1:
			m_table->setPlotDesignation(SciDAVis::X);
			break;

		case 2:
			m_table->setPlotDesignation(SciDAVis::Y);
			break;

		case 3:
			m_table->setPlotDesignation(SciDAVis::Z);
			break;

		case 4:
			m_table->setPlotDesignation(SciDAVis::xErr);
			break;

		case 5:
			m_table->setPlotDesignation(SciDAVis::yErr);
			break;
	}
}

void TableDialog::showPrecisionBox(int item)
{
	switch(item)
	{
		case 0:
			{
				precisionBox->hide();
				break;
			}
		case 1:
			{
				precisionBox->show();
				break;
			}
		case 2:
			{
				precisionBox->show();
				break;
			}
	}
}

void TableDialog::updatePrecision(int prec)
{
	setNumericFormat(formatBox->currentIndex(), prec, applyToRightCols->isChecked());
}

void TableDialog::updateDisplay(int item)
{
	labelFormat->show();
	formatBox->show();
	formatBox->clear();
	formatBox->setEditable ( false );
	labelNumeric->hide();
	precisionBox->hide();

	if (item == 0)
	{
		formatBox->addItem( tr( "Default" ) );
		formatBox->addItem( tr( "Decimal: 1000" ) );
		formatBox->addItem( tr( "Scientific: 1E3" ) );

		labelNumeric->show();
		precisionBox->show();
	}
	else
	{
		switch (item)
		{
			case 1:
				labelFormat->hide();
				formatBox->hide();
				break;

			case 2:
				formatBox->setEditable ( true );

				formatBox->addItem("dd/MM/yyyy");
				formatBox->addItem("dd.MM.yyyy");
				formatBox->addItem("dd MM yyyy");
				formatBox->addItem("yyyyMMdd");
				formatBox->addItem("yyyy-MM-dd");
				break;

			case 3:
				{
					formatBox->setEditable ( true );

					formatBox->addItem("h");
					formatBox->addItem("h ap");
					formatBox->addItem("h AP");
					formatBox->addItem("h:mm");
					formatBox->addItem("h:mm ap");
					formatBox->addItem("hh:mm");
					formatBox->addItem("h:mm:ss");
					formatBox->addItem("h:mm:ss.zzz");
					formatBox->addItem("mm:ss");
					formatBox->addItem("mm:ss.zzz");
					formatBox->addItem("hmm");
					formatBox->addItem("hmmss");
					formatBox->addItem("hhmmss");
				}
				break;

			case 4:
				{
					QDate date=QDate::currentDate();
					formatBox->addItem(QDate::shortMonthName(date.month()));
					formatBox->addItem(QDate::longMonthName(date.month()));
					formatBox->addItem(QDate::shortMonthName(date.month()).left(1));
				}
				break;

			case 5:
				{
					QDate date=QDate::currentDate();
					formatBox->addItem(QDate::shortDayName(date.dayOfWeek()));
					formatBox->addItem(QDate::longDayName(date.dayOfWeek()));
					formatBox->addItem(QDate::shortDayName(date.dayOfWeek()).left(1));
				}
				break;
		}
	}
}

void TableDialog::setDateTimeFormat(int type, const QString& format, bool allRightColumns)
{
	// TODO
	/*
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	bool ok = false;
	int sc = m_table->selectedColumn();
	if (allRightColumns){
		for (int i = sc; i<m_table->columnCount(); i++){
			if (type == SciDAVis::Date)
				ok = m_table->setDateFormat(format, i);
			else if (type == SciDAVis::Time)
				ok = m_table->setTimeFormat(format, i);
			if (!ok)
				break;
		}
	}
	else if (type == SciDAVis::Date)
		ok = m_table->setDateFormat(format, sc);
	else if (type == SciDAVis::Time)
		ok = m_table->setTimeFormat(format, sc);

	QApplication::restoreOverrideCursor();

	if (!ok){
		QMessageBox::critical(this, tr("Error"), tr("Couldn't guess the source data format, please specify it using the 'Format' box!")+"\n\n"+
				tr("For more information about the supported date/time formats please read the Qt documentation for the QDateTime class!"));
		return;
	}

	if (formatBox->findText(format) < 0){
		formatBox->insertItem(0, format);
		formatBox->setCurrentText(format);
	}
	m_table->notifyChanges();
	*/
}

void TableDialog::setNumericFormat(int type, int prec, bool allRightColumns)
{
	// TODO
	/*
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	int sc = m_table->selectedColumn();
	if (allRightColumns)
	{
		for (int i = sc; i<m_table->columnCount(); i++)
			m_table->setColNumericFormat(type, prec, i);
	}
	else
		m_table->setColNumericFormat(type, prec, sc);

	m_table->notifyChanges();
	QApplication::restoreOverrideCursor();
	*/
}

void TableDialog::setTextFormat(bool allRightColumns)
{
	int sc = m_table->selectedColumn();
// TODO
/*	if (allRightColumns){
		for (int i = sc; i<m_table->columnCount(); i++)
			m_table->setTextFormat(i);
	}
	else*/
		m_table->setTextFormat(sc);
}

void TableDialog::setDayFormat(const QString& format, bool allRightColumns)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	int sc = m_table->selectedColumn();
	if (allRightColumns){
		for (int i = sc; i<m_table->columnCount(); i++)
			m_table->setDayFormat(format, i);
	}
	else
		m_table->setDayFormat(format, sc);

	QApplication::restoreOverrideCursor();
	m_table->notifyChanges();
}

void TableDialog::setMonthFormat(const QString& format, bool allRightColumns)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	int sc = m_table->selectedColumn();
	if (allRightColumns){
		for (int i = sc; i<m_table->columnCount(); i++)
			m_table->setMonthFormat(format, i);
	}
	else
		m_table->setMonthFormat(format, sc);

	QApplication::restoreOverrideCursor();
	m_table->notifyChanges();
}
