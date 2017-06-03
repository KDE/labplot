/***************************************************************************
    File                 : ImportOpj.cpp
    Project              : LabPlot
    Description          : Import Origin project
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)

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
	DEBUG("Parsing done. Starting conversion ...");

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
                int columnCount = spread.columns.size();
                if(!columnCount) // do not add spreads without cols
                        continue;
                importSpreadsheet(opj, spread);
        }

	// excels
	for (unsigned int e = 0; e < opj.excelCount(); ++e) {
			Origin::Excel excelwb = opj.excel(e);
			for (unsigned int s = 0; s < excelwb.sheets.size(); ++s) {
				Origin::SpreadSheet spread = excelwb.sheets[s];
				int columnCount = spread.columns.size();
				if(!columnCount) // do not add spreads without cols
					continue;
				spread.name = excelwb.name;
				// scidavis does not have windows with multiple sheets
				if (s > 0)
					spread.name.append("@").append(std::to_string(s+1));

				spread.maxRows = excelwb.maxRows;
				importSpreadsheet(opj, spread);
			}
	}

	// TODO: matrices
        for (unsigned int m = 0; m < opj.matrixCount(); ++m) {
                Origin::Matrix matrix = opj.matrix(m);
		// importMatrix(opj, matrix);
                unsigned int layers = matrix.sheets.size();
                for (unsigned int l = 0; l < layers; ++l) {
                        Origin::MatrixSheet& layer = matrix.sheets[l];
                        int columnCount = layer.columnCount;
                        int rowCount = layer.rowCount;
			//TODO
                        //Matrix* Matrix = mw->newMatrix(matrix.name.c_str(), rowCount, columnCount);
                        //if (!Matrix)
                        //        return false;
                        //Matrix->setWindowLabel(matrix.label.c_str());
                        //Matrix->setFormula(layer.command.c_str());
                        //Matrix->setColumnsWidth(layer.width * SciDAVis_scaling_factor);

			QVector<qreal> values;
			values.resize(rowCount*columnCount);
			for (int i = 0; i < min((int)values.size(),(int)layer.data.size()); i++)
				values[i]=layer.data[i];

			//Matrix->setCells(values);
			//Matrix->saveCellsToMemory();

			QChar format;
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
			//Matrix->setNumericFormat(format, prec);
                        //Matrix->forgetSavedCells();
                        //Matrix->showNormal();

                }
	}

	return 0;
}

int ImportOpj::importSpreadsheet(const OriginFile &opj, const Origin::SpreadSheet &spread) {
	int cols = spread.columns.size();
	int rows = spread.maxRows;
	if (!cols) // do not create spreadsheets without columns
		return -1;

	QLocale locale = mw->locale();
	Spreadsheet* spreadsheet = new Spreadsheet(0, spread.name.c_str());
	spreadsheet->setRowCount(rows);
	spreadsheet->setColumnCount(cols);

	for (int j = 0; j < cols; ++j) {
		Origin::SpreadColumn column = spread.columns[j];
		Column *col = spreadsheet->column(j);

		QString name(column.name.c_str());
		col->setName(name.replace(QRegExp(".*_"),""));
//		if (column.command.size() > 0)
//			col->setFormula(Interval<int>(0,maxrows), QString(column.command.c_str()));
		col->setComment(QString(column.comment.c_str()));
//		spreadsheet->setColumnWidth(j, (int)column.width*SciDAVis_scaling_factor);

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
			col->setPlotDesignation(AbstractColumn::xErr);
			break;
		case Origin::SpreadColumn::YErr:
			col->setPlotDesignation(AbstractColumn::yErr);
			break;
		case Origin::SpreadColumn::Label:
		default:
			col->setPlotDesignation(AbstractColumn::noDesignation);
		}

		//TODO
		switch(column.valueType) {
			case Origin::Numeric:
			case Origin::TextNumeric: {
			/*
			A TextNumeric column in Origin is a column whose filled cells contain either a double or a string.
			In SciDAVis there is no equivalent column type.
			Set the SciDAVis column type as 'Numeric' or 'Text' depending on the type of first element in column.
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
						if (!setAsText) {
							col->setValueAt(i, datavalue);
						} else { // convert double to string for Text columns
							col->setTextAt(i, locale.toString(datavalue, 'g', 16));
						}
					} else { // string
						if (!setAsText && i==0) {
							col->setColumnMode(AbstractColumn::Text);
							setAsText = true;
						}
						col->setTextAt(i, boost::get<string>(column.data[i]).c_str());
					}
			}
			int f=0;
			if(column.numericDisplayType != 0) {
				switch(column.valueTypeSpecification) {
				case 0: //Decimal 1000
					f=1;
					break;
				case 1: //Scientific
					f=2;
					break;
				case 2: //Engeneering
				case 3: //Decimal 1,000
					f=0;
				break;
				}

				//Double2StringFilter *filter = static_cast<Double2StringFilter*>(col->outputFilter());
				//filter->setNumericFormat(f);
				//filter->setNumDigits(column.decimalPlaces);
			}
			break;
		}
		case Origin::Text:
		case Origin::Date:
		case Origin::Time:
		case Origin::Month:
		case Origin::Day:
			//TODO
			break;
		default:
			break;
		}

		//TODO
	}



//	if (spread.hidden || spread.loose)
//		mw->hideWindow(spreadsheet);
	mw->addAspectToProject(spreadsheet);

	return 0;
}

int ImportOpj::importMatrix(const OriginFile &opj, const Origin::Matrix &matrix) {
	//TODO

	return 0;
}
