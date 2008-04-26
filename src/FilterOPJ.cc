//LabPlot : FilterOPJ.cc

#include <stdio.h>
#include <math.h>

#include <qstring.h>
#include <qregexp.h>
#include <kdebug.h>
#include "FilterOPJ.h"

#include <OPJFile.h>

FilterOPJ::FilterOPJ(MainWin *mw, QString filename)
	: mw(mw),filename(filename)
{}

int FilterOPJ::import() {
	kDebug()<<"FilterOPJ::import() : "<<filename<<endl;
/*
	OPJFile opj(qPrintable(filename));
	qPrintable(filename);	// needed for some reason
	kDebug()<<"	Parsing OPJ file "<<filename<<" ..."<<endl;
	int status = opj.Parse();
	kDebug()<<"	Parsing OPJ file DONE. status = "<<status<<endl;

	kDebug()<<"	Import OPJ file ..."<<endl;
	kDebug()<<"		Number of Spreadsheets : "<<opj.numSpreads()<<endl;
	kDebug()<<"		Number of Matrices : "<<opj.numMatrices()<<endl;
	kDebug()<<"		Number of Functions : "<<opj.numFunctions()<<endl;
	kDebug()<<"		Number of Graphs : "<<opj.numGraphs()<<endl;
	kDebug()<<"		Number of Notes : "<<opj.numNotes()<<endl;

	for (int s=0;s<opj.numSpreads();s++) {
		Spreadsheet *spread = mw->newSpreadsheet();
		kDebug()<<"	SPREADNAME "<<s<<" = "<<opj.spreadName(s)<<endl;
		spread->setTitle(opj.spreadName(s));

		int nr_cols = opj.numCols(s), maxrows = opj.maxRows(s);
		spread->setColumnCount(nr_cols);
		spread->setRowCount(maxrows);

		spread->resetHeader();
		for (int j=0;j<nr_cols;j++) {
			QString name(opj.colName(s,j));
			spread->setColumnName(j,name.replace(QRegExp(".*_"),""));
			spread->setColumnType(j,opj.colType(s,j));

			for (int i=0;i<opj.numRows(s,j);i++) {
				double *v = (double *) opj.oData(s,j,i,true);

				if(strcmp(opj.colType(s,j),"LABEL")) {	// number
					if(fabs(*v)>0 && fabs(*v)<2.0e-300)	// empty entry
						continue;
					spread->setText(i,j,QString::number(*v));
				}
				else					// label
					spread->setText(i,j,QString((char *) opj.oData(s,j,i)));
			}
		}
	}
	*/
	// TODO
/*	for (int s=0;s<opj.numMatrices();s++) {
		kDebug()<<"		Matrix "<<s+1<<" : "<<opj.matrixName(s)<<" (ParentFolder : "<<opj.matrixParentFolder(s)<<")"<<endl;
		kDebug()<<"			Label : "<<opj.matrixLabel(s)<<" Cols/Rows : "<<opj.numMatrixCols(s)<<'/'<<opj.numMatrixRows(s)<<endl;
		kDebug()<<"			Formula : "<<opj.matrixFormula(s)<<" DisplayType : "<<opj.matrixNumDisplayType(s)<<endl;

		Spreadsheet *spread = mw->newSpreadsheet();
		QTable *table = spread->Table();
		kDebug()<<"	SPREADNAME "<<s<<" = "<<opj.matrixName(s)<<endl;
		spread->setTitle(opj.matrixName(s));
		int nr_cols = opj.numMatrixCols(s), nr_rows = opj.numMatrixRows(s);
		table->setNumCols(nr_cols);
		table->setNumRows(nr_rows);
		for (int j=0;j<nr_cols;j++) {
			if(j < 26)
				table->horizontalHeader()->setLabel( j, QChar(j+65)+' '+i18n("{double}")+" [Y]");
			else
				table->horizontalHeader()->setLabel( j,
					QString(QChar(65+j/26-1)) + QString(QChar(65+j%26)) + ' ' + i18n("{double}")+" [Y]");

			for (int i=0;i<nr_rows;i++) {
				double v = opj.matrixData(s,j,i);

				LTableItem *item;
				if(fabs(v)>0 && fabs(v)<2.0e-300)	// empty entry
					continue;
				item = new LTableItem( table, QTableItem::OnTyping,QString::number(v));
				table->setItem(i, j, item);
			}
		}

		QString setnotes = spread->Notes();
		setnotes.append(QString(opj.matrixLabel(s))+"\n");
		setnotes.append(QString(opj.matrixFormula(s))+"\n");
		spread->setNotes(setnotes);
	}

	QString notes = mw->getProject()->Notes();
	for (int s=0;s<opj.numNotes();s++) {
		kDebug()<<"		Note "<<s+1<<" : "<<opj.noteName(s)<<" (ParentFolder : "<<opj.noteParentFolder(s)<<")"<<endl;
		kDebug()<<"			Label : "<<opj.noteLabel(s)<<" Text : "<<opj.noteText(s)<<endl;
		notes.append(QString(opj.noteLabel(s))+":\n");
		notes.append(opj.noteText(s));
	}
	for (int s=0;s<opj.numFunctions();s++) {
		kDebug()<<"		Function "<<s+1<<" : "<<opj.functionName(s)<<" (Type : "<<opj.functionType(s)<<")"<<endl;
		kDebug()<<"			Start/End : "<<opj.functionBegin(s)<<"/"<<opj.functionEnd(s)<<"  Points : "<<opj.functionPoints(s)<<endl;
		kDebug()<<"			Formula : "<<opj.functionFormula(s)<<endl;
		notes.append(QString(opj.functionName(s)+QString(": ")+QString(opj.functionFormula(s))+" "));
		notes.append(QString::number(opj.functionBegin(s))+" - "+QString::number(opj.functionEnd(s)));
		notes.append(" ("+QString::number(opj.functionPoints(s))+" Points)");
		// TODO (later) : import into function
	}

	for (int s=0;s<opj.numGraphs();s++) {
		kDebug()<<"		Graph "<<s+1<<" : "<<opj.graphName(s)<<" (ParentFolder : "<<opj.graphParentFolder(s)<<")"<<endl;
		kDebug()<<"			Label : "<<opj.graphLabel(s)<<" Layers : "<<opj.numLayers(s)<<endl;

		Worksheet *work = mw->newWorksheet();
		Plot2DSimple *plot = (Plot2DSimple *) work->newPlot();
		work->setTitle(opj.graphName(s));
		work->enableTimeStamp(false);
		plot->Title()->setTitle("");

		rect r = opj.graphRect(s);
		kDebug()<<"Rect : "<<r.left<<" "<<r.right<<" "<<r.top<<" "<<r.bottom<<endl;
		kDebug()<<"Number of Layers : "<<opj.numLayers(s)<<endl;
		for(int l=0;l<opj.numLayers(s);l++) {
			rect lr = opj.layerRect(s,l);
			kDebug()<<"LayerRect : "<<lr.left<<" "<<lr.right<<" "<<lr.top<<" "<<lr.bottom<<endl;
#if LIBORIGIN_VERSION < 0x00071119
			kDebug()<<"Layer	x axis : "<<opj.layerXAxisTitle(s,l)<<endl;
			kDebug()<<"Layer	y axis : "<<opj.layerYAxisTitle(s,l)<<endl;
			Label *xlabel = new Label(parseOriginText(QString::fromLocal8Bit(opj.layerXAxisTitle(s,l))));
			Label *ylabel = new Label(parseOriginText(QString::fromLocal8Bit(opj.layerYAxisTitle(s,l))));
			kDebug()<<"Layer	legend : "<<opj.layerLegend(s,l)<<endl;
#else
			kDebug()<<"Layer	x axis : "<<opj.layerXAxisTitle(s,l).txt<<endl;
			kDebug()<<"Layer	y axis : "<<opj.layerYAxisTitle(s,l).txt<<endl;
			Label *xlabel = new Label(parseOriginText(opj.layerXAxisTitle(s,l).txt));
			Label *ylabel = new Label(parseOriginText(opj.layerYAxisTitle(s,l).txt));
			kDebug()<<"Layer	legend : "<<opj.layerLegend(s,l).txt<<endl;
#endif
			plot->getAxis(0)->setLabel(xlabel);
			plot->getAxis(1)->setLabel(ylabel);
			//if(strlen(opj.layerLegend(g,l))>0)
			//	graph->newLegend(parseOriginText(QString::fromLocal8Bit(opj.layerLegend(g,l))));

			kDebug()<<"Layer	nr curves : "<<opj.numCurves(s,l)<<endl;
			for(int c=0;c<opj.numCurves(s,l);c++) {
				kDebug()<<"	Curve "<<c+1<<" :"<<endl;
				int cstyle = opj.curveType(s,l,c);
*/
				/* case OPJFile::Line:
					style=Graph::Line;
				case OPJFile::Scatter:
					style=Graph::Scatter;
				case OPJFile::LineSymbol:
					style=Graph::LineSymbols;
				case OPJFile::ErrorBar:
				case OPJFile::XErrorBar:
					style=Graph::ErrorBars;
				case OPJFile::Column:
					style=Graph::VerticalBars;
				case OPJFile::Bar:
					style=Graph::HorizontalBars;
				case OPJFile::Histogram:
					style=Graph::Histogram;
				*/
/*				kDebug()<<"	Curve Style "<<cstyle<<endl;

				QString data(opj.curveDataName(s,l,c));
				QString tableName;
				switch(data[0].latin1()) {
				case 'T': {
					kDebug()<<"	Table data"<<endl;
					tableName = data.right(data.length()-2);
					kDebug()<<"	Table Name : "<<tableName<<endl;

					// do we have errorbars ? : use next curve
					bool errorbars=false;
					if(c != opj.numCurves(s,l)-1) {
						int nextstyle = opj.curveType(s,l,c+1);
						// TODO : what if xerrorbars ?
						if(nextstyle == OPJFile::ErrorBar || nextstyle == OPJFile::XErrorBar)
							errorbars=true;
					}

					if(errorbars)
						kDebug()<<"	X/Y/DY Column : "<<opj.curveXColName(s,l,c)<<' '<<opj.curveYColName(s,l,c)<<' '<<opj.curveYColName(s,l,c+1)<<endl;
					else
						kDebug()<<"	X/Y Column : "<<opj.curveXColName(s,l,c)<<' '<<opj.curveYColName(s,l,c)<<endl;

					Spreadsheet *ss = mw->getSpreadsheet(tableName);
					if(ss == 0)
						break;
					int xindex = ss->getColumnIndex(opj.curveXColName(s,l,c));
					int yindex = ss->getColumnIndex(opj.curveYColName(s,l,c));
					int zindex;
					if(errorbars) {
						zindex=ss->getColumnIndex(opj.curveYColName(s,l,c+1));
						kDebug()<<"	X/Y/DY column : "<<xindex<<' '<<yindex<<' '<<zindex<<endl;
					}
					else
						kDebug()<<"	X/Y column : "<<xindex<<' '<<yindex<<endl;
					int nr_rows = ss->filledRows(xindex);
					kDebug()<<"	nr of rows : "<<nr_rows<<endl;

					Symbol *symbol = new Symbol();
					symbol->setSize((int)ceil(opj.curveSymbolSize(s,l,c)));
					QColor symbolcolor = translateOriginColor(opj.curveSymbolColor(s,l,c));
					symbol->setColor(symbolcolor);

					switch(opj.curveSymbolType(s,l,c)>>8) {
					case 0:
						symbol->setFill(FFULL);
						symbol->setFillColor(symbolcolor);
						break;
					case 1:
					case 2:
					case 8:
					case 9:
					case 10:
					case 11:
						symbol->setFill(FFULL);
						symbol->setFillColor(translateOriginColor(opj.curveSymbolFillColor(s,l,c)));
						break;
					}
					setSymbolType(symbol,opj.curveSymbolType(s,l,c));

					Style *style = new Style();
					switch(cstyle) {
						case OPJFile::Scatter :
							style->setType(NOLINESTYLE);
							break;
						case OPJFile::LineSymbol :
							style->setType(LINESTYLE);
							break;
					}

					kDebug()<<"Curve line width = "<<opj.curveLineWidth(s,l,c)<<endl;
					kDebug()<<"Curve line color = "<<translateOriginColor(opj.curveLineColor(s,l,c)).name()<<endl;
					kDebug()<<"Curve line style = "<<opj.curveLineStyle(s,l,c)<<endl;
					style->setColor(translateOriginColor(opj.curveLineColor(s,l,c)));
					style->setWidth((int)ceil(opj.curveLineWidth(s,l,c)));
					style->setPenStyle(translateOriginLineStyle(opj.curveLineStyle(s,l,c)));

					QTable *table = ss->Table();
					double xmin,xmax,ymin,ymax;
					if(errorbars) {
						Point3D *ptr = new Point3D[nr_rows];
						double zmin,zmax;
						int row=-1,i=0;
						while(row<table->numRows()) {
							row++;
							if(table->text(row,xindex).isEmpty())
								continue;
							double x=table->text(row,xindex).toDouble();
							double y=table->text(row,yindex).toDouble();
							double z=table->text(row,zindex).toDouble();
							if (i == 0) {
								xmin=xmax=x;
								ymin=ymax=y;
								zmin=zmax=z;
							}
							else {
								x<xmin?xmin=x:0;
								x>xmax?xmax=x:0;
								y<ymin?ymin=y:0;
								y>ymax?ymax=y:0;
								z<zmin?zmin=z:0;
								z>zmax?zmax=z:0;
							}

							ptr[i++].setPoint(x,y,z);
						}

						LRange range[3];
						range[0] = LRange(xmin,xmax);
						range[1] = LRange(ymin,ymax);
						range[2] = LRange(zmin,zmax);

						Graph3D *g = new Graph3D(tableName.latin1(),tableName.latin1(),range,SDATA,P2D,
								style,symbol,ptr,nr_rows,1);
						// g->setLabel(label);

						switch(cstyle) {
						case OPJFile::Line :
						case OPJFile::Scatter :
						case OPJFile::LineSymbol :
							work->addGraph3D(g,P2D);
							break;
						}

						c++;	// next curve already used as errorbar
					}
					else {
						Point *ptr = new Point[nr_rows];
						int row=-1,i=0;
						while(row<table->numRows()) {
							row++;
							if(table->text(row,xindex).isEmpty())
								continue;
							double x=table->text(row,xindex).toDouble();
							double y=table->text(row,yindex).toDouble();
							if (i == 0) {
								xmin=xmax=x;
								ymin=ymax=y;
       						 	}
       						 	else {
								x<xmin?xmin=x:0;
								x>xmax?xmax=x:0;
								y<ymin?ymin=y:0;
								y>ymax?ymax=y:0;
							}
							ptr[i++].setPoint(x,y);
						}

						LRange range[2];
						range[0] = LRange(xmin,xmax);
						range[1] = LRange(ymin,ymax);

						Graph2D *g = new Graph2D(tableName.latin1(),tableName.latin1(),range,SDATA,P2D,
								style,symbol,ptr,nr_rows);
						// g->setLabel(label);

						switch(cstyle) {
						case OPJFile::Line :
						case OPJFile::Scatter :
						case OPJFile::LineSymbol :
							work->addGraph2D(g,P2D);
							break;
						}
					}
					}; break;
				case 'F':
					kDebug()<<"Function data (TODO)"<<endl;
					// TODO
					break;
				}
			}

			// axis range
			vector<double> xrange=opj.layerXRange(s,l);
			vector<double> yrange=opj.layerYRange(s,l);
			LRange range[2];
			range[0] = LRange(xrange[0],xrange[1]);
			range[1] = LRange(yrange[0],yrange[1]);
			plot->setActRanges(range);

			// axis scale
			kDebug()<<"	X Scale : "<<opj.layerXScale(s,l)<<endl;
			kDebug()<<"	Y Scale : "<<opj.layerYScale(s,l)<<endl;
			Axis *xaxis = plot->getAxis(0);
			Axis *yaxis = plot->getAxis(1);
			switch(opj.layerXScale(s,l)) {
			case 0:
				xaxis->setScale(LINEAR);
				break;
			case 1:
				xaxis->setScale(LOG10);
				break;
			case 7:
				xaxis->setScale(LN);
				break;
			case 8:
				xaxis->setScale(LOG2);
				break;
			default:
				xaxis->setScale(LINEAR);
			}
			switch(opj.layerYScale(s,l)) {
			case 0:
				yaxis->setScale(LINEAR);
				break;
			case 1:
				yaxis->setScale(LOG10);
				break;
			case 7:
				yaxis->setScale(LN);
				break;
			case 8:
				yaxis->setScale(LOG2);
				break;
			default:
				yaxis->setScale(LINEAR);
			}

			// TODO : tics
//		      vector<int> ticksX=opj.layerXTicks(g,l);
//		      vector<int> ticksY=opj.layerYTicks(g,l);

			// grid
			vector<graphGrid> grids = opj.layerGrid(s,l);
			xaxis->enableMajorGrid(grids[0].hidden?false:true);
			xaxis->enableMinorGrid(grids[1].hidden?false:true);
			yaxis->enableMajorGrid(grids[2].hidden?false:true);
			yaxis->enableMinorGrid(grids[3].hidden?false:true);
			// grid style
			xaxis->setMajorGridColor(translateOriginColor(grids[0].color));
			xaxis->setMinorGridColor(translateOriginColor(grids[1].color));
			yaxis->setMajorGridColor(translateOriginColor(grids[2].color));
			yaxis->setMinorGridColor(translateOriginColor(grids[3].color));
			xaxis->setMajorGridWidth((int)ceil(grids[0].width));
			xaxis->setMinorGridWidth((int)ceil(grids[1].width));
			yaxis->setMajorGridWidth((int)ceil(grids[2].width));
			yaxis->setMinorGridWidth((int)ceil(grids[3].width));
			xaxis->setMajorGridType(translateOriginLineStyle(grids[0].style));
			xaxis->setMinorGridType(translateOriginLineStyle(grids[1].style));
			yaxis->setMajorGridType(translateOriginLineStyle(grids[2].style));
			yaxis->setMinorGridType(translateOriginLineStyle(grids[3].style));
		}
		work->updatePixmap();
	}
	kDebug()<<"	ResultsLogString : "<<opj.resultsLogString()<<endl;
	notes.append(opj.resultsLogString());
	mw->getProject()->setNotes(notes);
*/
	kDebug()<<"	Import OPJ file DONE"<<endl;

	return 0;
}

