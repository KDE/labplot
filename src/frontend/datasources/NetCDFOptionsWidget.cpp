/*
	File                 : NetCDFOptionsWidget.cpp
	Project              : LabPlot
	Description          : widget providing options for the import of NetCDF data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2017 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "NetCDFOptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/datasources/filters/NetCDFFilter.h"
#include "backend/lib/macros.h"

#include <KUrlComboBox>

/*!
   \class NetCDFOptionsWidget
   \brief Widget providing options for the import of NetCDF data

   \ingroup frontend
*/
NetCDFOptionsWidget::NetCDFOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget)
	: QWidget(parent)
	, m_fileWidget(fileWidget) {
	ui.setupUi(parent);

	QStringList headers;
	headers << i18n("Name") << i18n("Type") << i18n("Properties") << i18n("Values");
	ui.twContent->setHeaderLabels(headers);
	// type column is hidden
	ui.twContent->hideColumn(1);
	ui.twContent->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.twContent->setAlternatingRowColors(true);
	ui.twContent->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.bRefreshPreview->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));

	connect(ui.twContent, &QTreeWidget::itemSelectionChanged, this, &NetCDFOptionsWidget::netcdfTreeWidgetSelectionChanged);
	connect(ui.bRefreshPreview, &QPushButton::clicked, fileWidget, &ImportFileWidget::refreshPreview);
}

void NetCDFOptionsWidget::clear() {
	ui.twContent->clear();
	ui.twPreview->clear();
}

void NetCDFOptionsWidget::updateContent(NetCDFFilter* filter, const QString& fileName) {
	ui.twContent->clear();

	QTreeWidgetItem* rootItem = ui.twContent->invisibleRootItem();
	filter->parse(fileName, rootItem);
	ui.twContent->insertTopLevelItem(0, rootItem);
	ui.twContent->expandAll();
	ui.twContent->resizeColumnToContents(0);
	ui.twContent->resizeColumnToContents(2);
}

/*!
	updates the selected var name of a NetCDF file when the tree widget item is selected
*/
void NetCDFOptionsWidget::netcdfTreeWidgetSelectionChanged() {
	QDEBUG(Q_FUNC_INFO << ", SELECTED ITEMS =" << ui.twContent->selectedItems());

	if (ui.twContent->selectedItems().isEmpty())
		return;

	QTreeWidgetItem* item = ui.twContent->selectedItems().first();
	if (item->data(1, Qt::DisplayRole).toString() == QLatin1String("variable"))
		m_fileWidget->refreshPreview();
	else if (item->data(1, Qt::DisplayRole).toString().contains(QLatin1String("attribute"))) {
		// reads attributes (only for preview)
		auto filter = static_cast<NetCDFFilter*>(m_fileWidget->currentFileFilter());
		QString fileName = m_fileWidget->m_cbFileName->currentText();
		QString name = item->data(0, Qt::DisplayRole).toString();
		QString varName = item->data(1, Qt::DisplayRole).toString().split(QLatin1Char(' '))[0];
		QDEBUG("name =" << name << "varName =" << varName);

		QString importedText = filter->readAttribute(fileName, name, varName);
		DEBUG("importedText =" << STDSTRING(importedText));

		QStringList lineStrings = importedText.split(QLatin1Char('\n'));
		int rows = lineStrings.size();
		ui.twPreview->setRowCount(rows);
		ui.twPreview->setColumnCount(0);
		for (int i = 0; i < rows; ++i) {
			QStringList lineString = lineStrings[i].split(QLatin1Char(' '));
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
		DEBUG("non showable object selected in NetCDF tree widget");
}

/*!
	return list of selected NetCDF item names
	selects all items if nothing is selected
*/
const QStringList NetCDFOptionsWidget::selectedNames() const {
	DEBUG(Q_FUNC_INFO);
	QStringList names;

	if (ui.twContent->selectedItems().size() == 0)
		ui.twContent->selectAll();

	for (auto* item : ui.twContent->selectedItems())
		names << item->text(0);
	QDEBUG(Q_FUNC_INFO << ", selected names =" << names);

	return names;
}
