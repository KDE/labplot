/***************************************************************************
	File                 : HypothesisTestView.h
	Project              : LabPlot
	Description          : View class for statistical tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke >alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef HYPOTHESISTESTVIEW_H
#define HYPOTHESISTESTVIEW_H

#include <QWidget>

class HypothesisTest;
class QTextEdit;
class QPrinter;

class HypothesisTestView : public QWidget {
	Q_OBJECT

public:
	explicit HypothesisTestView(HypothesisTest*);
	~HypothesisTestView() override;

public Q_SLOTS:
	void print(QPrinter*) const;

private:
	HypothesisTest* m_test{nullptr};
	QTextEdit* m_textEdit{nullptr};
};

#endif // HYPOTHESISTESTVIEW_H
