/*
	File                 : FITSOptionsWidget.cpp
	Project              : LabPlot
	Description          : Widget providing options for the import of FITS data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
	SPDX-FileCopyrightText: 2017-2025 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FITSOptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/lib/macros.h"

FITSOptionsWidget::FITSOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget)
	: QWidget(parent)
	, m_fileWidget(fileWidget) {
	ui.setupUi(parent);

	ui.twExtensions->headerItem()->setText(0, i18n("Content"));
	ui.twExtensions->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.twExtensions->setAlternatingRowColors(true);
	ui.twExtensions->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.bRefreshPreview->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));

	connect(ui.twExtensions, &QTreeWidget::itemSelectionChanged, this, &FITSOptionsWidget::fitsTreeWidgetSelectionChanged);
	connect(ui.bRefreshPreview, &QPushButton::clicked, fileWidget, &ImportFileWidget::refreshPreview);
}

void FITSOptionsWidget::clear() {
	ui.twExtensions->clear();
	ui.twPreview->clear();
}

QString FITSOptionsWidget::currentExtensionName() {
	if (ui.twExtensions->currentItem() != nullptr && ui.twExtensions->currentItem()->text(0) != i18n("Primary header"))
		return ui.twExtensions->currentItem()->text(ui.twExtensions->currentColumn());

	return {};
}

void FITSOptionsWidget::updateContent(FITSFilter* filter, const QString& fileName) {
	DEBUG(Q_FUNC_INFO << ", file name = " << STDSTRING(fileName));
	ui.twExtensions->clear();
	filter->parseExtensions(fileName, ui.twExtensions, true);
	DEBUG(Q_FUNC_INFO << ", DONE");
}

/*!
	updates the selected var name of a FITS file when the tree widget item is selected
*/
// TODO
void FITSOptionsWidget::fitsTreeWidgetSelectionChanged() {
	QDEBUG(Q_FUNC_INFO << ", SELECTED ITEMS =" << ui.twExtensions->selectedItems());

	if (ui.twExtensions->selectedItems().isEmpty())
		return;

	QTreeWidgetItem* item = ui.twExtensions->selectedItems().first();
	int column = ui.twExtensions->currentColumn();

	WAIT_CURSOR_AUTO_RESET;
	const QString& itemText = item->text(column);
	QString selectedExtension;
	// TODO: same as extensionName() ?
	int extType = 0;
	if (itemText.contains(QLatin1String("IMAGE #")) || itemText.contains(QLatin1String("ASCII_TBL #")) || itemText.contains(QLatin1String("BINARY_TBL #")))
		extType = 1;
	else if (!itemText.compare(i18n("Primary header")))
		extType = 2;
	if (extType == 0) {
		if (item->parent() != nullptr) {
			if (item->parent()->parent() != nullptr)
				selectedExtension = item->parent()->parent()->text(0) + QStringLiteral("[") + item->text(column) + QStringLiteral("]");
		}
	} else if (extType == 1) {
		if (item->parent() != nullptr) {
			if (item->parent()->parent() != nullptr) {
				bool ok;
				int hduNum = itemText.right(1).toInt(&ok);
				selectedExtension = item->parent()->parent()->text(0) + QStringLiteral("[") + QString::number(hduNum - 1) + QStringLiteral("]");
			}
		}
	} else {
		if (item->parent()->parent() != nullptr)
			selectedExtension = item->parent()->parent()->text(column);
	}

	if (!selectedExtension.isEmpty()) {
		auto filter = static_cast<FITSFilter*>(m_fileWidget->currentFileFilter());
		bool readFitsTableToMatrix;
		const auto importedStrings = filter->readChdu(selectedExtension, &readFitsTableToMatrix, ui.sbPreviewLines->value());
		Q_EMIT m_fileWidget->enableImportToMatrix(readFitsTableToMatrix);

		const int rows = importedStrings.size();
		ui.twPreview->clear();

		ui.twPreview->setRowCount(rows);
		const int maxColumns = 300;
		for (int i = 0; i < rows; ++i) {
			auto lineString = importedStrings[i];
			int colCount;
			if (i == 0) {
				colCount = lineString.size() > maxColumns ? maxColumns : lineString.size();
				ui.twPreview->setColumnCount(colCount);
			}
			colCount = lineString.size() > maxColumns ? maxColumns : lineString.size();

			for (int j = 0; j < colCount; ++j) {
				auto* newItem = new QTableWidgetItem(lineString[j]);
				ui.twPreview->setItem(i, j, newItem);
			}
		}
		ui.twPreview->resizeColumnsToContents();
	}
}

// return full path of file name and [extension] appended
const QString FITSOptionsWidget::extensionName(bool* ok) {
	const auto* item = ui.twExtensions->currentItem();
	if (!item)
		return {};

	const int currentColumn = ui.twExtensions->currentColumn();
	QString itemText = item->text(currentColumn);
	QDEBUG(Q_FUNC_INFO << ", item text:" << itemText)

	ExtensionType extType = ExtensionType::UNKNOWN;
	if (itemText.contains(QLatin1String("IMAGE #")) || itemText.contains(QLatin1String("ASCII_TBL #")) || itemText.contains(QLatin1String("BINARY_TBL #")))
		extType = ExtensionType::IMAGE_OR_TBL;
	else if (!itemText.compare(QLatin1String("Primary header")))
		extType = ExtensionType::PRIMARY;
	//DEBUG(Q_FUNC_INFO << ", extension type:" << extType)

	switch (extType) {
	case ExtensionType::UNKNOWN:
		return itemText;
	case ExtensionType::IMAGE_OR_TBL: {
		int hduNum = itemText.right(1).toInt(ok);
		return QString::number(hduNum - 1);
		}
	case ExtensionType::PRIMARY:
		break;
	}

	return {};
}
