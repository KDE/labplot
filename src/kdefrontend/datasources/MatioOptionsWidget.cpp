/***************************************************************************
File                 : MatioOptionsWidget.cpp
Project              : LabPlot
Description          : widget providing options for the import of Matio data
--------------------------------------------------------------------
Copyright            : (C) 2015-2017 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "MatioOptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/datasources/filters/MatioFilter.h"
#include "backend/lib/macros.h"

#include <KUrlComboBox>

 /*!
	\class MatioOptionsWidget
	\brief Widget providing options for the import of Matio data

	\ingroup kdefrontend
 */
MatioOptionsWidget::MatioOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget)
		: QWidget(parent), m_fileWidget(fileWidget) {
	ui.setupUi(parent);

	//QStringList headers;
	//TODO
	//headers << i18n("Name") << i18n("Type") << i18n("Properties") << i18n("Values");
	//ui.twContent->setHeaderLabels(headers);
	// type column is hidden
	//ui.twContent->hideColumn(1);
	ui.twContent->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.twContent->setAlternatingRowColors(true);
	//ui.twContent->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.bRefreshPreview->setIcon( QIcon::fromTheme("view-refresh") );

	//connect(ui.twContent, &QTreeWidget::itemSelectionChanged, this, &MatioOptionsWidget::matioTreeWidgetSelectionChanged);
	//connect(ui.bRefreshPreview, &QPushButton::clicked, fileWidget, &ImportFileWidget::refreshPreview);
}

void MatioOptionsWidget::clear() {
	//ui.twContent->clear();
	ui.twPreview->clear();
}

void MatioOptionsWidget::updateContent(MatioFilter *filter, const QString& fileName) {
	//ui.twContent->clear();

	//TODO: set table readonly

	// update variable list
	filter->parse(fileName);

	const int n = filter->varCount();
	//const QStringList varNames = filter->varNames();
	const QVector<QStringList> varsInfo = filter->varsInfo();
	ui.twContent->setRowCount(n);
	for (int j = 0; j < 7; j++) {
		for (int i = 0; i < n; i++) {
			QTableWidgetItem *item = new QTableWidgetItem(varsInfo.at(i).at(j));
			ui.twContent->setItem(i, j, item);
		}
		ui.twContent->resizeColumnToContents(j);
	}
}

/*!
	updates the selected var name of a Matio file when the tree widget item is selected
*/
/*void MatioOptionsWidget::netcdfTreeWidgetSelectionChanged() {
	DEBUG("netcdfTreeWidgetSelectionChanged()");
	QDEBUG("SELECTED ITEMS =" << ui.twContent->selectedItems());

	if (ui.twContent->selectedItems().isEmpty())
		return;

	QTreeWidgetItem* item = ui.twContent->selectedItems().first();
	if (item->data(1, Qt::DisplayRole).toString() == "variable")
		m_fileWidget->refreshPreview();
	else if (item->data(1, Qt::DisplayRole).toString().contains("attribute")) {
		// reads attributes (only for preview)
		auto filter = static_cast<MatioFilter*>(m_fileWidget->currentFileFilter());
		QString fileName = m_fileWidget->m_cbFileName->currentText();
		QString name = item->data(0, Qt::DisplayRole).toString();
		QString varName = item->data(1, Qt::DisplayRole).toString().split(' ')[0];
		QDEBUG("name =" << name << "varName =" << varName);

		QString importedText = filter->readAttribute(fileName, name, varName);
		DEBUG("importedText =" << STDSTRING(importedText));

		QStringList lineStrings = importedText.split('\n');
		int rows = lineStrings.size();
		ui.twPreview->setRowCount(rows);
		ui.twPreview->setColumnCount(0);
		for (int i = 0; i < rows; ++i) {
			QStringList lineString = lineStrings[i].split(' ');
			int cols = lineString.size();
			if (ui.twPreview->columnCount() < cols)
				ui.twPreview->setColumnCount(cols);

			for (int j = 0; j < cols; ++j) {
				auto* item = new QTableWidgetItem();
				item->setText(lineString[j]);
				ui.twPreview->setItem(i, j, item);
			}
		}
	} else
		DEBUG("non showable object selected in Matio tree widget");
}*/

/*!
	return list of selected Matio variable names
	selects all items if nothing is selected
*/
const QStringList MatioOptionsWidget::selectedNames() const {
	DEBUG("MatioOptionsWidget::selectedNames()");
	QStringList names;

	//if (ui.twContent->selectedItems().size() == 0)
	//	ui.twContent->selectAll();

	//for (auto* item : ui.twContent->selectedItems())
	//	names << item->text(0);
	QDEBUG("	MatioOptionsWidget: selected names =" << names);

	return names;
}
