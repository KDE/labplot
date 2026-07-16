import sys

from PySide6.QtWidgets import QApplication
from PySide6.QtCore import QRectF
from pylabplot import *

app = QApplication()

project = Project()

filter = AsciiFilter()

p = filter.properties()
p.headerEnabled = False
filter.setProperties(p)

spreadsheet = Spreadsheet("ENSO-data")
project.addChild(spreadsheet)

filter.readDataFromFile("ENSO/data.txt", spreadsheet, AbstractFileFilter.ImportMode.Replace)

if filter.lastError():
	print(f"Import error: {filter.lastError()}")
	sys.exit(-1)

worksheet = Worksheet("Worksheet")
project.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(15, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(15, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setTheme("Solarized Light")

ms = Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter)
worksheet.setLayout(Worksheet.Layout.VerticalLayout)
worksheet.setLayoutTopMargin(ms)
worksheet.setLayoutBottomMargin(ms)
worksheet.setLayoutLeftMargin(ms)
worksheet.setLayoutRightMargin(ms)
worksheet.setLayoutHorizontalSpacing(ms)
worksheet.setLayoutVerticalSpacing(ms)

plotArea = CartesianPlot("xy-plot")
plotArea.setType(CartesianPlot.Type.FourAxes)
plotArea.title().setText("El Niño-Southern Oscillation")
border = plotArea.borderType()
border = CartesianPlot.BorderTypeFlags.BorderLeft | CartesianPlot.BorderTypeFlags.BorderTop | CartesianPlot.BorderTypeFlags.BorderRight | CartesianPlot.BorderTypeFlags.BorderBottom
plotArea.setBorderType(border)
worksheet.addChild(plotArea)

for axis in plotArea.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.title().setText("Month")
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.title().setText("Atmospheric Pressure")

data = XYCurve("data")
data.setPlotType(Plot.PlotType.Scatter)
data.setYColumn(spreadsheet.column(0))
data.setXColumn(spreadsheet.column(1))
plotArea.addChild(data)

fit = XYFitCurve("fit")

fitData = fit.fitData()

fitData.modelCategory = nsl_fit_model_category.nsl_fit_model_custom
fitData.modelType = 0
fitData.model = "b1 + b2*cos( 2*pi*x/12 ) + b3*sin( 2*pi*x/12 ) + b5*cos( 2*pi*x/b4 ) + b6*sin( 2*pi*x/b4 ) + b8*cos( 2*pi*x/b7 ) + b9*sin( 2*pi*x/b7 )"
fitData.paramNames = ["b1", "b2", "b3", "b5", "b4", "b6", "b8", "b7", "b9"]
fitData.paramNamesUtf8 = ["b1", "b2", "b3", "b5", "b4", "b6", "b8", "b7", "b9"]
fitData.paramStartValues = [10.6415, 3.05277, 0.479257, -0.0808176, -0.699536, -0.243715, 0.39189, -0.300106, 0.532237]
fitData.paramFixed = [False] * len(fitData.paramNames)
fitData.paramLowerLimits = [-1 * sys.float_info.max] * len(fitData.paramNames)
fitData.paramUpperLimits = [sys.float_info.max] * len(fitData.paramNames)

fitData = XYFitCurve.initFitData(fitData)

fit.setYDataColumn(spreadsheet.column(0))
fit.setXDataColumn(spreadsheet.column(1))

fitData = fit.initStartValues(fitData)
fit.setFitData(fitData)
fit.recalculate()

plotArea.addChild(fit)

fit.recalculate()

plotArea.addLegend()

worksheet.view().show()

app.exec()
