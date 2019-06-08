/***************************************************************************
    File                 : FITSOptionsWidget.cpp
    Project              : LabPlot
    Description          : Widget providing options for the import of FITS data
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Fabian Kristof (fkristofszabolcs@gmail.com)
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)

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
************************************************************************** */

#include "FITSOptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/lib/macros.h"

FITSOptionsWidget::FITSOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget) : QWidget(parent), m_fileWidget(fileWidget) {
	ui.setupUi(parent);

	ui.twExtensions->headerItem()->setText(0, i18n("Content"));
	ui.twExtensions->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.twExtensions->setAlternatingRowColors(true);
	ui.twExtensions->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.bRefreshPreview->setIcon( QIcon::fromTheme("view-refresh") );

	connect(ui.twExtensions, &QTreeWidget::itemSelectionChanged, this, &FITSOptionsWidget::fitsTreeWidgetSelectionChanged);
	connect(ui.bRefreshPreview, &QPushButton::clicked, fileWidget, &ImportFileWidget::refreshPreview);
}

void FITSOptionsWidget::clear() {
	ui.twExtensions->clear();
	ui.twPreview->clear();
}

QString FITSOptionsWidget::currentExtensionName() {
	QString name;

	if (ui.twExtensions->currentItem() != nullptr && ui.twExtensions->currentItem()->text(0) != i18n("Primary header"))
		name = ui.twExtensions->currentItem()->text(ui.twExtensions->currentColumn());

	return name;
}

void FITSOptionsWidget::updateContent(FITSFilter* filter, const QString& fileName) {
	DEBUG("FITSOptionsWidget::updateContent() file name = " << fileName.toStdString());
	ui.twExtensions->clear();
	filter->parseExtensions(fileName, ui.twExtensions, true);
	DEBUG("FITSOptionsWidget::updateContent() DONE");
}

/*!
	updates the selected var name of a NetCDF file when the tree widget item is selected
*/
//TODO
void FITSOptionsWidget::fitsTreeWidgetSelectionChanged() {
	DEBUG("fitsTreeWidgetSelectionChanges()");
	QDEBUG("SELECTED ITEMS =" << ui.twExtensions->selectedItems());

	if (ui.twExtensions->selectedItems().isEmpty())
		return;

	QTreeWidgetItem* item = ui.twExtensions->selectedItems().first();
	int column = ui.twExtensions->currentColumn();

	WAIT_CURSOR;
	const QString& itemText = item->text(column);
	QString selectedExtension;
	int extType = 0;
	if (itemText.contains(QLatin1String("IMAGE #")) ||
	        itemText.contains(QLatin1String("ASCII_TBL #")) ||
	        itemText.contains(QLatin1String("BINARY_TBL #")))
		extType = 1;
	else if (!itemText.compare(i18n("Primary header")))
		extType = 2;
	if (extType == 0) {
		if (item->parent() != nullptr) {
			if (item->parent()->parent() != nullptr)
				selectedExtension = item->parent()->parent()->text(0) + QLatin1String("[") + item->text(column) + QLatin1String("]");
		}
	} else if (extType == 1) {
		if (item->parent() != nullptr) {
			if (item->parent()->parent() != nullptr) {
				bool ok;
				int hduNum = itemText.rightRef(1).toInt(&ok);
				selectedExtension = item->parent()->parent()->text(0) + QLatin1String("[") + QString::number(hduNum-1) + QLatin1String("]");
			}
		}
	} else {
		if (item->parent()->parent() != nullptr)
			selectedExtension = item->parent()->parent()->text(column);
	}

	if (!selectedExtension.isEmpty()) {
		auto filter = static_cast<FITSFilter*>(m_fileWidget->currentFileFilter());
		bool readFitsTableToMatrix;
		const QVector<QStringList> importedStrings = filter->readChdu(selectedExtension, &readFitsTableToMatrix, ui.sbPreviewLines->value());
		emit m_fileWidget->checkedFitsTableToMatrix(readFitsTableToMatrix);

		const int rows = importedStrings.size();
		ui.twPreview->clear();

		ui.twPreview->setRowCount(rows);
		int colCount = 0;
		const int maxColumns = 300;
		for (int i = 0; i < rows; ++i) {
			QStringList lineString = importedStrings[i];
			if (i == 0) {
				colCount = lineString.size() > maxColumns ? maxColumns : lineString.size();
				ui.twPreview->setColumnCount(colCount);
			}
			colCount = lineString.size() > maxColumns ? maxColumns : lineString.size();

			for (int j = 0; j < colCount; ++j) {
				auto* item = new QTableWidgetItem(lineString[j]);
				ui.twPreview->setItem(i, j, item);
			}
		}
		ui.twPreview->resizeColumnsToContents();
	}
	RESET_CURSOR;
}

/*!
	return list of selected FITS extension names
*/
const QStringList FITSOptionsWidget::selectedExtensions() const {
	QStringList names;
	for (const auto* item : ui.twExtensions->selectedItems())
		names << item->text(0);

	return names;
}

const QString FITSOptionsWidget::extensionName(bool* ok) {
	if (ui.twExtensions->currentItem() != nullptr) {
		const QTreeWidgetItem* item = ui.twExtensions->currentItem();
		const int currentColumn = ui.twExtensions->currentColumn();
		QString itemText = item->text(currentColumn);
		int extType = 0;
		if (itemText.contains(QLatin1String("IMAGE #")) ||
		        itemText.contains(QLatin1String("ASCII_TBL #")) ||
		        itemText.contains(QLatin1String("BINARY_TBL #")))
			extType = 1;
		else if (!itemText.compare(i18n("Primary header")))
			extType = 2;

		if (extType == 0) {
			if (item->parent() != nullptr && item->parent()->parent() != nullptr)
				return item->parent()->parent()->text(0) + QLatin1String("[")+ item->text(currentColumn) + QLatin1String("]");
		} else if (extType == 1) {
			if (item->parent() != nullptr && item->parent()->parent() != nullptr) {
				int hduNum = itemText.rightRef(1).toInt(ok);
				return item->parent()->parent()->text(0) + QLatin1String("[") + QString::number(hduNum-1) + QLatin1String("]");
			}
		} else {
			if (item->parent()->parent() != nullptr)
				return item->parent()->parent()->text(currentColumn);
		}
	}

	return QString();
}
