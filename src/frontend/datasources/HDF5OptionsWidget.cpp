/*
	File                 : HDF5OptionsWidget.cpp
	Project              : LabPlot
	Description          : widget providing options for the import of HDF5 data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "HDF5OptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/datasources/filters/HDF5Filter.h"
#include "backend/lib/macros.h"

/*!
   \class HDF5OptionsWidget
   \brief Widget providing options for the import of HDF5 data

   \ingroup kdefrontend
*/
HDF5OptionsWidget::HDF5OptionsWidget(QWidget* parent, ImportFileWidget* fileWidget)
	: QWidget(parent)
	, m_fileWidget(fileWidget) {
	ui.setupUi(parent);

	QStringList hdf5headers;
	hdf5headers << i18n("Name") << i18n("Link") << i18n("Type") << i18n("Properties") << i18n("Attributes");
	ui.twContent->setHeaderLabels(hdf5headers);
	ui.twContent->setAlternatingRowColors(true);
	// link and type column are hidden
	ui.twContent->hideColumn(1);
	ui.twContent->hideColumn(2);
	ui.twContent->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.twContent->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.bRefreshPreview->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));

	connect(ui.twContent, &QTreeWidget::itemSelectionChanged, this, &HDF5OptionsWidget::hdf5TreeWidgetSelectionChanged);
	connect(ui.bRefreshPreview, &QPushButton::clicked, fileWidget, &ImportFileWidget::refreshPreview);
}

void HDF5OptionsWidget::clear() {
	ui.twContent->clear();
	ui.twPreview->clear();
}

int HDF5OptionsWidget::updateContent(HDF5Filter* filter, const QString& fileName) {
	ui.twContent->clear();
	QTreeWidgetItem* rootItem = ui.twContent->invisibleRootItem();
	int status = filter->parse(fileName, rootItem);
	if (status != 0) // parsing failed
		return status;

	ui.twContent->insertTopLevelItem(0, rootItem);
	ui.twContent->expandAll();
	ui.twContent->resizeColumnToContents(0);
	ui.twContent->resizeColumnToContents(3);

	return 0;
}

/*!
	updates the selected data set of a HDF5 file when a new tree widget item is selected
*/
void HDF5OptionsWidget::hdf5TreeWidgetSelectionChanged() {
	DEBUG(Q_FUNC_INFO);
	auto items = ui.twContent->selectedItems();
	QDEBUG("SELECTED ITEMS =" << items);

	if (items.isEmpty())
		return;

	QTreeWidgetItem* item = items.first();
	if (item->data(2, Qt::DisplayRole).toString() == i18n("data set"))
		m_fileWidget->refreshPreview();
	else
		DEBUG(Q_FUNC_INFO << ", non data set selected in HDF5 tree widget");
}

/*!
	return list of selected HDF5 item names
	selects first item if nothing is selected
*/
const QStringList HDF5OptionsWidget::selectedNames() const {
	DEBUG(Q_FUNC_INFO);
	QStringList names;

	if (ui.twContent->selectedItems().size() == 0 && ui.twContent->topLevelItem(0)) {
		const auto& child = ui.twContent->topLevelItem(0)->child(0);
		if (child && child->child(0))
			ui.twContent->setCurrentItem(child->child(0));
		else
			ui.twContent->setCurrentItem(child);
	}

	// the data link is saved in the second column
	for (auto* item : ui.twContent->selectedItems())
		names << item->text(1);
	QDEBUG(Q_FUNC_INFO << ", selected names =" << names);

	return names;
}
