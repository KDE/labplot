/*
    File                 : FITSHeaderEditAddUnitDialog.h
    Project              : LabPlot
    Description          : Widget for adding or modifying FITS header keyword units
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2017 Fabian Kristof <fkristofszabolcs@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
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

private Q_SLOTS:
	void unitChanged();
};

#endif // FITSHEADEREDITADDUNITDIALOG_H
