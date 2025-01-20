#include <iostream>

#include <QApplication>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	AsciiFilter filter;
	auto p = filter.properties();

	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(";");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.commentCharacter = QStringLiteral("#");
	p.intAsDouble = false;
	p.removeQuotes = true;
	p.dateTimeFormat = QStringLiteral("MM/dd/yyyy");

	filter.setProperties(p);

	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	filter.readDataFromFile(QStringLiteral("data.txt"), &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	if (!filter.lastError().isEmpty()) {
		std::cout << "Import error: " << filter.lastError().toStdString() << std::endl;
		return -1;
	}

	std::cout << "Number of columns: " << spreadsheet.columnCount() << std::endl;
	std::cout << "Number of rows: " << spreadsheet.rowCount() << std::endl;

	spreadsheet.column(0)->setColumnMode(AbstractColumn::ColumnMode::Text);
	spreadsheet.column(1)->setColumnMode(AbstractColumn::ColumnMode::Integer);
	spreadsheet.column(2)->setColumnMode(AbstractColumn::ColumnMode::Double);
	spreadsheet.column(3)->setColumnMode(AbstractColumn::ColumnMode::Day);

	std::cout << "First Name: " << spreadsheet.column(0)->textAt(1).toStdString() << std::endl;
	std::cout << "Age: " << spreadsheet.column(1)->integerAt(1) << std::endl;
	std::cout << "Height: " << spreadsheet.column(2)->doubleAt(1) << std::endl;
	std::cout << "Date of Birth: " << spreadsheet.column(3)->dateAt(1).toString().toStdString() << std::endl;
}
