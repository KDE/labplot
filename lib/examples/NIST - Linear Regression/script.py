import sys

from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QFont
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

project = project()

lld = Folder("Lower Level of Difficulty")
project.addChild(lld)

allFits = Worksheet("All Fits")
lld.addChild(allFits)

allFits.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
allFits.setPageRect(QRectF(0, 0, w, h))

allFits.setLayout(Worksheet.Layout.VerticalLayout)
allFits.setLayoutTopMargin(Worksheet.convertToSceneUnits(1.2, Worksheet.Unit.Centimeter))

wsTextLabel = TextLabel("WS Text Label")
wsTextLabel.setText("Lower Level of Difficulty")

allFits.addChild(wsTextLabel)

wsTextLabel.setVerticalAlignment(WorksheetElement.VerticalAlignment.Center)
wsTextLabel.setHorizontalAlignment(WorksheetElement.HorizontalAlignment.Center)

# wsTextLabel.position().point.setX(Worksheet.convertToSceneUnits(-0.3, Worksheet.Unit.Centimeter))
# wsTextLabel.position().point.setY(Worksheet.convertToSceneUnits(9.5, Worksheet.Unit.Centimeter))
# wsTextLabel.setPosition(wsTextLabel.position())


position = WorksheetElement.PositionWrapper()
position.point = QPointF(Worksheet.convertToSceneUnits(-0.3, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(9.5, Worksheet.Unit.Centimeter))
wsTextLabel.setPosition(position)

filter = AsciiFilter()

p = filter.properties()
p.headerEnabled = False
filter.setProperties(p)

spreadsheet = Spreadsheet("Norris Data")
lld.addChild(spreadsheet)

filter.readDataFromFile("lib/examples/NIST - Linear Regression/norris_data.txt", spreadsheet, AbstractFileFilter.ImportMode.Replace)

if filter.lastError():
	print(f"Import error: {filter.lastError()}")
	sys.exit(-1)

plotArea = CartesianPlot("Plot Area")
plotArea.setType(CartesianPlot.Type.FourAxes)

for axis in plotArea.children(AspectType.Axis):
    axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
    axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

allFits.addChild(plotArea)

plotArea.title().setText("Norris")
plotArea.title().setVerticalAlignment(WorksheetElement.VerticalAlignment.Top)
plotArea.title().setHorizontalAlignment(WorksheetElement.HorizontalAlignment.Center)
position = WorksheetElement.PositionWrapper()
position.point = QPointF(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-0.5, Worksheet.Unit.Centimeter))
position.verticalPosition = WorksheetElement.VerticalPosition.Top
plotArea.title().setPosition(position)

plot = XYCurve("Plot")
plot.setLineType(XYCurve.LineType.NoLine)
plot.setPlotType(Plot.PlotType.Scatter)
plot.setXColumn(spreadsheet.column(0))
plot.setYColumn(spreadsheet.column(1))
plotArea.addChild(plot)

fit = XYFitCurve("Fit to 'Plot'")

fitData = fit.fitData()

fitData.modelCategory = nsl_fit_model_category.nsl_fit_model_custom
fitData.modelType = 0
fitData.model = "B0 + B1*x"
fitData.paramNames = ["B0", "B1"]
fitData.paramNamesUtf8 = ["B0", "B1"]
fitData.paramStartValues = [1, 1]
fitData.paramFixed = [False] * len(fitData.paramNames)
fitData.paramLowerLimits = [-sys.float_info.max]* len(fitData.paramNames)
fitData.paramUpperLimits = [sys.float_info.max] * len(fitData.paramNames)

fitData = XYFitCurve.initFitData(fitData)

fit.setDataSourceType(XYAnalysisCurve.DataSourceType.Curve)
fit.setDataSourceCurve(plot)

fitData = fit.initStartValues(fitData)
fit.setFitData(fitData)
fit.recalculate()

plotArea.addChild(fit)

fit.recalculate()

#####################################################################################################################################################################

filter2 = AsciiFilter()

p2 = filter2.properties()
p2.headerEnabled = False
filter2.setProperties(p2)

spreadsheet2 = Spreadsheet("Pontius Data")

lld.addChild(spreadsheet2)

filter2.readDataFromFile("lib/examples/NIST - Linear Regression/pontius_data.txt", spreadsheet2, AbstractFileFilter.ImportMode.Replace)

if filter2.lastError():
	print(f"Import error: {filter2.lastError()}")
	sys.exit(-1)

