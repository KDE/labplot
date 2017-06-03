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

ImportOpj::~ImportOpj() {
}

bool ImportOpj::importTables(const OriginFile &opj) {
        for (unsigned int s = 0; s < opj.spreadCount(); ++s) {
                Origin::SpreadSheet spread = opj.spread(s);
                int columnCount = spread.columns.size();
                if(!columnCount) //remove tables without cols
                        continue;
                importSpreadsheet(opj, spread);
        }

	return true;
}

bool ImportOpj::importSpreadsheet(const OriginFile &opj, const Origin::SpreadSheet &spread) {
	int cols = spread.columns.size();
	int rows = spread.maxRows;
	if (!cols) // do not create spreadsheets without columns
		return false;

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

	return true;
}

