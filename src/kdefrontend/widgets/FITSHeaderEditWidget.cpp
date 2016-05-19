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

FITSHeaderEditWidget::FITSHeaderEditWidget(AbstractDataSource *dataSource, QWidget *parent) :
    QWidget(parent) {
    ui.setupUi(this);
    initActions();
    initContextMenu();
    ui.twKeywordsTable->setColumnCount(3);
    ui.twKeywordsTable->setHorizontalHeaderItem(0, new QTableWidgetItem(i18n("Key")));
    ui.twKeywordsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(i18n("Value")));
    ui.twKeywordsTable->setHorizontalHeaderItem(2, new QTableWidgetItem(i18n("Comment")));

    ui.twKeywordsTable->installEventFilter(this);
    connect(ui.pbOpenFile, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(ui.pbSaveFile, SIGNAL(clicked()), this, SLOT(saveFile()));
    if (dataSource != NULL) {
        ui.gbOptions->hide();
    }
}

FITSHeaderEditWidget::~FITSHeaderEditWidget() {
}

void FITSHeaderEditWidget::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this,i18n("Open FITS file"), QDir::homePath(),
                                                    "FITS files (*.fits)");
    FITSFilter* fitsFilter = new FITSFilter();
    WAIT_CURSOR;
    fitsFilter->parseHeader(fileName, ui.twKeywordsTable);
    RESET_CURSOR;
    setWindowTitle(i18n("FITS header edit - ") + QFileInfo(fileName).fileName());

    delete fitsFilter;
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
