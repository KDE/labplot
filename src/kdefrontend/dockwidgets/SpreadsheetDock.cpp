/*
	File                 : SpreadsheetDock.cpp
	Project              : LabPlot
	Description          : widget for spreadsheet properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2013 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "kdefrontend/TemplateHandler.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QDir>

/*!
 \class SpreadsheetDock
 \brief Provides a widget for editing the properties of the spreadsheets currently selected in the project explorer.

 \ingroup kdefrontend
*/

SpreadsheetDock::SpreadsheetDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	ui.teComment->setFixedHeight(1.2 * ui.leName->height());

	connect(ui.leName, &QLineEdit::textChanged, this, &SpreadsheetDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &SpreadsheetDock::commentChanged);
	connect(ui.sbColumnCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &SpreadsheetDock::columnCountChanged);
	connect(ui.sbRowCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &SpreadsheetDock::rowCountChanged);
	connect(ui.cbShowComments, &QCheckBox::toggled, this, &SpreadsheetDock::commentsShownChanged);
	connect(ui.cbLinkingEnabled, &QCheckBox::toggled, this, &SpreadsheetDock::linkingChanged);
	connect(ui.cbLinkedSpreadsheet, &TreeViewComboBox::currentModelIndexChanged, this, &SpreadsheetDock::linkedSpreadsheetChanged);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Spreadsheet);
	ui.gridLayout->addWidget(templateHandler, 16, 0, 1, 4);
	templateHandler->show();
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &SpreadsheetDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &SpreadsheetDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &SpreadsheetDock::info);

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

	// check whether we have non-editable columns:
	bool nonEditable = false;
	for (auto* s : m_spreadsheetList) {
		if (dynamic_cast<DatapickerCurve*>(s->parentAspect())) {
			nonEditable = true;
			break;
		}
	}

	if (list.size() == 1) {
		ui.leName->setEnabled(true);
		ui.teComment->setEnabled(true);

		ui.leName->setText(m_spreadsheet->name());
		ui.teComment->setText(m_spreadsheet->comment());
	} else {
		// disable the fields "Name" and "Comment" if there are more than one spreadsheet
		ui.leName->setEnabled(false);
		ui.teComment->setEnabled(false);

		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}
	ui.leName->setStyleSheet(QString());
	ui.leName->setToolTip(QString());

	const QList<AspectType> topLevelClasses = {AspectType::Spreadsheet};
// needed for buggy compiler
#if __cplusplus < 201103L
	m_aspectTreeModel = std::auto_ptr<AspectTreeModel>(new AspectTreeModel(m_spreadsheet->project()));
#else
	m_aspectTreeModel = std::unique_ptr<AspectTreeModel>(new AspectTreeModel(m_spreadsheet->project()));
#endif
	m_aspectTreeModel->setSelectableAspects(topLevelClasses);
	m_aspectTreeModel->enableNumericColumnsOnly(true);
	// m_aspectTreeModel->enableNonEmptyNumericColumnsOnly(true);

	ui.cbLinkedSpreadsheet->setTopLevelClasses(topLevelClasses);
	ui.cbLinkedSpreadsheet->setModel(m_aspectTreeModel.get());

	// don't allow to select self spreadsheet!
	QList<const AbstractAspect*> aspects;
	for (auto* sh : m_spreadsheetList)
		aspects << sh;
	ui.cbLinkedSpreadsheet->setHiddenAspects(aspects);

	// show the properties of the first Spreadsheet in the list
	this->load();

	// undo functions
	connect(m_spreadsheet, &AbstractAspect::aspectDescriptionChanged, this, &SpreadsheetDock::aspectDescriptionChanged);
	connect(m_spreadsheet, &Spreadsheet::rowCountChanged, this, &SpreadsheetDock::spreadsheetRowCountChanged);
	connect(m_spreadsheet, &Spreadsheet::columnCountChanged, this, &SpreadsheetDock::spreadsheetColumnCountChanged);
	connect(m_spreadsheet, &Spreadsheet::linkingChanged, this, &SpreadsheetDock::spreadsheetLinkingChanged);
	connect(m_spreadsheet, &Spreadsheet::linkedSpreadsheetChanged, this, &SpreadsheetDock::spreadsheetLinkedSpreadsheetChanged);
	// TODO: show comments

	ui.lDimensions->setVisible(!nonEditable);
	ui.lRowCount->setVisible(!nonEditable);
	ui.sbRowCount->setVisible(!nonEditable);
	ui.lColumnCount->setVisible(!nonEditable);
	ui.sbColumnCount->setVisible(!nonEditable);
	ui.lFormat->setVisible(!nonEditable);
	ui.lShowComments->setVisible(!nonEditable);
	ui.cbShowComments->setVisible(!nonEditable);
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
		static_cast<SpreadsheetView*>(spreadsheet->view())->showComments(state);
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

void SpreadsheetDock::spreadsheetLinkingChanged(bool linking) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbLinkingEnabled->setChecked(linking);
}

void SpreadsheetDock::spreadsheetLinkedSpreadsheetChanged(const Spreadsheet* spreadsheet) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbLinkedSpreadsheet->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(spreadsheet));
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void SpreadsheetDock::load() {
	ui.sbColumnCount->setValue(m_spreadsheet->columnCount());
	ui.sbRowCount->setValue(m_spreadsheet->rowCount());

	auto* view = static_cast<SpreadsheetView*>(m_spreadsheet->view());
	ui.cbShowComments->setChecked(view->areCommentsShown());

	ui.cbLinkedSpreadsheet->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(m_spreadsheet->linkedSpreadsheet()));
	ui.cbLinkingEnabled->setChecked(m_spreadsheet->linking());
	linkingChanged(m_spreadsheet->linking()); // call this to update the widgets
}

void SpreadsheetDock::loadConfigFromTemplate(KConfig& config) {
	// extract the name of the template from the file name
	QString name;
	const int index = config.name().lastIndexOf(QLatin1Char('/'));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

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
	KConfigGroup group = config.group("Spreadsheet");

	ui.sbColumnCount->setValue(group.readEntry("ColumnCount", m_spreadsheet->columnCount()));
	ui.sbRowCount->setValue(group.readEntry("RowCount", m_spreadsheet->rowCount()));

	auto* view = static_cast<SpreadsheetView*>(m_spreadsheet->view());
	ui.cbShowComments->setChecked(group.readEntry("ShowComments", view->areCommentsShown()));
}

/*!
	saves spreadsheet properties to \c config.
 */
void SpreadsheetDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("Spreadsheet");
	group.writeEntry("ColumnCount", ui.sbColumnCount->value());
	group.writeEntry("RowCount", ui.sbRowCount->value());
	group.writeEntry("ShowComments", ui.cbShowComments->isChecked());
	config.sync();
}
