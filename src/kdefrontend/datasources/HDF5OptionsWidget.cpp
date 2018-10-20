/***************************************************************************
File                 : HDF5OptionsWidget.cpp
Project              : LabPlot
Description          : widget providing options for the import of HDF5 data
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
#include "HDF5OptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/datasources/filters/HDF5Filter.h"
#include "backend/lib/macros.h"

 /*!
	\class HDF5OptionsWidget
	\brief Widget providing options for the import of HDF5 data

	\ingroup kdefrontend
 */
HDF5OptionsWidget::HDF5OptionsWidget(QWidget* parent, ImportFileWidget* fileWidget) : QWidget(parent), m_fileWidget(fileWidget) {
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

	ui.bRefreshPreview->setIcon( QIcon::fromTheme("view-refresh") );

	connect( ui.twContent, SIGNAL(itemSelectionChanged()), SLOT(hdf5TreeWidgetSelectionChanged()) );
	connect( ui.bRefreshPreview, SIGNAL(clicked()), fileWidget, SLOT(refreshPreview()) );
}

void HDF5OptionsWidget::clear() {
	ui.twContent->clear();
	ui.twPreview->clear();
}

void HDF5OptionsWidget::updateContent(HDF5Filter *filter, const QString& fileName) {
	ui.twContent->clear();
	QTreeWidgetItem *rootItem = ui.twContent->invisibleRootItem();
	filter->parse(fileName, rootItem);

	ui.twContent->insertTopLevelItem(0, rootItem);
	ui.twContent->expandAll();
	ui.twContent->resizeColumnToContents(0);
	ui.twContent->resizeColumnToContents(3);
}

/*!
	updates the selected data set of a HDF5 file when a new tree widget item is selected
*/
void HDF5OptionsWidget::hdf5TreeWidgetSelectionChanged() {
	DEBUG("hdf5TreeWidgetSelectionChanged()");
	auto items = ui.twContent->selectedItems();
	QDEBUG("SELECTED ITEMS =" << items);

	if (items.isEmpty())
		return;

	QTreeWidgetItem* item = items.first();
	if (item->data(2, Qt::DisplayRole).toString() == i18n("data set"))
		m_fileWidget->refreshPreview();
	else
		DEBUG("non data set selected in HDF5 tree widget");
}

/*!
	return list of selected HDF5 item names
*/
const QStringList HDF5OptionsWidget::selectedHDF5Names() const {
	QStringList names;

	// the data link is saved in the second column
	for (auto* item : ui.twContent->selectedItems())
		names << item->text(1);

	return names;
}
