#ifndef SPREADSHEET_PRIVATE
#define SPREADSHEET_PRIVATE

#include "Spreadsheet.h"

class StatisticsSpreadsheet;

class SpreadsheetPrivate : public QObject {
public:
	explicit SpreadsheetPrivate(Spreadsheet*);

	QString name() const;

public:
	bool suppressSetCommentFinalizeImport{false};
	Spreadsheet::Linking linking;
	Spreadsheet* q{nullptr};
	StatisticsSpreadsheet* statisticsSpreadsheet{nullptr};
	QVector<CartesianPlot*> m_usedInPlots; // plots using the columns prior to and after the import in replace mode, to be updated after the import
};

#endif // SPREADSHEET_PRIVATE
