/*
	File                 : MatioOptionsWidget.cpp
	Project              : LabPlot
	Description          : widget providing options for the import of Matio data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "MatioOptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/datasources/filters/MatioFilter.h"
#include "backend/lib/macros.h"

#include <KUrlComboBox>

/*!
   \class MatioOptionsWidget
   \brief Widget providing options for the import of Matio data

   \ingroup frontend
*/
MatioOptionsWidget::MatioOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget)
	: QWidget(parent)
	, m_fileWidget(fileWidget) {
	ui.setupUi(parent);

	ui.twContent->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.twContent->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.twContent->setAlternatingRowColors(true);
	ui.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.bRefreshPreview->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));

	connect(ui.twContent, &QTableWidget::itemSelectionChanged, this, &MatioOptionsWidget::selectionChanged);
	connect(ui.bRefreshPreview, &QPushButton::clicked, fileWidget, &ImportFileWidget::refreshPreview);
}

void MatioOptionsWidget::clear() {
	ui.twPreview->clear();
}

void MatioOptionsWidget::updateContent(MatioFilter* filter, const QString& fileName) {
	// update variable info
	filter->parse(fileName);

	const int n = filter->varCount();
	const QVector<QStringList> varsInfo = filter->varsInfo();
	ui.twContent->setRowCount(n);
	const int nrCols = 7;
	for (int j = 0; j < nrCols; j++) {
		for (int i = 0; i < n; i++) {
			auto* item = new QTableWidgetItem(varsInfo.at(i).at(j));
			item->setFlags(item->flags() ^ Qt::ItemIsEditable); // readonly
			ui.twContent->setItem(i, j, item);
		}
		ui.twContent->resizeColumnToContents(j);
	}
}

/*!
	updates the selected var name of a Matio file when the table widget item is selected
*/
void MatioOptionsWidget::selectionChanged() {
	DEBUG(Q_FUNC_INFO)
	// QDEBUG(Q_FUNC_INFO << ", SELECTED ITEMS =" << ui.twContent->selectedItems());

	if (ui.twContent->selectedItems().isEmpty())
		return;

	m_fileWidget->refreshPreview();
}

/*!
	return list of selected Matio variable names
	selects first item if nothing is selected
*/
const QStringList MatioOptionsWidget::selectedNames() const {
	DEBUG(Q_FUNC_INFO)
	QStringList names;

	if (ui.twContent->selectedItems().size() == 0)
		ui.twContent->selectRow(0);

	for (auto* item : ui.twContent->selectedItems())
		if (item->column() == 0)
			names << item->text();
	// QDEBUG(Q_FUNC_INFO << ", selected vars: " << names);

	return names;
}
