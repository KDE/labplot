/*
	File                 : SpreadsheetDock.cpp
	Project              : LabPlot
	Description          : widget for spreadsheet properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2013 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetDock.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/TemplateHandler.h"

#include <KConfig>
#include <KConfigGroup>

/*!
 \class SpreadsheetDock
 \brief Provides a widget for editing the properties of the spreadsheets currently selected in the project explorer.

 \ingroup frontend
*/

SpreadsheetDock::SpreadsheetDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	retranslateUi();

	connect(ui.sbColumnCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &SpreadsheetDock::columnCountChanged);
	connect(ui.sbRowCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &SpreadsheetDock::rowCountChanged);
	connect(ui.cbShowComments, &QCheckBox::toggled, this, &SpreadsheetDock::commentsShownChanged);
	connect(ui.cbShowSparklines, &QCheckBox::toggled, this, &SpreadsheetDock::sparklinesShownChanged);

	connect(ui.cbLinkingEnabled, &QCheckBox::toggled, this, &SpreadsheetDock::linkingChanged);
	connect(ui.cbLinkedSpreadsheet, &TreeViewComboBox::currentModelIndexChanged, this, &SpreadsheetDock::linkedSpreadsheetChanged);

	m_templateHandler = new TemplateHandler(this, QLatin1String("Spreadsheet"));
	ui.gridLayout->addWidget(m_templateHandler, 17, 0, 1, 4);
	connect(m_templateHandler, &TemplateHandler::loadConfigRequested, this, &SpreadsheetDock::loadConfigFromTemplate);
	connect(m_templateHandler, &TemplateHandler::saveConfigRequested, this, &SpreadsheetDock::saveConfigAsTemplate);
	connect(m_templateHandler, &TemplateHandler::info, this, &SpreadsheetDock::info);
}

void SpreadsheetDock::retranslateUi() {
	// tooltip texts
	QString info = i18n("Enable linking to synchronize the number of rows with another spreadsheet");
	ui.lLinkingEnabled->setToolTip(info);
	ui.cbLinkingEnabled->setToolTip(info);

	info = i18n("Spreadsheet to synchronize the number of rows with");
	ui.lLinkedSpreadsheet->setToolTip(info);
	ui.cbLinkedSpreadsheet->setToolTip(info);
}

/*!
	set the current spreadsheet(s)
*/
void SpreadsheetDock::setSpreadsheets(const QList<Spreadsheet*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_spreadsheetList = list;
	m_spreadsheet = list.first();
	setAspects(list);

	// check if we have read-only spreadsheets
	bool readOnly = false;
	for (auto* s : m_spreadsheetList) {
		if (s->readOnly()) {
			readOnly = true;
			break;
		}
	}

	ui.lDimensions->setVisible(!readOnly);
	ui.lRowCount->setVisible(!readOnly);
	ui.sbRowCount->setVisible(!readOnly);
	ui.lColumnCount->setVisible(!readOnly);
	ui.sbColumnCount->setVisible(!readOnly);
	ui.lFormat->setVisible(!readOnly);
	ui.lShowComments->setVisible(!readOnly);
	ui.cbShowComments->setVisible(!readOnly);
	ui.lShowSparklines->setVisible(!readOnly);
	ui.cbShowSparklines->setVisible(!readOnly);
	ui.lLinking->setVisible(!readOnly);
	ui.lLinkingEnabled->setVisible(!readOnly);
	ui.cbLinkingEnabled->setVisible(!readOnly);
	ui.lLinkedSpreadsheet->setVisible(!readOnly);
	ui.cbLinkedSpreadsheet->setVisible(!readOnly);
	m_templateHandler->setVisible(!readOnly);

	if (readOnly)
		return;

	auto* model = aspectModel();
	model->setSelectableAspects({AspectType::Spreadsheet});
	model->enableNumericColumnsOnly(true);
	// model->enableNonEmptyNumericColumnsOnly(true);

	ui.cbLinkedSpreadsheet->setTopLevelClasses({AspectType::Folder, AspectType::Workbook, AspectType::Spreadsheet});
	ui.cbLinkedSpreadsheet->setModel(model);

	// don't allow to select self spreadsheet!
	QList<const AbstractAspect*> aspects;
	for (auto* sh : m_spreadsheetList)
		aspects << sh;
	ui.cbLinkedSpreadsheet->setHiddenAspects(aspects);

	// show the properties of the first Spreadsheet in the list
	this->load();

	// undo functions
	connect(m_spreadsheet, &Spreadsheet::rowCountChanged, this, &SpreadsheetDock::spreadsheetRowCountChanged);
	connect(m_spreadsheet, &Spreadsheet::columnCountChanged, this, &SpreadsheetDock::spreadsheetColumnCountChanged);
	connect(m_spreadsheet, &Spreadsheet::showCommentsChanged, this, &SpreadsheetDock::spreadsheetShowCommentsChanged);
	connect(m_spreadsheet, &Spreadsheet::showSparklinesChanged, this, &SpreadsheetDock::spreadsheetShowSparklinesChanged);
	connect(m_spreadsheet, &Spreadsheet::linkingChanged, this, &SpreadsheetDock::spreadsheetLinkingChanged);
	connect(m_spreadsheet, &Spreadsheet::linkedSpreadsheetChanged, this, &SpreadsheetDock::spreadsheetLinkedSpreadsheetChanged);
}

