/*
	File                 : ReplaceWidget.h
	Project              : LabPlot
	Description          : search&replace widget for the spreadhsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REPLACENWIDGET_H
#define REPLACENWIDGET_H

#include "ui_replacewidget.h"

class ReplaceWidget : public QWidget {
	Q_OBJECT

public:
	explicit ReplaceWidget(QWidget* parent = nullptr);
	~ReplaceWidget() override;

private:
	Ui::ReplaceWidget ui;

private Q_SLOTS:

Q_SIGNALS:
};

#endif
