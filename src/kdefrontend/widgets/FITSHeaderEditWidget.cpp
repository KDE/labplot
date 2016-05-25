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
    connectActions();
    initContextMenu();
    fitsFilter = new FITSFilter();
    ui.twKeywordsTable->setColumnCount(3);
    ui.twExtensions->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.twExtensions->headerItem()->setText(0, i18n("Extensions"));
    ui.twKeywordsTable->setHorizontalHeaderItem(0, new QTableWidgetItem(i18n("Key")));
    ui.twKeywordsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(i18n("Value")));
    ui.twKeywordsTable->setHorizontalHeaderItem(2, new QTableWidgetItem(i18n("Comment")));
    ui.twKeywordsTable->installEventFilter(this);

    if (dataSource != NULL) {
        ui.gbOptions->hide();
    }
    connect(ui.pbOpenFile, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(ui.pbSaveFile, SIGNAL(clicked()), this, SLOT(saveFile()));
    connect(ui.twExtensions, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(fillTable(QTreeWidgetItem*, int)));
}

FITSHeaderEditWidget::~FITSHeaderEditWidget() {
    delete fitsFilter;
}

void FITSHeaderEditWidget::fillTable(QTreeWidgetItem *item, int col) {
    WAIT_CURSOR;
    QString itemText = item->text(col);
    QString selectedExtension;
    if (itemText.contains(QLatin1String("IMAGE #"))) {
        //filter - find IMAGE #
    } else if (itemText.contains(QLatin1String("ASCII_TBL #"))) {

    } else if (itemText.contains(QLatin1String("BINARY_TBL #"))) {

    } else if (!itemText.compare(QLatin1String("Primary header"))) {
        if (item->parent()->parent() != 0) {
            selectedExtension = item->parent()->parent()->text(col);
        }
    } else {
        if (item->parent() != 0) {
            if (item->parent()->parent() != 0)
                selectedExtension = item->parent()->parent()->text(0) +"["+ item->text(col)+"]";
        }
    }
    if (!selectedExtension.isEmpty()) {
        fitsFilter->parseHeader(selectedExtension, ui.twKeywordsTable);
    }
    RESET_CURSOR;
}

void FITSHeaderEditWidget::openFile() {
    /*QString fileName = QFileDialog::getOpenFileName(this,i18n("Open FITS file"), QDir::homePath(),
                                                    i18n("FITS files (*.fits)"));*/

    KConfigGroup conf(KSharedConfig::openConfig(), "FITSHeaderEditWidget");
    QString dir = conf.readEntry("LastDir", "");
    QString fileName = QFileDialog::getOpenFileName(this, i18n("Open FITS file"), dir,
                                                    i18n("FITS files (*.fits)"));
    if (fileName.isEmpty())
        return;

    int pos = fileName.lastIndexOf(QDir::separator());
    if (pos!=-1) {
        QString newDir = fileName.left(pos);
        if (newDir!=dir)
            conf.writeEntry("LastDir", newDir);
    }

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
        foreach (QTreeWidgetItem* item, ui.twExtensions->selectedItems()) {
            item->setSelected(false);
        }
        fitsFilter->parseExtensions(fileName, root);
        ui.twExtensions->resizeColumnToContents(0);
    }
    fitsFilter->parseHeader(root->child(root->childCount()-1)->text(0), ui.twKeywordsTable);
    RESET_CURSOR;
}

void FITSHeaderEditWidget::saveFile() {
}

void FITSHeaderEditWidget::initActions() {
    action_add_keyword = new QAction(i18n("Add new keyword"), this);
    action_remove_keyword = new QAction(i18n("Remove keyword"), this);
    action_update_keyword = new QAction(i18n("Update keyword"), this);
}

void FITSHeaderEditWidget::connectActions() {
    connect(action_add_keyword, SIGNAL(triggered()), this, SLOT(addKeyword()));
    connect(action_remove_keyword, SIGNAL(triggered()), this, SLOT(removeKeyword()));
    connect(action_update_keyword, SIGNAL(triggered()), this, SLOT(updateKeyword()));
}

void FITSHeaderEditWidget::initContextMenu() {
    m_KeywordActionsMenu = new QMenu(this);
    m_KeywordActionsMenu->addAction(action_add_keyword);
    m_KeywordActionsMenu->addAction(action_remove_keyword);
    m_KeywordActionsMenu->addAction(action_update_keyword);
}

void FITSHeaderEditWidget::addKeyword() {
    FITSHeaderEditNewKeywordDialog* newKeywordDialog = new FITSHeaderEditNewKeywordDialog;

    if (newKeywordDialog->exec() == KDialog::Accepted) {
        //TODO - new keywords (?)
        FITSFilter::Keyword newKeyWord = newKeywordDialog->newKeyword();
        int lastRow = ui.twKeywordsTable->rowCount();
        ui.twKeywordsTable->setRowCount(lastRow + 1);
        QTableWidgetItem* newKeyWordItem = new QTableWidgetItem(newKeyWord.key);
        newKeyWordItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        ui.twKeywordsTable->setItem(lastRow, 0, newKeyWordItem);

        newKeyWordItem = new QTableWidgetItem(newKeyWord.value);
        newKeyWordItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        ui.twKeywordsTable->setItem(lastRow, 1, newKeyWordItem);

        newKeyWordItem = new QTableWidgetItem(newKeyWord.comment);
        newKeyWordItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        ui.twKeywordsTable->setItem(lastRow, 2, newKeyWordItem);
    }
}

void FITSHeaderEditWidget::removeKeyword() {
    int removeKeyWordMb = KMessageBox::questionYesNo(this,"Are you sure you want to delete this keyword?",
                                                     "Confirm deletion");
    if (removeKeyWordMb == KMessageBox::Yes) {
        //TODO removed keywords-  ?
    }
}

void FITSHeaderEditWidget::updateKeyword() {
}

bool FITSHeaderEditWidget::eventFilter(QObject * watched, QEvent * event) {
    if (event->type() == QEvent::ContextMenu) {
        QContextMenuEvent *cm_event = static_cast<QContextMenuEvent*>(event);
        QPoint global_pos = cm_event->globalPos();
        if (watched == ui.twKeywordsTable) {
            if (ui.twExtensions->selectedItems().size() != 0) {
                m_KeywordActionsMenu->exec(global_pos);
            }
        }
        else
            return QWidget::eventFilter(watched, event);
        return true;
    } else
        return QWidget::eventFilter(watched, event);
}
