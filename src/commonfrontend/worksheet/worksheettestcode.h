#include <math.h>

void WorksheetView::startTestCode() {
//   QGraphicsRectItem *rect = scene()->addRect(QRectF(0, 0, 50, 50));
// 		rect->setFlag(QGraphicsItem::ItemIsMovable, true);
// rect->setFlag(QGraphicsItem::ItemIsSelectable, true);
// 
//   QGraphicsRectItem *rect2 = scene()->addRect(QRectF(50, 50, 50, 50));
// 		rect2->setFlag(QGraphicsItem::ItemIsMovable, true);
// rect2->setFlag(QGraphicsItem::ItemIsSelectable, true);
//  
// //  return;

  QRectF pageRect = m_model->scene()->sceneRect();
  
  #define SCALE_MIN CartesianCoordinateSystem::Scale::LIMIT_MIN
  #define SCALE_MAX CartesianCoordinateSystem::Scale::LIMIT_MAX
  
  const double pw = pageRect.width();
  const double ph = pageRect.height();
  
  {
	DecorationPlot *plot = new DecorationPlot("plot1");
	plot->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
	plot->graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	m_worksheet->addChild(plot);
// 	plot->graphicsItem()->setSelected(true);
	
	
	CartesianCoordinateSystem *coordSys = new CartesianCoordinateSystem("coords1");
	
	QList<CartesianCoordinateSystem::Scale *> scales;
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, 2), pw * 0.3, pw * 0.4, -2, 2);
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(3, 6), pw * 0.4 + 2, pw * 0.65, 3, 6);
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(7, SCALE_MAX), pw * 0.65 + 5, pw * 0.7, 7, 10);
	coordSys->setXScales(scales);
	scales.clear();
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, 4), ph * 0.8, ph * 0.65, 1, 4);
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(8, SCALE_MAX), ph * 0.65 - 5, ph * 0.5, 8, 10);
	coordSys->setYScales(scales);
	
	plot->addChild(coordSys);
	
	
	PlotArea *plotArea = new PlotArea("plot area");
	plotArea->setRect(QRectF(-2, 0, 12, 10));
	plotArea->setClippingEnabled(true);
	coordSys->addChild(plotArea);
	
	/*
	WorksheetRectangleElement *rect = new WorksheetRectangleElement("rect1");
	rect->setRect(QRectF(12, 12, 300, 3));
	coordSys->addChild(rect);
	WorksheetRectangleElement *rect2 = new WorksheetRectangleElement("rect2");
	rect2->setRect(QRectF(0, 0, 40, 30));
	m_worksheet->addChild(rect2);
	WorksheetRectangleElement *rect3 = new WorksheetRectangleElement("rect3");
	rect3->setRect(QRectF(pageRect.width() / 2 - 2, pageRect.height() / 2 - 2, 10 + 4, 120 + 4));
	m_worksheet->addChild(rect3);
	
	WorksheetElementGroup *group1 = new WorksheetElementGroup("some items");
		group1->addChild(new WorksheetRectangleElement("rect 1", QRectF(5, 5, 20, 20)));
		group1->addChild(new WorksheetRectangleElement("rect 1", QRectF(4, 5, 25, 15)));
		group1->addChild(new WorksheetRectangleElement("rect 1", QRectF(5, 3, 26, 25)));
	plotArea->addChild(group1);
	*/
	/*
	Axis *xAxis1 = new Axis("x axis 1", Axis::axisBottom);
	plot->addChild(xAxis1);
	Axis *yAxis1 = new Axis("y axis 1", Axis::axisLeft);
	plot->addChild(yAxis1);
	*/
	Axis *xAxis2 = new Axis("x axis 1", Axis::axisBottom);
	coordSys->addChild(xAxis2);
	xAxis2->setMajorTicksLength(3);
	xAxis2->setMinorTicksLength(1);
	xAxis2->setMinorTicksNumber(3);
	xAxis2->setMajorTicksNumber(13);
	xAxis2->setStart(-2);
	xAxis2->setEnd(10);
	xAxis2->setTickStart(-2);
	xAxis2->setTickEnd(10);
	
	Axis *yAxis2 = new Axis("y axis 1", Axis::axisLeft);
	yAxis2->setMajorTicksLength(3);
	yAxis2->setMinorTicksLength(1);
	yAxis2->setMinorTicksNumber(4);
	yAxis2->setStart(0);
	yAxis2->setEnd(10);
	yAxis2->setTickStart(0);
	yAxis2->setTickEnd(10);
	yAxis2->setMajorTicksNumber(11);
	coordSys->addChild(yAxis2);
	
	Axis *xAxis3 = new Axis("x axis 2", Axis::axisTop);
	xAxis3->setOffset(10);
	xAxis3->setStart(-2);
	xAxis3->setEnd(10);
	xAxis3->setTickStart(-2);
	xAxis3->setTickEnd(10);
	xAxis3->setMajorTicksNumber(13);
	coordSys->addChild(xAxis3);
	Axis *yAxis3 = new Axis("y axis 2", Axis::axisRight);
	yAxis3->setOffset(10);
	yAxis3->setStart(0);
	yAxis3->setEnd(10);
	yAxis3->setTickStart(0);
	yAxis3->setTickEnd(10);
	yAxis3->setMajorTicksNumber(11);
	yAxis3->setMajorTicksDirection(Axis::ticksBoth);
	yAxis3->setMinorTicksDirection(Axis::ticksBoth);
	#if 0
	yAxis3->setTickStart(0.5);
	yAxis3->setTickEnd(9.5);
	yAxis3->setMajorTicksNumber(9);
	#endif
	yAxis3->setStart(0);
	yAxis3->setEnd(10);
	yAxis3->setTickStart(0);
	yAxis3->setTickEnd(10);
	coordSys->addChild(yAxis3);
	
