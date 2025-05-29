import sys

from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QFont, QColor
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

project = project()

filter1 = AsciiFilter()

p1 = filter1.properties()
p1.headerEnabled = False
filter1.setProperties(p1)

spreadsheet1 = Spreadsheet("data")
project.addChild(spreadsheet1)

filter1.readDataFromFile("lib/examples/Space Debris/Breakups.txt", spreadsheet1, AbstractFileFilter.ImportMode.Replace)

if filter1.lastError():
	print(f"Import error: {filter1.lastError()}")
	sys.exit(-1)

filter2 = AsciiFilter()

p2 = filter2.properties()
p2.headerEnabled = False
filter2.setProperties(p2)

spreadsheet2 = Spreadsheet("data")
project.addChild(spreadsheet2)

filter2.readDataFromFile("lib/examples/Space Debris/LEO.txt", spreadsheet2, AbstractFileFilter.ImportMode.Replace)

if filter2.lastError():
	print(f"Import error: {filter2.lastError()}")
	sys.exit(-1)

filter3 = AsciiFilter()

p3 = filter3.properties()
p3.headerEnabled = False
filter3.setProperties(p3)

spreadsheet3 = Spreadsheet("data")
project.addChild(spreadsheet3)

filter3.readDataFromFile("lib/examples/Space Debris/GEO.txt", spreadsheet3, AbstractFileFilter.ImportMode.Replace)

if filter3.lastError():
	print(f"Import error: {filter3.lastError()}")
	sys.exit(-1)

te = QTextEdit()
fo6 = QFont()
fo6.setPointSizeF(Worksheet.convertToSceneUnits(6, Worksheet.Unit.Point))

