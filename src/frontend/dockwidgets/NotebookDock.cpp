/*
	File                 : NotebookDock.cpp
	Project              : LabPlot
	Description          : widget for Notebook properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2015-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NotebookDock.h"
#include "backend/notebook/Notebook.h"

#include <cantor/panelplugin.h>

#include <QAction>

NotebookDock::NotebookDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	// 	ui.tabWidget->setMovable(true); //don't allow to move tabs until we properly keep track of the help panel's position
	setBaseWidgets(ui.leName, ui.teComment);

	// SLOTs
	// General
	connect(ui.bEvaluate, &QPushButton::pressed, this, &NotebookDock::evaluate);
	connect(ui.bRestart, &QPushButton::pressed, this, &NotebookDock::restartBackend);
}

void NotebookDock::setNotebooks(QList<Notebook*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_notebooks = list;
	m_notebook = list.first();
	setAspects(list);

	// remove the available panel plugins first
	int k = 0;
	int prev_index = ui.tabWidget->currentIndex();
	for (int i : index) {
		ui.tabWidget->removeTab(i - k);
		++k;
	}

	ui.leName->setStyleSheet(QString());
	ui.leName->setToolTip(QString());

	if (m_notebooks.size() == 1) {
		// show name/comment
		ui.leName->setText(m_notebook->name());
		ui.teComment->setText(m_notebook->comment());

		// add available panel plugins
		const auto& plugins = m_notebooks.first()->getPlugins();
		index.clear();
		for (auto* plugin : plugins) {
			// skip the "File Browser" plugin
			// in the new code of Cantor the plugin id is set as the object name and we can use it.
			// for the older version we need to rely on the translated name...
			// TODO: remove the dependency on the plugin name later.
			if (plugin->objectName() == QLatin1String("FileBrowserPanel") || plugin->name() == i18n("File Browser"))
				continue;

			connect(plugin, &Cantor::PanelPlugin::visibilityRequested, this, &NotebookDock::visibilityRequested);
			int i = ui.tabWidget->addTab(plugin->widget(), plugin->name());
			index.append(i);

			if (plugin->objectName() == QLatin1String("HelpPanel"))
				m_helpPanelIndex = i;
			else if (plugin->objectName() == QLatin1String("DocumentationPanel"))
				m_documentationPanelIndex = i;
		}
	} else {
		// don't show any name/comment when multiple notebooks were selected
		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}

	ui.tabWidget->setCurrentIndex(prev_index);

	if (m_notebook->part()) {
		ui.bEvaluate->show();
		ui.bRestart->show();
	} else {
		ui.bEvaluate->hide();
		ui.bRestart->hide();
	}
}

void NotebookDock::retranslateUi() {
}

//*************************************************************
//**** SLOTs for changes triggered in NotebookDock *****
//*************************************************************
// "General"-tab
void NotebookDock::evaluate() {
	for (auto* nb : m_notebooks)
		nb->evaluate();
}

void NotebookDock::restartBackend() {
	for (auto* nb : m_notebooks)
		nb->restart();
}

/*!
 * this slot is called when the visibility for one of the panels in Cantor is requested.
 * At the moment this can only happen for the integrated help in Maxima and in R and
 * for the integrated documentation.
 */
void NotebookDock::visibilityRequested() {
	const auto& name = QObject::sender()->objectName();
	if (name == QLatin1String("HelpPanel"))
		ui.tabWidget->setCurrentIndex(m_helpPanelIndex);
	else if (name == QLatin1String("DocumentationPanel"))
		ui.tabWidget->setCurrentIndex(m_documentationPanelIndex);
}
