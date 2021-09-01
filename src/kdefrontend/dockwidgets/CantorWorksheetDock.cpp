/*
    File                 : CantorWorksheetDock.cpp
    Project              : LabPlot
    Description          : widget for CantorWorksheet properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
    SPDX-FileCopyrightText: 2015-2018 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CantorWorksheetDock.h"
#include "backend/cantorWorksheet/CantorWorksheet.h"
#include <KParts/ReadWritePart>
#include <QAction>

CantorWorksheetDock::CantorWorksheetDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	ui.tabWidget->setMovable(true);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	//SLOTs
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &CantorWorksheetDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &CantorWorksheetDock::commentChanged);
	connect(ui.bEvaluate, &QPushButton::pressed, this, &CantorWorksheetDock::evaluateWorksheet);
	connect(ui.bRestart, &QPushButton::pressed, this, &CantorWorksheetDock::restartBackend);
}

void CantorWorksheetDock::setCantorWorksheets(QList<CantorWorksheet*> list) {
	m_initializing = true;
	m_cantorworksheetlist = list;
	m_worksheet = list.first();
	m_aspect = list.first();

	//show name/comment
	ui.leName->setText(m_worksheet->name());
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");
	ui.teComment->setText(m_worksheet->comment());

	//show all available plugins
	int k = 0;
	int prev_index = ui.tabWidget->currentIndex();
	for (int i : index) {
		ui.tabWidget->removeTab(i-k);
		++k;
	}

	if (m_cantorworksheetlist.size() == 1) {
		QList<Cantor::PanelPlugin*> plugins = m_cantorworksheetlist.first()->getPlugins();
		index.clear();
		for (auto* plugin : plugins) {
			if (plugin->name() == QLatin1String("File Browser"))
				continue;
			connect(plugin, &Cantor::PanelPlugin::visibilityRequested, this, &CantorWorksheetDock::visibilityRequested);
			plugin->setParentWidget(this);
			int i = ui.tabWidget->addTab(plugin->widget(), plugin->name());
			index.append(i);
		}
	}
	ui.tabWidget->setCurrentIndex(prev_index);

	if (m_worksheet->part()) {
		ui.bEvaluate->show();
		ui.bRestart->show();
	} else {
		ui.bEvaluate->hide();
		ui.bRestart->hide();
	}

	//SIGNALs/SLOTs
	connect(m_worksheet, &AbstractAspect::aspectDescriptionChanged, this, &CantorWorksheetDock::aspectDescriptionChanged);
	m_initializing = false;
}

//*************************************************************
//**** SLOTs for changes triggered in CantorWorksheetDock *****
//*************************************************************
// "General"-tab
void CantorWorksheetDock::evaluateWorksheet() {
	m_worksheet->part()->action("evaluate_worksheet")->trigger();
}

void CantorWorksheetDock::restartBackend() {
	m_worksheet->part()->action("restart_backend")->trigger();
}

/*!
 * this slot is called when the visibility for one of the panels in Cantor is requested.
 * At the moment this can only happen for the integrated help in Maxima, R, etc.
 * Here we hard-code the selection of the second tab being for the help.
 * TODO: improve this logic without hard-coding for a fixed index.
 */
void CantorWorksheetDock::visibilityRequested() {
	ui.tabWidget->setCurrentIndex(1);
}
