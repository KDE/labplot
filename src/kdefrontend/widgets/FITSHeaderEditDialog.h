/***************************************************************************
File                 : FITSHeaderEditDialog.cpp
Project              : LabPlot
Description          : Dialog for listing/editing FITS header keywords
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
#ifndef FITSHEADEREDITDIALOG_H
#define FITSHEADEREDITDIALOG_H

#include "FITSHeaderEditWidget.h"
#include <QDialog>

class QPushButton;

class FITSHeaderEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit FITSHeaderEditDialog(QWidget* parent = nullptr);
	~FITSHeaderEditDialog() override;
	bool saved() const;

private:
	FITSHeaderEditWidget* m_headerEditWidget;
	bool m_saved{false};
	QPushButton* m_okButton;

private slots:
	void save();
	void headersChanged(bool);
};

#endif // FITSHEADEREDITDIALOG_H