void FilterOPJ::setSymbolType(Symbol *symbol,int type) {
	switch(type & 0xFF) {
	case 0: //NoSymbol
		symbol->setType(Symbol::SNONE);
		break;
	case 1: //Rect
		symbol->setType(Symbol::SRECT);
		break;
	case 2: //Ellipse
	case 20://Sphere
		symbol->setType(Symbol::SCIRCLE);
		break;
	case 3: //UTriangle
		symbol->setType(Symbol::STRIANGLE);
		break;
	case 4: //DTriangle
		symbol->setType(Symbol::SUTRIANGLE);
		break;
	case 5: //Diamond
		symbol->setType(Symbol::SDIAMOND);
		break;
	case 6: //Cross +
		symbol->setType(Symbol::SPLUS);
		break;
	case 7: //Cross x
		symbol->setType(Symbol::SCROSS);
		break;
	case 8: //Snow
	case 18: //Star
		symbol->setType(Symbol::SSTAR);
		break;
	case 9: //Horizontal -
		symbol->setType(Symbol::SMINUS);
		break;
	case 10: //Vertical |
		symbol->setType(Symbol::SVBAR);
		break;
	case 15: //LTriangle
		symbol->setType(Symbol::SRTRIANGLE);
		break;
	case 16: //RTriangle
		symbol->setType(Symbol::SLTRIANGLE);
		break;
	case 17: //Hexagon
		symbol->setType(Symbol::SHEXAGON);
		break;
	case 19: //Pentagon
		symbol->setType(Symbol::SPENTA);
		break;
	default:
		symbol->setType(Symbol::SNONE);
	}
}

