/***************************************************************************
    File                 : ImportOpj.cpp
    Project              : LabPlot
    Description          : Import Origin project
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
	adapted from SciDAVis (importOPJ.cpp)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "ImportOpj.h"
#include "kdefrontend/MainWin.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"

#include <liborigin/OriginFile.h>

#include <QProgressBar>
#include <QStatusBar>

/*!
    \class ImportOpj
    \brief Importing an Origin project.

	\ingroup kdefrontend
 */
ImportOpj::ImportOpj(MainWin* parent, const QString& filename) : mw(parent) {
	DEBUG("Opj import started ...");

	OriginFile opj((const char *)filename.toLocal8Bit());
	int status = opj.parse();
	DEBUG("Parsing done with status " << status);
	DEBUG("Starting conversion ...");

	importTables(opj);
        //TODO  
//      importGraphs(opj);
//      importNotes(opj);
//      if(filename.endsWith(".opj", Qt::CaseInsensitive))
//              createProjectTree(opj);

}

int ImportOpj::importTables(const OriginFile &opj) {
	// spreadsheets 
	for (unsigned int s = 0; s < opj.spreadCount(); ++s) {
		Origin::SpreadSheet spread = opj.spread(s);
		importSpreadsheet(opj, spread);
	}

	// excels
	for (unsigned int e = 0; e < opj.excelCount(); ++e) {
			Origin::Excel excelwb = opj.excel(e);
			for (unsigned int s = 0; s < excelwb.sheets.size(); ++s) {
				Origin::SpreadSheet spread = excelwb.sheets[s];
				int columnCount = spread.columns.size();
				if(!columnCount) // do not add spread without cols
					continue;
				spread.name = excelwb.name;
				// scidavis does not have windows with multiple sheets
				if (s > 0)
					spread.name.append("@").append(std::to_string(s+1));

				spread.maxRows = excelwb.maxRows;
				importSpreadsheet(opj, spread);
			}
	}

	// matrices
	for (unsigned int m = 0; m < opj.matrixCount(); ++m) {
		Origin::Matrix matrix = opj.matrix(m);
		importMatrix(opj, matrix);
	}

	return 0;
}

