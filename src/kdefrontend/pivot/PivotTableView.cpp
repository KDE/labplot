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

#include "PivotTableView.h"
#include "backend/pivot/PivotTable.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"

#include <QInputDialog>
#include <QFile>
#include <QHBoxLayout>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QTableView>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

/*!
	\class PivotTableView
	\brief View class for PivotTable

	\ingroup commonfrontend
 */
PivotTableView::PivotTableView(PivotTable* pivotTable, bool readOnly) : QWidget(),
	m_pivotTable(pivotTable),
	m_tableView(new QTableView(this)),
	m_readOnly(readOnly) {

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(m_tableView);
	if (m_readOnly)
		m_tableView->setEditTriggers(QTableView::NoEditTriggers);

	init();
}

PivotTableView::~PivotTableView() {

}

void PivotTableView::init() {
	initActions();
	initMenus();

	m_tableView->setModel(m_pivotTable->dataModel());
	m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	//hierarchical headers
	//TODO:
}

void PivotTableView::initActions() {

}

void PivotTableView::initMenus() {


}

void PivotTableView::connectActions() {

}

void PivotTableView::fillToolBar(QToolBar* toolBar) {
	Q_UNUSED(toolBar);
}

/*!
 * Populates the menu \c menu with the pivot table and pivot table view relevant actions.
 * The menu is used
 *   - as the context menu in PivotTableView
 *   - as the "pivot table menu" in the main menu-bar (called form MainWin)
 *   - as a part of the pivot table context menu in project explorer
 */
void PivotTableView::createContextMenu(QMenu* menu) {
	Q_ASSERT(menu);
}

void PivotTableView::goToCell() {
	bool ok;

	int col = QInputDialog::getInt(nullptr, i18n("Go to Cell"), i18n("Enter column"), 1, 1, m_tableView->model()->columnCount(), 1, &ok);
	if (!ok) return;

	int row = QInputDialog::getInt(nullptr, i18n("Go to Cell"), i18n("Enter row"), 1, 1, m_tableView->model()->rowCount(), 1, &ok);
	if (!ok) return;

	goToCell(row-1, col-1);
}

void PivotTableView::goToCell(int row, int col) {
	QModelIndex index = m_tableView->model()->index(row, col);
	m_tableView->scrollTo(index);
	m_tableView->setCurrentIndex(index);
}

bool PivotTableView::exportView() {
	return true;
}

bool PivotTableView::printView() {
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

bool PivotTableView::printPreview() {
	QPrintPreviewDialog* dlg = new QPrintPreviewDialog(this);
	connect(dlg, &QPrintPreviewDialog::paintRequested, this, &PivotTableView::print);
	return dlg->exec();
}

/*!
  prints the complete spreadsheet to \c printer.
 */
void PivotTableView::print(QPrinter* printer) const {
	WAIT_CURSOR;
	QPainter painter (printer);

	RESET_CURSOR;
}

void PivotTableView::exportToFile(const QString& path, const bool exportHeader, const QString& separator, QLocale::Language language) const {
	Q_UNUSED(exportHeader);
	Q_UNUSED(separator);
	Q_UNUSED(language);
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	PERFTRACE("export pivot table to file");

}

void PivotTableView::exportToLaTeX(const QString & path, const bool exportHeaders,
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
