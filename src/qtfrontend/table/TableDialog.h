/***************************************************************************
    File                 : TableDialog.h
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
#ifndef TABLEDIALOG_H
#define TABLEDIALOG_H

#include <QDialog>

class QPushButton;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QLabel;
class QTextEdit;
class QSpinBox;
class Table;

//! Column options dialog
class TableDialog : public QDialog
{
    Q_OBJECT

public:
    TableDialog(Table *t, QWidget* parent, Qt::WFlags fl = 0 );

private slots:
	void prevColumn();
	void nextColumn();
	void selectColumn(int sc);
	void updateColumn(int);
	void changeColWidth(int width);
	void showPrecisionBox(int item);
	void updatePrecision(int prec);
	void setPlotDesignation(int i);
	void accept();
	void apply();
	void updateDisplay(int item);
	void enablePrecision(int f);
	void setNumericFormat(int type, int prec, bool allRightColumns);
	void setDayFormat(const QString& format, bool allRightColumns);
	void setMonthFormat(const QString& format, bool allRightColumns);

signals:
	void nameChanged(const QString&);
	void enumRightCols(bool);
	void changeWidth(const QString&, bool);

private:
    void setDateTimeFormat(int type, const QString& format, bool allRightColumns);
	void setTextFormat(bool allRightColumns);
    void closeEvent( QCloseEvent *);

    Table * m_table;

    QPushButton* buttonOk;
    QPushButton* buttonCancel;
	QPushButton* buttonApply;
	QPushButton* buttonPrev;
	QPushButton* buttonNext;
    QComboBox* boxSelectColumn;
    QLineEdit* colName;
    QCheckBox* enumerateAllBox;
    QCheckBox* applyToRightCols;
    QCheckBox* applyToAllBox;
    QComboBox* formatBox;
	QComboBox* displayBox;
    QComboBox* columnsBox;
    QSpinBox* colWidth, *precisionBox;
	QLabel *labelNumeric, *labelFormat;
	QTextEdit *comments;
	QCheckBox *boxShowTableComments;
};

#endif // TABLEDIALOG_H
