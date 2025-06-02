/*
	File                 : GeneralTest.h
	Project              : LabPlot
	Description          : Base class for statistical tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Devanshu Agarwal <agarwaldevanshu8@gmail.com>
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GENERALTEST_H
#define GENERALTEST_H

#include "backend/core/AbstractPart.h"
#include "frontend/statistics/GeneralTestView.h"

#include <QAbstractItemModel>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>

class Spreadsheet;
class Column;
class TableModel;
class QVBoxLayout;

#define RESULT_LINES_COUNT 10

class GeneralTest : public AbstractPart {
	Q_OBJECT

public:
	explicit GeneralTest(const QString& name, const AspectType& type);
	~GeneralTest() override;

	// Structure representing a cell in an HTML table.
	struct HtmlText {
		QString data;
		int level;
		bool isHeader;
		QString tooltip;
		int rowSpanCount;
		int columnSpanCount;
		HtmlText(QVariant cellData, int lvl = 0, bool header = false, QString tip = QString(), int rowSpan = 1, int colSpan = 1) {
			data = cellData.toString();
			level = lvl;
			isHeader = header;
			tooltip = tip;
			rowSpanCount = rowSpan;
			columnSpanCount = colSpan;
		}
	};

	enum GeneralErrorType { ErrorUnqualSize, ErrorEmptyColumn, NoError };

	// Column selection methods.
	void setColumns(const QVector<Column*>&);

	// Getters for test properties.
	QString getTestName() const;
	QString getStatsTable() const;

	QVBoxLayout* getSummaryLayout() const;
	QAbstractItemModel* getInputStatsTableModel() const;

	// Virtual methods.
	QMenu* createContextMenu() override;
	QWidget* view() const override;
	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

public Q_SLOTS:
	void clearInputStats();

Q_SIGNALS:
	void changed();
	void requestProjectContextMenu(QMenu*);

protected:
	QVector<Column*> m_columns;

	QString m_currentTestName;
	QString m_statsTable;

	QVBoxLayout* m_summaryLayout{nullptr};
	QLabel* m_resultLabels[RESULT_LINES_COUNT];

	TableModel* m_inputStatsTableModel;

	// Helper functions with renamed versions.
	int extractTestType(int test);
	int extractTestSubtype(int test);

	QString formatRoundedValue(QVariant number, int precision = 3);
	double calculateSum(const Column* column, int N = -1);
	double calculateSumOfSquares(const Column* column, int N = -1);
	void countUniquePartitions(Column* column, int& np, int& totalRows);

	GeneralErrorType computeColumnStats(const Column* column, int& count, double& sum, double& mean, double& stdDev);
	GeneralErrorType computePairedColumnStats(const Column* column1, const Column* column2, int& count, double& sum, double& mean, double& stdDev);
	GeneralErrorType computeCategoricalStats(Column* column1,
											 Column* column2,
											 int n[],
											 double sum[],
											 double mean[],
											 double stdDev[],
											 QMap<QString, int>& colName,
											 const int& np,
											 const int& totalRows);

	QString buildHtmlTable(int row, int column, QVariant* data);
	QString buildHtmlTableFromCells(const QList<HtmlText*>& cells);

	QString formatHtmlLine(const QString& msg, const QString& color = QLatin1String("black"));
	void displayLine(const int& index, const QString& msg, const QString& color = QLatin1String("black"));
	void displayTooltip(const int& index, const QString& msg);
	void displayError(const QString& errorMsg);

	mutable GeneralTestView* m_view{nullptr};
};

#endif // GENERALTEST_H
