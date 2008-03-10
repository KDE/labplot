//LabPlot Plot2DSimple.cc

#include <KDebug>
#include <QPainter>
//#include <QPointArray>
#include <math.h>
#include "Plot2DSimple.h"
#include "Set2D.h"
#include "Axis.h"
#include "settype.h"

// simple 2D Plot class
Plot2DSimple::Plot2DSimple()
	: Plot2D()
{}

/*QStringList Plot2DSimple::Info() {
	QStringList s;
	s<<"2D";
	s<<QString::number(position.X())+QString(" , ")+QString::number(position.Y());
	s<<QString::number(size.X())+QString(" X ")+QString::number(size.Y());
	if (transparent)
		s<<QString("yes");
	else
		s<<QString("no");
	s<<bgcolor.color().name();
	s<<gbgcolor.color().name();

	return s;
}

// fill between graph fillg1 and fillg2
void Plot2DSimple::drawFill(QPainter *p,int w, int h) {
	if(fillg1 > (int) graphlist->Number() || fillg2 > (int) graphlist->Number() )
		return;

	const int xmin = (int)(w*(size.X()*p1.X()+position.X()));
	const int xmax = (int)(w*(size.X()*p2.X()+position.X()));
	const int ymin = (int)(h*(size.Y()*p1.Y()+position.Y()));
	const int ymax = (int)(h*(size.Y()*p2.Y()+position.Y()));
	// clipping rect with some space (clipoffset)
	p->setClipRect(xmin-clipoffset,ymin-clipoffset,xmax-xmin+2*clipoffset,ymax-ymin+2*clipoffset);

	GRAPHType s1 = graphlist->getType(fillg1);
	GRAPHType s2 = graphlist->getType(fillg2);

	if (s1 == GRAPH2D && s2 ==  GRAPH2D) {
		Graph2D *g1 = graphlist->getGraph2D(fillg1);
		Graph2D *g2 = graphlist->getGraph2D(fillg2);
		QPointArray pa(g1->Number()+g2->Number());

		if(g1->Number() != g2->Number())
			filltype=0;

		int ptnr=0;
		for(int i=0;i<g1->Number();i++) {
			double x=xmin,y=ymax;
			calculateXY(g1->Data()[i],&x,&y,w,h);
			if(x==xmin) x=xmin+1;	// don't overdraw axes
			if(y==ymin) y=ymin+1;	// don't overdraw axes
			if(filltype==1) {
				if(g1->Data()[i].Y()<g2->Data()[i].Y())
					continue;
			}
			else if(filltype==2){
				if(g1->Data()[i].Y()>g2->Data()[i].Y())
					continue;
			}
			else if(filltype==3){
				if(g1->Data()[i].X()<region->rMin() || g1->Data()[i].X()>region->rMax())
					continue;
			}
			pa[ptnr++] = QPoint((int)x,(int)y);
		}
		for(int i=g2->Number()-1;i>=0;i--) {
			double x=xmin,y=ymax;
			calculateXY(g2->Data()[i],&x,&y,w,h);
			if(x==xmin) x=xmin+1;	// don't overdraw axes
			if(y==ymin) y=ymin+1;	// don't overdraw axes
			if(filltype==1) {
				if(g1->Data()[i].Y()<g2->Data()[i].Y())
					continue;
			}
			else if(filltype==2) {
				if(g1->Data()[i].Y()>g2->Data()[i].Y())
					continue;
			}
			else if(filltype==3){
				if(g2->Data()[i].X()<region->rMin() || g2->Data()[i].X()>region->rMax())
					continue;
			}

			pa[ptnr++] = QPoint((int)x,(int)y);
		}

		pa.resize(ptnr);
		p->setPen(Qt::NoPen);
		p->setBrush(fillbrush);
		p->drawPolygon(pa);
	}
}
*/
void Plot2DSimple::calculateXY(Point d,double *x, double *y, int w, int h) {
	const int xmin = (int)(w*(size.X()*p1.X()+position.X()));
	const int xmax = (int)(w*(size.X()*p2.X()+position.X()));
	const int ymin = (int)(h*(size.Y()*p1.Y()+position.Y()));
	const int ymax = (int)(h*(size.Y()*p2.Y()+position.Y()));
	double minx = actrange[0].Min();
	double maxx = actrange[0].Max();
	double miny = actrange[1].Min();
	double maxy = actrange[1].Max();

	switch(axis[0]->Scale()) {
		case LINEAR:	*x += (d.X() - minx) * (xmax-xmin)/(maxx-minx); break;
		case LOG10:
			if(d.X() <= 0)
				*x=xmin;
			else
				*x += (log10(d.X()/minx)) * (xmax-xmin)/(log10(maxx/minx));
			break;
		case LOG2:
			if(d.X() <= 0)
				*x=xmin;
			else
				*x += (log2(d.X()/minx)) * (xmax-xmin)/(log2(maxx/minx));
			break;
		case LN:
			if(d.X() <= 0)
				*x=xmin;
			else
				*x += (log(d.X()/minx)) * (xmax-xmin)/(log(maxx/minx));
			break;
		case SQRT:
			if(d.X() < 0)
				*x=xmin;
			else
				*x += (sqrt(d.X())-sqrt(minx)) * (xmax-xmin)/(sqrt(maxx)-sqrt(minx)); break;
		case SX2:	*x += ((d.X()*d.X())-(minx*minx))*(xmax-xmin)/((maxx*maxx)-(minx*minx)); break;
	}
	switch(axis[1]->Scale()) {
		case LINEAR:	*y -= (d.Y() - miny) * (ymax-ymin)/(maxy-miny); break;
		case LOG10:
			if(d.Y() <= 0)
				*y=ymax;
			else
				*y -= (log10(d.Y()/miny)) * (ymax-ymin)/(log10(maxy/miny));
			break;
		case LOG2:
			if(d.Y() <= 0)
				*y=ymax;
			else
				*y -= (log2(d.Y()/miny)) * (ymax-ymin)/(log2(maxy/miny));
			break;
		case LN:
			if(d.Y() <= 0)
				*y=ymax;
			else
				*y -= (log(d.Y()/miny)) * (ymax-ymin)/(log(maxy/miny));
			break;
		case SQRT:
			if(d.Y() < 0)
				*y=ymax;
			else
				*y -= (sqrt(d.Y())-sqrt(miny)) * (ymax-ymin)/(sqrt(maxy)-sqrt(miny));
			break;
		case SX2:	*y -= ((d.Y()*d.Y())-(miny*miny)) * (ymax-ymin)/((maxy*maxy)-(miny*miny)); break;
	}
}

