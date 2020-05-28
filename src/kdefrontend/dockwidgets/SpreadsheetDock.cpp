/***************************************************************************
    File                 : SpreadsheetDock.cpp
    Project              : LabPlot
    Description          : widget for spreadsheet properties
	--------------------------------------------------------------------
	Copyright            : (C) 2010-2019 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2013 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "SpreadsheetDock.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "kdefrontend/TemplateHandler.h"

#include <QDir>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KConfig>

 /*!
  \class SpreadsheetDock
  \brief Provides a widget for editing the properties of the spreadsheets currently selected in the project explorer.

  \ingroup kdefrontend
*/

SpreadsheetDock::SpreadsheetDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	//leComment = ui.teComment; // is not a lineedit

	connect(ui.leName, &QLineEdit::textChanged, this, &SpreadsheetDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &SpreadsheetDock::commentChanged);
	connect(ui.sbColumnCount, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SpreadsheetDock::columnCountChanged);
	connect(ui.sbRowCount, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SpreadsheetDock::rowCountChanged);
	connect(ui.cbShowComments, &QCheckBox::stateChanged, this, &SpreadsheetDock::commentsShownChanged);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::Spreadsheet);
	ui.gridLayout->addWidget(templateHandler, 11, 0, 1, 4);
	templateHandler->show();
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &SpreadsheetDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &SpreadsheetDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &SpreadsheetDock::info);
}

/*!
	set the current spreadsheet(s)
*/
void SpreadsheetDock::setSpreadsheets(QList<Spreadsheet*> list) {
	m_initializing = true;
	m_spreadsheetList = list;
	m_spreadsheet = list.first();
	m_aspect = list.first();


	//check whether we have non-editable columns:
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
		//disable the fields "Name" and "Comment" if there are more then one spreadsheet
		ui.leName->setEnabled(false);
		ui.teComment->setEnabled(false);

		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	//show the properties of the first Spreadsheet in the list
	this->load();

	// undo functions
	connect(m_spreadsheet, &AbstractAspect::aspectDescriptionChanged, this, &SpreadsheetDock::spreadsheetDescriptionChanged);
	connect(m_spreadsheet, &Spreadsheet::rowCountChanged, this, &SpreadsheetDock::spreadsheetRowCountChanged);
	connect(m_spreadsheet, &Spreadsheet::columnCountChanged, this, &SpreadsheetDock::spreadsheetColumnCountChanged);
	//TODO: show comments

	ui.lDimensions->setVisible(!nonEditable);
	ui.lRowCount->setVisible(!nonEditable);
	ui.sbRowCount->setVisible(!nonEditable);
	ui.lColumnCount->setVisible(!nonEditable);
	ui.sbColumnCount->setVisible(!nonEditable);
	ui.lFormat->setVisible(!nonEditable);
	ui.lShowComments->setVisible(!nonEditable);
	ui.cbShowComments->setVisible(!nonEditable);

	m_initializing = false;
}

//*************************************************************
//****** SLOTs for changes triggered in SpreadsheetDock *******
//*************************************************************
void SpreadsheetDock::commentChanged() {
	if (m_initializing)
		return;

	m_spreadsheet->setComment(ui.teComment->document()->toPlainText());
}

void SpreadsheetDock::rowCountChanged(int rows) {
	if (m_initializing)
		return;

	for (auto* spreadsheet : m_spreadsheetList)
		spreadsheet->setRowCount(rows);
}

void SpreadsheetDock::columnCountChanged(int columns) {
	if (m_initializing)
		return;

	for (auto* spreadsheet : m_spreadsheetList)
		spreadsheet->setColumnCount(columns);
}

/*!
  switches on/off  the comment header in the views of the selected spreadsheets.
*/
void SpreadsheetDock::commentsShownChanged(int state) {
	if (m_initializing)
		return;

	for (auto* spreadsheet : m_spreadsheetList)
		static_cast<SpreadsheetView*>(spreadsheet->view())->showComments(state);
}

//*************************************************************
//******** SLOTs for changes triggered in Spreadsheet *********
//*************************************************************
void SpreadsheetDock::spreadsheetDescriptionChanged(const AbstractAspect* aspect) {
	if (m_spreadsheet != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.teComment->toPlainText())
		ui.teComment->document()->setPlainText(aspect->comment());

	m_initializing = false;
}

void SpreadsheetDock::spreadsheetRowCountChanged(int count) {
	m_initializing = true;
  	ui.sbRowCount->setValue(count);
	m_initializing = false;
}

void SpreadsheetDock::spreadsheetColumnCountChanged(int count) {
	m_initializing = true;
  	ui.sbColumnCount->setValue(count);
	m_initializing = false;
}

void SpreadsheetDock::spreadsheetShowCommentsChanged(int checked) {
	m_initializing = true;
	ui.cbShowComments->setChecked(checked);
	m_initializing = false;
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void SpreadsheetDock::load() {
	ui.sbColumnCount->setValue(m_spreadsheet->columnCount());
	ui.sbRowCount->setValue(m_spreadsheet->rowCount());

	auto* view = static_cast<SpreadsheetView*>(m_spreadsheet->view());
	ui.cbShowComments->setChecked(view->areCommentsShown());
}

void SpreadsheetDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	const int index = config.name().lastIndexOf(QLatin1String("/"));
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
	KConfigGroup group = config.group( "Spreadsheet" );

	ui.sbColumnCount->setValue(group.readEntry("ColumnCount", m_spreadsheet->columnCount()));
	ui.sbRowCount->setValue(group.readEntry("RowCount", m_spreadsheet->rowCount()));

	auto* view = static_cast<SpreadsheetView*>(m_spreadsheet->view());
	ui.cbShowComments->setChecked(group.readEntry("ShowComments", view->areCommentsShown()));
}

/*!
	saves spreadsheet properties to \c config.
 */
void SpreadsheetDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group( "Spreadsheet" );
	group.writeEntry("ColumnCount", ui.sbColumnCount->value());
	group.writeEntry("RowCount", ui.sbRowCount->value());
	group.writeEntry("ShowComments",ui.cbShowComments->isChecked());
	config.sync();
}