plotArea2 = CartesianPlot("Plot Area2")
plotArea2.setType(CartesianPlot.Type.FourAxes)

for axis in plotArea2.children(AspectType.Axis):
    axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
    axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

allFits.addChild(plotArea2)

plotArea2.title().setText("Pontius")
plotArea2.title().setVerticalAlignment(WorksheetElement.VerticalAlignment.Top)
plotArea2.title().setHorizontalAlignment(WorksheetElement.HorizontalAlignment.Center)
position2 = WorksheetElement.PositionWrapper()
position2.point = QPointF(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-0.5, Worksheet.Unit.Centimeter))
position2.verticalPosition = WorksheetElement.VerticalPosition.Top
plotArea2.title().setPosition(position2)

plot2 = XYCurve("Plot")
plot2.setLineType(XYCurve.LineType.NoLine)
plot2.setPlotType(Plot.PlotType.Scatter)
plot2.setXColumn(spreadsheet2.column(0))
plot2.setYColumn(spreadsheet2.column(1))
plotArea2.addChild(plot2)

fit2 = XYFitCurve("Fit to 'Plot'")

fitData2 = fit2.fitData()

fitData2.modelCategory = nsl_fit_model_category.nsl_fit_model_custom
fitData2.modelType = 0
fitData2.model = "B0 + B1*x + B2*(x**2)"
fitData2.paramNames = ["B0", "B1", "B2"]
fitData2.paramNamesUtf8 = ["B0", "B1", "B2"]
fitData2.paramStartValues = [0.1, 0.01, 0.02]
fitData2.paramFixed = [False] * len(fitData2.paramNames)
fitData2.paramLowerLimits = [-sys.float_info.max] * len(fitData2.paramNames)
fitData2.paramUpperLimits = [sys.float_info.max] * len(fitData2.paramNames)

fitData2 = XYFitCurve.initFitData(fitData2)

fit2.setDataSourceType(XYAnalysisCurve.DataSourceType.Curve)
fit2.setDataSourceCurve(plot2)

fitData2 = fit2.initStartValues(fitData2)
fit2.setFitData(fitData2)
fit2.recalculate()

plotArea2.addChild(fit2)

fit2.recalculate()

#####################################################################################################################################################################
#####################################################################################################################################################################
#####################################################################################################################################################################

ald = Folder("Average Level of Difficulty")
project.addChild(ald)

allFits2 = Worksheet("All Fits2")
ald.addChild(allFits2)

allFits2.setUseViewSize(False)
w2 = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
h2 = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
allFits2.setPageRect(QRectF(0, 0, w2, h2))

allFits2.setLayout(Worksheet.Layout.VerticalLayout)
allFits2.setLayoutTopMargin(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))

wsTextLabel2 = TextLabel("WS Text Label2")
wsTextLabel2.setText("Average Level of Difficulty")

wsTextLabel2.setVerticalAlignment(WorksheetElement.VerticalAlignment.Center)
wsTextLabel2.setHorizontalAlignment(WorksheetElement.HorizontalAlignment.Center)

# wsTextLabel2.position().point.setX(Worksheet.convertToSceneUnits(-0.4, Worksheet.Unit.Centimeter))
# wsTextLabel2.position().point.setY(Worksheet.convertToSceneUnits(9.5, Worksheet.Unit.Centimeter))
# wsTextLabel2.setPosition(wsTextLabel2.position())

position = WorksheetElement.PositionWrapper()
position.point = QPointF(Worksheet.convertToSceneUnits(-0.4, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(9.5, Worksheet.Unit.Centimeter))
wsTextLabel2.setPosition(position)

allFits2.addChild(wsTextLabel2)

filter3 = AsciiFilter()

p3 = filter3.properties()
p3.headerEnabled = False
filter3.setProperties(p3)

spreadsheet3 = Spreadsheet("NoInt1 Data")

ald.addChild(spreadsheet3)

filter3.readDataFromFile("lib/examples/NIST - Linear Regression/noint1_data.txt", spreadsheet3, AbstractFileFilter.ImportMode.Replace)

if filter3.lastError():
	print(f"Import error: {filter3.lastError()}")
	sys.exit(-1)

plotArea3 = CartesianPlot("Plot Area3")
plotArea3.setType(CartesianPlot.Type.FourAxes)

for axis in plotArea3.children(AspectType.Axis):
    axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
    axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

allFits2.addChild(plotArea3)

plotArea3.title().setText("NoInt1")
plotArea3.title().setVerticalAlignment(WorksheetElement.VerticalAlignment.Top)
plotArea3.title().setHorizontalAlignment(WorksheetElement.HorizontalAlignment.Center)
position3 = WorksheetElement.PositionWrapper()
position3.point = QPointF(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-0.5, Worksheet.Unit.Centimeter))
position3.verticalPosition = WorksheetElement.VerticalPosition.Top
plotArea3.title().setPosition(position3)

