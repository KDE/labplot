/***************************************************************************
	File                 : HypothesisTest.cpp
	Project              : LabPlot
	Description          : Hypothesis Test
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-205 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef HYPOTHESISTEST_H
#define HYPOTHESISTEST_H

#include "backend/core/AbstractPart.h"

#include <QPair>

class Column;
class HypothesisTestView;

extern "C" {
#include "backend/nsl/nsl_statistical_test.h"
}

class HypothesisTest : public AbstractPart {
	Q_OBJECT

public:
	explicit HypothesisTest(const QString& name);
	~HypothesisTest() override;

	enum class Test { t_test_one_sample, t_test_two_sample, t_test_two_sample_paired, one_way_anova, mann_whitney_u_test, kruskal_wallis_test, log_rank_test };

	QMenu* createContextMenu() override;
	QWidget* view() const override;
	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	// getters and setter
	const QVector<Column*>& columns() const;
	virtual void setColumns(const QVector<Column*>&);
	QString resultHtml() const;

	void setTestMean(double mean);
	double testMean() const;

	void setSignificanceLevel(double alpha);
	double significanceLevel() const;

	void setTail(nsl_stats_tail_type);
	nsl_stats_tail_type tail() const;

	void setTest(Test);
	Test test() const;

	static QPair<int, int> variableCount(Test);
	static int hypothesisCount(Test);
	static QVector<QPair<QString, QString>> hypothesisText(Test);

	void recalculate();
	void resetResult();

Q_SIGNALS:
	void changed();
	void requestProjectContextMenu(QMenu*);

private:
	void performOneSampleTTest();
	void performTwoSampleTTest(bool paired);
	void performOneWayANOVATest();
	void performMannWhitneyUTest();
	void performKruskalWallisTest();
	void performLogRankTest();

	void addResultTitle(const QString&);
	void addResultSection(const QString&);
	void addResultLine(const QString& name, const QString& value);
	void addResultLine(const QString& name, double value);
	void addResultLine(const QString& name);

	QString m_result; // result html text
	QVector<Column*> m_columns;
	Test m_test{Test::t_test_one_sample};
	double m_testMean{0.0};
	double m_significanceLevel{0.05};
	nsl_stats_tail_type m_tail{nsl_stats_tail_type_two};
	mutable HypothesisTestView* m_view{nullptr};
};

#endif // HYPOTHESISTEST_H
