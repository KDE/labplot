/***************************************************************************
    File                 : HypothesisTest.h
    Project              : LabPlot
    Description          : Doing T-Test on data provided
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Alexander Semke(alexander.semke@web.de)

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

#ifndef HYPOTHESISTEST_H
#define HYPOTHESISTEST_H
#include <QVector>

#include "backend/core/AbstractPart.h"
#include "backend/lib/macros.h"

class HypothesisTestPrivate;
class HypothesisTestView;
class Spreadsheet;
class QAbstractItemModel;
class QString;
class Column;

class HypothesisTest : public AbstractPart {
    Q_OBJECT

public:
    explicit HypothesisTest(const QString& name);
    ~HypothesisTest() override;

    enum DataSourceType {DataSourceSpreadsheet, DataSourceDatabase};

    QAbstractItemModel* dataModel();
    QAbstractItemModel *horizontalHeaderModel();
    QString testName();
    void setDataSourceSpreadsheet(Spreadsheet* spreadsheet);
    void setColumns(QVector<Column*> cols);
    void setColumns(QStringList cols);
    void performTwoSampleTTest();
    DataSourceType dataSourceType() const;


    //virtual methods
//    QIcon icon() const override;
    QMenu* createContextMenu() override;
    QWidget* view() const override;

    bool exportView() const override;
    bool printView() override;
    bool printPreview() const override;

    void save(QXmlStreamWriter*) const override;
    bool load(XmlStreamReader*, bool preview) override;

    Spreadsheet* dataSourceSpreadsheet() const;

    void setDataSourceType(DataSourceType type);
    QStringList allColumns();

private:
    HypothesisTestPrivate* const d;
    mutable HypothesisTestView* m_view{nullptr};
    friend class HypothesisTestPrivate;

signals:
    void changed();
    void requestProjectContextMenu(QMenu*);
    void dataSourceTypeChanged(HypothesisTest::DataSourceType);
    void dataSourceSpreadsheetChanged(Spreadsheet*);
};

#endif // HypothesisTest_H
