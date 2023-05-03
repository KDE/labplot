#ifndef SPREADSHEET_PRIVATE
#define SPREADSHEET_PRIVATE

#include "Spreadsheet.h"

class SpreadsheetPrivate : public QObject {
public:
	explicit SpreadsheetPrivate(Spreadsheet*);

	QString name() const;

public:
	Spreadsheet::Linking linking;
	Spreadsheet* q;
};

#endif // SPREADSHEET_PRIVATE
