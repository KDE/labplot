import sys

from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QFont
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

project = project()

filter = AsciiFilter()

p = filter.properties()
p.headerEnabled = False
filter.setProperties(p)

spreadsheet = Spreadsheet("Spreadsheet")
project.addChild(spreadsheet)

filter.readDataFromFile("lib/examples/Data = Smooth + Rough/data.txt", spreadsheet, AbstractFileFilter.ImportMode.Replace)

if filter.lastError():
	print(f"Import error: {filter.lastError()}")
	sys.exit(-1)

worksheet = Worksheet("Plot data from Spreadsheet")
project.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setLayout(Worksheet.Layout.VerticalLayout)
worksheet.setLayoutTopMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutBottomMargin(Worksheet.convertToSceneUnits(0.4, Worksheet.Unit.Centimeter))
worksheet.setLayoutLeftMargin(Worksheet.convertToSceneUnits(0.4, Worksheet.Unit.Centimeter))
worksheet.setLayoutRightMargin(Worksheet.convertToSceneUnits(0.4, Worksheet.Unit.Centimeter))

worksheet.setLayoutHorizontalSpacing(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))
worksheet.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(0.4, Worksheet.Unit.Centimeter))

fo7 = QFont()
fo7.setPointSizeF(Worksheet.convertToSceneUnits(7, Worksheet.Unit.Point))

fo10 = QFont()
fo10.setPointSizeF(10)
te10 = QTextEdit()
te10.setFont(fo10)

plotArea1 = CartesianPlot("Plot data from Spreadsheet")
plotArea1.setType(CartesianPlot.Type.FourAxes)

plotArea1.setSymmetricPadding(False)
plotArea1.setHorizontalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea1.setVerticalPadding(Worksheet.convertToSceneUnits(0.8, Worksheet.Unit.Centimeter))
plotArea1.setRightPadding(Worksheet.convertToSceneUnits(0.8, Worksheet.Unit.Centimeter))
plotArea1.setBottomPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))

# border1 = plotArea1.plotArea().borderType()
# border1.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border1.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border1.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border1.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea1.plotArea().setBorderType(border1)

# plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, True)

# rangeY1 = plotArea1.range(CartesianCoordinateSystem.Dimension.Y, 0)
# rangeY1.setRange(300, 650)
# plotArea1.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY1)
# plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)

for axis in plotArea1.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        te10.setText("index")
        axis.title().setText(te10.toHtml())
        axis.majorGridLine().setStyle(Qt.PenStyle.SolidLine)
        axis.minorGridLine().setStyle(Qt.PenStyle.DashLine)
        axis.setLabelsFont(fo7)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        te10.setText("data")
        axis.title().setText(te10.toHtml())
        axis.majorGridLine().setStyle(Qt.PenStyle.SolidLine)
        axis.minorGridLine().setStyle(Qt.PenStyle.DashLine)
        axis.setLabelsFont(fo7)

worksheet.addChild(plotArea1)

