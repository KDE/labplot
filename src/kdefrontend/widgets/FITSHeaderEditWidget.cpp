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
    initActions();
    connectActions();
    initContextMenu();
    m_fitsFilter = new FITSFilter();
    ui.twKeywordsTable->setColumnCount(3);
    ui.twExtensions->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.twExtensions->headerItem()->setText(0, i18n("Extensions"));
    ui.twKeywordsTable->setHorizontalHeaderItem(0, new QTableWidgetItem(i18n("Key")));
    ui.twKeywordsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(i18n("Value")));
    ui.twKeywordsTable->setHorizontalHeaderItem(2, new QTableWidgetItem(i18n("Comment")));
    ui.twKeywordsTable->installEventFilter(this);

    if (dataSource != NULL) {
        ui.pbOpenFile->hide();
    }
    connect(ui.pbOpenFile, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(ui.twExtensions, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(fillTable(QTreeWidgetItem*, int)));
}

FITSHeaderEditWidget::~FITSHeaderEditWidget() {
    delete m_fitsFilter;
}

void FITSHeaderEditWidget::fillTable() {
    if (!m_extensionDatas.contains(m_seletedExtension)) {
        m_extensionDatas[m_seletedExtension].keywords = m_fitsFilter->chduKeywords(m_seletedExtension);
        m_fitsFilter->parseHeader(m_seletedExtension, ui.twKeywordsTable);
    } else {
        QList<FITSFilter::Keyword> keywords = m_extensionDatas[m_seletedExtension].keywords;
        foreach (const FITSFilter::Keyword& key, m_extensionDatas[m_seletedExtension].updates.newKeywords) {
            keywords.append(key);
        }

        ui.twKeywordsTable->setRowCount(keywords.size());
        QTableWidgetItem* item;
        for (int i = 0; i < keywords.size(); ++i) {
            item = new QTableWidgetItem(keywords.at(i).key);
            item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            ui.twKeywordsTable->setItem(i, 0, item );

            item = new QTableWidgetItem(keywords.at(i).value);
            item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            ui.twKeywordsTable->setItem(i, 1, item );

            item = new QTableWidgetItem(keywords.at(i).comment);
            item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            ui.twKeywordsTable->setItem(i, 2, item );
        }
        ui.twKeywordsTable->resizeColumnsToContents();
    }
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
        if (!(m_seletedExtension == selectedExtension)) {
            m_seletedExtension = selectedExtension;
            fillTable();
        }
    }
    //TODO added keywords/removed keywords
    RESET_CURSOR;
}

void FITSHeaderEditWidget::openFile() {

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
        m_fitsFilter->parseExtensions(fileName, ui.twExtensions);
        ui.twExtensions->resizeColumnToContents(0);
    }
    m_seletedExtension = root->child(root->childCount()-1)->text(0);
    fillTable();
    RESET_CURSOR;
}

void FITSHeaderEditWidget::save() {
    foreach (const QString& fileName, m_extensionDatas.keys()) {
        qDebug() << "Saving " << fileName;
        if (m_extensionDatas[fileName].updates.newKeywords.size() > 0) {
            m_fitsFilter->addNewKeyword(fileName,m_extensionDatas[fileName].updates.newKeywords);
        }
        if (m_extensionDatas[fileName].updates.removedKeywords.size() > 0) {
            m_fitsFilter->deleteKeyword(fileName, m_extensionDatas[fileName].updates.removedKeywords);
        }
        //TODO update
    }
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
        FITSFilter::Keyword newKeyWord = newKeywordDialog->newKeyword();
        QList<FITSFilter::Keyword> currentKeywords = m_extensionDatas[m_seletedExtension].keywords;

        foreach (const FITSFilter::Keyword& keyword, currentKeywords) {
            if (keyword.operator==(newKeyWord)) {
                KMessageBox::information(this, i18n("Cannot add keyword, keyword already added"), i18n("Cannot add keyword"));
                return;
            }
        }

        foreach (const FITSFilter::Keyword& keyword, m_extensionDatas[m_seletedExtension].updates.newKeywords) {
            if (keyword.operator==(newKeyWord)) {
                KMessageBox::information(this, i18n("Cannot add keyword, keyword already added"), i18n("Cannot add keyword"));
                return;
            }
        }

        foreach (const QString& keyword, mandatoryKeywords()) {
            if (!keyword.compare(newKeyWord.key)) {
                KMessageBox::information(this, i18n("Cannot add mandatory keyword, they are already present"),
                                         i18n("Cannot add keyword"));
                return;
            }
        }

        m_extensionDatas[m_seletedExtension].updates.newKeywords.append(newKeyWord);

        qDebug() << "Updates====";
        qDebug() << "New Keywords: ";
        foreach (const FITSFilter::Keyword& keyw, m_extensionDatas[m_seletedExtension].updates.newKeywords) {
            qDebug() << keyw.key << " " << keyw.value << " " << keyw.comment;
        }
        qDebug() << "Remove Keywords: ";
        foreach (const FITSFilter::Keyword& keyw, m_extensionDatas[m_seletedExtension].updates.removedKeywords) {
            qDebug() << keyw.key << " " << keyw.value << " " << keyw.comment;
        }

        qDebug() << "Updated Keywords: ";
        foreach (const FITSFilter::Keyword& keyw, m_extensionDatas[m_seletedExtension].updates.updatedKeywords) {
            qDebug() << keyw.key << " " << keyw.value << " " << keyw.comment;
        }

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
    delete newKeywordDialog;
}

void FITSHeaderEditWidget::removeKeyword() {
    int removeKeyWordMb = KMessageBox::questionYesNo(this,"Are you sure you want to delete this keyword?",
                                                     "Confirm deletion");
    if (removeKeyWordMb == KMessageBox::Yes) {
        int row = ui.twKeywordsTable->currentRow();
        QString key = ui.twKeywordsTable->item(row, 0)->text();

        bool remove = true;
        foreach (const QString& k, mandatoryKeywords()) {
            if (!k.compare(key)) {
                remove = false;
                break;
            }
        }

        if (remove) {
            FITSFilter::Keyword toRemove;
            toRemove.key = key;
            toRemove.value = ui.twKeywordsTable->item(row, 1)->text();
            toRemove.comment = ui.twKeywordsTable->item(row, 2)->text();
            ui.twKeywordsTable->removeRow(row);

            m_extensionDatas[m_seletedExtension].keywords.removeAt(row);
            m_extensionDatas[m_seletedExtension].updates.removedKeywords.append(toRemove);
        } else {
            KMessageBox::information(this, i18n("Cannot remove mandatory keyword!"),i18n("Removing keyword"));
        }
    }
}

void FITSHeaderEditWidget::updateKeyword() {
}

QList<QString> FITSHeaderEditWidget::mandatoryKeywords() const {
    QList<QString> mandatoryKeywords;
    const QTreeWidgetItem* currentItem = ui.twExtensions->currentItem();
    if (currentItem->parent()->text(0).compare(QLatin1String("Images"))) {
        mandatoryKeywords = FITSFilter::mandatoryImageExtensionKeywords();
    } else {
        mandatoryKeywords = FITSFilter::mandatoryTableExtensionKeywords();
    }
    return mandatoryKeywords;
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
