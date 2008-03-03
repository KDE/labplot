// LabPlot : Plot2D.cc

#include <math.h>
#include <KDebug>
#include <KLocale>
#include <QPainter>
#include "Plot2D.h"
#include "Axis.h"

// general 2D Plot class
Plot2D::Plot2D()
	: Plot() 
{
	kdDebug()<<"Plot2D::Plot2D()"<<endl;
	type = PLOT2D;
/*	QFont font;
	if(p==0)
		kdDebug()<<"WARNING : no Worksheet defined!"<<endl;
	else {
		MainWin *mw = p->getMainWin();
		if(mw)
			font = mw->defaultFont();
	}
	font.setPointSize((int)(0.7*font.pointSize()));	// for axes label
	kdDebug()<<"Plot2D()"<<endl;
*/
	axis.append(new Axis());
	axis.append(new Axis());
	axis.append(new Axis());
	axis.append(new Axis());
	// TODO : font
	axis[0]->getLabel()->setText(i18n("x-Axis"));
	axis[1]->getLabel()->setText(i18n("y-Axis"));
	axis[2]->getLabel()->setText(i18n("y2-Axis"));
	axis[3]->getLabel()->setText(i18n("x2-Axis"));

/* OLD :
	// set default axis settings for all axes
	for(int i=0;i<4;i++)
		readAxisSettings(&axis[i],P2D,i);

	// set default font
	font.setPointSize((int)(0.7*font.pointSize()));	// for tic label
	axis[0].setTickLabelFont(font);
	axis[1].setTickLabelFont(font);
	axis[2].setTickLabelFont(font);
	axis[3].setTickLabelFont(font);
*/
	kdDebug()<<"Plot2D::Plot2D() DONE"<<endl;
}

void Plot2D::setActRange(LRange* r, int i) {
	kdDebug()<<"Plot2D::setActRange("<<i<<")"<<endl;
	LRange tmp;

	double offset=0;//(r->rMax()-r->rMin())/10;

	tmp = LRange(r->rMin()-offset,r->rMax()+offset);
	actrange[i]=tmp;
}

void Plot2D::setActRanges(LRange* r) {
	setActRange(&r[0],0);
	setActRange(&r[1],1);
}

/*void Plot2D::setBorder(int item, bool on) {
	kdDebug()<<"Plot2D::setBorder()"<<endl;
	const int unit = 5, numunit = 40, numunit2 = 20;
	int w = worksheet->width(), h = worksheet->height();

	int xmin = (int)(w*(size.X()*p1.X()+position.X()));
	int xmax = (int)(w*(size.X()*p2.X()+position.X()));
	int ymin = (int)(h*(size.Y()*p1.Y()+position.Y()));
	int ymax = (int)(h*(size.Y()*p2.Y()+position.Y()));

	if(item == 0) {
		if (on) {
			if (!axis[0].getLabel()->Title().isEmpty()) ymax -= axis[0].getLabel()->Font().pointSize();
			if (axis[0].MajorTicksEnabled())   ymax -= unit+numunit2;
			if (axis[0].MinorTicksEnabled())   ymax -= unit;
		}
		else  {
			if (!axis[0].getLabel()->Title().isEmpty()) ymax += axis[0].getLabel()->Font().pointSize();
			if (axis[0].MajorTicksEnabled())   ymax += unit+numunit2;
			if (axis[0].MinorTicksEnabled())   ymax += unit;
		}
	}
	if(item == 1) {
		if (on) {
			if (!axis[1].getLabel()->Title().isEmpty()) xmin += axis[1].getLabel()->Font().pointSize();
			if (axis[1].MajorTicksEnabled())   xmin += unit+numunit;
			if (axis[1].MinorTicksEnabled())   xmin += unit;
		}
		else {
			if (!axis[1].getLabel()->Title().isEmpty()) xmin -= axis[1].getLabel()->Font().pointSize();
			if (axis[1].MajorTicksEnabled())   xmin -= unit+numunit;
			if (axis[1].MinorTicksEnabled())   xmin -= unit;
		}
	}
	if(item == 2) {
		if (on) {
			if (!axis[2].getLabel()->Title().isEmpty()) xmax -= axis[2].getLabel()->Font().pointSize();
			if (axis[2].MajorTicksEnabled())   xmax -= unit+numunit;
			if (axis[2].MinorTicksEnabled())   xmax -= unit;
		}
		else {
			if (!axis[2].getLabel()->Title().isEmpty()) xmax += axis[2].getLabel()->Font().pointSize();
			if (axis[2].MajorTicksEnabled())   xmax += unit+numunit;
			if (axis[2].MinorTicksEnabled())   xmax += unit;

		}
	}
	if(item == 3) {
		if (on) {
			if (!axis[3].getLabel()->Title().isEmpty()) ymin += axis[3].getLabel()->Font().pointSize();
			if (axis[3].MajorTicksEnabled())   ymin += unit+numunit2;
			if (axis[3].MinorTicksEnabled())   ymin += unit;
		}
		else {
			if (!axis[3].getLabel()->Title().isEmpty()) ymin -= axis[3].getLabel()->Font().pointSize();
			if (axis[3].MajorTicksEnabled())   ymin -= unit+numunit2;
			if (axis[3].MinorTicksEnabled())   ymin -= unit;
		}
	}
	setXMin(xmin,w);
	setXMax(xmax,w);
	setYMin(ymin,h);
	setYMax(ymax,h);
}*/

