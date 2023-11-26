#include "SpreadsheetHeaderView.h"

#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

class TwoHeaderWidget : public QWidget {
public:
	TwoHeaderWidget(QWidget* parent = nullptr)
		: QWidget(parent) {
		// Create a QTableWidget
		tableWidget.setRowCount(5);
		tableWidget.setColumnCount(3);

		// Create some sample data
		QTableWidgetItem* item;
		for (int i = 0; i < 5; ++i) {
			for (int j = 0; j < 3; ++j) {
				item = new QTableWidgetItem(QLatin1String("Item %1-%2").arg(i + 1).arg(j + 1));
				tableWidget.setItem(i, j, item);
			}
		}

		// Create the main header
		SpreadsheetHeaderView* mainHeader = new SpreadsheetHeaderView();

		// Create the first horizontal header
		SpreadsheetCommentsHeaderView* firstHorizontalHeader = new SpreadsheetCommentsHeaderView();
		mainHeader->addSlaveHeader(firstHorizontalHeader);
		firstHorizontalHeader->setSectionResizeMode(QHeaderView::Stretch);

		// Create the second horizontal header
		SpreadsheetSparkLineHeaderView* secondHorizontalHeader = new SpreadsheetSparkLineHeaderView();
		mainHeader->addSlaveHeader(secondHorizontalHeader);
		secondHorizontalHeader->setSectionResizeMode(QHeaderView::Fixed);

		// Create a vertical layout
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(mainHeader);
		layout->addWidget(&tableWidget);
		layout->addWidget(firstHorizontalHeader);
		layout->addWidget(secondHorizontalHeader);
	}

private:
	QTableWidget tableWidget;
};
