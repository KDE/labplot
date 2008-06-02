#include "Plot.h"
#include "elements/Set.h"
#include "elements/Axis.h"

#include <KDebug>
#include <KLocale>
#include <QDateTime>
#include <QPainter>

#include <cmath>

Plot::Plot(){
	kDebug()<<"Plot::Plot()"<<endl;
	title = new Label(i18n("Title"));
	// OLD : title = new Label(i18n("Title"),font,QColor(Qt::black));
	title->setPosition( QPoint(0.4,0.04) );

	background = QBrush(Qt::white);
	areabackground = QBrush(Qt::yellow);	// Qt::white
/*	bgcolor.setColor(config->readColorEntry("BackgroundColor",&Qt::white));
	bgcolor.setStyle((Qt::BrushStyle)config->readNumEntry("BackgroundBrush",Qt::SolidPattern));
	gbgcolor.setColor(config->readColorEntry("GraphBackgroundColor",&Qt::white));
	gbgcolor.setStyle((Qt::BrushStyle)config->readNumEntry("GraphBackgroundBrush",Qt::SolidPattern));
*/
	position = Point(0,0);
	size = Point(1,1);
/*	position.setPoint(config->readDoubleNumEntry("Position X",0.0),config->readDoubleNumEntry("Position Y",0.0));
	size.setPoint(config->readDoubleNumEntry("Size X",1.0),config->readDoubleNumEntry("Size Y",1.0));
*/
	transparent=false;

	p1.setPoint(.15,.15);
	p2.setPoint(.95,.85);
}

//! build the tic label string according to atlf
QString Plot::TicLabel(int atlf, int prec, QString dtf, double value) const {
	QString label;

	switch(atlf) {
	case Axis::LABELSFORMAT_AUTO:
		label = QString::number(value,'g',prec);
		break;
	case Axis::LABELSFORMAT_NORMAL:
		label = QString::number(value,'f',prec);
		break;
	case Axis::LABELSFORMAT_SCIENTIFIC:
		label = QString::number(value,'e',prec);
		break;
	case Axis::LABELSFORMAT_POWER10:
		label = "10<span style=\"vertical-align:super\">"+ QString::number(log10(value),'g',prec)+"</span>";
		break;
	case Axis::LABELSFORMAT_POWER2:
		label = "2<span style=\"vertical-align:super\">"+ QString::number(log2(value),'g',prec)+"</span>";
		break;
	case Axis::LABELSFORMAT_POWERE:
		label = "e<span style=\"vertical-align:super\">"+ QString::number(log(value),'g',prec)+"</span>";
		break;
	case Axis::LABELSFORMAT_FSQRT:
		label = "sqrt("+ QString::number(value*value,'g',prec) + ")";
		break;
	case Axis::LABELSFORMAT_TIME: {
		QTime time;
		time=time.addMSecs((int) (value*1000));

		QString format;
		if(fabs(value)<1)
			format="z";
		else if(fabs(value)<10) {
			format="s.zzz";
			if (prec==0)
				format="s";
			else if (prec==1) {
				// round to 100 ms
				int ms=time.msec();
				time=time.addMSecs(-ms);
				ms = 100*(int)rint(ms/100);
				time=time.addMSecs(ms);
			}
			else if (prec==2) {
				// round to 10 ms
				int ms=time.msec();
				time=time.addMSecs(-ms);
				ms = 10*(int)rint(ms/10);
				time=time.addMSecs(ms);
			}
		}
		else if (fabs(value)<3600) {
			format = "m:ss";
			if (prec==0) {
				int s=time.second();
				// round to full minute
				time=time.addSecs(-s);
				if(s>=30)
					time=time.addSecs(60);
				format="m";
			}
			else if (prec==1) {
				// round to 10 seconds
				int s=time.second();
				time=time.addSecs(-s);
				s = 10*(int)rint(s/(int)10);
				time=time.addSecs(s);
			}
		}
		else {
			// TODO : round minutes
			format="h:mm:ss";
		}

		// overwrite auto format
		if (dtf != i18n("auto"))
			format = dtf;
		label=time.toString(format);
		kDebug()<<"VALUE in Time Format : "<<label<<endl;
		}
		break;
	case Axis::LABELSFORMAT_DATE: {
		QDate date(1970,1,1);
		date=date.addDays((int) value);
		QString format("dd.MM.yyyy");
		if (dtf != i18n("auto"))
			format = dtf;
			label=date.toString(format);
			kDebug()<<"VALUE in Date Format ( "<<format<<") : "<<label<<endl;
		}
		break;
	case Axis::LABELSFORMAT_DATETIME: {
		QDate date(1970,1,1);
		QDateTime datetime(date);
//		kDebug()<<"value = "<<(int) value<<endl;
		datetime=datetime.addSecs((int)value);
		QString format("dd.MM.yyyy h:mm:ss");
		if (dtf != i18n("auto"))
			format = dtf;
		label = datetime.toString(format);
//		kDebug()<<"VALUE in DateTime Format ( "<<format<<") : "<<label<<endl;
		}
		break;
	case Axis::LABELSFORMAT_DEGREE:
		label = QString::number(180.0/M_PI*value,'f',prec)+'°';
		break;
	}

	return label;
}

