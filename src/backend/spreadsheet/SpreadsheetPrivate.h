#ifndef SPREADSHEET_PRIVATE
#define SPREADSHEET_PRIVATE

#include "Spreadsheet.h"

class CartesianPlot;
class StatisticsSpreadsheet;

class SpreadsheetPrivate : public QObject {
public:
	explicit SpreadsheetPrivate(Spreadsheet*);

	QString name() const;

public:
	bool suppressSetCommentFinalizeImport{false};
	bool readOnly{false};
	bool showComments{false};
	bool showSparklines{false};
	const Spreadsheet* linkedSpreadsheet{nullptr};
	QString linkedSpreadsheetPath;
	Spreadsheet* q{nullptr};
	Column* firstColumn{nullptr}; // used to connect to the signals related to the row count changes
	StatisticsSpreadsheet* statisticsSpreadsheet{nullptr};
	QVector<CartesianPlot*> m_usedInPlots; // plots using the columns prior to and after the import in replace mode, to be updated after the import
	QVector<const AbstractColumn*> m_involvedColumns; // columns which changed

	void updateCommentsHeader();
	void updateSparklinesHeader();
};

#endif // SPREADSHEET_PRIVATE