int ImportOpj::importSpreadsheet(const OriginFile &opj, const Origin::SpreadSheet &spread) {
	Q_UNUSED(opj);
	int cols = spread.columns.size();
	int rows = spread.maxRows;
	if (!cols) // do not create spreadsheet without columns
		return -1;

	QLocale locale = mw->locale();
	Spreadsheet* spreadsheet = new Spreadsheet(0, spread.name.c_str());
	spreadsheet->setRowCount(rows);
	spreadsheet->setColumnCount(cols);

	int scaling_factor = 10; //in Origin width is measured in characters while here in pixels --- need to be accurate
	for (int j = 0; j < cols; ++j) {
		Origin::SpreadColumn column = spread.columns[j];
		Column *col = spreadsheet->column(j);

		QString name(column.name.c_str());
		col->setName(name.replace(QRegExp(".*_"),""));
		if (column.command.size() > 0)
			col->setFormula(Interval<int>(0, rows), QString(column.command.c_str()));
		col->setComment(QString(column.comment.c_str()));
		col->setWidth((int)column.width * scaling_factor);

		switch(column.type){
		case Origin::SpreadColumn::X:
			col->setPlotDesignation(AbstractColumn::X);
			break;
		case Origin::SpreadColumn::Y:
			col->setPlotDesignation(AbstractColumn::Y);
			break;
		case Origin::SpreadColumn::Z:
			col->setPlotDesignation(AbstractColumn::Z);
			break;
		case Origin::SpreadColumn::XErr:
			col->setPlotDesignation(AbstractColumn::XError);
			break;
		case Origin::SpreadColumn::YErr:
			col->setPlotDesignation(AbstractColumn::YError);
			break;
		case Origin::SpreadColumn::Label:
		case Origin::SpreadColumn::NONE:
		default:
			col->setPlotDesignation(AbstractColumn::NoDesignation);
		}

		QString format;
		switch(column.valueType) {
		case Origin::Numeric:
		case Origin::TextNumeric: {
			/*
			A TextNumeric column in Origin is a column whose filled cells contain either a double or a string.
			Here there is no equivalent column type.
			Set the column type as 'Numeric' or 'Text' depending on the type of first element in column.
			TODO: Add a "per column" flag, settable at import dialog, to choose between both types.
			 */
			Origin::variant value;
			double datavalue;
			bool setAsText = false;
			col->setColumnMode(AbstractColumn::Numeric);
			for (int i = 0; i < std::min((int)column.data.size(), rows); ++i) {
				value = column.data[i];
				if (value.type() == typeid(double)) {
					datavalue = boost::get<double>(value);
					if (datavalue == _ONAN) continue; // mark for empty cell
					if (!setAsText)
						col->setValueAt(i, datavalue);
					else	// convert double to string for Text columns
						col->setTextAt(i, locale.toString(datavalue, 'g', 16));
				} else { // string
					if (!setAsText && i == 0) {
						col->setColumnMode(AbstractColumn::Text);
						setAsText = true;
					}
					col->setTextAt(i, boost::get<string>(column.data[i]).c_str());
				}
			}
			if (column.numericDisplayType != 0) {
				int f = 0;
				switch(column.valueTypeSpecification) {
				case Origin::Decimal:
					f=1;
					break;
				case Origin::Scientific:
					f=2;
					break;
				case Origin::Engineering:
				case Origin::DecimalWithMarks:
					break;
				}

				//TODO
				Q_UNUSED(f);
				//Double2StringFilter *filter = static_cast<Double2StringFilter*>(col->outputFilter());
				//filter->setNumericFormat(f);
				//filter->setNumDigits(column.decimalPlaces);
			}
			break;
		}
		case Origin::Text:
			col->setColumnMode(AbstractColumn::Text);
			for (int i = 0; i < min((int)column.data.size(), rows); ++i)
				col->setTextAt(i, boost::get<string>(column.data[i]).c_str());
			break;
		case Origin::Time: {
			switch(column.valueTypeSpecification + 128) {
			case Origin::TIME_HH_MM:
				format="hh:mm";
				break;
			case Origin::TIME_HH:
				format="hh";
				break;
			case Origin::TIME_HH_MM_SS:
				format="hh:mm:ss";
				break;
			case Origin::TIME_HH_MM_SS_ZZ:
				format="hh:mm:ss.zzz";
				break;
			case Origin::TIME_HH_AP:
				format="hh ap";
				break;
			case Origin::TIME_HH_MM_AP:
				format="hh:mm ap";
				break;
			case Origin::TIME_MM_SS:
				format="mm:ss";
				break;
			case Origin::TIME_MM_SS_ZZ:
				format="mm:ss.zzz";
				break;
			case Origin::TIME_HHMM:
				format="hhmm";
				break;
			case Origin::TIME_HHMMSS:
				format="hhmmss";
				break;
			case Origin::TIME_HH_MM_SS_ZZZ:
				format="hh:mm:ss.zzz";
				break;
			}

			for (int i = 0; i < min((int)column.data.size(), rows); ++i)
				col->setValueAt(i, boost::get<double>(column.data[i]));
			col->setColumnMode(AbstractColumn::DateTime);
			// TODO
			//DateTime2StringFilter *filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			//filter->setFormat(format);
			break;
		}
		case Origin::Date: {
			switch(column.valueTypeSpecification) {
			case Origin::DATE_DD_MM_YYYY:
				format="dd/MM/yyyy";
				break;
			case Origin::DATE_DD_MM_YYYY_HH_MM:
				format="dd/MM/yyyy HH:mm";
				break;
			case Origin::DATE_DD_MM_YYYY_HH_MM_SS:
				format="dd/MM/yyyy HH:mm:ss";
				break;
			case Origin::DATE_DDMMYYYY:
			case Origin::DATE_DDMMYYYY_HH_MM:
			case Origin::DATE_DDMMYYYY_HH_MM_SS:
				format="dd.MM.yyyy";
					break;
			case Origin::DATE_MMM_D:
				format="MMM d";
				break;
			case Origin::DATE_M_D:
				format="M/d";
				break;
			case Origin::DATE_D:
				format="d";
				break;
			case Origin::DATE_DDD:
			case Origin::DATE_DAY_LETTER:
				format="ddd";
				break;
			case Origin::DATE_YYYY:
				format="yyyy";
				break;
			case Origin::DATE_YY:
				format="yy";
				break;
			case Origin::DATE_YYMMDD:
			case Origin::DATE_YYMMDD_HH_MM:
			case Origin::DATE_YYMMDD_HH_MM_SS:
			case Origin::DATE_YYMMDD_HHMM:
			case Origin::DATE_YYMMDD_HHMMSS:
				format="yyMMdd";
				break;
			case Origin::DATE_MMM:
			case Origin::DATE_MONTH_LETTER:
				format="MMM";
				break;
			case Origin::DATE_M_D_YYYY:
				format="M-d-yyyy";
				break;
			default:
				format="dd.MM.yyyy";
			}

			for (int i = 0; i < min((int)column.data.size(), rows); ++i)
				col->setValueAt(i, boost::get<double>(column.data[i]));
			col->setColumnMode(AbstractColumn::DateTime);
			//TODO
			//DateTime2StringFilter *filter = static_cast<DateTime2StringFilter*>(scidavis_column->outputFilter());
			//filter->setFormat(format);
			break;
		}
		case Origin::Month: {
			switch (column.valueTypeSpecification) {
			case Origin::MONTH_MMM:
				format = "MMM";
				break;
			case Origin::MONTH_MMMM:
				format = "MMMM";
				break;
			case Origin::MONTH_LETTER:
				format = "M";
				break;
			}

			for (int i = 0; i < min((int)column.data.size(), rows); ++i)
				col->setValueAt(i, boost::get<double>(column.data[i]));
			col->setColumnMode(AbstractColumn::Month);
			//TODO
			//DateTime2StringFilter *filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			//filter->setFormat(format);
			break;
		}
		case Origin::Day: {
			switch(column.valueTypeSpecification) {
			case Origin::DAY_DDD:
				format = "ddd";
				break;
			case Origin::DAY_DDDD:
				format = "dddd";
				break;
			case Origin::DAY_LETTER:
				format = "d";
				break;
			}

			for (int i = 0; i < min((int)column.data.size(), rows); ++i)
				col->setValueAt(i, boost::get<double>(column.data[i]));
			col->setColumnMode(AbstractColumn::Day);
			// TODO
			//DateTime2StringFilter *filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			//filter->setFormat(format);
			break;
		}
		case Origin::ColumnHeading:
		case Origin::TickIndexedDataset:
		case Origin::Categorical:
			break;
		}
	}



//	if (spread.hidden || spread.loose)
//		mw->hideWindow(spreadsheet);
	mw->addAspectToProject(spreadsheet);

	return 0;
}

