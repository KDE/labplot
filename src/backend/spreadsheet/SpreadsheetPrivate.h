#ifndef SPREADSHEET_PRIVATE
#define SPREADSHEET_PRIVATE

#include "Spreadsheet.h"

class SpreadsheetPrivate : public QObject {
public:
	explicit SpreadsheetPrivate(Spreadsheet*);

	void updateLinks(const AbstractColumn* = nullptr);
	QString name() const;

public:
	Spreadsheet::Linking linking;
	Spreadsheet* q;
};

#endif // SPREADSHEET_PRIVATE
