/***************************************************************************
File                 : HDFOptionsWidget.cpp
Project              : LabPlot
Description          : widget providing options for the import of HDF data
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
#include "HDFOptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/datasources/filters/HDFFilter.h"
#include "backend/lib/macros.h"

 /*!
	\class HDFOptionsWidget
	\brief Widget providing options for the import of HDF data

	\ingroup kdefrontend
 */

HDFOptionsWidget::HDFOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget) : QWidget(parent), m_fileWidget(fileWidget) {
	ui.setupUi(parent);

	QStringList hdfheaders;
	hdfheaders << i18n("Name") << i18n("Link") << i18n("Type") << i18n("Properties") << i18n("Attributes");
	ui.twContent->setHeaderLabels(hdfheaders);
	ui.twContent->setAlternatingRowColors(true);
	// link and type column are hidden
	ui.twContent->hideColumn(1);
	ui.twContent->hideColumn(2);
	ui.twContent->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.bRefreshPreview->setIcon( QIcon::fromTheme("view-refresh") );

	connect( ui.twContent, SIGNAL(itemSelectionChanged()), SLOT(hdfTreeWidgetSelectionChanged()) );
	connect( ui.bRefreshPreview, SIGNAL(clicked()), SLOT(refreshPreview()) );
}

void HDFOptionsWidget::clear() {
	ui.twContent->clear();
	ui.twPreview->clear();
}

void HDFOptionsWidget::updateContent(HDFFilter *filter, QString fileName) {
	ui.twContent->clear();
	QTreeWidgetItem *rootItem = ui.twContent->invisibleRootItem();
	filter->parse(fileName, rootItem);

	ui.twContent->insertTopLevelItem(0, rootItem);
	ui.twContent->expandAll();
	ui.twContent->resizeColumnToContents(0);
	ui.twContent->resizeColumnToContents(3);
}

/*!
	updates the selected data set of a HDF file when a new tree widget item is selected
*/
void HDFOptionsWidget::hdfTreeWidgetSelectionChanged() {
	DEBUG("hdfTreeWidgetSelectionChanged()");
	auto items = ui.twContent->selectedItems();
	QDEBUG("SELECTED ITEMS =" << items);

	if (items.isEmpty())
		return;

	QTreeWidgetItem* item = items.first();
	if (item->data(2, Qt::DisplayRole).toString() == i18n("data set"))
		m_fileWidget->refreshPreview();
	else
		DEBUG("non data set selected in HDF tree widget");
}

/*!
	return list of selected HDF item names
*/
const QStringList HDFOptionsWidget::selectedHDFNames() const {
	QStringList names;

	// the data link is saved in the second column
	for (auto* item: ui.twContent->selectedItems())
		names << item->text(1);

	return names;
}