Qt::PenStyle FilterOPJ::translateOriginLineStyle(int linestyle) const {
	Qt::PenStyle style=Qt::SolidLine;

	// TODO
/*	switch (linestyle) {
		case OPJFile::Solid:
			style=Qt::SolidLine;
			break;
		case OPJFile::Dash:
		case OPJFile::ShortDash:
			style=Qt::DashLine;
			break;
		case OPJFile::Dot:
		case OPJFile::ShortDot:
			style=Qt::DotLine;
			break;
		case OPJFile::DashDot:
		case OPJFile::ShortDashDot:
			style=Qt::DashDotLine;
			break;
		case OPJFile::DashDotDot:
		       	style=Qt::DashDotDotLine;
			break;
	}
*/
	return style;
}

QColor FilterOPJ::translateOriginColor(int color) const {
	QColor qcolor=Qt::black;
	switch(color) {
	case 0:
		qcolor=Qt::black;
		break;
	case 1:
		qcolor=Qt::red;
		break;
	case 2:
		qcolor=Qt::green;
		break;
	case 3:
		qcolor=Qt::blue;
		break;
	case 4:
		qcolor=Qt::cyan;
		break;
	case 5:
		qcolor=Qt::magenta;
		break;
	case 6:
		qcolor=Qt::yellow;
		break;
	case 7:
		qcolor=Qt::darkYellow;
		break;
	case 8:	//Navy
		qcolor=Qt::darkBlue;
		break;
	case 9: // Purple
		qcolor=Qt::darkMagenta;
		break;
	case 10: // Wine
		qcolor=Qt::darkRed;
		break;
	case 11: // Olive
		qcolor=Qt::darkGreen;
		break;
	case 12:
		qcolor=Qt::darkCyan;
		break;
	case 13: //Royal
		qcolor=QColor("#0000A0");
		break;
	case 14: // Orange
		qcolor=QColor("#FF8000");
		break;
	case 15: // Violet
		qcolor=QColor("#8000FF");
		break;
	case 16: // Pink
		qcolor=QColor("#FF0080");
		break;
	case 17:
		qcolor=Qt::white;
		break;
	case 18:
		qcolor=Qt::lightGray;
		break;
	case 19:
		qcolor=Qt::gray;
		break;
	case 20: // LTYellow
		qcolor=QColor("#FFFF80");
		break;
	case 21: // LTCyan
		qcolor=QColor("#80FFFF");
		break;
	case 22: // LTMagenta
		qcolor=QColor("#FF80FF");
		break;
	case 23:
		qcolor=Qt::darkGray;
		break;
	default:
		qcolor=Qt::black;
	}

	return qcolor;
}

