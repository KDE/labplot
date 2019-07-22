/***************************************************************************
File                 : CursorDock.cpp
Project              : LabPlot
Description 	     : This dock represents the data from the cursors in the cartesian plots
--------------------------------------------------------------------
Copyright            : (C) 2019 Martin Marmsoler (martin.marmsoler@gmail.com)

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

#include "CursorDock.h"
#include "ui_cursordock.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/WorksheetPrivate.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/TreeModel.h"

CursorDock::CursorDock(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CursorDock)
{
	ui->setupUi(this);
	ui->tvCursorData->setModel(nullptr);

	connect(ui->bCollapseAll, &QPushButton::clicked, this, &CursorDock::collapseAll);
	connect(ui->bExpandAll, &QPushButton::clicked, this, &CursorDock::expandAll);

	connect(ui->cbCursor0en, &QCheckBox::clicked, this, &CursorDock::cursor0EnableChanged);
	connect(ui->cbCursor1en, &QCheckBox::clicked, this, &CursorDock::cursor1EnableChanged);
}

void CursorDock::setPlots(QVector<CartesianPlot*> list) {
	m_initializing = true;
	m_plotList = list;
	m_plot = list.first();

	bool cursor0Enabled = m_plot->cursor0Enable();
	bool cursor1Enabled = m_plot->cursor1Enable();
	ui->cbCursor0en->setChecked(cursor0Enabled);
	ui->cbCursor1en->setChecked(cursor1Enabled);

	ui->tvCursorData->setColumnHidden(WorksheetPrivate::TreeModelColumn::CURSOR0, !cursor0Enabled);
	ui->tvCursorData->setColumnHidden(WorksheetPrivate::TreeModelColumn::CURSOR1, !cursor1Enabled);
	if (!cursor0Enabled)
		ui->tvCursorData->setColumnHidden(WorksheetPrivate::TreeModelColumn::CURSORDIFF, !cursor0Enabled);
	else if(cursor1Enabled)
		ui->tvCursorData->setColumnHidden(WorksheetPrivate::TreeModelColumn::CURSORDIFF, !cursor0Enabled);
	else
		ui->tvCursorData->setColumnHidden(WorksheetPrivate::TreeModelColumn::CURSORDIFF, true);

	ui->tvCursorData->expandAll();

	connect(m_plot, &CartesianPlot::cursor0EnableChanged, this, &CursorDock::plotCursor0EnableChanged);
	connect(m_plot, &CartesianPlot::cursor1EnableChanged, this, &CursorDock::plotCursor1EnableChanged);

	m_initializing = false;
}

CursorDock::~CursorDock() {
	delete ui;
}

void CursorDock::collapseAll() {
	ui->tvCursorData->collapseAll();
}

void CursorDock::expandAll() {
	ui->tvCursorData->expandAll();
}

void CursorDock::setCursorTreeViewModel(TreeModel* model) {
	ui->tvCursorData->setModel(model);
}

void CursorDock::cursor0EnableChanged(bool enable) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setCursor0Enable(enable);
}

void CursorDock::cursor1EnableChanged(bool enable) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setCursor1Enable(enable);
}

// #############################################################
// back from plot
// #############################################################
void CursorDock::plotCursor0EnableChanged(bool enable) {
	m_initializing = true;
	ui->cbCursor0en->setChecked(enable);
	ui->tvCursorData->setColumnHidden(WorksheetPrivate::TreeModelColumn::CURSOR0, !enable);
	if (!enable)
		ui->tvCursorData->setColumnHidden(WorksheetPrivate::TreeModelColumn::CURSORDIFF, !enable);
	else if (ui->cbCursor1en->isChecked())
		ui->tvCursorData->setColumnHidden(WorksheetPrivate::TreeModelColumn::CURSORDIFF, !enable);
	m_initializing = false;
}

void CursorDock::plotCursor1EnableChanged(bool enable) {
	m_initializing = true;
	ui->cbCursor1en->setChecked(enable);
	ui->tvCursorData->setColumnHidden(WorksheetPrivate::TreeModelColumn::CURSOR1, !enable);
	if (!enable)
		ui->tvCursorData->setColumnHidden(WorksheetPrivate::TreeModelColumn::CURSORDIFF, !enable);
	else if (ui->cbCursor0en->isChecked())
		ui->tvCursorData->setColumnHidden(WorksheetPrivate::TreeModelColumn::CURSORDIFF, !enable);
	m_initializing = false;
}

