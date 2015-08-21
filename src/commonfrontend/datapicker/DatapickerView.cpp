/***************************************************************************
    File                 : DatapickerView.cpp
    Project              : LabPlot
    Description          : View class for Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)

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
#include "backend/core/AbstractAspect.h"
#include "backend/core/AbstractPart.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/datapicker/Image.h"
#include "commonfrontend/workbook/WorkbookView.h"

#include <QTabWidget>
#include <QHBoxLayout>
#include <QMenu>
#include <QDebug>

#include <KAction>
#include <KLocale>

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
    m_tabWidget->setMovable(true);
    m_tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tabWidget->setMinimumSize(200, 200);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_tabWidget);

    //add tab for each children view
    m_initializing = true;
    foreach(const AbstractAspect* aspect, m_datapicker->children<AbstractAspect>()){
        handleAspectAdded(aspect);
        foreach(const AbstractAspect* aspect, aspect->children<AbstractAspect>()){
            handleAspectAdded(aspect);
        }
    }
    m_initializing = false;

    //SIGNALs/SLOTs
    connect(m_datapicker, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(handleDescriptionChanged(const AbstractAspect*)));
    connect(m_datapicker, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(handleAspectAdded(const AbstractAspect*)));
    connect(m_datapicker, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)), this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
    connect(m_datapicker, SIGNAL(datapickerItemSelected(int)), this, SLOT(itemSelected(int)));
    connect(m_datapicker, SIGNAL(childAspectAdded(const AbstractAspect*)), this, SLOT(handleAspectAdded(const AbstractAspect*)));

    connect(m_tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));
    connect(m_tabWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTabContextMenu(QPoint)));
    connect(m_tabWidget, SIGNAL(tabMoved(int,int)), this, SLOT(tabMoved(int,int)));
}

DatapickerView::~DatapickerView() {
    //delete all children views here, its own view will be deleted in ~AbstractPart()
    foreach(const AbstractAspect* aspect, m_datapicker->children<AbstractAspect>()) {
        foreach(const AbstractAspect* aspect, aspect->children<AbstractAspect>()) {
            const AbstractPart* part = dynamic_cast<const AbstractPart*>(aspect);
            if (part)
                part->deleteView();
        }
        const AbstractPart* part = dynamic_cast<const AbstractPart*>(aspect);
        if (part)
            part->deleteView();
    }
}

int DatapickerView::currentIndex() const {
    return m_tabWidget->currentIndex();
}

//##############################################################################
//#########################  Private slots  ####################################
//##############################################################################
/*!
  called when the current tab was changed. Propagates the selection of \c Spreadsheet
  or of a \c Image object to \c Datapicker.
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
    m_tabWidget->setCurrentIndex(index);
}

void DatapickerView::showTabContextMenu(const QPoint& point) {
    QMenu* menu = 0;
    AbstractAspect* aspect = m_datapicker->child<AbstractAspect>(m_tabWidget->currentIndex());
    Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
    if (spreadsheet) {
        menu = spreadsheet->createContextMenu();
    } else {
        Image* image = dynamic_cast<Image*>(aspect);
        if (image)
            menu = image->createContextMenu();
    }

    if (menu)
        menu->exec(m_tabWidget->mapToGlobal(point));
}

void DatapickerView::handleDescriptionChanged(const AbstractAspect* aspect) {
    int index;
    if (aspect->parentAspect() == m_datapicker)
        index= m_datapicker->indexOfChild<AbstractAspect>(aspect);
    else
        index = m_datapicker->indexOfChild<AbstractAspect>(aspect->parentAspect());

    if (index != -1 && index<m_tabWidget->count())
        m_tabWidget->setTabText(index, aspect->name());
}

void DatapickerView::handleAspectAdded(const AbstractAspect* aspect) {
    const AbstractPart* part = dynamic_cast<const AbstractPart*>(aspect);
    if (!part)
        return;

    int index;
    if (aspect->parentAspect() == m_datapicker)
        index= m_datapicker->indexOfChild<AbstractAspect>(aspect);
    else
        index = m_datapicker->indexOfChild<AbstractAspect>(aspect->parentAspect());

    m_tabWidget->insertTab(index, part->view(), aspect->name());
    m_tabWidget->setCurrentIndex(index);
    m_tabWidget->setTabIcon(m_tabWidget->count(), aspect->icon());
    this->tabChanged(index);
}

void DatapickerView::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
    int index;
    if (aspect->parentAspect() == m_datapicker) {
        index= m_datapicker->indexOfChild<AbstractAspect>(aspect);
    } else {
        const AbstractPart* part = dynamic_cast<const AbstractPart*>(aspect);
        if (!part)
            return;

        index = m_datapicker->indexOfChild<AbstractAspect>(aspect->parentAspect());
    }

    m_tabWidget->removeTab(index);
}
