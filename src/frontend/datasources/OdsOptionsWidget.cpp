/*
	File                 : OdsOptionsWidget.cpp
	Project              : LabPlot
	Description          : Widget providing options for the import of Open Document Spreadsheet (ods) data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "OdsOptionsWidget.h"
#include "backend/datasources/filters/OdsFilter.h"
#include "backend/lib/macros.h"
#include "frontend/datasources/ImportFileWidget.h"

OdsOptionsWidget::OdsOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget)
	: QWidget(parent)
	, m_fileWidget(fileWidget) {
	ui.setupUi(parent);
	ui.twDataRegions->headerItem()->setText(0, i18n("Data sheets"));
	ui.twDataRegions->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.twDataRegions->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.twDataRegions->setAlternatingRowColors(true);
	ui.twDataRegions->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

	ui.bRefreshPreview->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
	ui.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(ui.twDataRegions, &QTreeWidget::itemSelectionChanged, this, &OdsOptionsWidget::sheetSelectionChanged);
	connect(ui.bRefreshPreview, &QPushButton::clicked, fileWidget, &ImportFileWidget::refreshPreview);
}

OdsOptionsWidget::~OdsOptionsWidget() {
}

void OdsOptionsWidget::updateContent(OdsFilter* filter, const QString& fileName) {
	DEBUG(Q_FUNC_INFO)
	ui.twDataRegions->clear();
	auto* rootItem = ui.twDataRegions->invisibleRootItem();
	filter->parse(fileName, rootItem);

	ui.twDataRegions->insertTopLevelItem(0, rootItem);
	ui.twDataRegions->expandAll();

	// select first sheet
	if (ui.twDataRegions->selectedItems().isEmpty()) {
		const auto* tli = ui.twDataRegions->topLevelItem(0);
		for (int i = 0; i < tli->childCount(); i++) { // sheets
			auto* sheet = tli->child(i);
			if (sheet) {
				ui.twDataRegions->setCurrentItem(sheet);
				return;
			}
		}
	}
}

void OdsOptionsWidget::sheetSelectionChanged() {
	DEBUG(Q_FUNC_INFO)
#ifdef HAVE_ORCUS
	WAIT_CURSOR;

	const auto& selectedItems = ui.twDataRegions->selectedItems();
	if (selectedItems.isEmpty()) {
		DEBUG(Q_FUNC_INFO << ", no items selected!")
		RESET_CURSOR;
		return;
	}

	if (selectedItems.size() > 1)
		Q_EMIT enableDataPortionSelection(false);
	else // one selected item
		Q_EMIT enableDataPortionSelection(true);

	auto* item = selectedItems.last();
	auto* const filter = static_cast<OdsFilter*>(m_fileWidget->currentFileFilter());

	QString sheetName;
	// if sheet name is selected maybe show full sheet?
	if (item) {
		sheetName = item->text(0);
	} else {
		RESET_CURSOR;
		return;
	}

	DEBUG(Q_FUNC_INFO << ", sheet name = " << sheetName.toStdString())
	if (!sheetName.isEmpty()) {
		const auto importedStrings = filter->preview(sheetName, ui.sbPreviewLines->value());
		const auto rowCount = importedStrings.size();
		m_previewString = importedStrings;

		ui.twPreview->clear();
		const bool firstRowAsHeader = m_fileWidget->useFirstRowAsColNames();
		DEBUG("first row as header enabled = " << firstRowAsHeader)
		ui.twPreview->setRowCount(rowCount - firstRowAsHeader);

		int colCount = 0;
		const int maxColumns = 100;
		for (int row = 0; row < rowCount; ++row) {
			auto lineString = importedStrings.at(row);
			const int size = lineString.size();
			colCount = std::min(maxColumns, size);
			if (row == 0) {
				ui.twPreview->setColumnCount(colCount);

				// set column header
				if (firstRowAsHeader) {
					for (int col = 0; col < colCount; ++col) {
						auto* item = new QTableWidgetItem(lineString.at(col));
						ui.twPreview->setHorizontalHeaderItem(col, item);
					}
				} else {
					for (int col = 0; col < colCount; ++col) {
						auto colName = AbstractFileFilter::convertFromNumberToColumn(col + filter->firstColumn());
						auto* item = new QTableWidgetItem(colName);

						ui.twPreview->setHorizontalHeaderItem(col, item);
					}
				}
			}

			auto* item = new QTableWidgetItem(QString::number(row - firstRowAsHeader + 1));
			ui.twPreview->setVerticalHeaderItem(row - firstRowAsHeader, item);

			for (int col = 0; col < colCount; ++col) {
				auto* item = new QTableWidgetItem(lineString.at(col));
				ui.twPreview->setItem(row - firstRowAsHeader, col, item);
			}
		}
		ui.twPreview->resizeColumnsToContents();
	}

	RESET_CURSOR;
#endif
}

QStringList OdsOptionsWidget::selectedOdsSheetNames() const {
	const auto& items = ui.twDataRegions->selectedItems();
	DEBUG(Q_FUNC_INFO << ", number of selected items = " << items.size())

	QStringList names;
	for (const auto* item : items) {
		if (item->parent()) { // child of sheet
			QString sheetName = item->parent()->text(0);
			sheetName = sheetName.split(QLatin1Char('/')).last();
			// DEBUG(Q_FUNC_INFO << ", sheet name = " << STDSTRING(sheetName))
			names.push_back(QString(sheetName + QLatin1Char('!') + item->text(0)));
		}
	}

	return names;
}

QVector<QStringList> OdsOptionsWidget::previewString() const {
	return m_previewString;
}