void Plot2D::draw(QPainter *p, int w, int h) {
	kdDebug()<<"Plot2D::draw() w/h :"<<w<<h<<endl;
	kdDebug()<<"	TYPE = "<<type<<endl;
/*
	if(aspect_enabled) {	// set aspect ratio to 1
		int wsize = (int) fmin(w,h);
		w=h=wsize;
	}
*/
	int xmin = (int)(w*(size.X()*p1.X()+position.X()));
	int xmax = (int)(w*(size.X()*p2.X()+position.X()));
	int ymin = (int)(h*(size.Y()*p1.Y()+position.Y()));
	int ymax = (int)(h*(size.Y()*p2.Y()+position.Y()));

	kdDebug()<<"	XMIN-MXAX/YMIN-YMAX = "<<xmin<<'-'<<xmax<<','<<ymin<<'-'<<ymax<<endl;
	kdDebug()<<"	corner1 = "<<p1.X()<<'/'<<p1.Y()<<" corner2 = "<<p2.X()<<'/'<<p2.Y()<<endl;

	if (!transparent) {
		// background color
		p->setBrush(background);
		p->setPen(Qt::NoPen);
		p->drawRect((int)(w*position.X()),(int)(h*position.Y()),(int)(w*size.X()),(int)(h*size.Y()));

		// graph background color
		if(areabackground.style() != Qt::SolidPattern) {	// don't draw plot background pattern in graph area
			p->setBrush(QBrush(Qt::white,Qt::SolidPattern));
			p->setPen(Qt::NoPen);
			p->drawRect(xmin,ymin,xmax-xmin,ymax-ymin);
		}
		p->setBrush(areabackground);
		p->setPen(Qt::NoPen);
		p->drawRect(xmin,ymin,xmax-xmin,ymax-ymin);
	}

	title->draw(p,position,size,w,h,0);

	drawAxes(p,w,h);
	drawBorder(p,w,h);

	// fill between curves
//	if(fill_enabled)
//		drawFill(p,w,h);

	drawCurves(p,w,h);

/*	TScale xscale = axis[0].Scale();
	TScale yscale = axis[1].Scale();
	if (baseline_enabled) {
		double min = actrange[1].rMin();
		double max = actrange[1].rMax();
		int y=0;
		switch(yscale) {
		case LINEAR:
			y = ymax - (int) ((baseline-min)/(max-min)*(double)(ymax-ymin));
			break;
		case LOG10:
			y = ymax - (int) (log10(baseline/min)/log10(max/min)*(double)(ymax-ymin));
			break;
		case LOG2:
			y = ymax - (int) (log2(baseline/min)/log2(max/min)*(double)(ymax-ymin));
			break;
		case LN:
			y = ymax - (int) (log(baseline/min)/log(max/min)*(double)(ymax-ymin));
			break;
		case SQRT:
			y = ymax - (int) (sqrt(baseline-min)/sqrt(max-min)*(double)(ymax-ymin));
			break;
		case SX2:
			y = ymax - (int) (pow(baseline-min,2)/pow(max-min,2)*(double)(ymax-ymin));
			break;
		default: break;
		}
		//kdDebug()<< "Y BASLINE @ "<<y<<endl;
		p->drawLine(xmin,y,xmax,y);
	}
	if (xbaseline_enabled) {
		double min = actrange[0].rMin();
		double max = actrange[0].rMax();
		int x=0;
		switch(xscale) {
		case LINEAR:
			x = xmin + (int) ((xbaseline-min)/(max-min)*(double)(xmax-xmin));
			break;
		case LOG10:
			x = xmin + (int) (log10(xbaseline/min)/log10(max/min)*(double)(xmax-xmin));
			break;
		case LOG2:
			x = xmin + (int) (log2(xbaseline/min)/log2(max/min)*(double)(xmax-xmin));
			break;
		case LN:
			x = xmin + (int) (log(xbaseline/min)/log(max/min)*(double)(xmax-xmin));
			break;
		case SQRT:
			x = xmin + (int) (sqrt(xbaseline-min)/sqrt(max-min)*(double)(xmax-xmin));
			break;
		case SX2:
			x = xmin + (int) (pow(xbaseline-min,2)/pow(max-min,2)*(double)(xmax-xmin));
			break;
		default: break;
		}
		//kdDebug()<< "X BASLINE @ "<<y<<endl;
		p->drawLine(x,ymin,x,ymax);
	}

	if (region_enabled) {
		double min = actrange[0].rMin();
		double max = actrange[0].rMax();

		int minx=0, maxx=0;
		switch(xscale) {
		case LINEAR:
			minx = xmin + (int) ((region->rMin()-min)/(max-min)*(double)(xmax-xmin));
			maxx = xmin + (int) ((region->rMax()-min)/(max-min)*(double)(xmax-xmin));
			break;
		case LOG10:
			minx = xmin + (int) (log10(region->rMin()/min)/log10(max/min)*(double)(xmax-xmin));
			maxx = xmin + (int) (log10(region->rMax()/min)/log10(max/min)*(double)(xmax-xmin));
			break;
		case LOG2:
			minx = xmin + (int) (log2(region->rMin()/min)/log2(max/min)*(double)(xmax-xmin));
			maxx = xmin + (int) (log2(region->rMax()/min)/log2(max/min)*(double)(xmax-xmin));
			break;
		case LN:
			minx = xmin + (int) (log(region->rMin()/min)/log(max/min)*(double)(xmax-xmin));
			maxx = xmin + (int) (log(region->rMax()/min)/log(max/min)*(double)(xmax-xmin));
			break;
		case SQRT:
			minx = xmin + (int) (sqrt(region->rMin()-min)/sqrt(max-min)*(double)(xmax-xmin));
			maxx = xmin + (int) (sqrt(region->rMax()-min)/sqrt(max-min)*(double)(xmax-xmin));
			break;
		case SX2:
			minx = xmin + (int) (pow(region->rMin()-min,2)/pow(max-min,2)*(double)(xmax-xmin));
			maxx = xmin + (int) (pow(region->rMax()-min,2)/pow(max-min,2)*(double)(xmax-xmin));
			break;
		default: break;
		}

		// kdDebug()<<"REGION : "<<minx<<" "<<maxx<<endl;

		if(minx != maxx) {
			p->drawLine(minx,ymin,minx,ymax);
			p->drawLine(maxx,ymin,maxx,ymax);
		}

		if (maxx-minx > 20) {
			int y = (ymax+ymin)/2;
			p->drawLine(minx+5,y,maxx-5,y);
			p->drawLine(minx+5,y,minx+10,y+5);
			p->drawLine(minx+5,y,minx+10,y-5);
			p->drawLine(maxx-5,y,maxx-10,y+5);
			p->drawLine(maxx-5,y,maxx-10,y-5);
		}
	}

	if(marks_enabled) {
		double min = actrange[0].rMin();
		double max = actrange[0].rMax();
		int minx=0, maxx=0;
		switch(xscale) {
		case LINEAR:
			minx = xmin + (int) ((markx->rMin()-min)/(max-min)*(double)(xmax-xmin));
			maxx = xmin + (int) ((markx->rMax()-min)/(max-min)*(double)(xmax-xmin));
			break;
		case LOG10:
			minx = xmin + (int) (log10(markx->rMin()/min)/log10(max/min)*(double)(xmax-xmin));
			maxx = xmin + (int) (log10(markx->rMax()/min)/log10(max/min)*(double)(xmax-xmin));
			break;
		case LOG2:
			minx = xmin + (int) (log2(markx->rMin()/min)/log2(max/min)*(double)(xmax-xmin));
			maxx = xmin + (int) (log2(markx->rMax()/min)/log2(max/min)*(double)(xmax-xmin));
			break;
		case LN:
			minx = xmin + (int) (log(markx->rMin()/min)/log(max/min)*(double)(xmax-xmin));
			maxx = xmin + (int) (log(markx->rMax()/min)/log(max/min)*(double)(xmax-xmin));
			break;
		case SQRT:
			minx = xmin + (int) (sqrt(markx->rMin()-min)/sqrt(max-min)*(double)(xmax-xmin));
			maxx = xmin + (int) (sqrt(markx->rMax()-min)/sqrt(max-min)*(double)(xmax-xmin));
			break;
		case SX2:
			minx = xmin + (int) (pow(markx->rMin()-min,2)/pow(max-min,2)*(double)(xmax-xmin));
			maxx = xmin + (int) (pow(markx->rMax()-min,2)/pow(max-min,2)*(double)(xmax-xmin));
			break;
		default: break;
		}
		min = actrange[1].rMin();
		max = actrange[1].rMax();
		int miny=0, maxy=0;
		switch(yscale) {
		case LINEAR:
			miny = ymax - (int) ((marky->rMin()-min)/(max-min)*(double)(ymax-ymin));
			maxy = ymax - (int) ((marky->rMax()-min)/(max-min)*(double)(ymax-ymin));
			break;
		case LOG10:
			miny = ymax - (int) (log10(marky->rMin()/min)/log10(max/min)*(double)(ymax-ymin));
			maxy = ymax - (int) (log10(marky->rMax()/min)/log10(max/min)*(double)(ymax-ymin));
			break;
		case LOG2:
			miny = ymax - (int) (log2(marky->rMin()/min)/log2(max/min)*(double)(ymax-ymin));
			maxy = ymax - (int) (log2(marky->rMax()/min)/log2(max/min)*(double)(ymax-ymin));
			break;
		case LN:
			miny = ymax - (int) (log(marky->rMin()/min)/log(max/min)*(double)(ymax-ymin));
			maxy = ymax - (int) (log(marky->rMax()/min)/log(max/min)*(double)(ymax-ymin));
			break;
		case SQRT:
			miny = ymax - (int) (sqrt(marky->rMin()-min)/sqrt(max-min)*(double)(ymax-ymin));
			maxy = ymax - (int) (sqrt(marky->rMax()-min)/sqrt(max-min)*(double)(ymax-ymin));
			break;
		case SX2:
			miny = ymax - (int) (pow(marky->rMin()-min,2)/pow(max-min,2)*(double)(ymax-ymin));
			maxy = ymax - (int) (pow(marky->rMax()-min,2)/pow(max-min,2)*(double)(ymax-ymin));
			break;
		default: break;
		}

		p->setPen(QPen(Qt::gray,1,Qt::DashLine));
		p->setFont(QFont("Adobe Times",8));
		QPointArray a;
		if(minx>xmin-clipoffset&&minx<xmax+clipoffset) {
			p->drawLine(minx,ymin,minx,ymax);	// mark x1
			a.setPoints(3,minx,ymax,minx+2,ymax+4,minx-2,ymax+4);
			p->drawPolygon(a);
			a.setPoints(3,minx,ymin,minx+2,ymin-4,minx-2,ymin-4);
			p->drawPolygon(a);
			p->drawText(minx+5,ymax+10,QString::number(markx->rMin()));
		}
		if(maxx>xmin-clipoffset&&maxx<xmax+clipoffset) {
			p->drawLine(maxx,ymin,maxx,ymax);	// mark x2
			a.setPoints(3,maxx,ymax,maxx+2,ymax+4,maxx-2,ymax+4);
			p->drawPolygon(a);
			a.setPoints(3,maxx,ymin,maxx+2,ymin-4,maxx-2,ymin-4);
			p->drawPolygon(a);
			p->drawText(maxx+5,ymax+10,QString::number(markx->rMax()));
		}
		if(miny>ymin-clipoffset&&miny<ymax+clipoffset) {
			p->drawLine(xmin,miny,xmax,miny);	// mark y1
			a.setPoints(3,xmin,miny,xmin-4,miny+2,xmin-4,miny-2);
			p->drawPolygon(a);
			a.setPoints(3,xmax,miny,xmax+4,miny+2,xmax+4,miny-2);
			p->drawPolygon(a);

			p->save();
			p->translate(xmin-5,miny-5);
			p->rotate(-90);
			p->drawText(0,0,QString::number(marky->rMin()));
			p->restore();
		}
		if(maxy>ymin-clipoffset&&maxy<ymax+clipoffset) {
			p->drawLine(xmin,maxy,xmax,maxy);	// mark y2
			a.setPoints(3,xmin,maxy,xmin-4,maxy+2,xmin-4,maxy-2);
			p->drawPolygon(a);
			a.setPoints(3,xmax,maxy,xmax+4,maxy+2,xmax+4,maxy-2);
			p->drawPolygon(a);
			p->save();
			p->translate(xmin-5,maxy-5);
			p->rotate(-90);
			p->drawText(0,0,QString::number(marky->rMax()));
			p->restore();
		}

		// value + diff
		p->drawText((int)((minx+maxx)/2.0),ymax+10,i18n("dx=")+QString::number(markx->rMax()-markx->rMin()));

		p->save();
		p->translate(xmin-5,(int)((miny+maxy)/2.0));
		p->rotate(-90);
		p->drawText(0,0,i18n("dy=")+QString::number(marky->rMax()-marky->rMin()));
		p->restore();

		p->setPen(Qt::black);
	}

	if(legend.Enabled()) {
//		kdDebug()<<"	Legend enabled"<<endl;

		if (type == PSURFACE) {		// legend can't do this :-(
			if (legend.X() == 0.7 && legend.Y() == 0.05 ) // replace the legend for surface plots first time
				legend.setPosition(0.83,0.05);

			int x = (int) (w*(size.X()*legend.X()+position.X()));
			int y = (int) (h*(size.Y()*legend.Y()+position.Y()));

			Plot2DSurface *plot = (Plot2DSurface *)this;
			plot->drawLegend(p,x,y);
			legend.draw(p,type,graphlist,position,size,w,h);
			plot->drawLegend(p,x,y);
		}
		else
			legend.draw(p,type,graphlist,position,size,w,h);

//		legend.draw(p,type,graphlist,position,size,w,h);
//		kdDebug()<<" drawing legend with pos = "<<position.X()<<' '<<position.Y()<<endl;
//		kdDebug()<<" 	size.X()*w/size.Y()*h = "<<size.X()*w<<' '<<size.Y()*h<<endl;

	}
	p->setPen(Qt::NoPen);
*/
	kdDebug()<<"Plot2D::draw() DONE"<<endl;
}

