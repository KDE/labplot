/***************************************************************************
    File                 : CantorWorksheetDock.cpp
    Project              : LabPlot
    Description          : widget for CantorWorksheet properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Garvit Khatri (garvitdelhi@gmail.com)
    Copyright            : (C) 2015-2018 Alexander Semke (alexander.semke@web.de)

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

#include "CantorWorksheetDock.h"
#include "backend/cantorWorksheet/CantorWorksheet.h"
#include <KParts/ReadWritePart>

CantorWorksheetDock::CantorWorksheetDock(QWidget* parent): QWidget(parent), m_worksheet(nullptr), m_initializing(false) {
	ui.setupUi(this);
	ui.tabWidget->setMovable(true);

	//SLOTs
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &CantorWorksheetDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &CantorWorksheetDock::commentChanged);
	connect( ui.evaluate_worksheet, SIGNAL(pressed()), this, SLOT(evaluateWorksheet()) );
	connect( ui.restart_backend, SIGNAL(pressed()), this, SLOT(restartBackend()) );
}

void CantorWorksheetDock::setCantorWorksheets(QList<CantorWorksheet*> list) {
	m_initializing = true;
	m_cantorworksheetlist = list;
	m_worksheet = list.first();

	//show name/comment
	ui.leName->setText(m_worksheet->name());
	ui.leComment->setText(m_worksheet->comment());

	//show all available plugins
	int k = 0;
	int prev_index = ui.tabWidget->currentIndex();
	for (int i : index) {
		ui.tabWidget->removeTab(i-k);
		++k;
	}

	if (m_cantorworksheetlist.size()==1) {
		QList<Cantor::PanelPlugin*> plugins = m_cantorworksheetlist.first()->getPlugins();
		index.clear();
		for (auto* plugin : plugins) {
			plugin->setParentWidget(this);
			int i = ui.tabWidget->addTab(plugin->widget(), plugin->name());
			index.append(i);
		}
	}
	ui.tabWidget->setCurrentIndex(prev_index);

	if (m_worksheet->part()) {
		ui.evaluate_worksheet->show();
		ui.restart_backend->show();
	} else {
		ui.evaluate_worksheet->hide();
		ui.restart_backend->hide();
	}

	//SIGNALs/SLOTs
	connect(m_worksheet, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),this, SLOT(worksheetDescriptionChanged(const AbstractAspect*)));
	m_initializing = false;
}

//*************************************************************
//**** SLOTs for changes triggered in CantorWorksheetDock *****
//*************************************************************
// "General"-tab
void CantorWorksheetDock::nameChanged(){
	if (m_initializing)
		return;

	m_worksheet->setName(ui.leName->text());
}

void CantorWorksheetDock::commentChanged(){
	if (m_initializing)
		return;

	m_worksheet->setComment(ui.leComment->text());
}

void CantorWorksheetDock::evaluateWorksheet() {
	m_worksheet->part()->action("evaluate_worksheet")->trigger();
}

void CantorWorksheetDock::restartBackend() {
	m_worksheet->part()->action("restart_backend")->trigger();
}

//*************************************************************
//******** SLOTs for changes triggered in CantorWorksheet ***********
//*************************************************************
void CantorWorksheetDock::worksheetDescriptionChanged(const AbstractAspect* aspect) {
	if (m_worksheet != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());
	m_initializing = false;
}