plot3 = XYCurve("Plot3")
plot3.setLineType(XYCurve.LineType.NoLine)
plot3.setPlotType(Plot.PlotType.Scatter)
plot3.setXColumn(spreadsheet3.column(0))
plot3.setYColumn(spreadsheet3.column(1))
plotArea3.addChild(plot3)

fit3 = XYFitCurve("Fit to 'Plot'")

fitData3 = fit3.fitData()

fitData3.modelCategory = nsl_fit_model_category.nsl_fit_model_custom
fitData3.modelType = 0
fitData3.model = "B1*x"
fitData3.paramNames = ["B1"]
fitData3.paramNamesUtf8 = ["B1"]
fitData3.paramStartValues = [1]
fitData3.paramFixed = [False] * len(fitData3.paramNames)
fitData3.paramLowerLimits = [-sys.float_info.max] * len(fitData3.paramNames)
fitData3.paramUpperLimits = [sys.float_info.max] * len(fitData3.paramNames)

fitData3 = XYFitCurve.initFitData(fitData3)

fit3.setDataSourceType(XYAnalysisCurve.DataSourceType.Curve)
fit3.setDataSourceCurve(plot3)

fitData3 = fit3.initStartValues(fitData3)
fit3.setFitData(fitData3)
fit3.recalculate()

plotArea3.addChild(fit3)

fit3.recalculate()

#####################################################################################################################################################################

filter4 = AsciiFilter()

p4 = filter4.properties()
p4.headerEnabled = False
filter4.setProperties(p4)

spreadsheet4 = Spreadsheet("NoInt2 Data")

ald.addChild(spreadsheet4)

filter4.readDataFromFile("lib/examples/NIST - Linear Regression/noint2_data.txt", spreadsheet4, AbstractFileFilter.ImportMode.Replace)

if filter4.lastError():
	print(f"Import error: {filter4.lastError()}")
	sys.exit(-1)

plotArea4 = CartesianPlot("Plot Area4")
plotArea4.setType(CartesianPlot.Type.FourAxes)

for axis in plotArea4.children(AspectType.Axis):
    axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
    axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

allFits2.addChild(plotArea4)

plotArea4.title().setText("NoInt2")
plotArea4.title().setVerticalAlignment(WorksheetElement.VerticalAlignment.Top)
plotArea4.title().setHorizontalAlignment(WorksheetElement.HorizontalAlignment.Center)
position4 = WorksheetElement.PositionWrapper()
position4.point = QPointF(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-0.5, Worksheet.Unit.Centimeter))
position4.verticalPosition = WorksheetElement.VerticalPosition.Top
plotArea4.title().setPosition(position4)

plot4 = XYCurve("Plot4")
plot4.setLineType(XYCurve.LineType.NoLine)
plot4.setPlotType(Plot.PlotType.Scatter)
plot4.setXColumn(spreadsheet4.column(0))
plot4.setYColumn(spreadsheet4.column(1))
plotArea4.addChild(plot4)

fit4 = XYFitCurve("Fit to 'Plot4'")

fitData4 = fit4.fitData()

fitData4.modelCategory = nsl_fit_model_category.nsl_fit_model_custom
fitData4.modelType = 0
fitData4.model = "B1*x"
fitData4.paramNames = ["B1"]
fitData4.paramNamesUtf8 = ["B1"]
fitData4.paramStartValues = [1]
fitData4.paramFixed = [False] * len(fitData4.paramNames)
fitData4.paramLowerLimits = [-sys.float_info.max] * len(fitData4.paramNames)
fitData4.paramUpperLimits = [sys.float_info.max] * len(fitData4.paramNames)

fitData4 = XYFitCurve.initFitData(fitData4)

fit4.setDataSourceType(XYAnalysisCurve.DataSourceType.Curve)
fit4.setDataSourceCurve(plot4)

fitData4 = fit4.initStartValues(fitData4)
fit4.setFitData(fitData4)
fit4.recalculate()

plotArea4.addChild(fit4)

fit4.recalculate()