void Plot2D::drawBorder(QPainter *p, int w, int h) {
	kdDebug()<<"Plot2D::drawBorder()"<<endl;
	int xmin = (int)(w*(size.X()*p1.X()+position.X()));
	int xmax = (int)(w*(size.X()*p2.X()+position.X()));
	int ymin = (int)(h*(size.Y()*p1.Y()+position.Y()));
	int ymax = (int)(h*(size.Y()*p2.Y()+position.Y()));
	kdDebug()<<"	xmin/xmax ymin/ymax "<<xmin<<'/'<<xmax<<' '<<ymin<<'/'<<ymax<<endl;

	if (axis[1]->BorderEnabled()) {
		p->setPen(QPen(axis[1]->BorderColor(),axis[1]->borderWidth()));
		p->drawLine(xmin,ymin,xmin,ymax);
	}
	if (axis[3]->BorderEnabled()) {
		p->setPen(QPen(axis[3]->BorderColor(),axis[3]->borderWidth()));
		p->drawLine(xmin,ymin,xmax,ymin);
	}
	if (axis[2]->BorderEnabled()) {
		p->setPen(QPen(axis[2]->BorderColor(),axis[2]->borderWidth()));
		p->drawLine(xmax,ymin,xmax,ymax);
	}
	if (axis[0]->BorderEnabled()) {
		p->setPen(QPen(axis[0]->BorderColor(),axis[0]->borderWidth()));
		p->drawLine(xmin,ymax,xmax,ymax);
	}
}