// 	plotArea->addChild(new WorksheetRectangleElement("rect 1", QRectF(2, 2, 2, 2)));
	
	Column *xc = new Column("xc", SciDAVis::Numeric);
	Column *yc = new Column("yc", SciDAVis::Numeric);
	for (int i=0; i<40; i++)	{
	  xc->setValueAt(i, i*0.25);
	  yc->setValueAt(i, i*i*0.01+1);
	}
	
	LineSymbolCurve *curve1 = new LineSymbolCurve("curve 1");
	curve1->setXColumn(xc);
	curve1->setYColumn(yc);
	plotArea->addChild(curve1);
	/*
	Column *xc2 = new Column("xc", SciDAVis::Numeric);
	Column *yc2 = new Column("yc", SciDAVis::Numeric);
	for (int i=0; i<40; i++)	{
	  xc2->setValueAt(i, (i-20)*0.25);
	  yc2->setValueAt(i, (i-20)*(i-20)*0.01/2+2);
	}
	LineSymbolCurve *curve2 = new LineSymbolCurve("curve 2");
	curve2->setXColumn(xc2);
	curve2->setYColumn(yc2);
	plotArea->addChild(curve2);
	*/
	
// 	
// 	
// 	Column *xc3 = new Column("xc", SciDAVis::Numeric);
// 	Column *yc3 = new Column("yc", SciDAVis::Numeric);
// 	for (int i=0; i<20; i++)	{
// 	  xc3->setValueAt(i, i*0.25);
// 	  yc3->setValueAt(i, i*i*0.01*2+3);
// 	}
// 	DropLineCurve *curve3 = new DropLineCurve("curve 3");
// 	curve3->setXColumn(xc3);
// 	curve3->setYColumn(yc3);
// 	
// 	WorksheetElementContainer *group2 = new WorksheetElementContainer("some more items");
// 	group2->addChild(curve3);
// 	plotArea->addChild(group2);
	

