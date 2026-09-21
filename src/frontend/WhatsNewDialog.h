/*
	File                 : WhatsNewDialog.h
	Project              : LabPlot
	Description          : Dialog showing what's new in the current release
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WHATSNEWDIALOG_H
#define WHATSNEWDIALOG_H

#include <QDialog>

class WhatsNewDialog : public QDialog {
	Q_OBJECT

public:
	explicit WhatsNewDialog(QWidget* parent = nullptr);
	~WhatsNewDialog() override;
};

#endif // WHATSNEWDIALOG_H
