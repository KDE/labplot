/***************************************************************************
File                 : FITSHeaderEditAddUnitDialog.h
Project              : LabPlot
Description          : Widget for adding or modifying FITS header keyword units
--------------------------------------------------------------------
Copyright            : (C) 2016-2017 by Fabian Kristof (fkristofszabolcs@gmail.com)
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
#ifndef FITSHEADEREDITADDUNITDIALOG_H
#define FITSHEADEREDITADDUNITDIALOG_H

#include <QDialog>
#include "ui_fitsheadereditaddunitwidget.h"

class QPushButton;

class FITSHeaderEditAddUnitDialog : public QDialog {
	Q_OBJECT

public:
	explicit FITSHeaderEditAddUnitDialog(const QString& unit = QString(), QWidget* parent = nullptr);
	~FITSHeaderEditAddUnitDialog();
	QString unit() const;

private:
	Ui::FITSHeaderEditAddUnitDialog  ui;
	QPushButton* m_okButton;

private slots:
	void unitChanged();
};

#endif // FITSHEADEREDITADDUNITDIALOG_H