// 	TODO	coordSys->addChild(plotArea);
// 		plot->addChild(coordSys);
  }
  
  // ####################
  
  {
	DecorationPlot *plot = new DecorationPlot("plot2");
	m_worksheet->addChild(plot);
	plot->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
	plot->graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	
	CartesianCoordinateSystem *coordSys = new CartesianCoordinateSystem("coords2");
	
	QList<CartesianCoordinateSystem::Scale *> scales;
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), pw * 0.3, pw * 0.7, -2, 10);
	coordSys->setXScales(scales);
	scales.clear();
	scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX), ph * 0.4, ph * 0.2, 1, 1000, 10);
	coordSys->setYScales(scales);
	
	plot->addChild(coordSys);
	PlotArea *plotArea = new PlotArea("plot area");
	plotArea->setRect(QRectF(-2, 1, 12, 1000));
	plotArea->setClippingEnabled(true);
	coordSys->addChild(plotArea);
	
	Axis *xAxis2 = new Axis("x axis 1", Axis::axisBottom);
	xAxis2->setOffset(1);
	coordSys->addChild(xAxis2);
	xAxis2->setMajorTicksLength(3);
	xAxis2->setMinorTicksLength(1);
	xAxis2->setMinorTicksNumber(3);
	Axis *yAxis2 = new Axis("y axis 1", Axis::axisLeft);
	yAxis2->setScale(Axis::ScaleLog10);
	yAxis2->setMajorTicksLength(5);
	yAxis2->setMinorTicksLength(3);
	yAxis2->setMinorTicksNumber(9);
	yAxis2->setMajorTicksNumber(4);
	yAxis2->setStart(1);
	yAxis2->setEnd(1000);
	yAxis2->setTickStart(1);
	yAxis2->setTickEnd(1000);
	coordSys->addChild(yAxis2);
	
	Axis *xAxis3 = new Axis("x axis 2", Axis::axisTop);
	xAxis3->setOffset(1000);
	coordSys->addChild(xAxis3);
	Axis *yAxis3 = new Axis("y axis 2", Axis::axisRight);
	yAxis3->setOffset(10);
	yAxis3->setMajorTicksLength(5);
	yAxis3->setMinorTicksLength(3);
	yAxis3->setMajorTicksDirection(Axis::ticksBoth);
	yAxis3->setMinorTicksDirection(Axis::ticksBoth);
	yAxis3->setStart(1);
	yAxis3->setEnd(1000);
	yAxis3->setTickStart(1);
	yAxis3->setTickEnd(1000);
	yAxis3->setMinorTicksNumber(9);
	yAxis3->setMajorTicksNumber(4);
	coordSys->addChild(yAxis3);
	
	Column *xc = new Column("xc", SciDAVis::Numeric);
	Column *yc = new Column("yc", SciDAVis::Numeric);
	for (int i=0; i<20; i++)	{
	  xc->setValueAt(i, i*0.25);
	  yc->setValueAt(i, i*i*2+1);
  }
  
  LineSymbolCurve *curve1 = new LineSymbolCurve("curve 1");
  curve1->setXColumn(xc);
  curve1->setYColumn(yc);
  
  Column *xc2 = new Column("xc", SciDAVis::Numeric);
  Column *yc2 = new Column("yc", SciDAVis::Numeric);
  for (int i=0; i<40; i++)	{
	  xc2->setValueAt(i, (i-20)*0.25);
	  yc2->setValueAt(i, (i-20)*(i-20)+2);
  }
  LineSymbolCurve *curve2 = new LineSymbolCurve("curve 2");
  curve2->setXColumn(xc2);
  curve2->setYColumn(yc2);
  
  Column *xc3 = new Column("xc", SciDAVis::Numeric);
  Column *yc3 = new Column("yc", SciDAVis::Numeric);
  for (int i=0; i<20; i++)	{
	  xc3->setValueAt(i, i*0.25);
	  yc3->setValueAt(i, i*i*6+3);
  }
  LineSymbolCurve *curve3 = new LineSymbolCurve("curve 3");
  curve3->setXColumn(xc3);
  curve3->setYColumn(yc3);
  
  //test for natural spline
  Column *xc4 = new Column("xc4", SciDAVis::Numeric);
  Column *yc4 = new Column("yc4", SciDAVis::Numeric);
  for (int i=0; i<10; i++)	{
	xc4->setValueAt(i, i + 0.5 * sin (i));
	yc4->setValueAt(i, i + cos (i * i));
  }
  
  LineSymbolCurve *curve4 = new LineSymbolCurve("curve 4");
  curve4->setXColumn(xc4);
  curve4->setYColumn(yc4);
  plotArea->addChild(curve4);	
	
	
    //test for periodic spline
  Column *xc5 = new Column("xc5", SciDAVis::Numeric);
  Column *yc5 = new Column("yc5", SciDAVis::Numeric);
  xc5->setValueAt(0, 0);
  xc5->setValueAt(1, 3);
  xc5->setValueAt(2,5);
  xc5->setValueAt(3,7);
  xc5->setValueAt(4, 9);
  yc5->setValueAt(0, 5);
  yc5->setValueAt(1, 50);
  yc5->setValueAt(2,10);
  yc5->setValueAt(3, 40);
  yc5->setValueAt(4, 5);
  
  LineSymbolCurve *curve5 = new LineSymbolCurve("curve 5");
  curve5->setXColumn(xc5);
  curve5->setYColumn(yc5);
  plotArea->addChild(curve5);	
  
  WorksheetElementContainer *group2 = new WorksheetElementContainer("some more items");
  group2->addChild(curve3);
  plotArea->addChild(group2);
  
  
  plotArea->addChild(curve2);
  plotArea->addChild(curve1);
  }
}
