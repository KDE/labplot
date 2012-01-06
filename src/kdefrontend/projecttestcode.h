#include <math.h>


#include "worksheet/plots/cartesian/CartesianPlot.h"
#include "worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "worksheet/WorksheetRectangleElement.h"
#include "worksheet/plots/cartesian/Axis.h"
#include "worksheet/plots/cartesian/XYCurve.h"
#include "worksheet/plots/PlotArea.h"
#include "lib/ActionManager.h"
#include "core/column/Column.h"

void MainWin::startTestCode(){
// 	Folder * folder = new Folder("Spreadsheets");
// 	this->addAspectToProject(folder);
// 	m_projectExplorer->setCurrentIndex(m_aspectTreeModel->modelIndexOfAspect(folder));
	
	Spreadsheet * spreadsheet1 = new Spreadsheet(0, 100, 2, i18n("Spreadsheet %1").arg(1));
	this->addAspectToProject(spreadsheet1);

	Column *xc = spreadsheet1->column(0);
	Column *yc = spreadsheet1->column(1);
	for (int i=0; i<40; i++)	{
	  xc->setValueAt(i, i*0.25);
	  yc->setValueAt(i, i*i*0.01+1);
	}
	
	Spreadsheet * spreadsheet2 = new Spreadsheet(0, 100, 2, i18n("Spreadsheet %1").arg(1));
	this->addAspectToProject(spreadsheet2);
	xc = spreadsheet2->column(0);
	yc = spreadsheet2->column(1);
	for (int i=0; i<20; i++)	{
	  xc->setValueAt(i, i*0.25);
	  yc->setValueAt(i, i*i*2+1);
  }
  
  
	Spreadsheet * spreadsheet3 = new Spreadsheet(0, 100, 2, i18n("Spreadsheet %1").arg(1));
	this->addAspectToProject(spreadsheet3);
	xc = spreadsheet3->column(0);
	yc = spreadsheet3->column(1);
	for (int i=0; i<40; i++)	{
		xc->setValueAt(i, (i-20)*0.25);
		yc->setValueAt(i, (i-20)*(i-20)+2);
	}
  
	Spreadsheet * spreadsheet4 = new Spreadsheet(0, 100, 2, i18n("Spreadsheet %1").arg(1));
	this->addAspectToProject(spreadsheet4);
	xc = spreadsheet4->column(0);
	yc = spreadsheet4->column(1);
	for (int i=0; i<20; i++)	{
		xc->setValueAt(i, i*0.25);
		yc->setValueAt(i, i*i*6+3);
	}

  //test for natural spline
	Spreadsheet * spreadsheet5 = new Spreadsheet(0, 100, 2, i18n("Spreadsheet %1").arg(1));
	spreadsheet5->setColumnCount(4);
	this->addAspectToProject(spreadsheet5);
	xc = spreadsheet5->column(0);
	yc = spreadsheet5->column(1);
	for (int i=0; i<10; i++)	{
		xc->setValueAt(i, i + 0.5 * sin (i));
		yc->setValueAt(i, i + cos (i * i));
	}
	
    //test for periodic spline
  	xc = spreadsheet5->column(2);
	yc = spreadsheet5->column(3);
	xc->setValueAt(0, 0);
	xc->setValueAt(1, 3);
	xc->setValueAt(2,5);
	xc->setValueAt(3,7);
	xc->setValueAt(4, 9);
	yc->setValueAt(0, 5);
	yc->setValueAt(1, 50);
	yc->setValueAt(2,10);
	yc->setValueAt(3, 40);
	yc->setValueAt(4, 5);
	
	//sqrt(x) and x^2
	Spreadsheet * spreadsheet6 = new Spreadsheet(0, 100, 2, i18n("Spreadsheet %1").arg(1));
	spreadsheet6->setColumnCount(3);
	this->addAspectToProject(spreadsheet6);
	xc = spreadsheet6->column(0);
	yc = spreadsheet6->column(1);
	Column* yc2 = spreadsheet6->column(2);
	for (int i=0; i<20; i++)	{
	  xc->setValueAt(i, 0.5*i);
	  yc->setValueAt(i, sqrt(0.5*i));
	  yc2->setValueAt(i, 1.25*i*i);
	}
	
	Worksheet* worksheet= new Worksheet(0,  i18n("Worksheet %1").arg(1));
	this->addAspectToProject(worksheet);
   

	#define SCALE_MIN CartesianCoordinateSystem::Scale::LIMIT_MIN
	#define SCALE_MAX CartesianCoordinateSystem::Scale::LIMIT_MAX
	
	QGraphicsView* view = qobject_cast<  QGraphicsView* >(worksheet->view());
	QRectF pageRect = view->scene()->sceneRect();
	const double pw = pageRect.width();
	const double ph = pageRect.height();
  /*
	  {
	CartesianPlot *plot = new CartesianPlot("plot1");
	worksheet->addChild(plot);

	CartesianCoordinateSystem *cSystem = qobject_cast<CartesianCoordinateSystem *>(plot->coordinateSystem());
	QList<CartesianCoordinateSystem::Scale *> scales;
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, 2), pw * 0.3, pw * 0.4, -2, 2);
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(3, 6), pw * 0.4 + 2, pw * 0.65, 3, 6);
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(7, SCALE_MAX), pw * 0.65 + 5, pw * 0.7, 7, 10);
	cSystem->setXScales(scales);
	scales.clear();
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, 4), ph * 0.8, ph * 0.65, 1, 4);
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(8, SCALE_MAX), ph * 0.65 - 5, ph * 0.5, 8, 10);
	cSystem->setYScales(scales);
	
	plot->plotArea()->setRect(QRectF(-2, 0, 12, 10));
	
// 	WorksheetRectangleElement *rect = new WorksheetRectangleElement("rect1");
// 	rect->setRect(QRectF(12, 12, 300, 3));
// 	coordSys->addChild(rect);
// 	WorksheetRectangleElement *rect2 = new WorksheetRectangleElement("rect2");
// 	rect2->setRect(QRectF(0, 0, 40, 30));
// 	m_worksheet->addChild(rect2);
// 	WorksheetRectangleElement *rect3 = new WorksheetRectangleElement("rect3");
// 	rect3->setRect(QRectF(pageRect.width() / 2 - 2, pageRect.height() / 2 - 2, 10 + 4, 120 + 4));
// 	m_worksheet->addChild(rect3);
// 	
// 	WorksheetElementGroup *group1 = new WorksheetElementGroup("some items");
// 		group1->addChild(new WorksheetRectangleElement("rect 1", QRectF(5, 5, 20, 20)));
// 		group1->addChild(new WorksheetRectangleElement("rect 1", QRectF(4, 5, 25, 15)));
// 		group1->addChild(new WorksheetRectangleElement("rect 1", QRectF(5, 3, 26, 25)));
// 	plotArea->addChild(group1);
	

	Axis *xAxis2 = new Axis("x axis 1", Axis::AxisHorizontal);
	plot->addChild(xAxis2);
	xAxis2->setMajorTicksLength(3);
	xAxis2->setMinorTicksLength(1);
	xAxis2->setMinorTicksNumber(3);
	xAxis2->setMajorTicksNumber(13);
	xAxis2->setStart(-2);
	xAxis2->setEnd(10);
	
	Axis *yAxis2 = new Axis("y axis 1",  Axis::AxisVertical);
	plot->addChild(yAxis2);
	yAxis2->setMajorTicksLength(3);
	yAxis2->setMinorTicksLength(1);
	yAxis2->setMinorTicksNumber(4);
	yAxis2->setStart(0);
	yAxis2->setEnd(10);
	yAxis2->setMajorTicksNumber(11);

	Axis *xAxis3 = new Axis("x axis 2", Axis::AxisHorizontal);
	plot->addChild(xAxis3);
	xAxis3->setOffset(10);
	xAxis3->setStart(-2);
	xAxis3->setEnd(10);
	xAxis3->setMajorTicksNumber(13);

	Axis *yAxis3 = new Axis("y axis 2",  Axis::AxisVertical);
	plot->addChild(yAxis3);
	yAxis3->setOffset(10);
	yAxis3->setStart(0);
	yAxis3->setEnd(10);
	yAxis3->setMajorTicksNumber(11);
	yAxis3->setMajorTicksDirection(Axis::ticksBoth);
	yAxis3->setMinorTicksDirection(Axis::ticksBoth);

	
// 	plot->addChild(new WorksheetRectangleElement("rect 1", QRectF(2, 2, 2, 2)));
	
	XYCurve *curve1 = new XYCurve("curve 1");
	plot->addChild(curve1);
	curve1->setXColumn(spreadsheet1->column(0));
	curve1->setYColumn(spreadsheet1->column(1));
  }
  */
  
  /*
	  {
	CartesianPlot *plot = new CartesianPlot("plot2");
	worksheet->addChild(plot);
	
	CartesianCoordinateSystem *cSystem = qobject_cast<CartesianCoordinateSystem *>(plot->coordinateSystem());
	QList<CartesianCoordinateSystem::Scale *> scales;
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), pw * 0.5, pw * 0.85, -2, 10);
	cSystem->setXScales(scales);
	scales.clear();
	scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX), ph * 0.4, ph * 0.2, 1, 1000, 10);
	cSystem->setYScales(scales);
	
	plot->plotArea()->setRect(QRectF(-2, 1, 12, 1000));
	
	Axis *xAxis2 = new Axis("x axis 1", Axis::AxisHorizontal);
	plot->addChild(xAxis2);
	xAxis2->setOffset(1);
	xAxis2->setMajorTicksLength(3);
	xAxis2->setMinorTicksLength(1);
	xAxis2->setMinorTicksNumber(3);
	
	Axis *yAxis2 = new Axis("y axis 1", Axis::AxisVertical);
	plot->addChild(yAxis2);
	yAxis2->setScale(Axis::ScaleLog10);
	yAxis2->setMajorTicksLength(5);
	yAxis2->setMinorTicksLength(3);
	yAxis2->setMinorTicksNumber(9);
	yAxis2->setMajorTicksNumber(4);
	yAxis2->setStart(1);
	yAxis2->setEnd(1000);

	Axis *xAxis3 = new Axis("x axis 2", Axis::AxisHorizontal);
	plot->addChild(xAxis3);
	xAxis3->setOffset(1000);
	
	Axis *yAxis3 = new Axis("y axis 2", Axis::AxisVertical);
	plot->addChild(yAxis3);
	yAxis3->setOffset(10);
	yAxis3->setMajorTicksLength(5);
	yAxis3->setMinorTicksLength(3);
	yAxis3->setMajorTicksDirection(Axis::ticksBoth);
	yAxis3->setMinorTicksDirection(Axis::ticksBoth);
	yAxis3->setStart(1);
	yAxis3->setEnd(1000);
	yAxis3->setMinorTicksNumber(9);
	yAxis3->setMajorTicksNumber(4);
	yAxis3->setLabelsPosition(Axis::LabelsIn);
 
	XYCurve *curve1 = new XYCurve("curve 1");
	plot->addChild(curve1);
	curve1->setXColumn(spreadsheet2->column(0));
	curve1->setYColumn(spreadsheet2->column(1));

	XYCurve *curve2 = new XYCurve("curve 2");
	plot->addChild(curve2);
	curve2->setXColumn(spreadsheet3->column(0));
	curve2->setYColumn(spreadsheet3->column(1));


	XYCurve *curve3 = new XYCurve("curve 3");
	plot->addChild(curve3);
	curve3->setXColumn(spreadsheet4->column(0));
	curve3->setYColumn(spreadsheet4->column(1));

	//test for natural spline
	XYCurve *curve4 = new XYCurve("curve 4");
	plot->addChild(curve4);
	curve4->setXColumn(spreadsheet5->column(0));
	curve4->setYColumn(spreadsheet5->column(1));


	//test for periodic spline
	XYCurve *curve5 = new XYCurve("curve 5");
	plot->addChild(curve5);
	curve5->setXColumn(spreadsheet5->column(2));
	curve5->setYColumn(spreadsheet5->column(3));

// 	WorksheetElementContainer *group2 = new WorksheetElementContainer("some more items");
// 	group2->addChild(curve3);
// 	plotArea->addChild(group2);
  }
*/
	
	 {
	CartesianPlot *plot = new CartesianPlot("plot3");
	worksheet->addChild(plot);

	CartesianCoordinateSystem *cSystem = qobject_cast<CartesianCoordinateSystem *>(plot->coordinateSystem());
	QList<CartesianCoordinateSystem::Scale *> scales;
// 	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), pw * 0.02, pw * 0.42, -2, 10);
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), pw * 0.2, pw * 0.8, 0, 10);
	cSystem ->setXScales(scales);
	scales.clear();
