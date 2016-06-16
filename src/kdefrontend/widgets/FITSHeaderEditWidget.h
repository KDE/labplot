/***************************************************************************
File                 : FITSHeaderEditWidget.h
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
#ifndef FITSHEADEREDITWIDGET_H
#define FITSHEADEREDITWIDGET_H

#include <QWidget>
#include <QAction>
#include <QMap>
#include "backend/datasources/AbstractDataSource.h"
#include "ui_fitsheadereditwidget.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "FITSHeaderEditNewKeywordDialog.h"

class FITSHeaderEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FITSHeaderEditWidget(AbstractDataSource* dataSource = 0, QWidget *parent = 0);
    ~FITSHeaderEditWidget();

private:
    Ui::FITSHeaderEditWidget ui;
    QAction* action_remove_keyword;
    QAction* action_add_keyword;
    QAction* action_update_keyword;
    QMenu* m_KeywordActionsMenu;

    struct HeaderUpdate {
        QList<FITSFilter::Keyword> newKeywords;
        QList<FITSFilter::Keyword> updatedKeywords;
        QList<FITSFilter::Keyword> removedKeywords;
    };

    struct ExtensionData {
        HeaderUpdate updates;
        QList<FITSFilter::Keyword> keywords;
    };


    QMap<QString, ExtensionData> m_extensionDatas;
    QString m_seletedExtension;

    FITSFilter* m_fitsFilter;
    AbstractDataSource* dataSource;

    void initActions();
    void initContextMenu();
    void connectActions();
    void fillTable();
    QList<QString> mandatoryKeywords() const;
    bool eventFilter(QObject*, QEvent*);
public slots:
    void save();
private slots:
    void openFile();

    void fillTable(QTreeWidgetItem* item, int col);

    void removeKeyword();
    void addKeyword();
    void updateKeyword(QTableWidgetItem* item);
};

#endif // FITSHEADEREDITWIDGET_H
