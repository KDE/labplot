/***************************************************************************
    File                 : GeneralTest.h
    Project              : LabPlot
    Description          : Doing Hypothesis-Test on data provided
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Devanshu Agarwal(agarwaldevanshu8@gmail.com)

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

#ifndef GENERALTEST_H
#define GENERALTEST_H

#include "backend/core/AbstractPart.h"
#include "kdefrontend/generalTest/GeneralTestView.h"

class Spreadsheet;
class QString;
class Column;
class QVBoxLayout;
class QLabel;
class QAbstractItemModel;
class MyTableModel;

class GeneralTest : public AbstractPart {
	Q_OBJECT

public:
	explicit GeneralTest(const QString& name, const AspectType& type);
	~GeneralTest() override;

	struct HtmlCell {
		QString data;
		int level;
		bool isHeader;
		QString tooltip;
		int rowSpanCount;
		int columnSpanCount;
		HtmlCell(QVariant data, int level = 0, bool isHeader = false, QString tooltip = QString(), int rowSpanCount = 1, int columnSpanCount = 1) {
			this->data = data.toString();
			this->level = level;
			this->isHeader = isHeader;
			this->tooltip = tooltip;
			this->rowSpanCount = rowSpanCount;
			this->columnSpanCount = columnSpanCount;
		}
	};

	enum GeneralErrorType {ErrorUnqualSize, ErrorEmptyColumn, NoError};

	void setDataSourceSpreadsheet(Spreadsheet* spreadsheet);
	Spreadsheet* dataSourceSpreadsheet() const;

	void setColumns(const QVector<Column*>& cols);
	void setColumns(QStringList cols);
	QStringList allColumns();
	QString testName();
	QString statsTable();

	QVBoxLayout* summaryLayout();
	QAbstractItemModel* inputStatsTableModel();

	//virtual methods
	//    QIcon icon() const override;
	QMenu* createContextMenu() override;
//    QWidget* view() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

public Q_SLOTS:
	void clearInputStatsTable();

Q_SIGNALS:
	void changed();
	void requestProjectContextMenu(QMenu*);
	void dataSourceSpreadsheetChanged(Spreadsheet*);

protected:
	Spreadsheet* m_dataSourceSpreadsheet{nullptr};
	QVector<Column*> m_columns;
	QStringList m_allColumns;

	QString m_currTestName;
	QString m_statsTable;

	QVBoxLayout* m_summaryLayout{nullptr};
	QLabel* m_resultLine[RESULTLINESCOUNT];

	MyTableModel* m_inputStatsTableModel;

	int testType(int test);
	int testSubtype(int test);

	QString round(QVariant number, int precision = 3);
	double findSum(const Column* column, int N = -1);
	double findSumSq(const Column* column, int N = -1);
	void countPartitions(Column* column, int& np, int& totalRows);

	GeneralErrorType findStats(const Column* column,int& count, double& sum, double& mean, double& std);
	GeneralErrorType findStatsPaired(const Column* column1, const Column* column2, int& count, double& sum, double& mean, double& std);
	GeneralErrorType findStatsCategorical(Column* column1, Column* column2, int n[], double sum[], double mean[], double std[], QMap<QString, int>& colName, const int& np, const int& totalRows);

	QString getHtmlTable(int row, int column, QVariant* rowMajor);
	QString getHtmlTable3(const QList<HtmlCell*>& rowMajor);

	QString getLine(const QString& msg, const QString& color = QLatin1String("black"));
	void printLine(const int& index, const QString& msg, const QString& color = QLatin1String("black"));
	void printTooltip(const int& index, const QString& msg);
	void printError(const QString& errorMsg);

	mutable GeneralTestView* m_view{nullptr};
};

#endif // GeneralTest_H
