/*
	File                 : CASSettingsDialog.h
	Project              : LabPlot
	Description          : CAS settings dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CASSETTINGSDIALOG_H
#define CASSETTINGSDIALOG_H

#include <KConfigDialog>

class CASSettingsDialog : public KConfigDialog {
	Q_OBJECT

public:
	explicit CASSettingsDialog(QWidget*);
	~CASSettingsDialog() override;
};

#endif
