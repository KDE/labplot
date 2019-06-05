/***************************************************************************
    File                 : PivotTableView.cpp
    Project              : LabPlot
    Description          : View class for PivotTable
    --------------------------------------------------------------------
    Copyright            : (C) 2019 by Alexander Semke (alexander.semke@web.de)

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

#include "HypothesisTestView.h"
#include "backend/hypothesis_test/HypothesisTest.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"

#include <QInputDialog>
#include <QFile>
#include <QVBoxLayout>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

#include <QDebug>

/*!
    \class HypothesisTestView
    \brief View class for Hypothesis Test

    \ingroup kdefrontend
 */

HypothesisTestView::HypothesisTestView(HypothesisTest* hypothesisTest) : QWidget(),
    m_hypothesisTest(hypothesisTest),
    m_tableView(new QTableView(this)),
    m_horizontalHeaderView(new QHeaderView(Qt::Horizontal, m_tableView)) {

    auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(m_tableView);

    m_horizontalHeaderView->setVisible(true);
    m_horizontalHeaderView->setEnabled(true);
    m_horizontalHeaderView->setSectionsClickable(true);

    m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    init();
}

HypothesisTestView::~HypothesisTestView() = default;

void HypothesisTestView::init() {
	initActions();
	initMenus();

    m_tableView->setModel(m_hypothesisTest->dataModel());
    m_horizontalHeaderView->setModel(m_hypothesisTest->horizontalHeaderModel());

//    connect(m_hypothesisTest, &HypothesisTest::changed, this, &HypothesisTestView::changed);
}

void HypothesisTestView::initActions() {

}

void HypothesisTestView::initMenus() {

}

void HypothesisTestView::connectActions() {

}

void HypothesisTestView::fillToolBar(QToolBar* toolBar) {
	Q_UNUSED(toolBar);
}

/*!
 * Populates the menu \c menu with the pivot table and pivot table view relevant actions.
 * The menu is used
 *   - as the context menu in PivotTableView
 *   - as the "pivot table menu" in the main menu-bar (called form MainWin)
 *   - as a part of the pivot table context menu in project explorer
 */
void HypothesisTestView::createContextMenu(QMenu* menu) {
	Q_ASSERT(menu);
}

void HypothesisTestView::goToCell() {
	bool ok;

	int col = QInputDialog::getInt(nullptr, i18n("Go to Cell"), i18n("Enter column"), 1, 1, m_tableView->model()->columnCount(), 1, &ok);
	if (!ok) return;

	int row = QInputDialog::getInt(nullptr, i18n("Go to Cell"), i18n("Enter row"), 1, 1, m_tableView->model()->rowCount(), 1, &ok);
	if (!ok) return;

	goToCell(row-1, col-1);
}

void HypothesisTestView::goToCell(int row, int col) {
	QModelIndex index = m_tableView->model()->index(row, col);
	m_tableView->scrollTo(index);
	m_tableView->setCurrentIndex(index);
}

bool HypothesisTestView::exportView() {
	return true;
}

bool HypothesisTestView::printView() {
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, this);
	dlg->setWindowTitle(i18nc("@title:window", "Print Spreadsheet"));

	bool ret;
	if ((ret = dlg->exec()) == QDialog::Accepted) {
		print(&printer);
	}
	delete dlg;
	return ret;
}

bool HypothesisTestView::printPreview() {
	QPrintPreviewDialog* dlg = new QPrintPreviewDialog(this);
    connect(dlg, &QPrintPreviewDialog::paintRequested, this, &HypothesisTestView::print);
	return dlg->exec();
}

/*!
  prints the complete spreadsheet to \c printer.
 */
void HypothesisTestView::print(QPrinter* printer) const {
	WAIT_CURSOR;
	QPainter painter (printer);

	RESET_CURSOR;
}

 void HypothesisTestView::changed() {

 }

void HypothesisTestView::exportToFile(const QString& path, const bool exportHeader, const QString& separator, QLocale::Language language) const {
	Q_UNUSED(exportHeader);
	Q_UNUSED(separator);
	Q_UNUSED(language);
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	PERFTRACE("export pivot table to file");

}

void HypothesisTestView::exportToLaTeX(const QString & path, const bool exportHeaders,
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

	PERFTRACE("export pivot table to latex");
}
