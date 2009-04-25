/***************************************************************************
    File                 : ExportSpreadsheetDialog.h
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
#ifndef EXPORT_TABLE_DIALOG_H
#define EXPORT_TABLE_DIALOG_H

#include <QDialog>
class QPushButton;
class QCheckBox;
class QComboBox;

//! Export ASCII dialog
class ExportSpreadsheetDialog : public QDialog
{
    Q_OBJECT

public:

	//! Constructor
	/**
	 * \param parent parent widget
	 * \param fl window flags
	 */
    ExportSpreadsheetDialog( QWidget* parent = 0, Qt::WFlags fl = 0 );
	//! Destructor
    ~ExportSpreadsheetDialog();

private:
    QPushButton* buttonOk;
	QPushButton* buttonCancel;
	QPushButton* buttonHelp;
    QCheckBox* boxNames;
    QCheckBox* boxSelection;
	QCheckBox* boxAllSpreadsheets;
    QComboBox* boxSeparator;
	QComboBox* boxSpreadsheet;

public slots:
	//! Set the column delimiter
	void setColumnSeparator(const QString& sep);
	//! Set the list of spreadsheets
	void setSpreadsheetNames(const QStringList& names);
	//! Select a spreadsheet
	void setActiveSpreadsheetName(const QString& name);

private slots:
	//! Enable/disable the spreadsheets combox box
	/**
	 * The spreadsheets combo box is disabled when
	 * the checkbox "all" is selected.
	 */
	void enableSpreadsheetName(bool ok);

protected slots:
	//! Accept changes
	void accept();
	//! Display help
	void help();

signals:
	//! Export one spreadsheet
	/**
	 * \param spreadsheetName name of the spreadsheet to export
	 * \param separator separator to be put between the columns
	 * \param exportColumnNames flag: column names in the first line or not
	 * \param exportSelection flag: export only selection or all cells
	 */
	void exportSpreadsheet(const QString& spreadsheetName, const QString& separator, bool exportColumnNames, bool exportSelection);
	//! Export all spreadsheets
	/**
	 * \param separator separator to be put between the columns
	 * \param exportColumnNames flag: column names in the first line or not
	 * \param exportSelection flag: export only selection or all cells
	 */
	void exportAllSpreadsheets(const QString& separator, bool exportColumnNames, bool exportSelection);

};

#endif // ifndef EXPORT_TABLE_DIALOG_H