QString strreverse(const QString &str) { 	//QString reversing
	QString out="";
	for(int i=str.length()-1; i>=0; --i)
		out+=str[i];

	return out;
}


QString FilterOPJ::parseOriginText(const QString &str) const {
	QStringList lines=str.split("\n");
	QString text="";
	for(int i=0; i<lines.size(); ++i) {
		if(i>0)
			text.append("\n");
		text.append(parseOriginTags(lines[i]));
	}
	return text;
}

QString FilterOPJ::parseOriginTags(const QString &str) const {
	QString line=str;
	//replace \l(...) and %(...) tags
	// TODO
/*	QRegExp rxline("\\\\\\s*l\\s*\\(\\s*\\d+\\s*\\)");
	QRegExp rxcol("\\%\\(\\d+\\)");
	// int pos = rxline.indexIn(line);
	int pos = rxline.search(line);
	while (pos > -1) {
		QString value = rxline.cap(0);
		int len=value.length();
		value.replace(QRegExp(" "),"");
		value="\\c{"+value.mid(3,value.length()-4)+"}";
		line.replace(pos, len, value);
		// pos = rxline.indexIn(line);
		pos = rxline.search(line);
	}
	//Lookbehind conditions are not supported - so need to reverse string
	QRegExp rx("\\)[^\\)\\(]*\\((?!\\s*[buig\\+\\-]\\s*\\\\)");
	QRegExp rxfont("\\)[^\\)\\(]*\\((?![^\\:]*\\:f\\s*\\\\)");
	QString linerev = strreverse(line);
	QString lBracket=strreverse("&lbracket;");
	QString rBracket=strreverse("&rbracket;");
	QString ltagBracket=strreverse("&ltagbracket;");
	QString rtagBracket=strreverse("&rtagbracket;");
	//int pos1=rx.indexIn(linerev);
	//int pos2=rxfont.indexIn(linerev);
	int pos1=rx.search(linerev);
	int pos2=rxfont.search(linerev);

	while (pos1>-1 || pos2>-1) {
		if(pos1==pos2) {
			QString value = rx.cap(0);
			int len=value.length();
			value=rBracket+value.mid(1,len-2)+lBracket;
			linerev.replace(pos1, len, value);
		}
		else if ((pos1>pos2&&pos2!=-1)||pos1==-1) {
			QString value = rxfont.cap(0);
			int len=value.length();
			value=rtagBracket+value.mid(1,len-2)+ltagBracket;
			linerev.replace(pos2, len, value);
		}
		else if ((pos2>pos1&&pos1!=-1)||pos2==-1) {
			QString value = rx.cap(0);
			int len=value.length();
			value=rtagBracket+value.mid(1,len-2)+ltagBracket;
			linerev.replace(pos1, len, value);
		}

		// pos1=rx.indexIn(linerev);
		// pos2=rxfont.indexIn(linerev);
		pos1=rx.search(linerev);
		pos2=rxfont.search(linerev);
	}
	linerev.replace(ltagBracket, "(");
	linerev.replace(rtagBracket, ")");

	line = strreverse(linerev);

	//replace \b(...), \i(...), \u(...), \g(...), \+(...), \-(...), \f:font(...) tags
	QString rxstr[]={
		"\\\\\\s*b\\s*\\(",
		"\\\\\\s*i\\s*\\(",
		"\\\\\\s*u\\s*\\(",
		"\\\\\\s*g\\s*\\(",
		"\\\\\\s*\\+\\s*\\(",
		"\\\\\\s*\\-\\s*\\(",
		"\\\\\\s*f\\:[^\\(]*\\("};
	int postag[]={0,0,0,0,0,0,0};
	QString ltag[]={"<b>","<i>","<u>","<font face=Symbol>","<sup>","<sub>","<font face=%1>"};
	QString rtag[]={"</b>","</i>","</u>","</font>","</sup>","</sub>","</font>"};
	QRegExp rxtags[7];
	for(int i=0; i<7; ++i)
		rxtags[i].setPattern(rxstr[i]+"[^\\(\\)]*\\)");

	bool flag=true;
	while(flag) {
		for(int i=0; i<7; ++i) {
			// postag[i] = rxtags[i].indexIn(line);
			postag[i] = rxtags[i].search(line);
			while (postag[i] > -1) {
				QString value = rxtags[i].cap(0);
				int len=value.length();
				//int pos2=value.indexOf("(");
				int pos2=value.find("(");
				if(i<6)
					value=ltag[i]+value.mid(pos2+1,len-pos2-2)+rtag[i];
				else {
					// int posfont=value.indexOf("f:");
					int posfont=value.find("f:");
					value=ltag[i].arg(value.mid(posfont+2,pos2-posfont-2))+value.mid(pos2+1,len-pos2-2)+rtag[i];
				}
				line.replace(postag[i], len, value);
				// postag[i] = rxtags[i].indexIn(line);
				postag[i] = rxtags[i].search(line);
			}
		}
		flag=false;
		for(int i=0; i<7; ++i) {
			// if(rxtags[i].indexIn(line)>-1) {
			if(rxtags[i].search(line)>-1) {
				flag=true;
				break;
			}
		}
	}

	//replace unclosed tags
	for(int i=0; i<6; ++i)
		line.replace(QRegExp(rxstr[i]), ltag[i]);
	rxfont.setPattern(rxstr[6]);
	// pos = rxfont.indexIn(line);
	pos = rxfont.search(line);
	while (pos > -1) {
		QString value = rxfont.cap(0);
		int len=value.length();
		// int posfont=value.indexOf("f:");
		int posfont=value.find("f:");
		value=ltag[6].arg(value.mid(posfont+2,len-posfont-3));
		line.replace(pos, len, value);
		// pos = rxfont.indexIn(line);
		pos = rxfont.search(line);
	}
*/

	line.replace("&lbracket;", "(");
	line.replace("&rbracket;", ")");

	return line;
}

//TODO: bug in grid dialog
//		scale/minor ticks checkbox
//		histogram: autobin export
//		if prec not setted - automac+4digits
