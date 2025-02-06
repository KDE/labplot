import sys
import pathlib

from PySide6.QtWidgets import QApplication

from pylabplot import AsciiFilter, Spreadsheet, AbstractFileFilter, AbstractColumn

def main():
	app = QApplication()

	filter = AsciiFilter()

	p = filter.properties()

	p.automaticSeparatorDetection = True
	p.separator = ";"
	p.headerEnabled = True
	p.headerLine = 1
	p.commentCharacter = "#"
	p.intAsDouble = False
	p.removeQuotes = True
	p.dateTimeFormat = "MM/dd/yyyy"

	filter.setProperties(p)

	spreadsheet = Spreadsheet("test", False)

	filter.readDataFromFile(f"{pathlib.Path(__file__).parent.resolve()}/data.txt", spreadsheet, AbstractFileFilter.ImportMode.Replace)

	if filter.lastError():
		print(f"Import error: {filter.lastError()}")
		sys.exit(-1)

	print(f"Number of columns: {spreadsheet.columnCount()}")
	print(f"Number of rows: {spreadsheet.rowCount()}")

	spreadsheet.column(0).setColumnMode(AbstractColumn.ColumnMode.Text)
	spreadsheet.column(1).setColumnMode(AbstractColumn.ColumnMode.Integer)
	spreadsheet.column(2).setColumnMode(AbstractColumn.ColumnMode.Double)
	spreadsheet.column(3).setColumnMode(AbstractColumn.ColumnMode.Day)

	print(f"First Name: {spreadsheet.column(0).textAt(1)}")
	print(f"Age: {spreadsheet.column(1).integerAt(1)}")
	print(f"Height: {spreadsheet.column(2).doubleAt(1)}")
	print(f"Date of Birth: {spreadsheet.column(3).dateAt(1).toString()}")

main()