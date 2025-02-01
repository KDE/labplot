/***************************************************************************
	File                 : GeneralTestView.cpp
	Project              : LabPlot
	Description          : View class for Hypothesis Tests' Table
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Devanshu Agarwal
						   (agarwaldevanshu8@gmail.com)
	Copyright            : (C) 2025 Kuntal Bar
						   (barkuntal6@gmail.com)
***************************************************************************/

#include "GeneralTestView.h"
#include "backend/statistics/GeneralTest.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "backend/statistics/TextEdit.h"

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
	\ingroup frontend
*/

GeneralTestView::GeneralTestView(GeneralTest* test)
	: QWidget(),
	m_generalTest(test),
	m_testName(new QLabel()),
	m_statsTable(new TextEdit()),
	m_summaryResults(new QWidget()),
	m_inputStatsWidget(new QWidget()),
	m_labelInputStatsTable(new QLabel()),
	m_inputStatsTable(new QTableView()),
	m_clearInputStats(new QPushButton()) {

	// Set up the input statistics widget.
	auto* inputStatsLayout = new QVBoxLayout(m_inputStatsWidget);
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
	m_labelInputStatsTable->setToolTip(i18n("Fill this table with pre-calculated statistic value and then press recalculate")
									   + QLatin1String("<br><br>")
									   + i18n("You can leave one or more columns empty if you feel they are not useful")
									   + QLatin1String("</h3>"));
	m_clearInputStats->setText(i18n("Clear"));
	m_clearInputStats->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

	layout->addWidget(m_inputStatsWidget);
	layout->addWidget(m_testName);
	layout->addWidget(m_statsTable);
	layout->addWidget(m_summaryResults);
	layout->addStretch(1);

	initializeComponents();
}

GeneralTestView::~GeneralTestView() {
	// Destructor
}

void GeneralTestView::initializeComponents() {
	initializeActions();
	initializeMenus();

	m_inputStatsTable->setModel(m_generalTest->getInputStatsTableModel());
	m_inputStatsTable->horizontalHeader()->setVisible(false);
	m_inputStatsTable->verticalHeader()->setVisible(false);
	m_inputStatsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	m_statsTable->setMouseTracking(true);
	connect(m_generalTest, &GeneralTest::changed, this, &GeneralTestView::updateDisplay);
	connect(m_clearInputStats, &QPushButton::clicked, m_generalTest, &GeneralTest::clearInputStats);
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

void GeneralTestView::resetResults() {
	for (int i = 0; i < RESULT_LINES_COUNT; ++i)
		m_resultLine[i]->clear();
}

void GeneralTestView::populateToolBar(QToolBar* toolBar) {
	Q_UNUSED(toolBar);
}

void GeneralTestView::buildContextMenu(QMenu* menu) {
	Q_ASSERT(menu);
	// Populate the context menu with actions if necessary.
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
	WAIT_CURSOR;
	QPainter painter(printer);
	RESET_CURSOR;
}

void GeneralTestView::updateDisplay() {
	m_testName->setText(m_generalTest->getTestName());
	m_testName->show();
	m_summaryResults->show();

	if (m_generalTest->getStatsTable().isEmpty())
		m_statsTable->hide();
	else {
		m_statsTable->setHtml(m_generalTest->getStatsTable());
		m_statsTable->show();
	}

	m_summaryResults->setLayout(m_generalTest->getSummaryLayout());

	if (m_inputStatsTable->model()->rowCount() > 0 &&
		m_inputStatsTable->model()->columnCount() > 0)
		m_inputStatsWidget->show();
	else
		m_inputStatsWidget->hide();
}

void GeneralTestView::exportDataToFile(const QString& path, bool exportHeader,
									   const QString& separator, QLocale::Language language) const {
	Q_UNUSED(exportHeader);
	Q_UNUSED(separator);
	Q_UNUSED(language);
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;
	// File export logic goes here.
}

void GeneralTestView::exportDataToLaTeX(const QString& path, bool exportHeaders,
										bool gridLines, bool captions, bool latexHeaders,
										bool skipEmptyRows, bool exportEntire) const {
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