int ImportOpj::importMatrix(const OriginFile &opj, const Origin::Matrix &matrix) {
	Q_UNUSED(opj);

	unsigned int layers = matrix.sheets.size();
	int scaling_factor = 10; //in Origin width is measured in characters while here in pixels --- need to be accurate
	for (unsigned int l = 0; l < layers; ++l) {
		Origin::MatrixSheet layer = matrix.sheets[l];
		int colCount = layer.columnCount;
		int rowCount = layer.rowCount;

		Matrix* m = new Matrix(0, matrix.name.c_str());
		m->setRowCount(rowCount);
		m->setColumnCount(colCount);
		if (!m)
			return false;
		m->setFormula(layer.command.c_str());
		for (int j = 0; j < colCount; j++)
			m->setColumnWidth(j, layer.width * scaling_factor);

		for (int i = 0; i < rowCount; i++) {
		for (int j = 0; j < colCount; j++) {
			m->setCell(i, j, layer.data[j + i*colCount]);
		}
		}

		char format = 'g';
		int prec = 6;
		switch (layer.valueTypeSpecification) {
		case 0: //Decimal 1000
			format='f';
			prec = layer.decimalPlaces;
			break;
		case 1: //Scientific
			format='e';
			prec = layer.decimalPlaces;
			break;
		case 2: //Engineering
		case 3: //Decimal 1,000
			format='g';
			prec = layer.significantDigits;
			break;
		}

		//TODO: prec not support by Matrix
		Q_UNUSED(prec);
		m->setNumericFormat(format);

		mw->addAspectToProject(m);
	}

	return 0;
}
