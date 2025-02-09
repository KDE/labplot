/*
	File                 : AsciiOptionsWidget.h
	Project              : LabPlot
	Description          : widget providing options for the import of ascii data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2009-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASCIIOPTIONSWIDGET_H
#define ASCIIOPTIONSWIDGET_H

#include "backend/datasources/filters/AsciiFilter.h"
#include "ui_asciioptionswidget.h"

class KConfig;

class AsciiOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit AsciiOptionsWidget(QWidget*, bool liveData = false);
	void showAsciiHeaderOptions(bool headerLinevisible, bool columnNamesVisible);
	void showTimestampOptions(bool);
	void applyFilterSettings(AsciiFilter::Properties& properties) const;
	void updateWidgets(const AsciiFilter::Properties& properties);
	void setSeparatingCharacter(QLatin1Char);

	bool isValid(QString& errorMessage);

	void loadSettings() const;
	void saveSettings() const;

	// save/load template
	void loadConfigFromTemplate(KConfig&) const;
	void saveConfigAsTemplate(KConfig&) const;

public Q_SLOTS:
	void headerChanged(bool) const;

Q_SIGNALS:
	void headerLineChanged(int);
	void columnModesChanged(const QString& s);

private:
	Ui::AsciiOptionsWidget ui;
	bool m_createTimeStampAvailable{false};
	bool m_liveData{false};
};

#endif
