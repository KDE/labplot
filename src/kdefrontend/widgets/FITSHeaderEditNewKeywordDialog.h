/***************************************************************************
File                 : FITSHeaderEditNewKeywordDialog.h
Project              : LabPlot
Description          : Widget for adding new keyword in the FITS edit widget
--------------------------------------------------------------------
Copyright            : (C) 2016 by Fabian Kristof (fkristofszabolcs@gmail.com)
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
#ifndef FITSHEADEREDITNEWKEYWORDDIALOG_H
#define FITSHEADEREDITNEWKEYWORDDIALOG_H

#include "backend/datasources/filters/FITSFilter.h"
#include "ui_fitsheadereditnewkeywordwidget.h"
#include <QAbstractButton>

#include <QDialog>

class QPushButton;
class QAbstractButton;
class FITSHeaderEditNewKeywordDialog : public QDialog {
	Q_OBJECT

public:
	explicit FITSHeaderEditNewKeywordDialog(QWidget *parent = 0);
	FITSFilter::Keyword newKeyword() const;

private:
	QPushButton* m_okButton;
	QPushButton* m_cancelButton;

	Ui::FITSHeaderEditNewKeywordDialog ui;
	FITSFilter::Keyword m_newKeyword;
	int okClicked();

private slots:
	void slotButtonClicked(QAbstractButton *button);
};

#endif // FITSHEADEREDITNEWKEYWORDDIALOG_H
