/***************************************************************************
File                 : ROOTOptionsWidget.cpp
Project              : LabPlot
Description          : widget providing options for the import of ROOT data
--------------------------------------------------------------------
Copyright            : (C) 2018 Christoph Roick (chrisito@gmx.de)
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

#include "ROOTOptionsWidget.h"

#include "ImportFileWidget.h"

#include "backend/datasources/filters/ROOTFilter.h"
#include "backend/lib/macros.h"

ROOTOptionsWidget::ROOTOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget) : QWidget(parent), m_fileWidget(fileWidget) {
	ui.setupUi(parent);
	histItem = new QTreeWidgetItem(ui.twContent, QStringList(i18n("Histograms")));
	histItem->setFlags(Qt::ItemIsEnabled);
	treeItem = new QTreeWidgetItem(ui.twContent, QStringList(i18n("Trees and Tuples")));
	treeItem->setFlags(Qt::ItemIsEnabled);

	connect(ui.twContent, &QTreeWidget::itemSelectionChanged, this, &ROOTOptionsWidget::rootObjectSelectionChanged);
	connect(ui.bRefreshPreview, &QPushButton::clicked, fileWidget, &ImportFileWidget::refreshPreview);
}

void ROOTOptionsWidget::clear() {
	qDeleteAll(histItem->takeChildren());
	qDeleteAll(treeItem->takeChildren());
	ui.twPreview->clearContents();
}

void ROOTOptionsWidget::updateContent(ROOTFilter *filter, const QString& fileName) {
	DEBUG("updateContent()");

	qDeleteAll(histItem->takeChildren());
	qDeleteAll(treeItem->takeChildren());
	leaves.clear();
	for (const QString& s : filter->listHistograms(fileName))
		new QTreeWidgetItem(histItem, QStringList(s));
	for (const QString& s : filter->listTrees(fileName)) {
		new QTreeWidgetItem(treeItem, QStringList(s));
		leaves[s] = filter->listLeaves(fileName, s);
	}
}

void ROOTOptionsWidget::rootObjectSelectionChanged() {
	DEBUG("rootObjectSelectionChanged()");
	auto items = ui.twContent->selectedItems();
	QDEBUG("SELECTED ITEMS =" << items);

	ui.twColumns->clear();

	if (items.isEmpty())
		return;

	QTreeWidgetItem* const p = items.first()->parent();
	if (p == histItem) {
		ui.twColumns->setColumnCount(1);

		auto center = new QTreeWidgetItem(ui.twColumns, QStringList(i18n("Bin Center")));
		center->setData(0, Qt::UserRole, QStringList(QStringLiteral("center")));
		center->setSelected(true);
		center->setFirstColumnSpanned(true);
		auto low = new QTreeWidgetItem(ui.twColumns, QStringList(i18n("Low Edge")));
		low->setData(0, Qt::UserRole, QStringList(QStringLiteral("low")));
		low->setFirstColumnSpanned(true);
		auto content = new QTreeWidgetItem(ui.twColumns, QStringList(i18n("Content")));
		content->setData(0, Qt::UserRole, QStringList(QStringLiteral("content")));
		content->setSelected(true);
		content->setFirstColumnSpanned(true);
		auto error = new QTreeWidgetItem(ui.twColumns, QStringList(i18n("Error")));
		error->setData(0, Qt::UserRole, QStringList(QStringLiteral("error")));
		error->setSelected(true);
		error->setFirstColumnSpanned(true);

		if (!histselected) {
			histselected = true;
			ui.sbFirst->setMaximum(0);
			ui.sbLast->setMaximum(0);
		}
	} else if (p == treeItem) {
		ui.twColumns->setColumnCount(2);

		for (const auto& l : leaves[items.first()->text(0)]) {
			auto leaf = new QTreeWidgetItem(ui.twColumns, l);
			bool ok = false;
			if (l.count() > 1) {
				QString index(l.back());
				if (index.at(0) == '[' && index.at(index.size() - 1) == ']') {
					size_t elements = index.mid(1, index.length() - 2).toUInt(&ok);
					if (ok) {
						leaf->setFlags(Qt::ItemIsEnabled);
						QStringList elname({l.at(l.count() - 2), QString()});
						QStringList eldata(elname);
						if (l.count() > 2)
							eldata.prepend(l.front());
						for (size_t i = 0; i < elements; ++i) {
							eldata.last() = elname.last() = QString("[%1]").arg(i);
							auto el = new QTreeWidgetItem(leaf, elname);
							el->setData(0, Qt::UserRole, eldata);
						}
					}
				}
			} else
				leaf->setFirstColumnSpanned(true);

			if (!ok)
				leaf->setData(0, Qt::UserRole, l);
		}

		ui.twColumns->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

		if (histselected) {
			histselected = false;
			ui.sbFirst->setMaximum(0);
			ui.sbLast->setMaximum(0);
		}
	}

	m_fileWidget->refreshPreview();
}

const QStringList ROOTOptionsWidget::selectedNames() const {
	QStringList names;

	for (const QTreeWidgetItem* const item : ui.twContent->selectedItems()) {
		if (item->parent() == histItem)
			names << QStringLiteral("Hist:") + item->text(0);
		else if (item->parent() == treeItem)
			names << QStringLiteral("Tree:") + item->text(0);
	}

	return names;
}

QVector<QStringList> ROOTOptionsWidget::columns() const {
	QVector<QStringList> cols;

	auto item = ui.twColumns->topLevelItem(0);
	while (item) {
		if (item->isSelected())
			cols << item->data(0,Qt::UserRole).toStringList();

		item = ui.twColumns->itemBelow(item);
	}

	return cols;
}

void ROOTOptionsWidget::setNRows(int nrows) {
	// try to retain the range settings:
	// - if nrows was not 0, keep start row,
	//   else set it to one after underflow
	// - if nrows didn't change, keep end row,
	//   else set it to one before overflow
	const int max = qMax(nrows - 1, 0);
	int firstval = ui.sbFirst->value();
	if (ui.sbFirst->maximum() == 0)
		firstval = qMin(nrows - 1, histselected ? 1 : 0);
	ui.sbFirst->setMaximum(max);
	ui.sbFirst->setValue(firstval);

	int lastval = max == ui.sbLast->maximum() ? ui.sbLast->value()
	                                          : qMax(max - (histselected ? 1 : 0), 0);
	ui.sbLast->setMaximum(max);
	ui.sbLast->setValue(lastval);
}
