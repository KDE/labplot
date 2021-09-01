/*
File                 : FITSHeaderEditDialog.cpp
Project              : LabPlot
Description          : Dialog for listing/editing FITS header keywords
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2016-2017 Fabian Kristof (fkristofszabolcs@gmail.com)
*/

/***************************************************************************
*                                                                         *
*  SPDX-License-Identifier: GPL-2.0-or-later
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
