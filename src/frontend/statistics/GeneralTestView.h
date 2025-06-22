/***************************************************************************
	File                 : GeneralTestView.h
	Project              : LabPlot
	Description          : View class for statistical tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke >alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef GENERALTESTVIEW_H
#define GENERALTESTVIEW_H

#include <QWidget>

class GeneralTest;
class QTextEdit;
class QPrinter;

class GeneralTestView : public QWidget {
	Q_OBJECT

public:
	explicit GeneralTestView(GeneralTest*);
	~GeneralTestView() override;

public Q_SLOTS:
	void print(QPrinter*) const;

private:
	GeneralTest* m_test{nullptr};
	QTextEdit* m_textEdit{nullptr};
};

#endif // GENERALTESTVIEW_H
