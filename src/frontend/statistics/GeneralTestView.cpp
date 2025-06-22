/***************************************************************************
	File                 : GeneralTestView.cpp
	Project              : LabPlot
	Description          : View class for Hypothesis Tests' Table
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 205 Alexander Semke >alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "GeneralTestView.h"
#include "backend/statistics/GeneralTest.h"

#include <QFile>
#include <QLabel>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QVBoxLayout>

#include <KLocalizedString>

/*!
	\class GeneralTestView
	\brief View class for Hypothesis Test
	\ingroup frontend
*/

GeneralTestView::GeneralTestView(GeneralTest* test) : QWidget()
	, m_test(test) {

	auto* layout = new QVBoxLayout(this);
	auto* m_resultLabel = new QLabel(this);
	layout->addWidget(m_resultLabel);

	auto* spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
	layout->addItem(spacer);

	connect(m_test, &GeneralTest::changed, [=]() {
		m_resultLabel->setText(m_test->resultHtml());
	});
}

GeneralTestView::~GeneralTestView() {
}

void GeneralTestView::initializeActions() {
	// Initialize actions here.
}

void GeneralTestView::initializeMenus() {
	// Initialize menus here.
}

void GeneralTestView::setupConnections() {
	// Additional connections can be set up here.
}

bool GeneralTestView::exportDisplay() {
	return true;
}

bool GeneralTestView::executePrintView() {
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, this);
	dlg->setWindowTitle(i18nc("@title:window", "Print Spreadsheet"));

	bool result = false;
	if ((result = (dlg->exec() == QDialog::Accepted)))
		renderToPrinter(&printer);
	delete dlg;
	return result;
}

bool GeneralTestView::previewPrintView() {
	auto* dlg = new QPrintPreviewDialog(this);
	connect(dlg, &QPrintPreviewDialog::paintRequested, this, &GeneralTestView::renderToPrinter);
	return dlg->exec();
}

void GeneralTestView::renderToPrinter(QPrinter* printer) const {
	// WAIT_CURSOR;
	// QPainter painter(printer);
	// RESET_CURSOR;
}

void GeneralTestView::exportDataToFile(const QString& path, bool exportHeader, const QString& separator, QLocale::Language language) const {
	Q_UNUSED(exportHeader);
	Q_UNUSED(separator);
	Q_UNUSED(language);
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;
	// File export logic goes here.
}

void GeneralTestView::exportDataToLaTeX(const QString& path,
										bool exportHeaders,
										bool gridLines,
										bool captions,
										bool latexHeaders,
										bool skipEmptyRows,
										bool exportEntire) const {
	Q_UNUSED(exportHeaders);
	Q_UNUSED(gridLines);
	Q_UNUSED(captions);
	Q_UNUSED(latexHeaders);
	Q_UNUSED(skipEmptyRows);
	Q_UNUSED(exportEntire);
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;
	// LaTeX export logic goes here.
}
