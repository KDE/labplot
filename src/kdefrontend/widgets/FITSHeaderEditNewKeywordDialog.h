/*
    File                 : FITSHeaderEditNewKeywordDialog.h
    Project              : LabPlot
    Description          : Widget for adding new keyword in the FITS edit widget
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
    SPDX-FileCopyrightText: 2016-2019 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef FITSHEADEREDITNEWKEYWORDDIALOG_H
#define FITSHEADEREDITNEWKEYWORDDIALOG_H

#include "backend/datasources/filters/FITSFilter.h"
#include "ui_fitsheadereditnewkeywordwidget.h"

#include <QDialog>

class QPushButton;
class QAbstractButton;

class FITSHeaderEditNewKeywordDialog : public QDialog {
	Q_OBJECT

public:
	explicit FITSHeaderEditNewKeywordDialog(QWidget* parent = nullptr);
	~FITSHeaderEditNewKeywordDialog();

	FITSFilter::Keyword newKeyword() const;

private:
	QPushButton* m_okButton;
	QPushButton* m_cancelButton;

	Ui::FITSHeaderEditNewKeywordDialog ui;
	FITSFilter::Keyword m_newKeyword;
	int okClicked();

private Q_SLOTS:
	void slotButtonClicked(QAbstractButton*);
};

#endif // FITSHEADEREDITNEWKEYWORDDIALOG_H
