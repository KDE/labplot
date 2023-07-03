/*
	File                 : StatisticsSpreadsheetDock.cpp
	Project              : LabPlot
	Description          : widget for statistics spreadsheet properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "StatisticsSpreadsheetDock.h"

#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/spreadsheet/StatisticsSpreadsheet.h"
#include "kdefrontend/TemplateHandler.h"

/*!
 \class StatisticsSpreadsheetDock
 \brief Provides a widget for editing the properties of the spreadsheets currently selected in the project explorer.

 \ingroup kdefrontend
*/

StatisticsSpreadsheetDock::StatisticsSpreadsheetDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Spreadsheet);
	ui.verticalLayout->addWidget(templateHandler);
	templateHandler->show();
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &StatisticsSpreadsheetDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &StatisticsSpreadsheetDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &StatisticsSpreadsheetDock::info);

}

/*!
	set the current spreadsheet(s)
*/
void StatisticsSpreadsheetDock::setSpreadsheets(const QList<StatisticsSpreadsheet*> list) {

}

//*************************************************************
//* SLOTs for changes triggered in StatisticsSpreadsheetDock **
//*************************************************************

//*************************************************************
//******** SLOTs for changes triggered in Spreadsheet *********
//*************************************************************

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void StatisticsSpreadsheetDock::load() {
}

void StatisticsSpreadsheetDock::loadConfigFromTemplate(KConfig& config) {

}

/*!
	loads saved spreadsheet properties from \c config.
 */
void StatisticsSpreadsheetDock::loadConfig(KConfig& config) {

}

/*!
	saves spreadsheet properties to \c config.
 */
void StatisticsSpreadsheetDock::saveConfigAsTemplate(KConfig& config) {

}
