/*
    File                 : DatapickerView.cpp
    Project              : LabPlot
    Description          : View class for Datapicker
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
    SPDX-FileCopyrightText: 2015-2020 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
#include <QTabBar>
#include <QTabWidget>

#include <KLocalizedString>

/*!
    \class DatapickerView
    \brief View class for Datapicker

    \ingroup commonfrontend
 */
DatapickerView::DatapickerView(Datapicker* datapicker) : QWidget(),
	m_tabWidget(new QTabWidget(this)),
	m_datapicker(datapicker) {

	m_tabWidget->setTabPosition(QTabWidget::South);
	m_tabWidget->setTabShape(QTabWidget::Rounded);
//     m_tabWidget->setMovable(true);
	m_tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	m_tabWidget->setMinimumSize(600, 600);

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_tabWidget);

	//add tab for each children view
	m_initializing = true;
	for (const auto* aspect : m_datapicker->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden)) {
		handleAspectAdded(aspect);
		for (const auto* child : aspect->children<AbstractAspect>()) {
			handleAspectAdded(child);
		}
	}
	m_initializing = false;

	//SIGNALs/SLOTs
	connect(m_datapicker, &Datapicker::aspectDescriptionChanged, this, &DatapickerView::handleDescriptionChanged);
	connect(m_datapicker, &Datapicker::aspectAdded, this, &DatapickerView::handleAspectAdded);
	connect(m_datapicker, &Datapicker::aspectAboutToBeRemoved, this, &DatapickerView::handleAspectAboutToBeRemoved);
	connect(m_datapicker, &Datapicker::datapickerItemSelected, this, &DatapickerView::itemSelected);

	connect(m_tabWidget, &QTabWidget::currentChanged, this, &DatapickerView::tabChanged);
	connect(m_tabWidget, &QTabWidget::customContextMenuRequested, this, &DatapickerView::showTabContextMenu);
	connect(m_tabWidget->tabBar(), &QTabBar::tabMoved, this, &DatapickerView::tabMoved);
}

DatapickerView::~DatapickerView() {
	//delete all children views here, its own view will be deleted in ~AbstractPart()
	for (const auto* aspect : m_datapicker->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden)) {
		for (const auto* child : aspect->children<AbstractAspect>()) {
			const auto* part = dynamic_cast<const AbstractPart*>(child);
			if (part)
				part->deleteView();
		}
		const auto* part = dynamic_cast<const AbstractPart*>(aspect);
		if (part)
			part->deleteView();
	}
}

void DatapickerView::fillToolBar(QToolBar* toolBar) {
	auto* view = static_cast<DatapickerImageView*>(m_datapicker->image()->view());
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

	if (index == -1)
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
	auto* aspect = m_datapicker->child<AbstractAspect>(m_tabWidget->currentIndex(), AbstractAspect::ChildIndexFlag::IncludeHidden);
	auto* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
	if (spreadsheet) {
		menu = spreadsheet->createContextMenu();
	} else {
		auto* image = dynamic_cast<DatapickerImage*>(aspect);
		if (image)
			menu = image->createContextMenu();
	}

	if (menu)
		menu->exec(m_tabWidget->mapToGlobal(point));
}

void DatapickerView::handleDescriptionChanged(const AbstractAspect* aspect) {
	if (aspect == m_datapicker)
		return;

	//determine the child that was changed and adjust the name of the corresponding tab widget
	int index = -1;
	QString name;
	if (aspect->parentAspect() == m_datapicker) {
		//datapicker curve was renamed
		index = m_datapicker->indexOfChild<AbstractAspect>(aspect, AbstractAspect::ChildIndexFlag::IncludeHidden);
		if (index != -1)
			name = aspect->name() + ": " + aspect->children<Spreadsheet>().constFirst()->name();
	} else {
		//data spreadsheet was renamed or one of its columns, which is not relevant here
		index = m_datapicker->indexOfChild<AbstractAspect>(aspect->parentAspect(), AbstractAspect::ChildIndexFlag::IncludeHidden);
		if (index != -1)
			name = aspect->parentAspect()->name() + ": " + aspect->name();
	}

	if (index != -1)
		m_tabWidget->setTabText(index, name);
}

void DatapickerView::handleAspectAdded(const AbstractAspect* aspect) {
	int index;
	QString name;
	const AbstractPart* part = dynamic_cast<const DatapickerImage*>(aspect);
	if (part) {
		index = 0;
		name = aspect->name();
	} else if (dynamic_cast<const DatapickerCurve*>(aspect)) {
		index = m_datapicker->indexOfChild<AbstractAspect>(aspect, AbstractAspect::ChildIndexFlag::IncludeHidden);
		const Spreadsheet* spreadsheet = static_cast<const Spreadsheet*>(aspect->child<AbstractAspect>(0));
		part = spreadsheet;
		name = aspect->name() + ": " + spreadsheet->name();
	} else
		return;

	m_tabWidget->insertTab(index, part->view(), name);
	m_tabWidget->setTabIcon(m_tabWidget->count(), aspect->icon());
}

void DatapickerView::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const auto* curve = dynamic_cast<const DatapickerCurve*>(aspect);
	if (curve) {
		int index = m_datapicker->indexOfChild<AbstractAspect>(aspect, AbstractAspect::ChildIndexFlag::IncludeHidden);
		m_tabWidget->removeTab(index);
	}
}
