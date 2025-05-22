import sys

from PySide6.QtWidgets import QApplication
from PySide6.QtCore import QRectF, QPointF
from PySide6.QtGui import QColor
from pylabplot import *

app = QApplication()

project = Project()

filter = AsciiFilter()

p = filter.properties()
p.headerEnabled = False
filter.setProperties(p)

spreadsheet = Spreadsheet("Spreadsheet")
project.addChild(spreadsheet)

filter.readDataFromFile("Helium Rydberg Spectrum/data.txt", spreadsheet, AbstractFileFilter.ImportMode.Replace)

if filter.lastError():
	print(f"Import error: {filter.lastError()}")
	sys.exit(-1)

worksheet = Worksheet("Worksheet")
project.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(16.3, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(12.7, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

ms = Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter)
worksheet.setLayout(Worksheet.Layout.HorizontalLayout)
worksheet.setLayoutTopMargin(ms)
worksheet.setLayoutBottomMargin(ms)
worksheet.setLayoutLeftMargin(ms)
worksheet.setLayoutRightMargin(ms)

worksheet.setLayoutHorizontalSpacing(ms)
worksheet.setLayoutVerticalSpacing(ms)

worksheet.setTheme("Monokai")

plotArea = CartesianPlot("Plot Area")
plotArea.setType(CartesianPlot.Type.FourAxes)
plotArea.title().setText("Rydberg Spectrum of Helium")

# border = plotArea.plotArea().borderType()
# border.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea.plotArea().setBorderType(border)

plotArea.setSymmetricPadding(False)
plotArea.setHorizontalPadding(Worksheet.convertToSceneUnits(2.1, Worksheet.Unit.Centimeter))
plotArea.setVerticalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea.setRightPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))
plotArea.setBottomPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))

worksheet.addChild(plotArea)

for axis in plotArea.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.title().setText("wavelength (nm)")
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.title().setText("Count")

config1 = XYCurve("config1")
plotArea.addChild(config1)
config1.setPlotType(Plot.PlotType.Line)
config1.setXColumn(spreadsheet.column(0))
config1.setYColumn(spreadsheet.column(1))
config1.symbol().setStyle(Symbol.Style.NoSymbols)
config1.setValuesType(XYCurve.ValuesType.NoValues)
config1.background().setType(Background.Type.Color)
config1.background().setColorStyle(Background.ColorStyle.VerticalLinearGradient)
config1.background().setPosition(Background.Position.Below)
config1.background().setFirstColor(config1.color())
config1.background().setSecondColor(QColor(255, 255, 255))
config1.background().setOpacity(0.4)

config2 = XYCurve("config2")
plotArea.addChild(config2)
config2.setPlotType(Plot.PlotType.Line)
config2.setXColumn(spreadsheet.column(2))
config2.setYColumn(spreadsheet.column(3))
config2.symbol().setStyle(Symbol.Style.NoSymbols)
config2.setValuesType(XYCurve.ValuesType.NoValues)
config2.background().setType(Background.Type.Color)
config2.background().setColorStyle(Background.ColorStyle.VerticalLinearGradient)
config2.background().setPosition(Background.Position.Below)
config2.background().setFirstColor(config2.color())
config2.background().setSecondColor(QColor(255, 255, 255))
config2.background().setOpacity(0.4)

config3 = XYCurve("config3")
plotArea.addChild(config3)
config3.setPlotType(Plot.PlotType.Line)
config3.setXColumn(spreadsheet.column(4))
config3.setYColumn(spreadsheet.column(5))
config3.symbol().setStyle(Symbol.Style.NoSymbols)
config3.setValuesType(XYCurve.ValuesType.NoValues)
config3.background().setType(Background.Type.Color)
config3.background().setColorStyle(Background.ColorStyle.VerticalLinearGradient)
config3.background().setPosition(Background.Position.Below)
config3.background().setFirstColor(config3.color())
config3.background().setSecondColor(QColor(255, 255, 255))
config3.background().setOpacity(0.4)
config3.setVisible(True)

plotArea.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)
rangeX = plotArea.range(CartesianCoordinateSystem.Dimension.X, 0)
rangeX.setRange(343, 343.7)
plotArea.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX)

plotArea.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
rangeY = plotArea.range(CartesianCoordinateSystem.Dimension.Y, 0)
rangeY.setRange(0, 10000)
plotArea.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY)

firstCurve = plotArea.children(AspectType.XYCurve)[0]
assert firstCurve

infoElement = InfoElement("Info Element", plotArea, firstCurve, 343.292)
plotArea.addChild(infoElement)

for curve in plotArea.children(AspectType.XYCurve):
    if curve != firstCurve:
        infoElement.addCurve(curve)

infoElement.title().setPosition(QPointF(Worksheet.convertToSceneUnits(4.6, Worksheet.Unit.Centimeter),Worksheet.convertToSceneUnits(1.3, Worksheet.Unit.Centimeter)))

infoElement.retransform()

worksheet.view().show()

app.exec()