//*************************************************************
//****** SLOTs for changes triggered in SpreadsheetDock *******
//*************************************************************
void SpreadsheetDock::rowCountChanged(int rows) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* spreadsheet : m_spreadsheetList)
		spreadsheet->setRowCount(rows);
}

void SpreadsheetDock::columnCountChanged(int columns) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* spreadsheet : m_spreadsheetList)
		spreadsheet->setColumnCount(columns);
}

/*!
  enable/disable the comment header in the views of the selected spreadsheets.
*/
void SpreadsheetDock::commentsShownChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* spreadsheet : m_spreadsheetList)
		spreadsheet->setShowComments(state);
}
/*!
  enable/disable the sparkline header in the views of the selected spreadsheets.
*/
void SpreadsheetDock::sparklinesShownChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* spreadsheet : m_spreadsheetList)
		spreadsheet->setShowSparklines(state);
}

void SpreadsheetDock::linkingChanged(bool linking) {
	ui.sbRowCount->setEnabled(!linking);
	ui.lLinkedSpreadsheet->setVisible(linking);
	ui.cbLinkedSpreadsheet->setVisible(linking);

	CONDITIONAL_LOCK_RETURN;

	for (auto* spreadsheet : m_spreadsheetList)
		spreadsheet->setLinking(linking);
}

void SpreadsheetDock::linkedSpreadsheetChanged(const QModelIndex& index) {
	// combobox was potentially red-highlighted because of a missing column
	// remove the highlighting when we have a valid selection now
	auto* aspect{static_cast<AbstractAspect*>(index.internalPointer())};
	if (aspect) {
		auto* cb{dynamic_cast<TreeViewComboBox*>(QObject::sender())};
		if (cb)
			cb->setStyleSheet(QString());
		auto* sh = dynamic_cast<Spreadsheet*>(aspect);
		if (sh) {
			for (auto* spreadsheet : m_spreadsheetList)
				spreadsheet->setLinkedSpreadsheet(sh);
		}
	}
}

//*************************************************************
//******** SLOTs for changes triggered in Spreadsheet *********
//*************************************************************
void SpreadsheetDock::spreadsheetRowCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRowCount->setValue(count);
}

void SpreadsheetDock::spreadsheetColumnCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbColumnCount->setValue(count);
}

void SpreadsheetDock::spreadsheetShowCommentsChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbShowComments->setChecked(checked);
}

void SpreadsheetDock::spreadsheetShowSparklinesChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbShowSparklines->setChecked(checked);
}

void SpreadsheetDock::spreadsheetLinkingChanged(bool linking) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbLinkingEnabled->setChecked(linking);
}

void SpreadsheetDock::spreadsheetLinkedSpreadsheetChanged(const Spreadsheet* spreadsheet) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbLinkedSpreadsheet->setAspect(spreadsheet);
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void SpreadsheetDock::load() {
	ui.sbColumnCount->setValue(m_spreadsheet->columnCount());
	ui.sbRowCount->setValue(m_spreadsheet->rowCount());
	ui.cbShowComments->setChecked(m_spreadsheet->showComments());
	ui.cbShowSparklines->setChecked(m_spreadsheet->showSparklines());
	ui.cbLinkedSpreadsheet->setAspect(m_spreadsheet->linkedSpreadsheet());
	ui.cbLinkingEnabled->setChecked(m_spreadsheet->linking());
	linkingChanged(m_spreadsheet->linking()); // call this to update the widgets
}

void SpreadsheetDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	const int size = m_spreadsheetList.size();
	if (size > 1)
		m_spreadsheet->beginMacro(i18n("%1 spreadsheets: template \"%2\" loaded", size, name));
	else
		m_spreadsheet->beginMacro(i18n("%1: template \"%2\" loaded", m_spreadsheet->name(), name));

	this->loadConfig(config);

	m_spreadsheet->endMacro();
}

/*!
	loads saved spreadsheet properties from \c config.
 */
void SpreadsheetDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("Spreadsheet"));
	ui.sbColumnCount->setValue(group.readEntry(QStringLiteral("ColumnCount"), m_spreadsheet->columnCount()));
	ui.sbRowCount->setValue(group.readEntry(QStringLiteral("RowCount"), m_spreadsheet->rowCount()));
	ui.cbShowComments->setChecked(group.readEntry(QStringLiteral("ShowComments"), m_spreadsheet->showComments()));
	ui.cbShowSparklines->setChecked(group.readEntry(QStringLiteral("ShowSparklines"), m_spreadsheet->showSparklines()));
}

/*!
	saves spreadsheet properties to \c config.
 */
void SpreadsheetDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("Spreadsheet"));
	group.writeEntry(QStringLiteral("ColumnCount"), ui.sbColumnCount->value());
	group.writeEntry(QStringLiteral("RowCount"), ui.sbRowCount->value());
	group.writeEntry(QStringLiteral("ShowComments"), ui.cbShowComments->isChecked());
	group.writeEntry(QStringLiteral("ShowSparklines"), ui.cbShowSparklines->isChecked());

	config.sync();
}
