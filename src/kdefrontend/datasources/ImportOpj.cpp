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
#include "backend/core/Workbook.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include "backend/note/Note.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"

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
	importGraphs(opj);
	importNotes(opj);
//      if(filename.endsWith(".opj", Qt::CaseInsensitive))
//              createProjectTree(opj);

}

int ImportOpj::importTables(const OriginFile &opj) {
	// excels (origin workbook with one or more sheets)
	for (unsigned int e = 0; e < opj.excelCount(); ++e) {
			Origin::Excel excelwb = opj.excel(e);
			if (excelwb.sheets.size() == 1) {	// single sheet -> spreadsheet
				Origin::SpreadSheet spread = excelwb.sheets[0];
				spread.name = excelwb.name;
				spread.label = excelwb.label;
				importSpreadsheet(0, opj, spread);
			}
			else {		// multiple sheets -> workbook
				Workbook *workbook = new Workbook(0, excelwb.name.c_str() + QString(" - ") + excelwb.label.c_str());
				for (unsigned int s = 0; s < excelwb.sheets.size(); ++s) {
					Origin::SpreadSheet spread = excelwb.sheets[s];
					importSpreadsheet(workbook, opj, spread);
				}
				mw->addAspectToProject(workbook);
			}
	}

	// matrices
	for (unsigned int m = 0; m < opj.matrixCount(); ++m) {
		Origin::Matrix matrix = opj.matrix(m);
		importMatrix(opj, matrix);
	}

	return 0;
}

int ImportOpj::importSpreadsheet(Workbook* workbook, const OriginFile &opj, const Origin::SpreadSheet &spread) {
	Q_UNUSED(opj);
	int cols = spread.columns.size();
	int rows = spread.maxRows;
	if (!cols) // do not create spreadsheet without columns
		return -1;

	QLocale locale = mw->locale();
	Spreadsheet* spreadsheet;
	if (workbook == 0)	// single sheet
		spreadsheet = new Spreadsheet(0, spread.name.c_str() + QString(" - ") + spread.label.c_str());
	else			// multiple sheets (TODO: name of sheets are not saved in liborigin: "Sheet1", "Sheet2", ...)
		spreadsheet = new Spreadsheet(0, spread.name.c_str());
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

				Double2StringFilter *filter = static_cast<Double2StringFilter*>(col->outputFilter());
				filter->setNumericFormat(f);
				filter->setNumDigits(column.decimalPlaces);
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

			DateTime2StringFilter *filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
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

			DateTime2StringFilter *filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
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

			DateTime2StringFilter *filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
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

			DateTime2StringFilter *filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
			break;
		}
		case Origin::ColumnHeading:
		case Origin::TickIndexedDataset:
		case Origin::Categorical:
			break;
		}
	}

	//TODO
//	if (spread.hidden || spread.loose)
//		mw->hideWindow(spreadsheet);

	if (workbook == 0) // single sheet
		mw->addAspectToProject(spreadsheet);
	else	// multiple sheets
		workbook->addChild(spreadsheet);

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

int ImportOpj::importNotes(const OriginFile &opj) {
//	int visible_count = 0;
	for(unsigned int n = 0; n < opj.noteCount(); ++n) {
		Origin::Note _note = opj.note(n);
		QString name = _note.name.c_str();
		QRegExp rx("^@(\\S+)$");
		if(rx.indexIn(name) == 0)
			name = name.mid(2, name.length() - 3);
		Note *note = new Note(name);
		if(!note)
			return -1;
		//note->setWindowLabel(_note.label.c_str());
		note->setNote(QString(_note.text.c_str()));

		// TODO
		//cascade the notes
		//int dx = 20;
		//int dy = note->parentWidget()->frameGeometry().height() - note->height();
		//note->parentWidget()->move(QPoint(visible_count*dx+xoffset*OBJECTXOFFSET, visible_count*dy));

		mw->addAspectToProject(note);

//		visible_count++;
	}
//	if(visible_count > 0)
//		xoffset++;
	return 0;
}

