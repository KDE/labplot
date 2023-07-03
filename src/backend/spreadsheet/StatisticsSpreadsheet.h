/*
	File                 : StatisticsSpreadsheet.h
	Project              : LabPlot
	Description          : Aspect providing a spreadsheet with the columns statistics for the parent spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STATISTICSSPREADSHEET_H
#define STATISTICSSPREADSHEET_H

#include "backend/spreadsheet/Spreadsheet.h"

class StatisticsSpreadsheet : public Spreadsheet {
	Q_OBJECT

public:
	explicit StatisticsSpreadsheet(Spreadsheet*, bool loading = false, AspectType type = AspectType::StatisticsSpreadsheet);
	~StatisticsSpreadsheet() override;

	QIcon icon() const override;
	// QMenu* createContextMenu() override;
	// void fillColumnContextMenu(QMenu*, Column*);
	// QWidget* view() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

private:
	void init();
	void updateStatisticsSpreadsheet();

	Spreadsheet* m_spreadsheet{nullptr};
};

#endif
