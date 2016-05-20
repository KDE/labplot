/***************************************************************************
File                 : FITSHeaderEditWidget.cpp
Project              : LabPlot
Description          : Widget for listing/editing FITS header keywords
--------------------------------------------------------------------
Copyright            : (C) 2016 by Fabian Kristof (fkristofszabolcs@gmail.com)
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

#include "FITSHeaderEditWidget.h"
#include "backend/datasources/filters/FITSFilter.h"
#include <QMenu>
#include <QTableWidget>
#include <QFileDialog>
#include <QContextMenuEvent>
#include <QTreeWidgetItem>
#include <QDebug>

FITSHeaderEditWidget::FITSHeaderEditWidget(AbstractDataSource *dataSource, QWidget *parent) :
    QWidget(parent) {
    ui.setupUi(this);
    setWindowTitle(i18n("FITS header edit"));
    initActions();
    initContextMenu();
    fitsFilter = new FITSFilter();
    ui.twKeywordsTable->setColumnCount(3);
    ui.twExtensions->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.twExtensions->headerItem()->setText(0, "Extensions");
    ui.twKeywordsTable->setHorizontalHeaderItem(0, new QTableWidgetItem(i18n("Key")));
    ui.twKeywordsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(i18n("Value")));
    ui.twKeywordsTable->setHorizontalHeaderItem(2, new QTableWidgetItem(i18n("Comment")));

    ui.twKeywordsTable->installEventFilter(this);
    connect(ui.pbOpenFile, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(ui.pbSaveFile, SIGNAL(clicked()), this, SLOT(saveFile()));
    if (dataSource != NULL) {
        ui.gbOptions->hide();
    }

    connect(ui.twExtensions, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(fillTable(QTreeWidgetItem*, int)));
}

FITSHeaderEditWidget::~FITSHeaderEditWidget() {
}

void FITSHeaderEditWidget::fillTable(QTreeWidgetItem *item, int col) {
    WAIT_CURSOR;
    QString itemText = item->text(col);
    QString selectedExtension;
    if (itemText.contains("IMAGE #")) {
        //filter - find IMAGE #
    } else if (itemText.contains("ASCII_TBL #")) {

    } else if (itemText.contains("BINARY_TBL #")) {

    } else if (!itemText.compare("Primary header")) {
        if (item->parent() != 0) {
            qDebug() << item->parent()->text(col);
            selectedExtension = item->parent()->text(col);
        }
    } else {
        if (item->parent() != 0) {
            selectedExtension = item->parent()->text(col) +"["+ item->text(col)+"]";
        }
    }
    if (!selectedExtension.isEmpty())
        fitsFilter->parseHeader(selectedExtension, ui.twKeywordsTable);
    RESET_CURSOR;
}

void FITSHeaderEditWidget::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this,i18n("Open FITS file"), QDir::homePath(),
                                                    "FITS files (*.fits)");
    if (!fileName.isEmpty()) {
        WAIT_CURSOR;
        QTreeWidgetItem* root = ui.twExtensions->invisibleRootItem();
        int childCount = root->childCount();
        bool opened = false;
        for (int i = 0; i < childCount; ++i) {
            if(root->child(i)->text(0) == fileName) {
                opened = true;
                break;
            }
        }
        if (!opened) {
            fitsFilter->parseExtensions(fileName, root);
            foreach (QTreeWidgetItem* item, ui.twExtensions->selectedItems()) {
                item->setSelected(false);
            }
            root->child(root->childCount()-1)->setExpanded(true);
            root->child(root->childCount()-1)->child(0)->setSelected(true);
        }
        fitsFilter->parseHeader(root->child(root->childCount()-1)->text(0), ui.twKeywordsTable);
        RESET_CURSOR;
    }
}

void FITSHeaderEditWidget::saveFile() {
}

void FITSHeaderEditWidget::initActions() {
    action_add_keyword = new QAction(i18n("Add new keyword"), this);
    action_remove_keyword = new QAction(i18n("Remove keyword"), this);
    action_update_keyword = new QAction(i18n("Update keyword"), this);
}

void FITSHeaderEditWidget::initContextMenu() {
    m_KeywordActionsMenu = new QMenu(this);
    m_KeywordActionsMenu->addAction(action_add_keyword);
    m_KeywordActionsMenu->addAction(action_remove_keyword);
    m_KeywordActionsMenu->addAction(action_update_keyword);
}

void FITSHeaderEditWidget::addKeyword() {
}

void FITSHeaderEditWidget::removeKeyword() {
}

void FITSHeaderEditWidget::updateKeyword() {
}

bool FITSHeaderEditWidget::eventFilter(QObject * watched, QEvent * event) {
    if (event->type() == QEvent::ContextMenu) {
        QContextMenuEvent *cm_event = static_cast<QContextMenuEvent*>(event);
        QPoint global_pos = cm_event->globalPos();
        if (watched == ui.twKeywordsTable) {
            m_KeywordActionsMenu->exec(global_pos);
        }
        else
            return QWidget::eventFilter(watched, event);
        return true;
    } else
        return QWidget::eventFilter(watched, event);
}
