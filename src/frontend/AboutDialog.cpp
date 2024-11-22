/*
    File                 : AboutDialog.cpp
    Project              : LabPlot
    Description          : Custom about dialog
    --------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AboutDialog.h"

/*!
	\class AboutDialog
	\brief Custom about dialog

	\ingroup kdefrontend
 */
AboutDialog::AboutDialog(const KAboutData& aboutData, QWidget* parent) : KAboutApplicationDialog(aboutData, parent) {

	// TODO: add library information (GSL version, etc.) in about dialog
	// TODO: custom stuff
}
