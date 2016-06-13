/***************************************************************************
    File                 : SpreadsheetDock.cpp
    Project              : LabPlot
    Description          : widget for spreadsheet properties
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2015 by Alexander Semke (alexander.semke@web.de)
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
#include "backend/spreadsheet/Spreadsheet.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/ThemeHandler.h"
#include <QDir>

 /*!
  \class SpreadsheetDock
  \brief Provides a widget for editing the properties of the spreadsheets currently selected in the project explorer.

  \ingroup kdefrontend
*/

SpreadsheetDock::SpreadsheetDock(QWidget* parent): QWidget(parent), m_spreadsheet(0), m_initializing(false) {
	ui.setupUi(this);

	connect(ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()));
	connect(ui.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()));
	connect(ui.sbColumnCount, SIGNAL(valueChanged(int)), this, SLOT(columnCountChanged(int)));
	connect(ui.sbRowCount, SIGNAL(valueChanged(int)), this, SLOT(rowCountChanged(int)));
	connect(ui.cbShowComments, SIGNAL(stateChanged(int)), this, SLOT(commentsShownChanged(int)));

	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::Spreadsheet);
	ui.gridLayout->addWidget(templateHandler, 11, 0, 1, 4);
	templateHandler->show();
	connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfigAsTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));

    ThemeHandler* themeHandler = new ThemeHandler(this, ThemeHandler::Spreadsheet);
    ui.gridLayout->addWidget(themeHandler);
}

/*!

*/
void SpreadsheetDock::setSpreadsheets(QList<Spreadsheet*> list){
	m_initializing = true;
	m_spreadsheetList = list;
	m_spreadsheet = list.first();

	if (list.size()==1){
		ui.leName->setEnabled(true);
		ui.leComment->setEnabled(true);

		ui.leName->setText(m_spreadsheet->name());
		ui.leComment->setText(m_spreadsheet->comment());
	}else{
		//disable the fields "Name" and "Comment" if there are more then one spreadsheet
		ui.leName->setEnabled(false);
		ui.leComment->setEnabled(false);

		ui.leName->setText("");
		ui.leComment->setText("");
  	}

	//show the properties of the first Spreadsheet in the list
	this->load();

	// undo functions
	connect(m_spreadsheet, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),
			this, SLOT(spreadsheetDescriptionChanged(const AbstractAspect*)));
	connect(m_spreadsheet, SIGNAL(rowCountChanged(int)),this, SLOT(spreadsheetRowCountChanged(int)));
	connect(m_spreadsheet, SIGNAL(columnCountChanged(int)),this, SLOT(spreadsheetColumnCountChanged(int)));
	//TODO: show comments

	m_initializing = false;
}

//*************************************************************
//****** SLOTs for changes triggered in SpreadsheetDock *******
//*************************************************************
void SpreadsheetDock::nameChanged(){
	if (m_initializing)
		return;

	m_spreadsheet->setName(ui.leName->text());
}

void SpreadsheetDock::commentChanged(){
	if (m_initializing)
		return;

	m_spreadsheet->setComment(ui.leComment->text());
}

void SpreadsheetDock::rowCountChanged(int rows){
	if (m_initializing)
		return;

	foreach(Spreadsheet* spreadsheet, m_spreadsheetList)
		spreadsheet->setRowCount(rows);
}

void SpreadsheetDock::columnCountChanged(int columns){
	if (m_initializing)
		return;

	foreach(Spreadsheet* spreadsheet, m_spreadsheetList)
		spreadsheet->setColumnCount(columns);
}

/*!
  switches on/off  the comment header in the views of the selected spreadsheets.
*/
void SpreadsheetDock::commentsShownChanged(int state){
	Spreadsheet* spreadsheet;
	foreach(spreadsheet, m_spreadsheetList)
		qobject_cast<SpreadsheetView*>(spreadsheet->view())->showComments(state);
}

//*************************************************************
//******** SLOTs for changes triggered in Spreadsheet *********
//*************************************************************
void SpreadsheetDock::spreadsheetDescriptionChanged(const AbstractAspect* aspect) {
	if (m_spreadsheet != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text()) {
		ui.leName->setText(aspect->name());
	} else if (aspect->comment() != ui.leComment->text()) {
		ui.leComment->setText(aspect->comment());
	}
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

	SpreadsheetView* view= qobject_cast<SpreadsheetView*>(m_spreadsheet->view());
	ui.cbShowComments->setChecked(view->areCommentsShown());
}

void SpreadsheetDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QDir::separator());
	if (index!=-1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_spreadsheetList.size();
	if (size>1)
		m_spreadsheet->beginMacro(i18n("%1 spreadsheets: template \"%2\" loaded", size, name));
	else
		m_spreadsheet->beginMacro(i18n("%1: template \"%2\" loaded", m_spreadsheet->name(), name));

	this->loadConfig(config);

	m_spreadsheet->endMacro();
}

/*!
	loads saved spreadsheet properties from \c config.
 */
void SpreadsheetDock::loadConfig(KConfig& config){
	KConfigGroup group = config.group( "Spreadsheet" );

	ui.sbColumnCount->setValue(group.readEntry("ColumnCount", m_spreadsheet->columnCount()));
	ui.sbRowCount->setValue(group.readEntry("RowCount", m_spreadsheet->rowCount()));

	SpreadsheetView* view= qobject_cast<SpreadsheetView*>(m_spreadsheet->view());
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
