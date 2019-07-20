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
#include "backend/generalTest/GeneralTest.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"

#include <QFile>
#include <QVBoxLayout>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QLabel>
#include <QTextEdit>
#include <QToolTip>

#include <KLocalizedString>

/*!
    \class GeneralTestView
    \brief View class for Hypothesis Test

    \ingroup kdefrontend
 */

GeneralTestView::GeneralTestView(GeneralTest* GeneralTest) : QWidget(),
    m_GeneralTest(GeneralTest),
    m_testName(new QLabel()),
    m_statsTable(new QTextEdit()),
    m_summaryResults(new QWidget()) {

    m_statsTable->setReadOnly(true);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_testName);
    layout->addWidget(m_statsTable);
    layout->addWidget(m_summaryResults);
    layout->addWidget(m_summaryResults);
    init();
}

GeneralTestView::~GeneralTestView() = default;

void GeneralTestView::init() {
    initActions();
    initMenus();

    m_statsTable->setMouseTracking(true);
    //    m_summaryResults->setStyleSheet("background-color:white; border: 0px; margin: 0px; padding 0px;qproperty-frame: false;");
    connect(m_GeneralTest, &GeneralTest::changed, this, &GeneralTestView::changed);
    connect(m_statsTable, &QTextEdit::cursorPositionChanged, this, &GeneralTestView::cursorPositionChanged);
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
    if ((ret = dlg->exec()) == QDialog::Accepted) {
        print(&printer);
    }
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
    m_testName->setText(m_GeneralTest->testName());
    m_statsTable->setHtml(m_GeneralTest->statsTable());
    m_summaryResults->setLayout(m_GeneralTest->summaryLayout());
}

void GeneralTestView::cursorPositionChanged() {
    QTextCursor cursor = m_statsTable->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    QMap<QString, QString> tooltips = m_GeneralTest->tooltips();
    if (!cursor.selectedText().isEmpty())
        QToolTip::showText(QCursor::pos(),
                           QString("%1")
                           .arg(tooltips.value(cursor.selectedText())));
    else
        QToolTip::hideText();
}

void GeneralTestView::exportToFile(const QString& path, const bool exportHeader, const QString& separator, QLocale::Language language) const {
    Q_UNUSED(exportHeader);
    Q_UNUSED(separator);
    Q_UNUSED(language);
    QFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        return;

    PERFTRACE("export pivot table to file");

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

    PERFTRACE("export pivot table to latex");
}
