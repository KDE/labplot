/*
    File                 : CursorDock.cpp
    Project              : LabPlot
    Description 	     : This dock represents the data from the cursors in the cartesian plots
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Martin Marmsoler <martin.marmsoler@gmail.com>
    SPDX-FileCopyrightText: 2019-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CursorDock.h"
#include "ui_cursordock.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/WorksheetPrivate.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/TreeModel.h"

#include <QKeyEvent>
#include <QMenu>
#include <QClipboard>

CursorDock::CursorDock(QWidget* parent) : QWidget(parent), ui(new Ui::CursorDock) {
	ui->setupUi(this);
	ui->tvCursorData->setModel(nullptr);

	ui->bCollapseAll->setIcon(QIcon::fromTheme(QLatin1String("collapse-all")));
	ui->bExpandAll->setIcon(QIcon::fromTheme(QLatin1String("expand-all")));

	ui->bCollapseAll->setToolTip(i18n("Collapse all curves"));
	ui->bExpandAll->setToolTip(i18n("Expand all curves"));

	connect(ui->bCollapseAll, &QPushButton::clicked, this, &CursorDock::collapseAll);
	connect(ui->bExpandAll, &QPushButton::clicked, this, &CursorDock::expandAll);
	connect(ui->cbCursor0en, &QCheckBox::clicked, this, &CursorDock::cursor0EnableChanged);
	connect(ui->cbCursor1en, &QCheckBox::clicked, this, &CursorDock::cursor1EnableChanged);

	//CTRL+C copies only the last cell in the selection, we want to copy the whole selection.
	//install event filters to handle CTRL+C key events.
	ui->tvCursorData->installEventFilter(this);

	ui->tvCursorData->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tvCursorData, &QTreeView::customContextMenuRequested,
			this, &CursorDock::contextMenuRequested);
}

void CursorDock::setWorksheet(Worksheet* worksheet) {
	m_initializing = true;

	ui->tvCursorData->setModel(worksheet->cursorModel());
	ui->tvCursorData->resizeColumnToContents(0);
	m_plotList = worksheet->children<CartesianPlot>();
	if (m_plotList.isEmpty())
		return;

	m_plot = m_plotList.first();

	bool cursor0Enabled = m_plot->cursor0Enable();
	bool cursor1Enabled = m_plot->cursor1Enable();
	ui->cbCursor0en->setChecked(cursor0Enabled);
	ui->cbCursor1en->setChecked(cursor1Enabled);

	ui->tvCursorData->setColumnHidden(static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), !cursor0Enabled);
	ui->tvCursorData->setColumnHidden(static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), !cursor1Enabled);
	if (cursor0Enabled && cursor1Enabled)
		ui->tvCursorData->setColumnHidden(static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), false);
	else
		ui->tvCursorData->setColumnHidden(static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), true);

	ui->tvCursorData->expandAll();

	// connect all plots as a workaround to not be able to know which plot is selected
	for (auto connection: selectedPlotsConnection)
		disconnect(connection);
	for (auto* plot : m_plotList) {
		selectedPlotsConnection << connect(plot, &CartesianPlot::cursor0EnableChanged, this, &CursorDock::plotCursor0EnableChanged);
		selectedPlotsConnection << connect(plot, &CartesianPlot::cursor1EnableChanged, this, &CursorDock::plotCursor1EnableChanged);
	}

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
	ui->tvCursorData->setColumnHidden(static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), !enable);
	if (enable && ui->cbCursor1en->isChecked())
		ui->tvCursorData->setColumnHidden(static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), false);
	else
		ui->tvCursorData->setColumnHidden(static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), true);

	m_initializing = false;
}

void CursorDock::plotCursor1EnableChanged(bool enable) {
	m_initializing = true;

	ui->cbCursor1en->setChecked(enable);
	ui->tvCursorData->setColumnHidden(static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), !enable);
	if (enable && ui->cbCursor0en->isChecked())
		ui->tvCursorData->setColumnHidden(static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), false);
	else
		ui->tvCursorData->setColumnHidden(static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), true);

	m_initializing = false;
}

bool CursorDock::eventFilter(QObject* obj, QEvent* event) {
	if (event->type() == QEvent::KeyPress && obj == ui->tvCursorData) {
		auto* key_event = static_cast<QKeyEvent*>(event);
		if (key_event->matches(QKeySequence::Copy)) {
			resultCopy();
			return true;
		}
	}
	return QWidget::eventFilter(obj, event);
}

void CursorDock::contextMenuRequested(QPoint pos) {
	auto* menu = new QMenu;
	menu->addAction(i18n("Copy Selection"), this, &CursorDock::resultCopy, QKeySequence::Copy);
	menu->addAction(i18n("Copy All"), this, &CursorDock::resultCopyAll);
	menu->exec(ui->tvCursorData->mapToGlobal(pos));
}

void CursorDock::resultCopy() {
	QString str;
	auto* model = ui->tvCursorData->model();
	auto* selection = ui->tvCursorData->selectionModel();
	const auto& indices = selection->selectedRows();
	for (auto index : indices) {
		int row = index.row();
		auto parent = index.parent();
		for (int col = 0; col < model->columnCount(); ++col) {
			if (col != 0)
				str += QLatin1Char('\t');
			str += model->data(model->index(row, col, parent)).toString();
		}

		str += QLatin1Char('\n');
	}

	QApplication::clipboard()->setText(str);
}

void CursorDock::resultCopyAll() {
	QString str;
	auto* model = ui->tvCursorData->model();
	for (int row = 0; row < model->rowCount(); ++row) {
		for (int col = 0; col < model->columnCount(); ++col) {
			if (col != 0)
				str += QLatin1Char('\t');
			str += model->data(model->index(row, col)).toString();
		}

		str += QLatin1Char('\n');

		//iterate over all children of the current row
		auto index = model->index(row, 0);
		for (int row = 0; row < model->rowCount(index); ++row) {
			for (int col = 0; col < model->columnCount(); ++col) {
				if (col != 0)
					str += QLatin1Char('\t');
				str += model->data(model->index(row, col, index)).toString();
			}

			str += QLatin1Char('\n');
		}
	}

	QApplication::clipboard()->setText(str);
}
