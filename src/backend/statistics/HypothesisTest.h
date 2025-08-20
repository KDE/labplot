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

class AbstractColumn;
class HypothesisTestPrivate;
class HypothesisTestView;

extern "C" {
#include "backend/nsl/nsl_statistical_test.h"
}

class HypothesisTest : public AbstractPart {
	Q_OBJECT

public:
	explicit HypothesisTest(const QString&);
	~HypothesisTest() override;

	enum class Test { t_test_one_sample, t_test_two_sample, t_test_two_sample_paired, one_way_anova, mann_whitney_u_test, kruskal_wallis_test, log_rank_test };

	QWidget* view() const override;
	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	// getters and setter
	const QVector<const AbstractColumn*>& dataColumns() const;
	void setDataColumns(const QVector<const AbstractColumn*>&);
	const QVector<QString>& dataColumnPaths() const;

	QString resultHtml() const;

	void setTestMean(double);
	double testMean() const;

	void setSignificanceLevel(double);
	double significanceLevel() const;

	void setTail(nsl_stats_tail_type);
	nsl_stats_tail_type tail() const;

	void setTest(Test);
	Test test() const;

	void recalculate();

	static QPair<int, int> variableCount(Test);
	static int hypothesisCount(Test);
	static QVector<QPair<QString, QString>> hypothesisText(Test);

	typedef HypothesisTestPrivate Private;

Q_SIGNALS:
	void changed();
	void requestProjectContextMenu(QMenu*);

private:
	Q_DECLARE_PRIVATE(HypothesisTest)
	HypothesisTestPrivate* const d_ptr;

protected:
	mutable HypothesisTestView* m_view{nullptr};
};

#endif // HYPOTHESISTEST_H
