/***************************************************************************
File                 : ExcelOptionsWidget.h
Project              : LabPlot
Description          : Widget providing options for the import of Excel (xlsx) data
--------------------------------------------------------------------
Copyright            : (C) 2021 by Fabian Kristof (fkristofszabolcs@gmail.com)
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

#ifndef EXCELOPTIONSWIDGET_H
#define EXCELOPTIONSWIDGET_H

#include "ui_exceloptionswidget.h"

#include <QWidget>
#include <QMap>
#include <QPair>
#include <QString>
#include <QVector>

class ExcelFilter;
class ImportFileWidget;

class QStringList;

class ExcelOptionsWidget : public QWidget
{
	Q_OBJECT

public:
    explicit ExcelOptionsWidget(QWidget*, ImportFileWidget*);
	~ExcelOptionsWidget();

    void updateContent(ExcelFilter* filter, const QString& fileName);
    QTableWidget* previewWidget() const { return ui.twPreview; }
    QStringList selectedExcelRegionNames() const;
    QVector<QStringList> previewString() const;
Q_SIGNALS:
    void enableDataPortionSelection(bool enable);
    void dataRegionSelectionChangedSignal();

private Q_SLOTS:
    void dataRegionSelectionChanged();
private:
    QMap<QPair<QString, int>,bool> m_regionIsPossibleToImportToMatrix;
    Ui::ExcelOptionsWidget ui;
    ImportFileWidget* m_fileWidget {nullptr};
    QVector<QStringList> m_previewString;
};

#endif // EXCELOPTIONSWIDGET_H
