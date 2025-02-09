/***************************************************************************
	File                 : GeneralTestView.h
	Project              : LabPlot
	Description          : View class for Hypothesis Tests'
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Devanshu Agarwal (agarwaldevanshu8@gmail.com)
	Copyright            : (C) 2025 Kuntal Bar (barkuntal6@gmail.com)

***************************************************************************/

#ifndef GENERALTESTVIEW_H
#define GENERALTESTVIEW_H

#include "backend/core/AbstractColumn.h"
#include "backend/lib/IntervalAttribute.h"

#include <QLocale>
#include <QWidget>

class Column;
class GeneralTest;
class AbstractAspect;

class QPrinter;
class QToolBar;
class QLabel;
class TextEdit;
class QTableView;
class QPushButton;

#define RESULT_LINES_COUNT 10

class GeneralTestView : public QWidget {
	Q_OBJECT

public:
	explicit GeneralTestView(GeneralTest* test);
	~GeneralTestView() override;

	bool exportDisplay();
	bool executePrintView();
	bool previewPrintView();

protected:
	void initializeComponents();
	void initializeActions();
	void initializeMenus();
	void setupConnections();

	void exportDataToFile(const QString& path, bool exportHeader, const QString& separator, QLocale::Language language) const;
	void
	exportDataToLaTeX(const QString& path, bool exportHeaders, bool gridLines, bool captions, bool latexHeaders, bool skipEmptyRows, bool exportEntire) const;

	GeneralTest* m_generalTest;
	QLabel* m_testName;
	TextEdit* m_statsTable;
	QWidget* m_summaryResults{nullptr};
	QLabel* m_resultLine[RESULT_LINES_COUNT];

	QWidget* m_inputStatsWidget;
	QLabel* m_labelInputStatsTable;
	QTableView* m_inputStatsTable;
	QPushButton* m_clearInputStats;

public Q_SLOTS:
	void buildContextMenu(QMenu* menu);
	void populateToolBar(QToolBar* toolBar);
	void renderToPrinter(QPrinter* printer) const;
	void updateDisplay();
	void resetResults();
};

#endif // GENERALTESTVIEW_H
