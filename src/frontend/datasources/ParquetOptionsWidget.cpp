/*
	File                 : ParquetOptionsWidget.cpp
	Project              : LabPlot
	Description          : widget providing options for the import of Apache Parquet/Arrow IPC/ORC data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "ParquetOptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/datasources/filters/ParquetFilter.h"

/*!
   \class ParquetOptionsWidget
   \brief Widget providing options for the import of Apache Parquet, Arrow IPC and ORC data

   \ingroup frontend
*/
ParquetOptionsWidget::ParquetOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget)
	: QWidget(parent)
	, m_fileWidget(fileWidget) {
	ui.setupUi(parent);

	ui.bRefreshPreview->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));

	connect(ui.lwColumns, &QListWidget::itemChanged, this, &ParquetOptionsWidget::selectionChanged);
	connect(ui.bRefreshPreview, &QPushButton::clicked, fileWidget, &ImportFileWidget::refreshPreview);
	connect(ui.bSelectAll, &QPushButton::clicked, this, &ParquetOptionsWidget::selectAll);
	connect(ui.bDeselectAll, &QPushButton::clicked, this, &ParquetOptionsWidget::deselectAll);
}

void ParquetOptionsWidget::clear() {
	ui.lwColumns->clear();
	ui.twPreview->clear();
}

void ParquetOptionsWidget::updateContent(ParquetFilter* filter, const QString& fileName) {
	ui.lwColumns->clear();

	// Read the column names from the file
	filter->preview(fileName, 0); // triggers readArrowTable which populates columnNames
	const QStringList names = filter->columnNames();

	ui.lwColumns->blockSignals(true);
	for (const auto& name : names) {
		auto* item = new QListWidgetItem(name, ui.lwColumns);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Checked);
	}
	ui.lwColumns->blockSignals(false);
}

int ParquetOptionsWidget::lines() const {
	return ui.sbPreviewLines->value();
}
QTableWidget* ParquetOptionsWidget::previewWidget() const {
	return ui.twPreview;
}

QStringList ParquetOptionsWidget::selectedColumnNames() const {
	QStringList names;
	for (int i = 0; i < ui.lwColumns->count(); ++i) {
		auto* item = ui.lwColumns->item(i);
		if (item->checkState() == Qt::Checked)
			names << item->text();
	}
	return names;
}

void ParquetOptionsWidget::selectionChanged() {
	m_fileWidget->refreshPreview();
}

void ParquetOptionsWidget::selectAll() {
	ui.lwColumns->blockSignals(true);
	for (int i = 0; i < ui.lwColumns->count(); ++i)
		ui.lwColumns->item(i)->setCheckState(Qt::Checked);
	ui.lwColumns->blockSignals(false);
	m_fileWidget->refreshPreview();
}

void ParquetOptionsWidget::deselectAll() {
	ui.lwColumns->blockSignals(true);
	for (int i = 0; i < ui.lwColumns->count(); ++i)
		ui.lwColumns->item(i)->setCheckState(Qt::Unchecked);
	ui.lwColumns->blockSignals(false);
	m_fileWidget->refreshPreview();
}
