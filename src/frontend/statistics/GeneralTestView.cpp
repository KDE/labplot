/***************************************************************************
	File                 : GeneralTestView.cpp
	Project              : LabPlot
	Description          : View class for statistical tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke >alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "GeneralTestView.h"
#include "backend/statistics/GeneralTest.h"

#include <QPrinter>
#include <QTextEdit>
#include <QVBoxLayout>

/*!
	\class GeneralTestView
	\brief View class for statistical tests showing the HTML formatted results.
	\ingroup frontend
*/
GeneralTestView::GeneralTestView(GeneralTest* test) : QWidget()
	, m_test(test) {

	auto* layout = new QVBoxLayout(this);
	m_textEdit = new QTextEdit(this);
	m_textEdit->setReadOnly(true);
	layout->addWidget(m_textEdit);

	// show the initial/default result and connect to the changes to update it
	m_textEdit->setText(m_test->resultHtml());
	connect(m_test, &GeneralTest::changed, [=]() {
		m_textEdit->setText(m_test->resultHtml());
	});
}

GeneralTestView::~GeneralTestView() = default;

void GeneralTestView::print(QPrinter* printer) const {
	m_textEdit->print(printer);
}