void Plot2D::drawAxesTicks(QPainter *p, int w, int h, int k) {
	kdDebug()<<"Plot2D::drawAxesTicks()"<<endl;
	Axis *a = axis[k];

	int xmin = (int)(w*(size.X()*p1.X()+position.X()));
	int xmax = (int)(w*(size.X()*p2.X()+position.X()));
	int ymin = (int)(h*(size.Y()*p1.Y()+position.Y()));
	int ymax = (int)(h*(size.Y()*p2.Y()+position.Y()));

	int tmpi, axistype=0;	// 0 : x, 1 : y
	switch(k) {
	case 0: tmpi=0; break;
	case 1: tmpi=2; axistype=1; break;
	case 2: tmpi=6; axistype=1; break;
	case 3: tmpi=4; break;
	}

	if (a->MajorTicksEnabled() && a->Enabled()) {
		double min = actrange[axistype].rMin();
		double max = actrange[axistype].rMax();
		kdDebug()<<"	MIN/MAX "<<min<<max<<endl;
		TScale scale = a->Scale();
		int pos = a->Position();
		if(pos) {
			p->setPen(Qt::SolidLine);
			if (axistype == 0)
				p->drawLine(xmin,(int)((ymax+ymin)/2.0),xmax,(int)((ymax+ymin)/2.0));
			else
				p->drawLine((int)((xmin+xmax)/2.0),ymin,(int)((xmax+xmin)/2.0),ymax);
		}

		if(a->tickType() == 0 || scale != LINEAR) {	// Tick Type NUMBER and all nonlinear scales
			int t=-1;		// number of major tics
			switch (scale) {
			case LINEAR: case SQRT : case SX2:
				t = (int) a->MajorTicks();
//				if(t==-1)
//					t=autoTicks(min,max);
				break;
			case LOG10: t = (int) log10(max/min)+2; break;
			case LOG2: t = (int) log2(max/min)+2; break;
			case LN: t = (int) log(max/min)+2; break;
			}
			if(t==0) t=-1;
			kdDebug()<<"	T="<<t<<endl;

			for (int i = 0;i <= t; i++) {
				int x1=0, x2=0, y1=0,y2=0;

				if(axistype == 0) {	// X Major Ticks
					switch(scale) {
					case LINEAR: {
						x1 = xmin+i*(xmax-xmin)/t;
						x2 = xmin+(i+1)*(xmax-xmin)/t;
						} break;
					case LOG10: {
						double gap = 1.0-log10(pow(10,ceil(log10(min)))/min);	// fragment of decade to shift left
						double decade = (xmax-xmin)/(log10(max/min));		// width of decade
						x1 = (int)(xmin+((i-gap)*decade));
						x2 = (int)(xmin+((i+1-gap)*decade));
						} break;
					case LOG2: {
						double gap = 1.0-log2(pow(2,ceil(log2(min)))/min);		// fragment of decade to shift left
						double decade = (xmax-xmin)/(log2(max/min));		// width of decade
						x1 = (int)(xmin+((i-gap)*decade));
						x2 = (int)(xmin+((i+1-gap)*decade));
						} break;
					case LN: {
						double gap = 1.0-log(pow(M_E,ceil(log(min)))/min);	// fragment of decade to shift left
						double decade = (xmax-xmin)/(log(max/min));		// width of decade
						x1 = (int)(xmin+((i-gap)*decade));
						x1 = (int)(xmin+((i+1-gap)*decade));
						} break;
					case SQRT: {
						x1 = xmin+(int)(sqrt((double)i)*(xmax-xmin)/sqrt((double)t));
						x2 = xmin+(int)(sqrt((double)(i+1))*(xmax-xmin)/sqrt((double)t));
						} break;
					case SX2: {
						x1 = xmin+(i*i)*(xmax-xmin)/(t*t);
						x2 = xmin+(i+1)*(i+1)*(xmax-xmin)/(t*t);
						} break;
					}

					if(x1<=xmax+1 && x1>=xmin-1) { // major tics
						p->setPen(QPen(a->TickColor(),a->majorTickWidth()));
						if(k==0) {	// x
							int ypos = ymax;
							if(pos)
								ypos = (int) ((ymax+ymin)/2.0);
							switch(a->TickPos()) {
							case 0: p->drawLine(x1,ypos,x1,ypos+10); break;
							case 1: p->drawLine(x1,ypos-10,x1,ypos); break;
							case 2: p->drawLine(x1,ypos-10,x1,ypos+10); break;
							case 3: break;
							}
						}
						else {		// x2
							int ypos = ymin;
							if(pos)
								ypos = (int) ((ymax+ymin)/2.0);
							switch(a->TickPos()) {
							case 0: p->drawLine(x1,ypos-10,x1,ypos); break;
							case 1: p->drawLine(x1,ypos,x1,ypos+10); break;
							case 2: p->drawLine(x1,ypos-10,x1,ypos+10); break;
							case 3: break;
							}
						}

						if (a->MajorGridEnabled()) {
							p->setPen(QPen(a->majorGridColor(),a->majorGridWidth(),a->MajorGridType()));
							p->drawLine(x1,ymin,x1,ymax);
							p->setPen(Qt::SolidLine);
						}
					}
				}
				else {			// Y Major Ticks
					switch(scale) {
					case LINEAR:
						y1 = ymax-i*(ymax-ymin)/t;
						y2 = ymax-(i+1)*(ymax-ymin)/t;
						break;
					case LOG10: {
						double gap = 1.0-log10(pow(10,ceil(log10(min)))/min);	// fragment of decade to shift left
						double decade = (double)(ymax-ymin)/(log10(max/min));		// width of decade
						y1 = (int)(ymax-((i-gap)*decade));
						y2 = (int)(ymax-((i+1-gap)*decade));
						} break;
					case LOG2: {
						double gap = 1.0-log2(pow(2,ceil(log2(min)))/min);	// fragment of decade to shift left
						double decade = (ymax-ymin)/(log2(max/min));		// width of decade
						y1 = (int)(ymax-(int)((i-gap)*decade));
						y2 = (int)(ymax-(int)((i+1-gap)*decade));
						} break;
					case LN:{
						double gap = 1.0-log(pow(M_E,ceil(log(min)))/min);	// fragment of decade to shift left
						double decade = (ymax-ymin)/(log(max/min));		// width of decade
						y1 = (int)(ymax-((i-gap)*decade));
						y2 = (int)(ymax-((i+1-gap)*decade));
						} break;
					case SQRT:
						y1 = ymax-(int)(sqrt((double)i)*(ymax-ymin)/sqrt((double)t));
						y2 = ymax-(int)(sqrt((double)(i+1))*(ymax-ymin)/sqrt((double)t));
						break;
					case SX2:
						y1 = ymax-(i*i)*(ymax-ymin)/(t*t);
						y2 = ymax-(i+1)*(i+1)*(ymax-ymin)/(t*t);
						break;
					}

					if(y1<=ymax+1 && y1>=ymin-1) { // major tics
						p->setPen(QPen(a->TickColor(),a->majorTickWidth()));
						if(k==1) {	// y
							int xpos = xmin;
							if(pos)
								xpos = (int) ((xmax+xmin)/2.0);
							switch(a->TickPos()) {
							case 0: p->drawLine(xpos-10,y1,xpos,y1); break;
							case 1: p->drawLine(xpos,y1,xpos+10,y1); break;
							case 2: p->drawLine(xpos-10,y1,xpos+10,y1); break;
							case 3: break;
							}
						}
						else {		// y2
							int xpos = xmax;
							if(pos)
								xpos = (int) ((xmax+xmin)/2.0);
							switch(a->TickPos()) {
							case 0: p->drawLine(xpos,y1,xpos+10,y1); break;
							case 1: p->drawLine(xpos-10,y1,xpos,y1); break;
							case 2: p->drawLine(xpos-10,y1,xpos+10,y1); break;
							case 3: break;
							}
						}
						if (a->MajorGridEnabled()) {
							p->setPen(QPen(a->majorGridColor(),a->majorGridWidth(),a->MajorGridType()));
							p->drawLine(xmin,y1,xmax,y1);
							p->setPen(Qt::SolidLine);
						}
					}
				}

				//if (a->tickLabelEnabled() && graphlist->Number() > 0) {		// Numbers
				if (a->tickLabelEnabled() && set.size() > 0 ) {		// Numbers
					QColor c = a->TickLabelColor();
					QFont f = a->TickLabelFont();
					double dx = max-min, value=0;

					switch(scale) {
					case LINEAR: case SQRT: case SX2: value = min + i*dx/t; break;
					case LOG10: value = pow(10,ceil(log10(min)))*pow(10.0,i-1); break;
					case LOG2: value = pow(2,ceil(log2(min)))*pow(2.0,i-1); break;
					case LN: value = pow(M_E,ceil(log(min)))*pow(M_E,i-1); break;
					}

					// round small values
					int prec = a->TickLabelPrecision();
					if(scale == LINEAR && prec>0)
							value = dx*round(value*pow(10.0,prec)/dx)/pow(10.0,prec);

					// apply scale and shift value
					value = value*a->Scaling()+a->Shift();

					TFormat atlf = a->TickLabelFormat();
					QString label = TicLabel(atlf, prec, a->DateTimeFormat(), value);

					// apply prefix & suffix
					label.prepend(a->TickLabelPrefix());
					label.append(a->TickLabelSuffix());

					// draw tic label
					QFontMetrics fm(f);
					int gap = (int)(a->TickLabelPosition());

					switch(k) {
					case 0:	y1 = ymax + gap + (int)(size.Y()*fm.ascent()/2); break;
					case 1:	x1 = xmin - gap - (int)(size.X()*fm.ascent()); break;
					case 2:	x1 = xmax + gap + (int)(size.X()*fm.ascent()); break;
					case 3:	y1 = ymin - gap - (int)(size.Y()*fm.ascent()/2); break;
					}

					if(pos) {
						switch(k) {
						case 0:	y1 += (int) ((ymin-ymax)/2.0); break;
						case 1:	x1 += (int) ((xmax-xmin)/2.0); break;
						case 2:	x1 += (int) ((xmin-xmax)/2.0); break;
						case 3:	y1 += (int) ((ymax-ymin)/2.0); break;
						}
					}

					p->save();
					p->translate(x1,y1);
					p->rotate(a->TickLabelRotation());
					f.setPointSize((int)(f.pointSize()*size.X()));	// resize tic label
					if (atlf == AUTO || atlf == NORMAL || atlf == SCIENTIFIC) {
						p->setPen(c);
						p->setFont(f);

						if(axistype == 0) {
							if(x1<=xmax+1 && x1>=xmin-1)
								p->drawText(-fm.width(label)/2,fm.ascent()/2,label);
						}
						else {
							if(y1<=ymax+1 && y1>=ymin-1)
								p->drawText(-fm.width(label)/2,fm.ascent()/2-1,label);
						}
					}
					// TODO
					/*else {	// rich text label
						QSimpleRichText *richtext = new QSimpleRichText(label,f);
						QColorGroup cg;
						cg.setColor(QColorGroup::Text,c);

						if(axistype == 0) {
							if(x1<=xmax+1 && x1>=xmin-1)
								richtext->draw(p,-richtext->width()/4, -fm.ascent()/2,QRect(),cg);
						}
						else {
							if(y1<=ymax+1 && y1>=ymin-1)
								richtext->draw(p,-richtext->width(),(int)(-richtext->height()/2.0-1), QRect(),cg);
						}
						delete richtext;
					}*/
					p->restore();
				}

				if (a->MinorTicksEnabled() && i != t ) {	// Minor Ticks
					for (int j=1;j <= a->MinorTicks();j++) {
						if(scale == LOG10 && j > 8)	// maximal 8 minor ticks possible
							break;

						if(axistype == 0) {	// X Minor Ticks
							int x=0;
							if(scale == LINEAR)
								x = x1+j*(x2-x1)/(a->MinorTicks()+1);
							else if (scale == LOG10)
								x = (int)(x1+(x2-x1)*log10((double)(j+1)));
							// TODO : SQRT minor tics ?
							// other scales have no minor tics

							if(x<=xmax+1 && x>=xmin-1) { // minor tics
								p->setPen(QPen(a->TickColor(),a->minorTickWidth()));
								if(k==0) {	// x
									int ypos = ymax;
									if(pos)
										ypos = (int) ((ymax+ymin)/2.0);
									switch(a->TickPos()) {
									case 0: p->drawLine(x,ypos,x,ypos+5); break;
									case 1: p->drawLine(x,ypos-5,x,ypos); break;
									case 2: p->drawLine(x,ypos-5,x,ypos+5); break;
									case 3: break;
									}
								}
								else {		// x2
									int ypos = ymin;
									if(pos)
										ypos = (int) ((ymax+ymin)/2.0);
									switch(a->TickPos()) {
									case 0: p->drawLine(x,ypos-5,x,ypos); break;
									case 1: p->drawLine(x,ypos,x,ypos+5); break;
									case 2: p->drawLine(x,ypos-5,x,ypos+5); break;
									case 3: break;
									}
								}
								if (a->MinorGridEnabled() && j != a->MinorTicks()+1) {
									p->setPen(QPen(a->minorGridColor(),
										a->minorGridWidth(),a->MinorGridType()));
									p->drawLine(x,ymin,x,ymax);
									p->setPen(Qt::SolidLine);
								}
							}
						}
						else {	// Y Minor Ticks
							int y=0;
							if(scale == LINEAR)
								y = y1+j*(y2-y1)/(a->MinorTicks()+1);
							else if (scale == LOG10)
								y = (int)(y1+(double)(y2-y1)*log10((double)(j+1)));
							// all other scales have minor tics = 0

							if(y<=ymax+1 && y>=ymin-1) { // minor tics
								p->setPen(QPen(a->TickColor(),a->minorTickWidth()));
								if(k==1) {	// y
									int xpos = xmin;
									if(pos)
										xpos = (int) ((xmax+xmin)/2.0);
									switch(a->TickPos()) {
									case 0: p->drawLine(xpos-5,y,xpos,y); break;
									case 1: p->drawLine(xpos,y,xpos+5,y); break;
									case 2: p->drawLine(xpos-5,y,xpos+5,y); break;
									case 3: break;
									}
								}
								else {		// y2
									int xpos = xmax;
									if(pos)
										xpos = (int) ((xmax+xmin)/2.0);
									switch(a->TickPos()) {
									case 0: p->drawLine(xpos,y,xpos+5,y); break;
									case 1: p->drawLine(xpos-5,y,xpos,y); break;
									case 2: p->drawLine(xpos-5,y,xpos+5,y); break;
									case 3: break;
									}
								}
								if (a->MinorGridEnabled() && j != a->MinorTicks()+1) {
									p->setPen(QPen(a->minorGridColor(),
										a->minorGridWidth(),a->MinorGridType()));
									p->drawLine(xmin,y,xmax,y);
								}
								p->setPen(Qt::SolidLine);
							}
						}
					}
				}
			}
		}
		else {		// Tic Type INCREMENT
			// use axis Major Ticks as increment
			double inc = a->MajorTicks();
			double dx=max-min;
//			kdDebug()<<"	DX = "<<dx<<endl;
			if(inc == -1)	// auto tics
				inc = pow(10,floor(log10(dx)));
			if(inc >= dx || inc <=0 )	// reset if needed
				inc = dx/10;
//			kdDebug()<<"	INC = "<<inc<<endl;

			int t=(int) ceil(dx/inc);
//			kdDebug()<<"	T = "<<t<<endl;
			double c = inc*floor(min/inc);
//			kdDebug()<<"	C = "<<c<<endl;
			for(int i=0;i<=t;i++) {
				double value = c+i*inc;
				int x1=0, y1=0;

				if(axistype == 0) {	// X Major Ticks
					x1 = xmin+(int)((value-min)*(xmax-xmin)/dx);
//					kdDebug()<<"	VALUE : "<<value<<endl;
//					kdDebug()<<"	X1 : "<<x1<<endl;
					if(x1<=xmax+1 && x1>=xmin-1) {
						p->setPen(QPen(a->TickColor(),a->majorTickWidth()));
						if(k==0) {	// x
							int ypos = ymax;
							if(pos)
								ypos = (int) ((ymax+ymin)/2.0);
							switch(a->TickPos()) {
							case 0: p->drawLine(x1,ypos,x1,ypos+10); break;
							case 1: p->drawLine(x1,ypos-10,x1,ypos); break;
							case 2: p->drawLine(x1,ypos-10,x1,ypos+10); break;
							case 3: break;
							}
						}
						else {		// x2
							int ypos = ymin;
							if(pos)
								ypos = (int) ((ymax+ymin)/2.0);
							switch(a->TickPos()) {
							case 0: p->drawLine(x1,ypos-10,x1,ypos); break;
							case 1: p->drawLine(x1,ypos,x1,ypos+10); break;
							case 2: p->drawLine(x1,ypos-10,x1,ypos+10); break;
							case 3: break;
							}
						}

						if (a->MajorGridEnabled()) {
							p->setPen(QPen(a->majorGridColor(),a->majorGridWidth(),a->MajorGridType()));
							p->drawLine(x1,ymin,x1,ymax);
							p->setPen(Qt::SolidLine);
						}
					}
				}
				else {	// Y Major Axis
					y1 = ymax-(int)((value-min)*(ymax-ymin)/dx);
//					kdDebug()<<"	VALUE : "<<value<<endl;
//					kdDebug()<<"	Y1 : "<<y1<<endl;

					if(y1<=ymax+1 && y1>=ymin-1) { // major tics
						p->setPen(QPen(a->TickColor(),a->majorTickWidth()));
						if(k==1) {	// y
							int xpos = xmin;
							if(pos)
								xpos = (int) ((xmax+xmin)/2.0);
							switch(a->TickPos()) {
							case 0: p->drawLine(xpos-10,y1,xpos,y1); break;
							case 1: p->drawLine(xpos,y1,xpos+10,y1); break;
							case 2: p->drawLine(xpos-10,y1,xpos+10,y1); break;
							case 3: break;
							}
						}
						else {		// y2
							int xpos = xmax;
							if(pos)
								xpos = (int) ((xmax+xmin)/2.0);
							switch(a->TickPos()) {
							case 0: p->drawLine(xpos,y1,xpos+10,y1); break;
							case 1: p->drawLine(xpos-10,y1,xpos,y1); break;
							case 2: p->drawLine(xpos-10,y1,xpos+10,y1); break;
							case 3: break;
							}
						}
						if (a->MajorGridEnabled()) {
							p->setPen(QPen(a->majorGridColor(),a->majorGridWidth(),a->MajorGridType()));
							p->drawLine(xmin,y1,xmax,y1);
							p->setPen(Qt::SolidLine);
						}
					}
				}

				if (a->tickLabelEnabled() && set.size() > 0) {		// Numbers
					QColor c = a->TickLabelColor();
					QFont f = a->TickLabelFont();

					// round small values
					int prec = a->TickLabelPrecision();
					if(prec>0)
						value = inc*round(value*pow(10.0,prec)/inc)/pow(10.0,prec);

					// apply scale and shift value
					value = value*a->Scaling()+a->Shift();

					TFormat atlf = a->TickLabelFormat();
					QString label = TicLabel(atlf, prec, a->DateTimeFormat(), value);

					// apply prefix & suffix
					label.prepend(a->TickLabelPrefix());
					label.append(a->TickLabelSuffix());

					// draw tic label
					QFontMetrics fm(f);
					int gap = (int)(a->TickLabelPosition());

					switch(k) {
					case 0:	y1=ymax+gap+(int)(size.Y()*fm.ascent()/2); break;
					case 1:	x1=xmin-gap-(int)(size.X()*fm.ascent()); break;
					case 2:	x1=xmax+gap+(int)(size.X()*fm.ascent()); break;
					case 3:	y1=ymin-gap-(int)(size.Y()*fm.ascent()/2); break;
					}

					if(pos) {
						switch(k) {
						case 0:	y1 += (int) ((ymin-ymax)/2.0); break;
						case 1:	x1 += (int) ((xmax-xmin)/2.0); break;
						case 2:	x1 += (int) ((xmin-xmax)/2.0); break;
						case 3:	y1 += (int) ((ymax-ymin)/2.0); break;
						}
					}

					p->save();
					p->translate(x1,y1);
					p->rotate(a->TickLabelRotation());
					if (atlf == AUTO || atlf == NORMAL || atlf == SCIENTIFIC) {
						p->setPen(c);
						p->setFont(f);

						if(axistype == 0) {
							if(x1<=xmax+1 && x1>=xmin)
								p->drawText(-fm.width(label)/2,fm.ascent()/2,label);
						}
						else {
							if(y1<=ymax+1 && y1>=ymin-1)
								p->drawText(-fm.width(label)/2,fm.ascent()/2-1,label);
						}
					}
					// TODO
					/*else {	// rich text label
						QSimpleRichText *richtext = new QSimpleRichText(label,f);
						QColorGroup cg;
						cg.setColor(QColorGroup::Text,c);

						if(axistype == 0) {
							if(x1<=xmax+1 && x1>=xmin)
								richtext->draw(p,-richtext->width()/4, -fm.ascent()/2,QRect(),cg);
						}
						else {
							if(y1<=ymax+1 && y1>=ymin-1)
								richtext->draw(p,-richtext->width(),(int)(-richtext->height()/2.0-1), QRect(),cg);
						}
						delete richtext;
					}*/
					p->restore();
				}

				if (a->MinorTicksEnabled()) {	// Minor Ticks
					int tt = a->MinorTicks()+1;
					for (int j=1;j < tt;j++) {
						if(axistype == 0) {	// X Minor Ticks
							int x=0;
							if(scale == LINEAR)
								x = x1+(int)(j*(xmax-xmin)/dx*inc/tt)+1;
							//else if (scale == LOG10)	// TODO : check
							//	x=(int)(x1+t*dx*log10((double)(j)));
							// TODO : SQRT minor tics ?
//							kdDebug()<<"	X ("<<j<<")  ="<<x<<endl;
							// other scales have no minor tics

							if(x<=xmax+1 && x>=xmin-1) { // minor tics
								p->setPen(QPen(a->TickColor(),a->minorTickWidth()));
								if(k==0) {	// x
									int ypos = ymax;
									if(pos)
										ypos = (int) ((ymax+ymin)/2.0);
									switch(a->TickPos()) {
									case 0: p->drawLine(x,ypos,x,ypos+5); break;
									case 1: p->drawLine(x,ypos-5,x,ypos); break;
									case 2: p->drawLine(x,ypos-5,x,ypos+5); break;
									case 3: break;
									}
								}
								else {		// x2
									int ypos = ymin;
									if(pos)
										ypos = (int) ((ymax+ymin)/2.0);
									switch(a->TickPos()) {
									case 0: p->drawLine(x,ypos-5,x,ypos); break;
									case 1: p->drawLine(x,ypos,x,ypos+5); break;
									case 2: p->drawLine(x,ypos-5,x,ypos+5); break;
									case 3: break;
									}
								}
								if (a->MinorGridEnabled() && j != a->MinorTicks()+1) {
									p->setPen(QPen(a->minorGridColor(),
										a->minorGridWidth(),a->MinorGridType()));
									p->drawLine(x,ymin,x,ymax);
									p->setPen(Qt::SolidLine);
								}
							}
						}
						else {	// Y Minor Ticks
							int y=0;
							if(scale == LINEAR)
								y = y1-(int)(j*(ymax-ymin)/dx*inc/tt);
							//else if (scale == LOG10)	// TODO : check
							//	y=(int)(y1+(y2-y1)*log10((double)(j)));
							// all other scales have minor tics = 0
//							kdDebug()<<"	Y ("<<j<<")  ="<<y<<endl;

							if(y<=ymax+1 && y>=ymin-1) { // minor tics
								p->setPen(QPen(a->TickColor(),a->minorTickWidth()));
								if(k==1) {	// y
									int xpos=xmin;
									if(pos)
										xpos = (int) ((xmax+xmin)/2.0);
									switch(a->TickPos()) {
									case 0: p->drawLine(xpos-5,y,xpos,y); break;
									case 1: p->drawLine(xpos,y,xpos+5,y); break;
									case 2: p->drawLine(xpos-5,y,xpos+5,y); break;
									case 3: break;
									}
								}
								else {		// y2
									int xpos=xmax;
									if(pos)
										xpos = (int) ((xmax+xmin)/2.0);
									switch(a->TickPos()) {
									case 0: p->drawLine(xpos,y,xpos+5,y); break;
									case 1: p->drawLine(xpos-5,y,xpos,y); break;
									case 2: p->drawLine(xpos-5,y,xpos+5,y); break;
									case 3: break;
									}
								}
								if (a->MinorGridEnabled() && j != a->MinorTicks()+1) {
									p->setPen(QPen(a->minorGridColor(),
										a->minorGridWidth(),a->MinorGridType()));
									p->drawLine(xmin,y,xmax,y);
								}
								p->setPen(Qt::SolidLine);
							}
						}
					}
				}
			}
		}
	}
}