// 	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), ph * 0.4, ph * 0.2, 1, 10);
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX), ph * 0.8, ph * 0.2, 0, 10);
	cSystem ->setYScales(scales);
	
// 	plot->plotArea()->setRect(QRectF(-2, -2, 14, 14));
	plot->plotArea()->setRect(QRectF(0,0,10, 10));
	plot->plotArea()->setBackgroundFirstColor(Qt::yellow);
	
	Axis *xAxis2 = new Axis("x axis 1", Axis::AxisHorizontal);
	plot->addChild(xAxis2);
	xAxis2->setMajorTicksLength(20);
	xAxis2->setMinorTicksLength(7);
	xAxis2->setMinorTicksNumber(3);
	Axis *yAxis2 = new Axis("y axis 1", Axis::AxisVertical);
	plot->addChild(yAxis2);
	yAxis2->setMajorTicksLength(20);
	yAxis2->setMinorTicksLength(7);
	yAxis2->setMinorTicksNumber(3);
	yAxis2->setMajorTicksNumber(6);
	yAxis2->setStart(0);
	yAxis2->setEnd(10);

	Axis *xAxis3 = new Axis("x axis 2", Axis::AxisHorizontal);
	plot->addChild(xAxis3);
	xAxis3->setMajorTicksLength(20);
	xAxis3->setMinorTicksLength(7);
	xAxis3->setOffset(10);
	Axis *yAxis3 = new Axis("y axis 2", Axis::AxisVertical);
	plot->addChild(yAxis3);
	yAxis3->setOffset(10);
	yAxis3->setMajorTicksLength(20);
	yAxis3->setMinorTicksLength(7);
	yAxis3->setMajorTicksDirection(Axis::ticksBoth);
	yAxis3->setMinorTicksDirection(Axis::ticksBoth);
	yAxis3->setStart(0);
	yAxis3->setEnd(10);
	yAxis3->setMinorTicksNumber(9);
	yAxis3->setMajorTicksNumber(4);
	yAxis3->setLabelsPosition(Axis::LabelsIn);

	XYCurve *curve1 = new XYCurve("sqrt(x)");
	plot->addChild(curve1);
	curve1->setXColumn(spreadsheet6->column(0));
	curve1->setYColumn(spreadsheet6->column(1));

	XYCurve *curve2 = new XYCurve("x^2");
	plot->addChild(curve2);
	curve2->setXColumn(spreadsheet6->column(0));
	curve2->setYColumn(spreadsheet6->column(2));
  }
	
	m_projectExplorer->setCurrentAspect(worksheet);
}