worksheet = Worksheet("Plot")
project.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(25, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(23, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setLayout(Worksheet.Layout.NoLayout)

###################################################################################################################################################################
###################################################################################################################################################################

plotArea1 = CartesianPlot("Plot data from Spreadsheet")
plotArea1.setType(CartesianPlot.Type.FourAxes)

plotArea1.setRect(QRectF(Worksheet.convertToSceneUnits(0.3, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(0.3, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(24.5, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(7, Worksheet.Unit.Centimeter)))

plotArea1.setSymmetricPadding(False)
plotArea1.setHorizontalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea1.setVerticalPadding(Worksheet.convertToSceneUnits(1.1, Worksheet.Unit.Centimeter))
plotArea1.setRightPadding(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
plotArea1.setBottomPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))

# border1 = plotArea1.plotArea().borderType()
# border1.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border1.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border1.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border1.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea1.plotArea().setBorderType(border1)
# plotArea1.plotArea().borderLine().setWidth(0)

te.clear()
te.setFontPointSize(10)
te.append("Breakups per Year")
plotArea1.title().setText(te.toHtml())

plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, True)

rangeY1 = plotArea1.range(CartesianCoordinateSystem.Dimension.Y, 0)
rangeY1.setRange(0, 10)
plotArea1.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY1)
plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)

for axis in plotArea1.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        te.clear()
        te.setFontPointSize(8)
        te.append("Year")
        axis.title().setText(te.toHtml())
        axis.majorGridLine().setStyle(Qt.PenStyle.SolidLine)
        axis.minorGridLine().setStyle(Qt.PenStyle.DotLine)
        axis.setLabelsFont(fo6)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        te.clear()
        te.setFontPointSize(8)
        te.append("breakups")
        axis.title().setText(te.toHtml())
        axis.majorGridLine().setStyle(Qt.PenStyle.SolidLine)
        axis.minorGridLine().setStyle(Qt.PenStyle.DotLine)
        axis.setLabelsFont(fo6)

worksheet.addChild(plotArea1)

config11 = XYCurve("2")
plotArea1.addChild(config11)
config11.setPlotType(Plot.PlotType.Scatter)
config11.dropLine().setDropLineType(XYCurve.DropLineType.X)
config11.dropLine().setStyle(Qt.PenStyle.SolidLine)
config11.setXColumn(spreadsheet1.column(0))
config11.setYColumn(spreadsheet1.column(1))
config11.setLineType(XYCurve.LineType.NoLine)
config11.symbol().setStyle(Symbol.Style.Circle)
config11.symbol().setSize(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
config11.setValuesType(XYCurve.ValuesType.NoValues)
config11.background().setPosition(Background.Position.No)

###################################################################################################################################################################
###################################################################################################################################################################

plotArea2 = CartesianPlot("xy-plot")
plotArea2.setType(CartesianPlot.Type.FourAxes)

plotArea2.setRect(QRectF(Worksheet.convertToSceneUnits(0.3, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(7.5, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(17, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(7, Worksheet.Unit.Centimeter)))

plotArea2.setSymmetricPadding(False)
plotArea2.setHorizontalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea2.setVerticalPadding(Worksheet.convertToSceneUnits(1.3, Worksheet.Unit.Centimeter))
plotArea2.setRightPadding(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
plotArea2.setBottomPadding(Worksheet.convertToSceneUnits(1.3, Worksheet.Unit.Centimeter))

# border2 = plotArea2.plotArea().borderType()
# border2.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border2.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border2.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border2.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea2.plotArea().setBorderType(border2)
plotArea2.plotArea().borderLine().setWidth(0)

te.clear()
te.setFontPointSize(10)
te.append("Near Earth Altitude Population")
plotArea2.title().setText(te.toHtml())

rangeX2 = plotArea2.range(CartesianCoordinateSystem.Dimension.X, 0)
rangeX2.setRange(0, 2000)
plotArea2.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX2)
plotArea2.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)

rangeY2 = plotArea2.range(CartesianCoordinateSystem.Dimension.Y, 0)
rangeY2.setRange(0, 7)
plotArea2.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY2)
plotArea2.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)

for axis in plotArea2.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        te.clear()
        te.setFontPointSize(8)
        te.append("Altitude [km]")
        axis.title().setText(te.toHtml())
        axis.majorGridLine().setStyle(Qt.PenStyle.SolidLine)
        axis.minorGridLine().setStyle(Qt.PenStyle.DotLine)
        axis.setLabelsFont(fo6)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        te.clear()
        te.setFontPointSize(8)
        te.append("density [10-8km-3]")
        axis.title().setText(te.toHtml())
        axis.majorGridLine().setStyle(Qt.PenStyle.SolidLine)
        axis.minorGridLine().setStyle(Qt.PenStyle.DotLine)
        axis.setLabelsFont(fo6)

worksheet.addChild(plotArea2)

config21 = XYCurve("xy-curve")
plotArea2.addChild(config21)
config21.setPlotType(Plot.PlotType.Line)
config21.setXColumn(spreadsheet2.column(0))
config21.setYColumn(spreadsheet2.column(1))
config21.symbol().setStyle(Symbol.Style.NoSymbols)
config21.setValuesType(XYCurve.ValuesType.NoValues)
config21.background().setType(Background.Type.Color)
config21.background().setColorStyle(Background.ColorStyle.VerticalLinearGradient)
config21.background().setPosition(Background.Position.Below)
config21.background().setFirstColor(QColor(70, 70, 70))
config21.background().setSecondColor(QColor(255, 255, 255))
config21.background().setOpacity(0.4)

rl21 = ReferenceLine(plotArea2, "reference line")
plotArea2.addChild(rl21)
rl21.setOrientation(ReferenceLine.Orientation.Vertical)
rl21.setPositionLogical(QPointF(775, 0))
rl21.line().setWidth(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Point))
rl21.line().setOpacity(1)
rl21.retransform()

rl22 = ReferenceLine(plotArea2, "reference line 1")
plotArea2.addChild(rl22)
rl22.setOrientation(ReferenceLine.Orientation.Vertical)
rl22.setPositionLogical(QPointF(840, 0))
rl22.line().setWidth(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Point))
rl22.line().setOpacity(1)
rl22.retransform()

###################################################################################################################################################################
###################################################################################################################################################################

plotArea3 = CartesianPlot("xy-plot 1")
plotArea3.setType(CartesianPlot.Type.FourAxes)