int ImportOpj::importGraphs(const OriginFile &opj) {
	for(unsigned int g = 0; g < opj.graphCount(); ++g) {
		Origin::Graph _graph = opj.graph(g);
		Worksheet *worksheet = new Worksheet(0, _graph.name.c_str());
		if (!worksheet)
			return -1;

//		worksheet->hide();//!hack used in order to avoid resize and repaint events
		worksheet->setComment(_graph.label.c_str());
		unsigned int layers = _graph.layers.size();
		for (unsigned int l = 0; l < layers; ++l) {
			Origin::GraphLayer& layer = _graph.layers[l];
			CartesianPlot* plot = new CartesianPlot("");
			if (!plot)
				return -2;

			if (!layer.legend.text.empty()) {
				CartesianPlotLegend* legend = new CartesianPlotLegend(plot, "");
				TextLabel* title = new TextLabel(legend->name(), TextLabel::PlotLegendTitle);
				title->setText(parseOriginText(QString::fromLocal8Bit(layer.legend.text.c_str())));
				//legend->title() = title;
				legend->addChild(title);
				plot->addChild(legend);
			}

			// TODO: we only support one legend
			//add texts
			//for (unsigned int i = 0; i < layer.texts.size(); ++i)
			//	plot->newLegend(parseOriginText(QString::fromLocal8Bit(layer.texts[i].text.c_str())));

			int auto_color = 0;
			int style = 0;
			for (unsigned int c = 0; c < layer.curves.size(); ++c) {
				Origin::GraphCurve& _curve = layer.curves[c];
				QString data(_curve.dataName.c_str());
				int color = 0;

				switch(_curve.type) {
				case Origin::GraphCurve::Line:
//					style = Graph::Line;
					break;
				case Origin::GraphCurve::Scatter:
//					style = Graph::Scatter;
					break;
				case Origin::GraphCurve::LineSymbol:
//					style = Graph::LineSymbols;
					break;
				case Origin::GraphCurve::ErrorBar:
				case Origin::GraphCurve::XErrorBar:
//					style = Graph::ErrorBars;
					break;
				case Origin::GraphCurve::Column:
//					style = Graph::VerticalBars;
					break;
				case Origin::GraphCurve::Bar:
//					style = Graph::HorizontalBars;
					break;
				case Origin::GraphCurve::Histogram:
//					style = Graph::Histogram;
					break;
				default:
					continue;
				}

/*				QString tableName;
				switch(data[0].toAscii()) {
				case 'T':
				case 'E': {
					tableName = data.right(data.length() - 2);
					Table* table = mw->table(tableName);
					if (!table)
						break;
					if(style == Graph::ErrorBars) {
						int flags=_curve.symbolType;
						graph->addErrorBars(QString("%1_%2").arg(tableName, _curve.xColumnName.c_str()), table, QString("%1_%2").arg(tableName, _curve.yColumnName.c_str()),
							((flags&0x10)==0x10?0:1), ceil(_curve.lineWidth), ceil(_curve.symbolSize), QColor(Qt::black),
							(flags&0x40)==0x40, (flags&2)==2, (flags&1)==1);
					} else if(style == Graph::Histogram) {
						graph->insertCurve(table, QString("%1_%2").arg(tableName, _curve.yColumnName.c_str()), style);
					} else {
						graph->insertCurve(table, QString("%1_%2").arg(tableName, _curve.xColumnName.c_str()), QString("%1_%2").arg(tableName, _curve.yColumnName.c_str()), style);
					}
					break;
				}
				//TODO
				}
*/

/*				CurveLayout cl = graph->initCurveLayout(style, layer.curves.size());
				cl.sSize = ceil(_curve.symbolSize*0.5);
				cl.penWidth = _curve.symbolThickness;
				color = _curve.symbolColor.regular;
				if((style == Graph::Scatter || style == Graph::LineSymbols) && color == 0xF7) // 0xF7 -Automatic color
					color = auto_color++;
				cl.symCol = color;
				switch(_curve.symbolType & 0xFF) {
				case 0: //NoSymbol
					cl.sType = 0;
					break;
				//TODO
				}
*/

				//TODO

			}

			worksheet->addChild(plot);
		}

		mw->addAspectToProject(worksheet);
	}

	return 0;
}

QString ImportOpj::parseOriginText(const QString &str) {
	QStringList lines = str.split("\n");
	QString text = "";
	for (int i = 0; i < lines.size(); ++i) {
		if(i > 0)
			text.append("\n");
		text.append(parseOriginTags(lines[i]));
	}

	return text;
}

QString ImportOpj::parseOriginTags(const QString &str) {
	QString line = str;
	//TODO

	return line;
}
