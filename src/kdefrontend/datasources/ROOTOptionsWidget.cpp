/*
File                 : ROOTOptionsWidget.cpp
Project              : LabPlot
Description          : widget providing options for the import of ROOT data
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2018 Christoph Roick (chrisito@gmx.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

void fillTree(QTreeWidgetItem* node, const ROOTFilter::Directory& dir)
{
	node->setFlags(Qt::ItemIsEnabled);
	for (const ROOTFilter::Directory& child : dir.children)
		fillTree(new QTreeWidgetItem(node, QStringList(child.name)), child);
	for (const auto& content : dir.content)
		(new QTreeWidgetItem(node, QStringList(content.first)))->setData(0, Qt::UserRole, content.second);
}

QHash<QStringList, QVector<QStringList> > findLeaves(
	QTreeWidgetItem* node,
	ROOTFilter* filter,
	const QString& fileName, const QStringList& path = QStringList{}
) {
	QHash<QStringList, QVector<QStringList> > leaves;
	if (node->childCount() > 0) {
		for (int i = 0; i < node->childCount(); ++i)
			leaves.unite(findLeaves(node->child(i), filter, fileName, path + QStringList(node->child(i)->text(0))));
	} else {
		leaves[path] = filter->listLeaves(fileName, node->data(0, Qt::UserRole).toLongLong());
	}
	return leaves;
}

void ROOTOptionsWidget::updateContent(ROOTFilter* filter, const QString& fileName) {
	DEBUG("updateContent()");

	qDeleteAll(histItem->takeChildren());
	qDeleteAll(treeItem->takeChildren());
	fillTree(histItem, filter->listHistograms(fileName));
	fillTree(treeItem, filter->listTrees(fileName));
	leaves = findLeaves(treeItem, filter, fileName);
}

void ROOTOptionsWidget::rootObjectSelectionChanged() {
	DEBUG("rootObjectSelectionChanged()");
	auto items = ui.twContent->selectedItems();
	QDEBUG("SELECTED ITEMS =" << items);

	ui.twColumns->clear();

	if (items.isEmpty()) {
		ui.twColumns->setHeaderHidden(true);
		return;
	}

	QTreeWidgetItem* p = items.first();
	QStringList path;
	while (p && p != histItem && p != treeItem) {
		path.prepend(p->text(0));
		p = p->parent();
	}

	if (p == histItem) {
		ui.twColumns->setColumnCount(1);
		ui.twColumns->setHeaderHidden(false);
		ui.twColumns->setHeaderLabels(QStringList(i18n("Histogram Data")));

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
		ui.twColumns->setHeaderHidden(false);
		ui.twColumns->setHeaderLabels(QStringList({i18n("Branch/Leaf"), i18n("Array Size")}));

		auto it = leaves.find(path);
		if (it != leaves.end()) {
			for (const auto& l : it.value()) {
				auto leaf = new QTreeWidgetItem(ui.twColumns, l);
				bool ok = false;
				if (l.count() > 1) {
					QString index(l.back());
					if (index.at(0) == '[' && index.at(index.size() - 1) == ']') {
						size_t elements = index.midRef(1, index.length() - 2).toUInt(&ok);
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
		}

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

	for (QTreeWidgetItem* item : ui.twContent->selectedItems()) {
		QString path;
		while (item && item != histItem && item != treeItem) {
			path.prepend('/' + item->text(0));
			item = item->parent();
		}
		path[0] = ':';
		if (item == histItem)
			names << QStringLiteral("Hist") + path;
		else if (item == treeItem)
			names << QStringLiteral("Tree") + path;
	}

	return names;
}

QVector<QStringList> ROOTOptionsWidget::columns() const {
	QVector<QStringList> cols;

	// ui.twColumns->selectedItems() returns the items in the order of selection.
	// Iterate through the tree to retain the displayed order.
	for (int t = 0; t < ui.twColumns->topLevelItemCount(); ++t) {
		auto titem = ui.twColumns->topLevelItem(t);
		if (titem->isSelected())
			cols << titem->data(0, Qt::UserRole).toStringList();
		for (int c = 0; c < titem->childCount(); ++c) {
			auto citem = titem->child(c);
			if (citem->isSelected())
				cols << citem->data(0, Qt::UserRole).toStringList();
		}
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