void Plot::resetRanges() {
	kDebug()<<"Plot::resetRanges()"<<endl;

	double xmin=0,xmax=1,ymin=0,ymax=1,zmin=0,zmax=1;
	for (int i=0;i<set.size();i++) {
		kDebug()<<"	using set "<<i<<endl;
		if(!set[i]->isShown())
			continue;	// do not use hidden graphs

		Set::SetType stype = set[i]->type();
		kDebug()<<"	Set "<<i<<" / Type = "<<stype<<endl;

		Range xrange,yrange,zrange;
		if(stype == Set::SET2D) {
			kDebug()<<"Set2D"<<endl;
			if(set[i] == 0) {
				kDebug()<<"ERROR : set == 0!"<<endl;
				continue;
			}
// 			xrange = ((Set2D *)set[i])->getRange(0);
// 			yrange = ((Set2D *)set[i])->getRange(1);
			xrange = set[i]->ranges[0];
			yrange = set[i]->ranges[1];
		}
		/*else if(s == GRAPH3D) {
			kDebug()<<"	GRAPH3D"<<endl;
			Graph3D *g = graphlist->getGraph3D(i);
			if(g == 0)
				kDebug()<<"ERROR : g==0!"<<endl;
			PType t = plot[api]->Type();
			kDebug()<<"	PType = "<<t<<endl;

			xrange = g->Range(0);
			if(t == P2D)	// x-y-dy
				yrange = g->ErrorDYRange();
			else
				yrange = g->Range(1);
			zrange = g->Range(2);
		}
		else if(s == GRAPHM) {
			kDebug()<<"GRAPHM"<<endl;
			GraphM *g = graphlist->getGraphM(i);
			xrange = g->Range(0);
			yrange = g->Range(1);
			zrange = g->Range(2);
		}
		else if(s == GRAPH4D) {
			kDebug()<<"GRAPH4D"<<endl;
			Graph4D *g = graphlist->getGraph4D(i);
			xrange = g->ErrorDXRange();
			yrange = g->ErrorDYRange();
		}
		else if(s == GRAPHIMAGE) {
			kDebug()<<"GRAPHIMAGE"<<endl;
			GraphIMAGE *g = graphlist->getGraphIMAGE(i);
			xrange = g->Range(0);
			yrange = g->Range(1);
		}*/

		if (i==0) {	// first set
			xmin=xrange.min();
			xmax=xrange.max();
			ymin=yrange.min();
			ymax=yrange.max();

			/*if (s == GRAPH3D || s == GRAPHM) {
				zmin=zrange.Min();
				zmax=zrange.Max();
			}*/
		}
		else {
			xrange.min()<xmin?xmin=xrange.min():0;
			xrange.max()>xmax?xmax=xrange.max():0;
			yrange.min()<ymin?ymin=yrange.min():0;
			yrange.max()>ymax?ymax=yrange.max():0;

			/*if (s == GRAPH3D || s == GRAPHM) {
				zrange.Min()<zmin?zmin=zrange.Min():0;
				zrange.Max()>zmax?zmax=zrange.Max():0;
			}*/
		}
	}

	// fix zero range	TODO : MIN
	if(xmax-xmin == 0) {
		kDebug()<<"WARNING: x range 0."<<endl;
		xmin -= 1.0;
		xmax += 1.0;
	}
	if(ymax-ymin == 0) {
		kDebug()<<"WARNING: y range 0."<<endl;
		ymin -= 1.0;
		ymax += 1.0;
	}
	if(zmax-zmin == 0) {
		kDebug()<<"WARNING: z range 0."<<endl;
		zmin -= 1.0;
		zmax += 1.0;
	}

	kDebug()<<"	xmin/xmax "<<xmin<<' '<<xmax<<endl;
	kDebug()<<"	ymin/ymax "<<ymin<<' '<<ymax<<endl;
	kDebug()<<"	zmin/zmax "<<zmin<<' '<<zmax<<endl;

	Range range[3];
	range[0] = Range(xmin,xmax);
	range[1] = Range(ymin,ymax);
	range[2] = Range(zmin,zmax);
	setRanges(range);

	//mw->setModified();

	kDebug()<<"Plot::resetRanges() DONE"<<endl;
}