void Plot2D::drawAxes(QPainter *p,int w, int h) {
	kdDebug()<<"Plot2D::drawAxes()"<<endl;
	const int unit = (int)(5*size.X());
	const int numunit = (int)(40*size.X()), numunit2 = (int)(20*size.X());

	int xmin = (int)(w*(size.X()*p1.X()+position.X()));
	int xmax = (int)(w*(size.X()*p2.X()+position.X()));
	int ymin = (int)(h*(size.Y()*p1.Y()+position.Y()));
	int ymax = (int)(h*(size.Y()*p2.Y()+position.Y()));
//	kdDebug()<<"	xmin/xmax ymin/ymax : "<<xmin<<'/'<<xmax<<' '<<ymin<<'/'<<ymax<<endl;
//	kdDebug()<<"	width/height : "<<w<<'/'<<h<<endl;

	// axes label
	kdDebug()<<"	drawing axis label"<<endl;
	Label *label = axis[3]->getLabel();	// x2
	if (label->X()==0 && label->Y()==0)	// default
		label->setPosition((xmin+(xmax-xmin)/2)/(double)w,
			(ymin-(unit+3*numunit2)*axis[3]->MajorTicksEnabled()-2*unit)/(double)h);
	if (axis[3]->Enabled())
		label->draw(p,position,size,w,h,0);

	label = axis[0]->getLabel();		// x
	if (label->X()==0 && label->Y()==0)	// default
		label->setPosition((xmin+(xmax-xmin)/2)/(double)w,
			(ymax+(unit+numunit2)*axis[0]->MajorTicksEnabled())/(double)h);
	if (axis[0]->Enabled())
		label->draw(p,position,size,w,h,0);

	label = axis[1]->getLabel();		// y
	if (label->X()==0 && label->Y()==0)	// default
		label->setPosition(0.01, (ymin+(ymax-ymin)/2)/(double)h);
	if (axis[1]->Enabled()) {
		p->save();
		label->draw(p,position,size,w,h,270);
		p->restore();
	}

	label = axis[2]->getLabel();		// y2
	if (label->X()==0 && label->Y()==0)	// default
		label->setPosition((xmax+(2*unit+numunit)*axis[2]->MajorTicksEnabled())/(double)w,
			(ymin+(ymax-ymin)/2)/(double)h);
	if (axis[2]->Enabled()) {
		p->save();
		label->draw(p,position,size,w,h,270);
		p->restore();
	}

	// axes tics and grid
	for (int i=0;i<axis.size();i++)
		drawAxesTicks(p, w, h, i);
	kdDebug()<<"Plot2D::drawAxes() DONE"<<endl;
}
/*
void Plot2D::saveAxes(QTextStream *t) {
	for (int i = 0; i < 4; i++)
		saveAxis(t,&axis[i]);
}

void Plot2D::openAxes(QTextStream *t, int version) {
	for(int i = 0;i<4;i++)
		openAxis(t,version,&axis[i]);
}

void Plot2D::saveXML(QDomDocument doc, QDomElement plottag) {
	QDomElement tag;

	for (int i = 0; i < 4; i++) {
		tag = axis[i].saveXML(doc,i);
   		plottag.appendChild( tag );
	}

	if(type == PSURFACE)
		((Plot2DSurface *)this)->saveSurfaceXML(doc,plottag);
}

void Plot2D::openXML(QDomElement e) {
//	kdDebug()<<"Plot2D::openXML()"<<endl;
	if(e.tagName() == "Axis")
		axis[e.attribute("id").toInt()].openXML(e.firstChild());

	if(type == PSURFACE)
		((Plot2DSurface *)this)->openSurfaceXML(e);
}
*/
