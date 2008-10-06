/***************************************************************************
    File                 : SymbolDialog.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Tool window to select special text characters
                           
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
#ifndef SYMBOLDIALOG_H
#define SYMBOLDIALOG_H

#include <QDialog>
class QPushButton;
class QSizePolicy;
class QGroupBox;
class QShortcut;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QButtonGroup;

//! Tools window to select special text characters
class SymbolDialog : public QDialog
{
    Q_OBJECT

public:
	//! Character set
	enum CharSet{
		lowerGreek = 0, /*!< lower case Greek letters */
		upperGreek = 1,  /*!< upper case Greek letters */
		mathSymbols = 2, /*!< mathematical symbols */
		arrowSymbols = 3, /*!< arrow symbols */
		numberSymbols = 4 /*!< number symbols (e.g. 1/2, vi)*/
	};

	//! Constructor
	/**
	 * \param charSet character set (lower- or uppercase)
	 * \param parent parent widget
	 * \param fl window flags
	 */
    SymbolDialog(CharSet charSet, QWidget* parent = 0, Qt::WFlags fl = 0 );
	//! Destructor
    ~SymbolDialog();

private:
	//! Show lowercase Greek characters
	void initLowerGreekChars();
	//! Show uppercase Greek characters
	void initUpperGreekChars();
	//! Show mathematical symbols
	void initMathSymbols();
	//! Show arrow symbols
	void initArrowSymbols();
	//! Show number symbols
	void initNumberSymbols();

    QButtonGroup * buttons;
	QPushButton * closeButton;
	int numButtons;
	QVBoxLayout * mainLayout;
	QGridLayout * gridLayout;

protected:
	//! Event handler: When the dialog gets the focus the first button is set to react on [return]
	void focusInEvent( QFocusEvent * event );

public slots:
	//! Change language (reset title)
    virtual void languageChange();
	//! Find and emit char from pressed button
	void getChar(int btnIndex);
	//! Find and emit char from current button ([return] pressed)
	void addCurrentChar();

signals:
	//! Emitted when a letter is to be added
	void addLetter(const QString&);
};

#endif // exportDialog_H
