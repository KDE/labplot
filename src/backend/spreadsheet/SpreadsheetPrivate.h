#ifndef SPREADSHEET_PRIVATE
#define SPREADSHEET_PRIVATE

#include "Spreadsheet.h"

class StatisticsSpreadsheet;

class SpreadsheetPrivate : public QObject {
public:
	explicit SpreadsheetPrivate(Spreadsheet*);

	QString name() const;

public:
	Spreadsheet::Linking linking;
	Spreadsheet* q{nullptr};
	StatisticsSpreadsheet* statisticsSpreadsheet{nullptr};
	QVector<CartesianPlot*> mChangingPlots;
};

#endif // SPREADSHEET_PRIVATE