plotArea3.setRect(QRectF(Worksheet.convertToSceneUnits(0.3, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(14.7, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(17, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(7, Worksheet.Unit.Centimeter)))

plotArea3.setSymmetricPadding(False)
plotArea3.setHorizontalPadding(Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter))
plotArea3.setVerticalPadding(Worksheet.convertToSceneUnits(1.3, Worksheet.Unit.Centimeter))
plotArea3.setRightPadding(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
plotArea3.setBottomPadding(Worksheet.convertToSceneUnits(1.3, Worksheet.Unit.Centimeter))

# border3 = plotArea3.plotArea().borderType()
# border3.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border3.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border3.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border3.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea3.plotArea().setBorderType(border3)
plotArea3.plotArea().borderLine().setWidth(0)

te.clear()
te.setFontPointSize(10)
te.append("Geosynchronous Altitude Population")
plotArea3.title().setText(te.toHtml())

rangeX3 = plotArea3.range(CartesianCoordinateSystem.Dimension.X, 0)
rangeX3.setRange(34, 37)
plotArea3.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX3)
plotArea3.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)

rangeY3 = plotArea3.range(CartesianCoordinateSystem.Dimension.Y, 0)
rangeY3.setScale(RangeT.Scale.Log10)
rangeY3.setRange(1, 10000)
plotArea3.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY3)
plotArea3.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)

for axis in plotArea3.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        te.clear()
        te.setFontPointSize(8)
        te.append("Altitude [km]")
        axis.title().setText(te.toHtml())
        axis.majorGridLine().setStyle(Qt.PenStyle.SolidLine)
        axis.minorGridLine().setStyle(Qt.PenStyle.DotLine)
        axis.setLabelsFont(fo6)
        axis.setScalingFactor(1000)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        te.clear()
        te.setFontPointSize(8)
        te.append("density [10-12km-3]")
        axis.title().setText(te.toHtml())
        axis.majorGridLine().setStyle(Qt.PenStyle.SolidLine)
        axis.minorGridLine().setStyle(Qt.PenStyle.DotLine)
        axis.setLabelsFont(fo6)

worksheet.addChild(plotArea3)

config31 = XYCurve("xy-curve")
plotArea3.addChild(config31)
config31.setPlotType(Plot.PlotType.Line)
config31.setXColumn(spreadsheet3.column(0))
config31.setYColumn(spreadsheet3.column(1))
config31.symbol().setStyle(Symbol.Style.NoSymbols)
config31.setValuesType(XYCurve.ValuesType.NoValues)
config31.background().setType(Background.Type.Color)
config31.background().setColorStyle(Background.ColorStyle.VerticalLinearGradient)
config31.background().setPosition(Background.Position.Below)
config31.background().setFirstColor(QColor(70, 70, 70))
config31.background().setSecondColor(QColor(255, 255, 255))
config31.background().setOpacity(0.4)

###################################################################################################################################################################
###################################################################################################################################################################

textLabel1 = TextLabel("Text Label")
worksheet.addChild(textLabel1)
te.clear()
te.setFontPointSize(5)
te.append("Sources:\n[1] https://orbitaldebris.jsc.nasa.gov/library/20180008451.pdf - History of On-Orbit Satellite Fragmentations\n[2] https://orbitaldebris.jsc.nasa.gov/photo-gallery/")
textLabel1.setText(te.toHtml())
textLabel1.setPosition(QPointF(Worksheet.convertToSceneUnits(-5.9, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-10.8, Worksheet.Unit.Centimeter)))

image1 = Image("LEO")
worksheet.addChild(image1)
image1.setFileName("lib/examples/Space Debris/LEO.jpg")
image1.setEmbedded(False)
image1.setOpacity(1)
image1.setWidth(Worksheet.convertToSceneUnits(7, Worksheet.Unit.Centimeter))
image1.setHeight(Worksheet.convertToSceneUnits(7, Worksheet.Unit.Centimeter))
image1.setKeepRatio(True)
image1.setPosition(QPointF(Worksheet.convertToSceneUnits(8.5, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter)))

image2 = Image("GEO")
worksheet.addChild(image2)
image2.setFileName("lib/examples/Space Debris/GEO.jpg")
image2.setEmbedded(False)
image2.setOpacity(1)
image2.setWidth(Worksheet.convertToSceneUnits(7, Worksheet.Unit.Centimeter))
image2.setHeight(Worksheet.convertToSceneUnits(5.6, Worksheet.Unit.Centimeter))
image2.setKeepRatio(True)
image2.setPosition(QPointF(Worksheet.convertToSceneUnits(8.5, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-6.5, Worksheet.Unit.Centimeter)))

###################################################################################################################################################################
###################################################################################################################################################################

worksheet.setTheme("Dark")

Project.retransformElements(project)
