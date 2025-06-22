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

class Column;
class GeneralTestView;

#define RESULT_LINES_COUNT 10

class GeneralTest : public AbstractPart {
	Q_OBJECT

public:
	explicit GeneralTest(const QString& name, const AspectType& type);
	~GeneralTest() override;

	enum GeneralErrorType { ErrorUnqualSize, ErrorEmptyColumn, NoError };

	void setColumns(const QVector<Column*>&);
	QString resultHtml() const;

	QMenu* createContextMenu() override;
	QWidget* view() const override;
	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

Q_SIGNALS:
	void changed();
	void requestProjectContextMenu(QMenu*);

private:
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

	mutable GeneralTestView* m_view{nullptr};

protected:
	QString m_result; // result html text
	QVector<Column*> m_columns;

	void addResultTitle(const QString&);
	void addResultSection(const QString&);
	void addResultLine(const QString& name, const QString& value);
	void addResultLine(const QString& name, double value);
	void addResultLine(const QString& name);
};

#endif // GENERALTEST_H
