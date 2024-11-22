/*
    File                 : AboutDialog.h
    Project              : LabPlot
    Description          : Custom about dialog
    --------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <KAboutApplicationDialog>

class AboutDialog: public KAboutApplicationDialog {
public:
	explicit AboutDialog(const KAboutData&, QWidget*);
};

#endif