void Plot2DSimple::drawCurves(QPainter *p,int w, int h) {
	kdDebug()<<"Plot2DSimple::drawCurves()"<<endl;
	const int xmin = (int)(w*(size.X()*p1.X()+position.X()));
	const int xmax = (int)(w*(size.X()*p2.X()+position.X()));
	const int ymin = (int)(h*(size.Y()*p1.Y()+position.Y()));
	const int ymax = (int)(h*(size.Y()*p2.Y()+position.Y()));
	//kdDebug()<<"xmin/xmax ymin/ymax : "<<xmin<<'/'<<xmax<<' '<<ymin<<'/'<<ymax<<endl;

	// clipping rect with some space (clipoffset)
	// TODO
	//p->setClipRect(xmin-clipoffset,ymin-clipoffset,xmax-xmin+2*clipoffset,ymax-ymin+2*clipoffset);

	double minx = actrange[0].Min();
	double maxx = actrange[0].Max();
	double miny = actrange[1].Min();
	double maxy = actrange[1].Max();

	for (int i=0; i < set.size() ; i++) {
		SetType stype = set[i]->Type();

		kdDebug()<<"Set "<<i<<endl;
		kdDebug()<<"Set type "<<stype<<endl;
		// TODO
		//if(set[i]->isShown() == false)
		//	continue;
// TODO
		if (stype == SET2D) {
			Set2D *s = (Set2D *) set[i];
//			kdDebug()<<"GRAPH2D Label = "<<g->getLabel()->simpleTitle()<<endl;

			//kdDebug()<<"Type T2D\n";
			//kdDebug()<<"xmin/xmax "<<xmin<<' '<<xmax<<endl;
			//kdDebug()<<"ymin/ymax "<<ymin<<' '<<ymax<<endl;
			//kdDebug()<<"xact1/xact2 "<<actrange[0].rMin()<<' '<<actrange[0].rMax()<<endl;
			//kdDebug()<<"yact1/yact2 "<<actrange[1].rMin()<<' '<<actrange[1].rMax()<<endl;

			Point *d = s->Data();
			double oldx = 0.0, oldy = 0.0;
			//QPointArray pa(g->Number());
			QVector<QPoint> pa;
			int pointindex=0;
			for(int j=0;j<s->Number();j++) {
				if(d[j].Masked() == true)
					continue;

				// TODO
				/*if(worksheet->getMainWin()->speedMode()) {
					int mod = (int) (g->Number()/worksheet->getMainWin()->speedModeValue());
					if(mod==0) mod=1;
					if(j%mod != 0) continue;	// speed mode
				}*/

				double x=xmin,y=ymax;
				calculateXY(d[j],&x,&y,w,h);

				//TODO
				// dont draw outside border
				// HACK : prevent drawing errors with too large coordinates
				if(oldy > 10*ymax && y > 10*ymax || oldy < -1000 && y < -1000 ||
					oldx > 10*xmax && x > 10*xmax || oldx < -1000 && x < -1000)
					;
				else {
					pa.resize(pointindex+1);
					pa[pointindex++] = QPoint((int)x,(int)y);
				}				

				// TODO
				//s->getAnnotateValues().draw(p,(int)x,(int)y,d[j].X(),d[j].Y());

				oldx = x;oldy = y;
			}
			// TODO
			pa.resize(pointindex);	// resize to clear all unneeded points (from draw outside border)
			// TODO
			//if(g->getStyle()->PointsSortingEnabled())
			//	sortPoints(pa,0,pa.size()-1);
			// TODO
			drawStyle(p,s->getStyle(),s->getSymbol(),pa,xmin,xmax,ymin,ymax);
		}
/*		else if (s == GRAPH3D)	{ // 2D error plot
			Graph3D *g = graphlist->getGraph3D(i);

			//kdDebug()<<"GRAPH3D\n";
			//kdDebug()<<"OK : "<<i<<" 3D number="<<g->Number()<<endl;
			//kdDebug()<<"OK : "<<i<<" NX/NY="<<g->NX()<<"/"<<g->NY()<<endl;

			Point3D *d = g->Data();
			double oldx=0, oldy=0;
			int N;
			if (g->NY()==0)
				N=g->NX();
			else
				N=(g->NX())*(g->NY());

			QPointArray pa(N), hpa(N), vpa(N);
			int pointindex=0;
			for(int j=0 ;j < N;j++) {
				if(d[j].Masked() == true)
					continue;

				if(worksheet->getMainWin()->speedMode()) {
					int mod = (int) (N/worksheet->getMainWin()->speedModeValue());
					if(mod==0) mod=1;
					if(j%mod != 0) continue;	// speed mode
				}
				double x=xmin,y=ymax;

				// see Graph2D
				switch(axis[0].Scale()) {
				case LINEAR:	x += (d[j].X() - minx) * (xmax-xmin)/(maxx-minx); break;
				case LOG10:	x += (log10(d[j].X()/minx)) * (xmax-xmin)/(log10(maxx/minx)); break;
				case LOG2:	x += (log2(d[j].X()/minx)) * (xmax-xmin)/(log2(maxx/minx)); break;
				case LN:		x += (log(d[j].X()/minx)) * (xmax-xmin)/(log(maxx/minx)); break;
				case SQRT:	x += (sqrt(d[j].X())-sqrt(minx)) * (xmax-xmin)/(sqrt(maxx)-sqrt(minx)); break;
				case SX2:	x += ((d[j].X()*d[j].X())-minx*minx)*(xmax-xmin)/(maxx*maxx-minx*minx); break;
				}

				double ybottom=0, ytop=0;

				switch(axis[1].Scale()) {
				case LINEAR: {
					y = ymax - (d[j].Y() - miny) * (ymax-ymin)/(maxy-miny);
					double diff = d[j].Z() *(ymax-ymin)/(maxy-miny);
					ybottom = y-diff;
					ytop = y+diff;
					}; break;
				case LOG10: {
					double yscale = (ymax-ymin)/log10(maxy/miny);
					y -= log10(d[j].Y()/miny) * yscale;
					ybottom = ymax - log10((d[j].Y()-d[j].Z())/miny) * yscale;
					ytop = ymax - log10((d[j].Y()+d[j].Z())/miny) * yscale;
					}; break;
				case LOG2: {
					double yscale = (ymax-ymin)/log2(maxy/miny);
					y -= log2(d[j].Y()/miny) * yscale;
					ybottom = ymax - log2((d[j].Y()-d[j].Z())/miny) * yscale;
					ytop = ymax - log2((d[j].Y()+d[j].Z())/miny) * yscale;
					}; break;
				case LN: {
					double yscale = (ymax-ymin)/log(maxy/miny);
					y -= log(d[j].Y()/miny) * yscale;
					ybottom = ymax - log((d[j].Y()-d[j].Z())/miny) * yscale;
					ytop = ymax - log((d[j].Y()+d[j].Z())/miny) * yscale;
					}; break;
				case SQRT: {
					double yscale = (ymax-ymin)/(sqrt(maxy)-sqrt(miny));
					y -= (sqrt(d[j].Y())-sqrt(miny)) * yscale;
					ybottom = ymax - (sqrt(d[j].Y()-d[j].Z())-sqrt(miny)) * yscale;
					ytop = ymax - (sqrt(d[j].Y()+d[j].Z())-sqrt(miny)) * yscale;
					}; break;
				case SX2: {
					double yscale = (ymax-ymin)/((maxy*maxy)-(miny*miny));
					y -= ((d[j].Y()*d[j].Y())-(miny*miny)) * yscale;
					ybottom = ymax - ((d[j].Y()-d[j].Z())*(d[j].Y()-d[j].Z())-(miny*miny)) * yscale;
					ytop = ymax - ((d[j].Y()+d[j].Z())*(d[j].Y()+d[j].Z())-(miny*miny)) * yscale;
					}; break;
				}

				//kdDebug()<<"ytop="<<ytop<<",ybottom="<<ybottom<<endl;

				// errorbar points;
				hpa[pointindex]=QPoint((int)x,(int)x);
				vpa[pointindex]=QPoint((int)ytop,(int)ybottom);

				// HACK : prevent drawing errors with too large coordinates
				if(oldy>10*ymax && y>10*ymax || oldy<-1000 && y<-1000 ||
					oldx>10*xmax && x>10*xmax || oldx<-1000 && x<-1000)
					;
				else
					pa[pointindex++]=QPoint((int)x,(int)y);

				g->getAnnotateValues().draw(p,(int)x,(int)y,d[j].X(),d[j].Y(),d[j].Z());

				oldx = x;oldy = y;
			}
			pa.resize(pointindex);	// resize to clear all unneeded points (from draw outside border)
			Style *style=g->getStyle();
			Symbol *symbol = g->getSymbol();
			if(style->PointsSortingEnabled())
				sortPoints(pa,0,pa.size()-1);
			drawStyle(p,style,symbol,pa,xmin,xmax,ymin,ymax);
			p->setPen(style->Color());
			symbol->errorBar()->draw(p,pa,hpa,vpa);
		}
		else if (s == GRAPH4D) {		// x-y-dx-dy or x-y-dy1-dy2
			Graph4D *g = graphlist->getGraph4D(i);

			//kdDebug()<<"GRAPH4D"<<endl;
			//kdDebug()<<"OK : "<<i<<" number="<<g->Number()<<endl;

			Point4D *d = g->Data();
			double oldx=0, oldy=0;
			int N=g->Number();
			bool gtype = g->GType();
			double xscale, yscale;
			QPointArray pa(N), hpa(N), vpa(N);
			int pointindex=0;

			for(int j=0 ;j < N;j++) {
				if(d[j].Masked() == true)
					continue;

				if(worksheet->getMainWin()->speedMode()) {
					int mod = (int) (N/worksheet->getMainWin()->speedModeValue());
					if(mod==0) mod=1;
					if(j%mod != 0) continue;	// speed mode
				}
				double x=0, y=0;
				double xright=0,xleft=0, ytop=0,ybottom=0;

				switch(axis[0].Scale()) {
				case LINEAR: {
					xscale = (xmax-xmin)/(maxx-minx);
					x = xmin + (d[j].X() - minx) * xscale;
					if (gtype == 0) {	// x-y-dx-dy
						double diffx = d[j].Z() * xscale;
						xright = x+diffx;
						xleft = x-diffx;
					}
					else
						xleft=xright=x;
					}; break;
				case LOG10: {
					xscale = (xmax-xmin)/(log10(maxx/minx));
					x = xmin + (log10(d[j].X()/minx)) * xscale;
					if (gtype == 0) {	// x-y-dx-dy
						xleft = xmin + (log10((d[j].X()-d[j].Z())/minx)) * xscale;
						xright = xmin + (log10((d[j].X()+d[j].Z())/minx)) * xscale;
					}
					else
						xleft=xright=x;
					}; break;
				case LOG2: {
					xscale = (xmax-xmin)/(log2(maxx/minx));
					x = xmin + (log2(d[j].X()/minx)) * xscale;
					if (gtype == 0) {	// x-y-dx-dy
						xleft = xmin + (log2((d[j].X()-d[j].Z())/minx)) * xscale;
						xright = xmin + (log2((d[j].X()+d[j].Z())/minx)) * xscale;
					}
					else
						xleft=xright=x;
					}; break;
				case LN: {
					xscale = (xmax-xmin)/(log(maxx/minx));
					x = xmin + (log(d[j].X()/minx)) * xscale;
					if (gtype == 0) {	// x-y-dx-dy
						xleft = xmin + (log((d[j].X()-d[j].Z())/minx)) * xscale;
						xright = xmin + (log((d[j].X()+d[j].Z())/minx)) * xscale;
					}
					else
						xleft=xright=x;
					}; break;
				case SQRT: {
					xscale = (xmax-xmin)/(sqrt(maxx)-sqrt(minx));

					x = xmin + (sqrt(d[j].X())-sqrt(minx)) * xscale;
					if (gtype == 0) {	// x-y-dx-dy
						xleft = xmin + (sqrt(d[j].X()-d[j].Z())-sqrt(minx)) * xscale;
						xright = xmin + (sqrt(d[j].X()+d[j].Z())-sqrt(minx)) * xscale;
					}
					else
						xleft=xright=x;
					}; break;
				case SX2: {
					xscale = (xmax-xmin)/(maxx*maxx-minx*minx);

					x = xmin + (d[j].X()*d[j].X()-minx*minx) * xscale;
					if (gtype == 0) {	// x-y-dx-dy
						xleft = xmin + ((d[j].X()-d[j].Z())*(d[j].X()-d[j].Z())-minx*minx) * xscale;
						xright = xmin + ((d[j].X()+d[j].Z())*(d[j].X()+d[j].Z())-minx*minx) * xscale;
					}
					else
						xleft=xright=x;
					}; break;
				}

				switch(axis[1].Scale()) {
				case LINEAR: {
					yscale = (ymax-ymin)/(maxy-miny);
					y = ymax - (d[j].Y() - miny) * yscale;
					if (gtype == 0) {	// x-y-dx-dy
						double diff = d[j].T() * yscale;
						ytop = y+diff;
						ybottom = y-diff;
					}
					else {
						double diff1 = d[j].Z() * yscale;
						double diff2 = d[j].T() * yscale;
						ybottom = y-diff1;
						ytop = y+diff2;
					}
					}; break;
				case LOG10: {
					yscale = (ymax-ymin)/log10(maxy/miny);
					y = ymax - log10(d[j].Y()/miny) * yscale;
					if (gtype == 0)
						ybottom = ymax - log10((d[j].Y()-d[j].T())/miny) * yscale;
					else
						ybottom = ymax - log10((d[j].Y()-d[j].Z())/miny) * yscale;
					ytop = ymax - log10((d[j].Y()+d[j].T())/miny) * yscale;
					}; break;
				case LOG2: {
					yscale = (ymax-ymin)/log2(maxy/miny);
					y = ymax - log2(d[j].Y()/miny) * yscale;
					if (gtype == 0)
						ybottom = ymax - log2((d[j].Y()-d[j].T())/miny) * yscale;
					else
						ybottom = ymax - log2((d[j].Y()-d[j].Z())/miny) * yscale;
					ytop = ymax - log2((d[j].Y()+d[j].T())/miny) * yscale;
					}; break;
				case LN: {
					yscale = (ymax-ymin)/log(maxy/miny);
					y = ymax - log(d[j].Y()/miny) * yscale;
					if (gtype == 0)
						ybottom = ymax - log((d[j].Y()-d[j].T())/miny) * yscale;
					else
						ybottom = ymax - log((d[j].Y()-d[j].Z())/miny) * yscale;
					ytop = ymax - log((d[j].Y()+d[j].T())/miny) * yscale;
					}; break;
				case SQRT: {
					yscale = (ymax-ymin)/(sqrt(maxy)-sqrt(miny));
					y = ymax - (sqrt(d[j].Y())-sqrt(miny)) * yscale;
					if (gtype == 0)
						ybottom = ymax - (sqrt(d[j].Y()-d[j].T())-sqrt(miny)) * yscale;
					else
						ybottom = ymax - (sqrt(d[j].Y()-d[j].Z())-sqrt(miny)) * yscale;
					ytop = ymax - (sqrt(d[j].Y()+d[j].T())-sqrt(miny)) * yscale;
					}; break;
				case SX2: {
					yscale = (ymax-ymin)/(maxy*maxy-miny*miny);
					y = ymax - ((d[j].Y()*d[j].Y())-miny*miny) * yscale;
					if (gtype == 0)
						ybottom = ymax - ((d[j].Y()-d[j].T())*(d[j].Y()-d[j].T())-miny*miny) * yscale;
					else
						ybottom = ymax - ((d[j].Y()-d[j].Z())*(d[j].Y()-d[j].Z())-miny*miny) * yscale;
					ytop = ymax - ((d[j].Y()+d[j].T())*(d[j].Y()+d[j].T())-miny*miny) * yscale;
					}; break;
				}

				//kdDebug()<<"xleft="<<xleft<<",xright="<<xright<<endl;
				//kdDebug()<<"ytop="<<ytop<<",ybottom="<<ybottom<<endl;

				// errorbar points;
				hpa[pointindex]=QPoint((int)xleft,(int)xright);
				vpa[pointindex]=QPoint((int)ytop,(int)ybottom);

				// HACK : prevent drawing errors with too large coordinates
				if(oldy>10*ymax && y>10*ymax || oldy<-1000 && y<-1000 ||
					oldx>10*xmax && x>10*xmax || oldx<-1000 && x<-1000)
					;
				else
					pa[pointindex++]=QPoint((int)x,(int)y);

				// TODO : use Z,T too ?
				g->getAnnotateValues().draw(p,(int)x,(int)y,d[j].X(),d[j].Y());

				oldx = x;oldy = y;
			}
			pa.resize(pointindex);	// resize to clear all unneeded points (from draw outside border)
			Style *style = g->getStyle();
			Symbol *symbol = g->getSymbol();
			if(style->PointsSortingEnabled())
				sortPoints(pa,0,pa.size()-1);
			drawStyle(p,style,symbol,pa,xmin,xmax,ymin,ymax);
			p->setPen(style->Color());
			symbol->errorBar()->draw(p,pa,hpa,vpa);
		}
*/
	}

	p->setClipping(false);
	kdDebug()<<"Plot2DSimple::drawCurves() DONE"<<endl;
}

