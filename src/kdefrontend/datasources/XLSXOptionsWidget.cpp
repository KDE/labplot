/*
	File                 : XLSXOptionsWidget.cpp
	Project              : LabPlot
	Description          : Widget providing options for the import of XLSX (Excel) data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Fabian Kristof (fkristofszabolcs@gmail.com)
	SPDX-FileCopyrightText: 2022-2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XLSXOptionsWidget.h"
#include "backend/lib/macros.h"
#include "src/backend/datasources/filters/XLSXFilter.h"
#include "src/kdefrontend/datasources/ImportFileWidget.h"

#ifdef HAVE_QXLSX
#include "xlsxcellrange.h"
#endif

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QIcon>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVector>

XLSXOptionsWidget::XLSXOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget)
	: QWidget(parent)
	, m_fileWidget(fileWidget) {
	ui.setupUi(parent);
	ui.twDataRegions->headerItem()->setText(0, i18n("Data regions"));
	ui.twDataRegions->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.twDataRegions->setAlternatingRowColors(true);
	ui.twDataRegions->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

	ui.bRefreshPreview->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
	ui.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(ui.twDataRegions, &QTreeWidget::itemSelectionChanged, this, &XLSXOptionsWidget::dataRegionSelectionChanged);
}

XLSXOptionsWidget::~XLSXOptionsWidget() {
}

void XLSXOptionsWidget::updateContent(XLSXFilter* filter, const QString& fileName) {
	DEBUG(Q_FUNC_INFO)
	ui.twDataRegions->clear();
	auto* rootItem = ui.twDataRegions->invisibleRootItem();
	filter->parse(fileName, rootItem);

	ui.twDataRegions->insertTopLevelItem(0, rootItem);
	ui.twDataRegions->expandAll();

	// select first data range
	auto items = ui.twDataRegions->selectedItems();
	if (items.size() == 0) {
		const auto* tli = ui.twDataRegions->topLevelItem(0);
		for (int i = 0; i < tli->childCount(); i++) { // sheets
			const auto* sheet = tli->child(i);
			if (sheet->childCount() > 0) { // select first range
				ui.twDataRegions->setCurrentItem(sheet->child(0));
				return;
			}
		}
	}
}

void XLSXOptionsWidget::dataRegionSelectionChanged() {
	DEBUG(Q_FUNC_INFO)
#ifdef HAVE_QXLSX
	WAIT_CURSOR;

	const auto& selectedItems = ui.twDataRegions->selectedItems();
	if (selectedItems.isEmpty()) {
		RESET_CURSOR;
		return;
	}

	if (selectedItems.size() > 1)
		emit enableDataPortionSelection(false);
	else if (selectedItems.size() == 1)
		emit enableDataPortionSelection(true);

	QXlsx::CellRange selectedRegion;
	QString sheetName;

	auto* item = selectedItems.last();
	int column = ui.twDataRegions->currentColumn();
	int row = ui.twDataRegions->currentIndex().row();
	const auto& selectedRegionText = item->text(column);
	auto* const filter = static_cast<XLSXFilter*>(m_fileWidget->currentFileFilter());

	selectedRegion = selectedRegionText;
	// if sheet name is selected maybe show full sheet?
	if (item->parent()) {
		sheetName = item->parent()->text(0);
	} else {
		RESET_CURSOR;
		return;
	}

	if (!sheetName.isEmpty() && selectedRegion.isValid()) {
		bool regionCanBeImportedToMatrix = false;

		if (selectedRegion.columnCount() > 100) {
			const int lastCol = selectedRegion.firstColumn() + 100;
			selectedRegion.setLastColumn(lastCol);
		}

		const auto importedStrings = filter->previewForDataRegion(sheetName, selectedRegion, &regionCanBeImportedToMatrix, ui.sbPreviewLines->value());
		m_previewString = importedStrings;
		// QDEBUG("PREVIEW:" << importedStrings)

		// enable the first row as column names option only if the data contains more than 1 row
		m_fileWidget->enableXLSXFirstRowAsColNames(importedStrings.size() > 1);

		emit m_fileWidget->enableImportToMatrix(regionCanBeImportedToMatrix);

		// sheet name - item row will identify the region
		const auto mapVal = qMakePair(sheetName, row);
		// this region was not currently selected
		if (m_regionIsPossibleToImportToMatrix.find(mapVal) != m_regionIsPossibleToImportToMatrix.end())
			m_regionIsPossibleToImportToMatrix.insert(mapVal, regionCanBeImportedToMatrix);
		else if (!item->isSelected()) // the item was deselected
			m_regionIsPossibleToImportToMatrix.remove(mapVal);

		const auto rows = importedStrings.size();
		ui.twPreview->clear();
		const bool firstRowAsHeader = m_fileWidget->xlsxUseFirstRowAsColNames();
		DEBUG("first row as header enabled = " << firstRowAsHeader)
		ui.twPreview->setRowCount(rows - firstRowAsHeader);

		int colCount = 0;
		const int maxColumns = 50;
		for (int i = 0; i < rows; ++i) {
			auto lineString = importedStrings.at(i);
			colCount = std::min(maxColumns, lineString.size());

			if (i == 0) {
				ui.twPreview->setColumnCount(colCount);

				if (firstRowAsHeader) {
					for (int col = 0; col < colCount; ++col) {
						auto* item = new QTableWidgetItem(lineString.at(col));
						ui.twPreview->setHorizontalHeaderItem(col, item);
					}
					continue; // data used as header
				} else {
					for (int col = 0; col < colCount; ++col) {
						auto colName = XLSXFilter::convertFromNumberToXLSXColumn(selectedRegion.firstColumn() + col);
						// DEBUG("COLUMN " << col + 1 << " NAME = " << STDSTRING(colName))
						//  TODO: show column modes?
						// auto* item = new QTableWidgetItem(colName + QStringLiteral(" {") + QLatin1String(ENUM_TO_STRING(AbstractColumn, ColumnMode,
						//	filter->columnModes().at(i))) + QStringLiteral("}"));
						auto* item = new QTableWidgetItem(colName);

						ui.twPreview->setHorizontalHeaderItem(col, item);
					}
				}
			}

			auto* item = new QTableWidgetItem(QString::number(selectedRegion.firstRow() + i - firstRowAsHeader));
			ui.twPreview->setVerticalHeaderItem(i - firstRowAsHeader, item);

			for (int j = 0; j < colCount; ++j) {
				auto* item = new QTableWidgetItem(lineString.at(j));
				ui.twPreview->setItem(i - firstRowAsHeader, j, item);
			}
		}
		ui.twPreview->resizeColumnsToContents();
	}

	RESET_CURSOR;
#endif
}

QStringList XLSXOptionsWidget::selectedXLSXRegionNames() const {
	const auto& items = ui.twDataRegions->selectedItems();
	DEBUG(Q_FUNC_INFO << ", selected items = " << items.size())

	QStringList names;
	for (const auto* item : items) {
		if (item->parent()) { // child of sheet
			const auto sheetName = item->parent()->text(0);
			// DEBUG(Q_FUNC_INFO << ", name = " << STDSTRING(sheetName))
			names.push_back({sheetName + QLatin1Char('!') + item->text(0)});
		}
	}

	return names;
}

QVector<QStringList> XLSXOptionsWidget::previewString() const {
	return m_previewString;
}
