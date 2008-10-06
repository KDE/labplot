/***************************************************************************
    File                 : ImportTableDialog.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief,
                           Tilman Benkert, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Import ASCII file(s) dialog
                           
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
#ifndef IMPORT_TABLE_DIALOG_H
#define IMPORT_TABLE_DIALOG_H

#include "lib/ExtensibleFileDialog.h"

#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>

class QGroupBox;
class QPushButton;

//! Import ASCII file(s) dialog
class ImportTableDialog: public ExtensibleFileDialog
{
	Q_OBJECT

public:
	//! Possible destinations for the imported data.
	/**
	 * Important: Keep this in sync with the initialization of #m_import_mode in initAdvancedOptions().
	 */
	enum ImportMode {
		NewTables, //!< create a new table for each file (default)
		NewColumns, //!< add each file as new columns to the current table
		NewRows, //!< add each file as new rows to the current table
		Overwrite //!< replace content of current table with the selected file (like importing a single file in previous versions of SciDAVis)
	};

	//! Constructor
	/**
	 * \param importModeEnabled flag: enable/disable import mode combo box
	 * \param parent parent widget (only affects placement of the dialog)
	 * \param extended flag: show/hide the advanced options on start-up
	 * \param flags window flags
	 */
	ImportTableDialog(bool importModeEnabled, QWidget * parent = 0, bool extended = true, Qt::WFlags flags = 0 );

	//! Return the selected import mode
	/**
	 * \sa ImportMode
	 */
	ImportMode importMode() const { return (ImportMode) m_import_mode->currentIndex(); }
	//! Return the selected column separator.
	const QString columnSeparator() const;
	//! Return the number of lines to be skipped at the start of each file.
	int ignoredLines() const { return m_ignored_lines->value(); }
	//! Whether to rename columns based on the first (non-skipped) line.
	bool renameColumns() const { return m_rename_columns->isChecked(); }
	//! Whether to replace sequences of whitespace charecters with a single space.
	bool simplifySpaces() const { return m_simplify_spaces->isChecked(); }
	//! Whether to remove whitespace from beginning and end of lines.
	bool stripSpaces() const { return m_strip_spaces->isChecked(); }
	//! Whether the user wants the import options to be saved.
	bool rememberOptions() const { return m_remember_options->isChecked(); }

	//! Set the selected columns separator.
	void setColumnSeparator(const QString &sep);
	
	//! Returns the locale chosen for the import by the user
	QLocale decimalSeparators();
	//! Returns whether a custom locale should be used
	bool useCustomLocale() const { return m_use_custom_locale->isChecked(); };
		
private slots:
	//! Display help for advanced options.
	void displayHelp();
	//! For #Overwrite mode, allow only one file to be selected.
	void updateImportMode(int mode);

private:
	void closeEvent(QCloseEvent*);
	//! Initialise #m_advanced_options and everything it contains.
	void initAdvancedOptions();

	//! Container widget for all advanced options.
	QGroupBox *m_advanced_options;
	QCheckBox *m_remember_options, *m_use_custom_locale;
	QPushButton *m_help_button;
	// the actual options
	QComboBox *m_import_mode, *m_column_separator, *boxDecimalSeparator;
	QSpinBox *m_ignored_lines;
	QCheckBox *m_rename_columns, *m_simplify_spaces, *m_strip_spaces;
};

#endif // ifndef IMPORT_TABLE_DIALOG_H
