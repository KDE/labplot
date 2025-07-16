/***************************************************************************
	File                 : PivotTableView.cpp
	Project              : LabPlot
	Description          : View class for PivotTable
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PivotTableView.h"
#include "HierarchicalHeaderView.h"
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

#include <KLocalizedString>

/*!
	\class PivotTableView
	\brief View class for PivotTable

	\ingroup frontend
 */
PivotTableView::PivotTableView(PivotTable* pivotTable) : QWidget(),
	m_pivotTable(pivotTable),
	m_tableView(new QTableView(this)),
	m_horizontalHeaderView(new HierarchicalHeaderView(Qt::Horizontal, m_tableView)),
	m_verticalHeaderView(new HierarchicalHeaderView(Qt::Vertical, m_tableView)) {

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(m_tableView);

	m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_tableView->setEditTriggers(QTableView::NoEditTriggers); // read-only mode

	m_tableView->setHorizontalHeader(m_horizontalHeaderView);
	m_horizontalHeaderView->setHighlightSections(true);
	m_horizontalHeaderView->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_horizontalHeaderView->setSectionsClickable(true);

	m_tableView->setVerticalHeader(m_verticalHeaderView);
	m_verticalHeaderView->setHighlightSections(true);
	m_verticalHeaderView->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_verticalHeaderView->setSectionsClickable(true);

	m_horizontalHeaderView->setHierarchicalModel(pivotTable->horizontalHeaderModel());
	m_verticalHeaderView->setHierarchicalModel(pivotTable->verticalHeaderModel());
	m_tableView->setModel(m_pivotTable->dataModel());

	init();
}

PivotTableView::~PivotTableView() = default;

void PivotTableView::init() {
	initActions();
	initMenus();
	connect(m_pivotTable, &PivotTable::changed, this, &PivotTableView::changed);
}

void PivotTableView::initActions() {

}

void PivotTableView::initMenus() {

}

void PivotTableView::connectActions() {

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
	const auto& index = m_tableView->model()->index(row, col);
	m_tableView->scrollTo(index);
	m_tableView->setCurrentIndex(index);
}

bool PivotTableView::exportView() {
	return true;
}

bool PivotTableView::printView() {
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, this);
	dlg->setWindowTitle(i18nc("@title:window", "Print Pivot Table"));

	bool ret;
	if ((ret = dlg->exec()) == QDialog::Accepted)
		print(&printer);

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

void PivotTableView::changed() {
	auto* horizontalHeaderModel = m_pivotTable->horizontalHeaderModel();
	auto* verticalHeaderModel = m_pivotTable->verticalHeaderModel();

	horizontalHeaderModel->setOrientation(Qt::Horizontal);
	verticalHeaderModel->setOrientation(Qt::Vertical);

	// qDebug() << " setting size for horizontal header";
	// qDebug() << " rows, cols = " << horizontalHeaderModel->rowCount() << ", " << horizontalHeaderModel->columnCount();
	horizontalHeaderModel->setBaseSectionSize(m_horizontalHeaderView->getBaseSectionSize());

	// qDebug() << "settign size for vertical header";
	// qDebug() << " rows, cols = " << verticalHeaderModel->rowCount() << ", " << verticalHeaderModel->columnCount();
	verticalHeaderModel->setBaseSectionSize(m_verticalHeaderView->getBaseSectionSize());
}

void PivotTableView::exportToFile(const QString& path, const bool exportHeader, const QString& separator, QLocale::Language language) const {
	Q_UNUSED(exportHeader);
	Q_UNUSED(separator);
	Q_UNUSED(language);
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	PERFTRACE(QLatin1String("export pivot table to file"));
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

	PERFTRACE(QLatin1String("export pivot table to latex"));
}