void Plot::drawStyle(QPainter *p, Style *style, Symbol *symbol, QVector<QPoint> pa, int xmin, int xmax, int ymin, int ymax) {
	kDebug()<<"Plot::drawStyle()"<<endl;
	bool filled = style->hasFill();
	QColor c = style->fillColor();
 	QPen pen( style->color(), style->width(),(Qt::PenStyle)style->penStyle() );
	p->setPen(pen);
	QBrush brush(c,style->brushStyle());

	// calculate baseline
	double min = actrange[1].min();
	double max = actrange[1].max();
	double minx = actrange[0].min();
	double maxx = actrange[0].max();

	//int basey = ymax - (int) ((baseline-min)/(max-min)*(double)(ymax-ymin));
	//int basex = xmin + (int) ((xbaseline-minx)/(maxx-minx)*(double)(xmax-xmin));
	int bw = style->boxWidth();
	//int rmin = xmin + (int) ((region->Min()-minx)/(maxx-minx)*(double)(xmax-xmin));
	//int rmax = xmin + (int) ((region->Max()-minx)/(maxx-minx)*(double)(xmax-xmin));
	//kDebug()<<"BASEX = "<<basex<<endl;
	//kDebug()<<"BASEY = "<<basey<<endl;


//	kDebug()<<"POINTARRAY "<<i<<'='<<pa[i].x()<<' '<<pa[i].y()<<endl;

	switch(style->type()) {
	case LINESTYLE:
		/*if (filled) {
			p->setPen(Qt::NoPen);
			p->setBrush(brush);

			//QPointArray fillpa(pa.size()+2);
			QVector<QPoint> fillpa(pa.size()+2);
			if(fabs((double)(rmax-rmin)) > 0) {	//draw only region
				int index=0,newindex=1;
				while(pa[index].x() < rmin)
					index++;
				fillpa[0] = QPoint(pa[index].x(),basey);
				while(pa[index].x() < rmax)
					fillpa[newindex++] = pa[index++];
				fillpa[newindex] = QPoint(pa[index-1].x(),basey);
				fillpa.resize(newindex+1);
			}
			else {
				fillpa[0] = QPoint(pa[0].x(),basey);
				for(unsigned int i=0;i<pa.size();i++)
					fillpa[i+1]=pa[i];
				fillpa[pa.size()+1]=QPoint(pa[pa.size()-1].x(),basey);
			//}
			p->drawPoints(fillpa);
			p->setPen(pen);
		}*/
		p->drawPolyline(pa);
		break;
	case NOLINESTYLE: break;
/*	case STEPSSTYLE: { // only for 2d
		QPointArray stepspa(2*pa.size());
		// start
		stepspa[0]=pa[0];
		stepspa[1]=QPoint((pa[0].x()+pa[1].x())/2,pa[0].y());
		// end
		stepspa[2*pa.size()-2]=QPoint((int)((pa[pa.size()-2].x()+pa[pa.size()-1].x())/2.0),pa[pa.size()-1].y());
		stepspa[2*pa.size()-1]=pa[pa.size()-1];
		for(unsigned int i=1;i<pa.size()-1;i++) {
			stepspa[2*i]=QPoint((int)((pa[i-1].x()+pa[i].x())/2.0),pa[i].y());		// left
			stepspa[2*i+1]=QPoint((int)((pa[i+1].x()+pa[i].x())/2.0),pa[i].y());		// right
		}
		if (filled) {
			QPointArray fillpa(stepspa.size()+2);
			p->setPen(Qt::NoPen);
			p->setBrush(brush);
			if(fabs((double)(rmax-rmin)) > 0) {	// draw only region
				int index=0,newindex=1;
				while(stepspa[index].x() < rmin)
					index++;
				fillpa[0] = QPoint(stepspa[index].x(),basey);
				while(stepspa[index].x() < rmax)
					fillpa[newindex++] = stepspa[index++];
				fillpa[newindex] = QPoint(stepspa[index-1].x(),basey);
				fillpa.resize(newindex+1);
			}
			else {
				fillpa[0]=QPoint(stepspa[0].x(),basey);
				for(unsigned int i=0;i<stepspa.size();i++)
					fillpa[i+1]=stepspa[i];
				fillpa[stepspa.size()+1]=QPoint(stepspa[stepspa.size()-1].x(),basey);
			}
			p->drawPolygon(fillpa);
			p->setPen(pen);
		}
		p->drawPolyline(stepspa);
		}; break;
	case BOXESSTYLE: // only for 2d
		for(unsigned int i=0;i<pa.size();i++) {
			QPointArray boxpa(4);

			if(style->AutoBoxWidth()) {
				if(i==0) {
					boxpa[0]=QPoint(pa[i].x()-(int)((pa[i+1].x()-pa[i].x())/2.0),pa[i].y());
					boxpa[1]=QPoint(pa[i].x()+(int)((pa[i+1].x()-pa[i].x())/2.0),pa[i].y());
					boxpa[2]=QPoint(pa[i].x()+(int)((pa[i+1].x()-pa[i].x())/2.0),basey);
					boxpa[3]=QPoint(pa[i].x()-(int)((pa[i+1].x()-pa[i].x())/2.0),basey);
				}
				else if(i==pa.size()-1) {
					boxpa[0]=QPoint(pa[i].x()-(int)((pa[i].x()-pa[i-1].x())/2.0),pa[i].y());
					boxpa[1]=QPoint(pa[i].x()+(int)((pa[i].x()-pa[i-1].x())/2.0),pa[i].y());
					boxpa[2]=QPoint(pa[i].x()+(int)((pa[i].x()-pa[i-1].x())/2.0),basey);
					boxpa[3]=QPoint(pa[i].x()-(int)((pa[i].x()-pa[i-1].x())/2.0),basey);
				}
				else {
					boxpa[0]=QPoint(pa[i].x()-(int)((pa[i].x()-pa[i-1].x())/2.0),pa[i].y());
					boxpa[1]=QPoint(pa[i].x()+(int)((pa[i+1].x()-pa[i].x())/2.0),pa[i].y());
					boxpa[2]=QPoint(pa[i].x()+(int)((pa[i+1].x()-pa[i].x())/2.0),basey);
					boxpa[3]=QPoint(pa[i].x()-(int)((pa[i].x()-pa[i-1].x())/2.0),basey);
				}
			}
			else {
				boxpa[0]=QPoint(pa[i].x()-bw/2,pa[i].y());
				boxpa[1]=QPoint(pa[i].x()+bw/2,pa[i].y());
				boxpa[2]=QPoint(pa[i].x()+bw/2,basey);
				boxpa[3]=QPoint(pa[i].x()-bw/2,basey);
			}

			p->setBrush(Qt::NoBrush);
			if (filled)
				p->setBrush(brush);
			p->drawPolygon(boxpa);
		}
		break;
	case IMPULSESSTYLE:	// only for 2d
		for (unsigned int i=0;i<pa.size();i++)
			p->drawLine(pa[i].x(),pa[i].y(),pa[i].x(),basey);
		break;
	case YBOXESSTYLE:	// only for 2d
		for(unsigned int i=0;i<pa.size();i++) {
			QPointArray boxpa(4);

			if(style->AutoBoxWidth()) {
				if(i==0) {
					boxpa[0]=QPoint(pa[i].x(),(int)(pa[i].y()-(pa[i+1].y()-pa[i].y())/2.0));
					boxpa[1]=QPoint(pa[i].x(),(int)(pa[i].y()+(pa[i+1].y()-pa[i].y())/2.0));
					boxpa[2]=QPoint(basex,pa[i].y()+(int)((pa[i+1].y()-pa[i].y())/2.0));
					boxpa[3]=QPoint(basex,pa[i].y()- (int)((pa[i+1].y()-pa[i].y())/2.0));
				}
				else if(i==pa.size()-1) {
					boxpa[0]=QPoint(pa[i].x(),pa[i].y()-(int)((pa[i].y()-pa[i-1].y())/2.0));
					boxpa[1]=QPoint(pa[i].x(),pa[i].y()+(int)((pa[i].y()-pa[i-1].y())/2.0));
					boxpa[2]=QPoint(basex,pa[i].y()+(int)((pa[i].y()-pa[i-1].y())/2.0));
					boxpa[3]=QPoint(basex, pa[i].y()-(int)((pa[i].y()-pa[i-1].y())/2.0));
				}
				else {
					boxpa[0]=QPoint(pa[i].x(),pa[i].y()-(int)((pa[i].y()-pa[i-1].y())/2.0));
					boxpa[1]=QPoint(pa[i].x(),pa[i].y()+(int)((pa[i+1].y()-pa[i].y())/2.0));
					boxpa[2]=QPoint(basex,pa[i].y()+(int)((pa[i+1].y()-pa[i].y())/2.0));
					boxpa[3]=QPoint(basex, pa[i].y()-(int)((pa[i].y()-pa[i-1].y())/2.0));
				}
			}
			else {
				boxpa[0]=QPoint(pa[i].x(),pa[i].y()-bw/2);
				boxpa[1]=QPoint(pa[i].x(),pa[i].y()+bw/2);
				boxpa[2]=QPoint(basex, pa[i].y()+bw/2);
				boxpa[3]=QPoint(basex, pa[i].y()-bw/2);
			}

			p->setBrush(Qt::NoBrush);
			if (filled)
				p->setBrush(brush);
			p->drawPolygon(boxpa);
		}
		break;
*/
	}

	// draw symbol
// 	for (int i=0;i<pa.size();i++)
// TODO		symbol->draw(p,pa[i]);
}
