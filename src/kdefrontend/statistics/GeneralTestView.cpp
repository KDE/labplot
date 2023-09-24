/***************************************************************************
	File                 : GeneralTestView.cpp
	Project              : LabPlot
	Description          : View class for Hypothesis Tests' Table
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

#include "GeneralTestView.h"
#include "backend/statistics/GeneralTest.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "backend/statistics/MyTextEdit.h"

#include <QFile>
#include <QVBoxLayout>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QStandardItemModel>

#include <KLocalizedString>

/*!
	\class GeneralTestView
	\brief View class for Hypothesis Test

	\ingroup kdefrontend
 */

GeneralTestView::GeneralTestView(GeneralTest* GeneralTest) : QWidget(),
	m_generalTest(GeneralTest),
	m_testName(new QLabel()),
	m_statsTable(new MyTextEdit()),
	m_summaryResults(new QWidget()),
	m_inputStatsWidget(new QWidget()),
	m_labelInputStatsTable(new QLabel()),
	m_inputStatsTable(new QTableView()),
	m_clearInputStats(new QPushButton()) {

	QVBoxLayout * inputStatsLayout = new QVBoxLayout(m_inputStatsWidget);
	inputStatsLayout->addWidget(m_labelInputStatsTable);
	inputStatsLayout->addWidget(m_inputStatsTable);
	inputStatsLayout->addWidget(m_clearInputStats);

	m_statsTable->setReadOnly(true);

	m_testName->setStyleSheet(QLatin1String("background-color: white"));
	m_statsTable->setStyleSheet(QLatin1String("background-color: white"));
	m_summaryResults->setStyleSheet(QLatin1String("QToolTip { color: black; background-color: yellow; border: 0px; }"));
	m_inputStatsWidget->setStyleSheet(QLatin1String("background-color: white"));

	m_testName->hide();
	m_statsTable->hide();
	m_summaryResults->hide();
	m_inputStatsWidget->hide();

	auto* layout = new QVBoxLayout(this);

	m_labelInputStatsTable->setText(QLatin1String("<h3>") + i18n("Statistic Table"));
	m_labelInputStatsTable->setToolTip(i18n("Fill this table with pre-calculated statistic value and then press recalculate") +
									   QLatin1String("<br><br>") +
									   i18n("You can leave one or more columns empty if you feel they are not useful") +
									   QLatin1String("</h3>"));
	m_clearInputStats->setText(i18n("Clear"));
	m_clearInputStats->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

	layout->addWidget(m_inputStatsWidget);
	layout->addWidget(m_testName);
	layout->addWidget(m_statsTable);
	layout->addWidget(m_summaryResults);
	layout->addStretch(1);
	init();
}

GeneralTestView::~GeneralTestView() {
}

void GeneralTestView::init() {
	initActions();
	initMenus();

	m_inputStatsTable->setModel(m_generalTest->inputStatsTableModel());

	m_inputStatsTable->horizontalHeader()->setVisible(false);
	m_inputStatsTable->verticalHeader()->setVisible(false);
	m_inputStatsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	m_statsTable->setMouseTracking(true);
	connect(m_generalTest, &GeneralTest::changed, this, &GeneralTestView::changed);
	connect(m_clearInputStats, &QPushButton::clicked, m_generalTest, &GeneralTest::clearInputStatsTable);
}

void GeneralTestView::initActions() {

}

void GeneralTestView::initMenus() {

}

void GeneralTestView::clearResult() {
	for (int i = 0; i < RESULTLINESCOUNT; i++)
		m_resultLine[i]->clear();
}

void GeneralTestView::connectActions() {

}

void GeneralTestView::fillToolBar(QToolBar* toolBar) {
	Q_UNUSED(toolBar);
}

/*!
 * Populates the menu \c menu with the pivot table and pivot table view relevant actions.
 * The menu is used
 *   - as the context menu in PivotTableView
 *   - as the "pivot table menu" in the main menu-bar (called form MainWin)
 *   - as a part of the pivot table context menu in project explorer
 */
void GeneralTestView::createContextMenu(QMenu* menu) {
	Q_ASSERT(menu);
}

bool GeneralTestView::exportView() {
	return true;
}

bool GeneralTestView::printView() {
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, this);
	dlg->setWindowTitle(i18nc("@title:window", "Print Spreadsheet"));

	bool ret;
	if ((ret = dlg->exec()) == QDialog::Accepted)
		print(&printer);
	delete dlg;
	return ret;
}

bool GeneralTestView::printPreview() {
	QPrintPreviewDialog* dlg = new QPrintPreviewDialog(this);
	connect(dlg, &QPrintPreviewDialog::paintRequested, this, &GeneralTestView::print);
	return dlg->exec();
}

/*!
  prints the complete spreadsheet to \c printer.
 */
void GeneralTestView::print(QPrinter* printer) const {
	WAIT_CURSOR;
	QPainter painter (printer);
	RESET_CURSOR;
}

void GeneralTestView::changed() {
	m_testName->setText(m_generalTest->testName());

	m_testName->show();
	m_summaryResults->show();

	if (m_generalTest->statsTable().isEmpty())
		m_statsTable->hide();
	else {
		m_statsTable->setHtml(m_generalTest->statsTable());
		m_statsTable->show();
	}

	m_summaryResults->setLayout(m_generalTest->summaryLayout());

	if (m_inputStatsTable->model()->rowCount() > 0 &&
			m_inputStatsTable->model()->columnCount() > 0)
		m_inputStatsWidget->show();
	else
		m_inputStatsWidget->hide();
}

void GeneralTestView::exportToFile(const QString& path, const bool exportHeader, const QString& separator, QLocale::Language language) const {
	Q_UNUSED(exportHeader);
	Q_UNUSED(separator);
	Q_UNUSED(language);
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;
}

void GeneralTestView::exportToLaTeX(const QString & path, const bool exportHeaders,
									const bool gridLines, const bool captions, const bool latexHeaders,
									const bool skipEmptyRows, const bool exportEntire) const {
	Q_UNUSED(exportHeaders);
	Q_UNUSED(gridLines);
	Q_UNUSED(captions);
	Q_UNUSED(latexHeaders);
	Q_UNUSED(skipEmptyRows);
	Q_UNUSED(exportEntire);
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;
}
