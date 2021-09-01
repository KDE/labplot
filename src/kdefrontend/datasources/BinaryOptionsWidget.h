/*
    File                 : BinaryOptionsWidget.h
    Project              : LabPlot
    Description          : widget providing options for the import of binary data
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009-2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-FileCopyrightText: 2009 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BINARYOPTIONSWIDGET_H
#define BINARYOPTIONSWIDGET_H

#include "ui_binaryoptionswidget.h"

class BinaryFilter;

class BinaryOptionsWidget : public QWidget {
    Q_OBJECT

public:
	explicit BinaryOptionsWidget(QWidget*);
	void applyFilterSettings(BinaryFilter*) const;
	void loadSettings() const;
	void saveSettings();

private:
	Ui::BinaryOptionsWidget ui;
};

#endif
