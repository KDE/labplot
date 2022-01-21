/*
    File                 : CantorWorksheetDock.cpp
    Project              : LabPlot
    Description          : widget for CantorWorksheet properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
    SPDX-FileCopyrightText: 2015-2022 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CantorWorksheetDock.h"
#include "backend/cantorWorksheet/CantorWorksheet.h"

#include <3rdparty/cantor/panelplugin.h>

#include <QAction>

CantorWorksheetDock::CantorWorksheetDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
// 	ui.tabWidget->setMovable(true); //don't allow to move tabs until we properly keep track of the help panel's position
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
	Lock lock(m_initializing);
	m_cantorworksheetlist = list;
	m_worksheet = list.first();
	m_aspect = list.first();

	//remove the available panel plugins first
	int k = 0;
	int prev_index = ui.tabWidget->currentIndex();
	for (int i : index) {
		ui.tabWidget->removeTab(i-k);
		++k;
	}

	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	if (m_cantorworksheetlist.size() == 1) {
		//show name/comment
		ui.leName->setText(m_worksheet->name());
		ui.teComment->setText(m_worksheet->comment());

		//add available panel plugins
		const auto& plugins = m_cantorworksheetlist.first()->getPlugins();
		index.clear();
		for (auto* plugin : plugins) {
			//skip the "File Browser" plugin
			//in the new code of Cantor the plugin id is set as the object name and we can use it.
			//for the older version we need to rely on the translated name...
			//TODO: remove the dependency on the plugin name later.
			if (plugin->objectName() == QLatin1String("FileBrowserPanel") || plugin->name() == i18n("File Browser"))
				continue;


			connect(plugin, &Cantor::PanelPlugin::visibilityRequested, this, &CantorWorksheetDock::visibilityRequested);
			int i = ui.tabWidget->addTab(plugin->widget(), plugin->name());
			index.append(i);

			if (plugin->objectName() == QLatin1String("HelpPanel"))
				m_helpPanelIndex = i;
		}
	} else {
		//don't show any name/comment when multiple notebooks were selected
		ui.leName->setText(QString());
		ui.teComment->setText(QString());
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
}

//*************************************************************
//**** SLOTs for changes triggered in CantorWorksheetDock *****
//*************************************************************
// "General"-tab
void CantorWorksheetDock::evaluateWorksheet() {
	for (auto* nb : m_cantorworksheetlist)
		nb->evaluate();
}

void CantorWorksheetDock::restartBackend() {
	for (auto* nb : m_cantorworksheetlist)
		nb->restart();
}

/*!
 * this slot is called when the visibility for one of the panels in Cantor is requested.
 * At the moment this can only happen for the integrated help in Maxima, R, etc.
 * Here we hard-code the selection of the second tab being for the help.
 * TODO: improve this logic without hard-coding for a fixed index.
 */
void CantorWorksheetDock::visibilityRequested() {
	ui.tabWidget->setCurrentIndex(m_helpPanelIndex);
}
