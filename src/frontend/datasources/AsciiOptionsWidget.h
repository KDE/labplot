/*
	File                 : AsciiOptionsWidget.h
	Project              : LabPlot
	Description          : widget providing options for the import of ascii data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2009-2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASCIIOPTIONSWIDGET_H
#define ASCIIOPTIONSWIDGET_H

#include "ui_asciioptionswidget.h"

class AsciiFilter;
class KConfig;

class AsciiOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit AsciiOptionsWidget(QWidget*);
	void showAsciiHeaderOptions(bool);
	void showTimestampOptions(bool);
	void applyFilterSettings(AsciiFilter*) const;
	void setSeparatingCharacter(QLatin1Char);

	void loadSettings() const;
	void saveSettings() const;

	// save/load template
	void loadConfigFromTemplate(KConfig&) const;
	void saveConfigAsTemplate(KConfig&) const;

public Q_SLOTS:
	void headerChanged(bool) const;

Q_SIGNALS:
	void headerLineChanged(int);

private:
	Ui::AsciiOptionsWidget ui;
	bool m_createTimeStampAvailable{false};
};

#endif