config11 = XYCurve("data")
plotArea1.addChild(config11)
config11.setPlotType(Plot.PlotType.Scatter)
config11.setXColumn(spreadsheet.column(0))
config11.setYColumn(spreadsheet.column(1))
config11.setLineType(XYCurve.LineType.NoLine)
config11.symbol().setStyle(Symbol.Style.Circle)
config11.symbol().setSize(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
config11.setValuesType(XYCurve.ValuesType.NoValues)
config11.background().setPosition(Background.Position.No)

config12 = XYSmoothCurve("smooth 1st iteration")
plotArea1.addChild(config12)
config12.setDataSourceType(XYFitCurve.DataSourceType.Curve)
config12.setDataSourceCurve(config11)
sData11 = config12.smoothData()
sData11.type = nsl_smooth_type.nsl_smooth_type_moving_average
sData11.points = 5
sData11.weight = nsl_smooth_weight_type.nsl_smooth_weight_uniform
sData11.mode = nsl_smooth_pad_mode.nsl_smooth_pad_none
sData11.autoRange = True
config12.setSmoothData(sData11)
config12.setLineInterpolationPointsCount(1)
config12.symbol().setStyle(Symbol.Style.NoSymbols)
config12.setValuesType(XYCurve.ValuesType.NoValues)
config12.background().setPosition(Background.Position.No)

config13 = XYSmoothCurve("smooth 2nd iteration")
plotArea1.addChild(config13)
config13.setDataSourceType(XYFitCurve.DataSourceType.Curve)
config13.setDataSourceCurve(config12)
sData12 = config13.smoothData()
sData12.type = nsl_smooth_type.nsl_smooth_type_moving_average
sData12.points = 5
sData12.weight = nsl_smooth_weight_type.nsl_smooth_weight_uniform
sData12.mode = nsl_smooth_pad_mode.nsl_smooth_pad_none
sData12.autoRange = True
config13.setSmoothData(sData12)
config13.setLineInterpolationPointsCount(1)
config13.symbol().setStyle(Symbol.Style.NoSymbols)
config13.setValuesType(XYCurve.ValuesType.NoValues)
config13.background().setPosition(Background.Position.No)

legend1 = CartesianPlotLegend("Legend1")
plotArea1.addChild(legend1)
legend1.setLabelFont(fo7)

###################################################################################################################################################################
###################################################################################################################################################################

plotArea2 = CartesianPlot ("xy-plot")
plotArea2.setType(CartesianPlot.Type.FourAxes)

plotArea2.setSymmetricPadding(False)
plotArea2.setHorizontalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea2.setVerticalPadding(Worksheet.convertToSceneUnits(0.8, Worksheet.Unit.Centimeter))
plotArea2.setRightPadding(Worksheet.convertToSceneUnits(0.8, Worksheet.Unit.Centimeter))
plotArea2.setBottomPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))

# border2 = plotArea2.plotArea().borderType()
# border2.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border2.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border2.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border2.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea2.plotArea().setBorderType(border2)

# plotArea2.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, True)

# rangeY2 = plotArea2.range(CartesianCoordinateSystem.Dimension.Y, 0)
# rangeY2.setRange(-120, 100)
# plotArea2.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY2)
# plotArea2.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)

for axis in plotArea2.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        te10.setText("index")
        axis.title().setText(te10.toHtml())
        axis.majorGridLine().setStyle(Qt.PenStyle.SolidLine)
        axis.minorGridLine().setStyle(Qt.PenStyle.DashLine)
        axis.setLabelsFont(fo7)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        te10.setText("rough")
        axis.title().setText(te10.toHtml())
        axis.majorGridLine().setStyle(Qt.PenStyle.SolidLine)
        axis.minorGridLine().setStyle(Qt.PenStyle.DashLine)
        axis.setLabelsFont(fo7)

worksheet.addChild(plotArea2)

config21 = XYCurve("rough 1st iteration")
plotArea2.addChild(config21)
config21.setPlotType(Plot.PlotType.Scatter)
config21.setXColumn(spreadsheet.column(0))
config21.setYColumn(config12.roughsColumn())
config21.setLineType(XYCurve.LineType.Line)
config21.symbol().setStyle(Symbol.Style.NoSymbols)
config21.setValuesType(XYCurve.ValuesType.NoValues)
config21.background().setPosition(Background.Position.No)

config22 = XYCurve("rough 2nd iteration")
plotArea2.addChild(config22)
config22.setPlotType(Plot.PlotType.Scatter)
config22.setXColumn(spreadsheet.column(0))
config22.setYColumn(config13.roughsColumn())
config22.setLineType(XYCurve.LineType.Line)
config22.symbol().setStyle(Symbol.Style.NoSymbols)
config22.setValuesType(XYCurve.ValuesType.NoValues)
config22.background().setPosition(Background.Position.No)

legend2 = CartesianPlotLegend("Legend2")
plotArea2.addChild(legend2)
legend2.setLabelFont(fo7)

###################################################################################################################################################################
###################################################################################################################################################################

Project.retransformElements(project)
