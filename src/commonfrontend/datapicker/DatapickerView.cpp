/***************************************************************************
    File                 : DatapickerView.cpp
    Project              : LabPlot
    Description          : View class for Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015-2016 by Alexander Semke (alexander.semke@web.de)

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

#include "DatapickerView.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/datapicker/DatapickerImage.h"
#include "commonfrontend/workbook/WorkbookView.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "commonfrontend/datapicker/DatapickerImageView.h"

#include <QHBoxLayout>
#include <QMenu>
#include <QTabWidget>

#include <KLocalizedString>

/*!
    \class DatapickerView
    \brief View class for Datapicker

    \ingroup commonfrontend
 */
DatapickerView::DatapickerView(Datapicker* datapicker) : QWidget(),
	m_tabWidget(new TabWidget(this)),
	m_datapicker(datapicker),
	lastSelectedIndex(0) {

	m_tabWidget->setTabPosition(QTabWidget::South);
	m_tabWidget->setTabShape(QTabWidget::Rounded);
//     m_tabWidget->setMovable(true);
	m_tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	m_tabWidget->setMinimumSize(600, 600);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(m_tabWidget);

	//add tab for each children view
	m_initializing = true;
	for (const auto* aspect : m_datapicker->children<AbstractAspect>(AbstractAspect::IncludeHidden)) {
		handleAspectAdded(aspect);
		for (const auto* child : aspect->children<AbstractAspect>()) {
			handleAspectAdded(child);
		}
	}
	m_initializing = false;

	//SIGNALs/SLOTs
	connect(m_datapicker, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(handleDescriptionChanged(const AbstractAspect*)));
	connect(m_datapicker, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(m_datapicker, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)), this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
	connect(m_datapicker, SIGNAL(datapickerItemSelected(int)), this, SLOT(itemSelected(int)));

	connect(m_tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));
	connect(m_tabWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTabContextMenu(QPoint)));
	connect(m_tabWidget, SIGNAL(tabMoved(int,int)), this, SLOT(tabMoved(int,int)));
}

DatapickerView::~DatapickerView() {
	//delete all children views here, its own view will be deleted in ~AbstractPart()
	for (const auto* aspect : m_datapicker->children<AbstractAspect>(AbstractAspect::IncludeHidden)) {
		for (const auto* child : aspect->children<AbstractAspect>()) {
			const AbstractPart* part = dynamic_cast<const AbstractPart*>(child);
			if (part)
				part->deleteView();
		}
		const AbstractPart* part = dynamic_cast<const AbstractPart*>(aspect);
		if (part)
			part->deleteView();
	}
}

void DatapickerView::fillToolBar(QToolBar* toolBar) {
	DatapickerImageView* view = dynamic_cast<DatapickerImageView*>(m_datapicker->image()->view());
	view->fillToolBar(toolBar);
}

void DatapickerView::createContextMenu(QMenu* menu) const {
	Q_ASSERT(menu);
	m_datapicker->image()->createContextMenu(menu);
}

int DatapickerView::currentIndex() const {
	return m_tabWidget->currentIndex();
}

//##############################################################################
//#########################  Private slots  ####################################
//##############################################################################
/*!
  called when the current tab was changed. Propagates the selection of \c Spreadsheet
  or of a \c DatapickerImage object to \c Datapicker.
*/
void DatapickerView::tabChanged(int index) {
	if (m_initializing)
		return;

	if (index==-1)
		return;

	m_datapicker->setChildSelectedInView(lastSelectedIndex, false);
	m_datapicker->setChildSelectedInView(index, true);
	lastSelectedIndex = index;
}

void DatapickerView::tabMoved(int from, int to) {
	Q_UNUSED(from);
	Q_UNUSED(to);
	//TODO:
// 	AbstractAspect* aspect = m_datapicker->child<AbstractAspect>(to);
// 	if (aspect) {
// 		m_tabMoving = true;
// 		AbstractAspect* sibling = m_datapicker->child<AbstractAspect>(from);
// 		qDebug()<<"insert: " << to << "  " <<  aspect->name() << ",  " << from << "  " << sibling->name();
// 		aspect->remove();
// 		m_datapicker->insertChildBefore(aspect, sibling);
// 		qDebug()<<"inserted";
// 		m_tabMoving = false;
// 	}
}

void DatapickerView::itemSelected(int index) {
	m_initializing = true;
	m_tabWidget->setCurrentIndex(index);
	m_initializing = false;
}

void DatapickerView::showTabContextMenu(QPoint point) {
	QMenu* menu = nullptr;
	AbstractAspect* aspect = m_datapicker->child<AbstractAspect>(m_tabWidget->currentIndex(), AbstractAspect::IncludeHidden);
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
	if (spreadsheet) {
		menu = spreadsheet->createContextMenu();
	} else {
		DatapickerImage* image = dynamic_cast<DatapickerImage*>(aspect);
		if (image)
			menu = image->createContextMenu();
	}

	if (menu)
		menu->exec(m_tabWidget->mapToGlobal(point));
}

void DatapickerView::handleDescriptionChanged(const AbstractAspect* aspect) {
	int index = -1;
	QString name;
	if (aspect->parentAspect() == m_datapicker) {
		//datapicker curve was renamed
		index= m_datapicker->indexOfChild<AbstractAspect>(aspect, AbstractAspect::IncludeHidden);
		name = aspect->name() + ": " + aspect->children<Spreadsheet>().constFirst()->name();
	} else {
		//data spreadsheet was renamed or one of its columns, which is not relevant here
		index = m_datapicker->indexOfChild<AbstractAspect>(aspect->parentAspect(), AbstractAspect::IncludeHidden);
		name = aspect->parentAspect()->name() + ": " + aspect->name();
	}

	if (index != -1)
		m_tabWidget->setTabText(index, name);
}

void DatapickerView::handleAspectAdded(const AbstractAspect* aspect) {
	int index;
	const AbstractPart* part;
	QString name;
	if (dynamic_cast<const DatapickerImage*>(aspect)) {
		index = 0;
		part = dynamic_cast<const AbstractPart*>(aspect);
		name = aspect->name();
	} else if (dynamic_cast<const DatapickerCurve*>(aspect)) {
		index = m_datapicker->indexOfChild<AbstractAspect>(aspect, AbstractAspect::IncludeHidden);
		const Spreadsheet* spreadsheet = dynamic_cast<const Spreadsheet*>(aspect->child<AbstractAspect>(0));
		Q_ASSERT(spreadsheet);
		part = dynamic_cast<const AbstractPart*>(spreadsheet);
		name = aspect->name() + ": " + spreadsheet->name();
	} else {
		return;
	}

	m_tabWidget->insertTab(index, part->view(), name);
	m_tabWidget->setTabIcon(m_tabWidget->count(), aspect->icon());
}

void DatapickerView::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const DatapickerCurve* curve = dynamic_cast<const DatapickerCurve*>(aspect);
	if (curve) {
		int index = m_datapicker->indexOfChild<AbstractAspect>(aspect, AbstractAspect::IncludeHidden);
		m_tabWidget->removeTab(index);
	}
}
