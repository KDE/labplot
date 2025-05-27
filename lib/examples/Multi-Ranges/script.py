import sys

from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QFont
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

project = project()

filter1 = AsciiFilter()

p1 = filter1.properties()
p1.headerEnabled = False
filter1.setProperties(p1)

spreadsheet1 = Spreadsheet("IPCE")
project.addChild(spreadsheet1)

filter1.readDataFromFile("lib/examples/Multi-Ranges/IPCE.txt", spreadsheet1, AbstractFileFilter.ImportMode.Replace)

if filter1.lastError():
	print(f"Import error: {filter1.lastError()}")
	sys.exit(-1)

filter2 = AsciiFilter()

p2 = filter2.properties()
p2.headerEnabled = False
filter2.setProperties(p2)

spreadsheet2 = Spreadsheet("Current")
project.addChild(spreadsheet2)

filter2.readDataFromFile("lib/examples/Multi-Ranges/Current.txt", spreadsheet2, AbstractFileFilter.ImportMode.Replace)

if filter2.lastError():
	print(f"Import error: {filter2.lastError()}")
	sys.exit(-1)

worksheet1 = Worksheet("single-range demo")
project.addChild(worksheet1)

worksheet1.setUseViewSize(False)
w1 = Worksheet.convertToSceneUnits(15, Worksheet.Unit.Centimeter)
h1 = Worksheet.convertToSceneUnits(10, Worksheet.Unit.Centimeter)
worksheet1.setPageRect(QRectF(0, 0, w1, h1))

worksheet1.setLayout(Worksheet.Layout.VerticalLayout)
worksheet1.setLayoutTopMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet1.setLayoutBottomMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet1.setLayoutLeftMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet1.setLayoutRightMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

worksheet1.setLayoutHorizontalSpacing(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter))
worksheet1.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter))

plotArea1 = CartesianPlot("IPCE1")
plotArea1.setType(CartesianPlot.Type.FourAxes)

plotArea1.setSymmetricPadding(False)
plotArea1.setHorizontalPadding(Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter))
plotArea1.setVerticalPadding(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
plotArea1.setRightPadding(Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter))
plotArea1.setBottomPadding(Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter))

# border = plotArea1.plotArea().borderType()
# border.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea1.plotArea().setBorderType(border)

for axis in plotArea1.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.title().setText("Wavelength (nm)")
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.title().setText("")
        axis.title().setVisible(False)

worksheet1.addChild(plotArea1)

config11 = XYCurve("IPCE")
plotArea1.addChild(config11)
config11.setPlotType(Plot.PlotType.Line)
config11.setXColumn(spreadsheet1.column(0))
config11.setYColumn(spreadsheet1.column(1))
config11.setLineType(XYCurve.LineType.Line)
config11.symbol().setStyle(Symbol.Style.NoSymbols)
config11.setValuesType(XYCurve.ValuesType.NoValues)
config11.background().setType(Background.Type.Color)
config11.background().setPosition(Background.Position.Below)
config11.background().setColorStyle(Background.ColorStyle.VerticalLinearGradient)
config11.background().setFirstColor(config11.color())
config11.background().setSecondColor("white")
config11.background().setOpacity(0.4)

config12 = XYCurve("Current Density")
plotArea1.addChild(config12)
config12.setPlotType(Plot.PlotType.Line)
config12.setXColumn(spreadsheet2.column(0))
config12.setYColumn(spreadsheet2.column(1))
config12.setLineType(XYCurve.LineType.Line)
config12.symbol().setStyle(Symbol.Style.NoSymbols)
config12.setValuesType(XYCurve.ValuesType.NoValues)
config12.background().setType(Background.Type.Color)
config12.background().setPosition(Background.Position.Below)
config12.background().setColorStyle(Background.ColorStyle.VerticalLinearGradient)
config12.background().setFirstColor(config12.color())
config12.background().setSecondColor("white")
config12.background().setOpacity(0.4)

# plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)
# rangeX1 = plotArea1.range(CartesianCoordinateSystem.Dimension.X, 0)
# rangeX1.setRange(350, 800)
# plotArea1.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX1)

# plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
# rangeY1 = plotArea1.range(CartesianCoordinateSystem.Dimension.Y, 0)
# rangeY1.setRange(0, 80)
# plotArea1.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY1)

worksheet1.view().show()

###################################################################################################################################################################
###################################################################################################################################################################

worksheet2 = Worksheet("multi-range demo")
project.addChild(worksheet2)

worksheet2.setUseViewSize(False)
w2 = Worksheet.convertToSceneUnits(15, Worksheet.Unit.Centimeter)
h2 = Worksheet.convertToSceneUnits(10, Worksheet.Unit.Centimeter)
worksheet2.setPageRect(QRectF(0, 0, w2, h2))

worksheet2.setLayout(Worksheet.Layout.VerticalLayout)
worksheet2.setLayoutTopMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet2.setLayoutBottomMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet2.setLayoutLeftMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet2.setLayoutRightMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

worksheet2.setLayoutHorizontalSpacing(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter))
worksheet2.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter))

plotArea2 = CartesianPlot("IPCE2")
plotArea2.setType(CartesianPlot.Type.FourAxes)

plotArea2.setSymmetricPadding(False)
plotArea2.setHorizontalPadding(Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter))
plotArea2.setVerticalPadding(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
plotArea2.setRightPadding(Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter))
plotArea2.setBottomPadding(Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter))

# border2 = plotArea2.plotArea().borderType()
# border2.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border2.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border2.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border2.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea2.plotArea().setBorderType(border2)

# rangeX2 = plotArea2.range(CartesianCoordinateSystem.Dimension.X, 0)
# rangeX2.setRange(350, 800)
# plotArea2.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX2)

# rangeY21 = plotArea2.range(CartesianCoordinateSystem.Dimension.Y, 0)
# rangeY21.setRange(0, 80)
# plotArea2.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY21)

plotArea2.addYRange()

# rangeY22 = plotArea2.range(CartesianCoordinateSystem.Dimension.Y, 1)
# rangeY22.setRange(0, 12)
# plotArea2.setRange(CartesianCoordinateSystem.Dimension.Y, 1, rangeY22)

# plotArea2.enableAutoScale(CartesianCoordinateSystem.Dimension.X, -1, False)
# plotArea2.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, -1, False)

plotArea2.addCoordinateSystem()
plotArea2.setCoordinateSystemRangeIndex(1, CartesianCoordinateSystem.Dimension.Y, 1)

worksheet2.addChild(plotArea2)

for axis in plotArea2.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.title().setText("Wavelength (nm)")
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.title().setText("IPCE (%)")
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Right:
        axis.title().setText("Current density (mA cm−2)")
        axis.setTitleOffsetX(Worksheet.convertToSceneUnits(42, Worksheet.Unit.Point))
        # axis.setMajorTicksDirection(Axis.TicksDirection(Axis.TicksFlags.ticksIn))
        axis.setLabelsPosition(Axis.LabelsPosition.In)
        axis.setCoordinateSystemIndex(1)
    else:
        axis.title().setText("")
        axis.title().setVisible(False)

config21 = XYCurve("IPCE")
plotArea2.addChild(config21)
config21.setPlotType(Plot.PlotType.Line)
config21.setXColumn(spreadsheet1.column(0))
config21.setYColumn(spreadsheet1.column(1))
config21.setLineType(XYCurve.LineType.Line)
config21.symbol().setStyle(Symbol.Style.NoSymbols)
config21.setValuesType(XYCurve.ValuesType.NoValues)
config21.background().setType(Background.Type.Color)
config21.background().setPosition(Background.Position.Below)
config21.background().setColorStyle(Background.ColorStyle.VerticalLinearGradient)
config21.background().setFirstColor(config21.color())
config21.background().setSecondColor("white")
config21.background().setOpacity(0.4)
for axis in plotArea2.children(AspectType.Axis):
    if axis.coordinateSystemIndex() == config21.coordinateSystemIndex():
        axis.setLabelsColor(config21.color())
        axis.majorTicksLine().setColor(config21.color())
        axis.minorTicksLine().setColor(config21.color())
        axis.line().setColor(config21.color())

config22 = XYCurve("Current Density")
plotArea2.addChild(config22)
config22.setPlotType(Plot.PlotType.Line)
config22.setXColumn(spreadsheet2.column(0))
config22.setYColumn(spreadsheet2.column(1))
config22.setLineType(XYCurve.LineType.Line)
config22.symbol().setStyle(Symbol.Style.NoSymbols)
config22.setValuesType(XYCurve.ValuesType.NoValues)
config22.background().setType(Background.Type.Color)
config22.background().setPosition(Background.Position.Below)
config22.background().setColorStyle(Background.ColorStyle.VerticalLinearGradient)
config22.background().setFirstColor(config22.color())
config22.background().setSecondColor("white")
config22.background().setOpacity(0.4)
config22.setCoordinateSystemIndex(1)
for axis in plotArea2.children(AspectType.Axis):
    if axis.coordinateSystemIndex() == config22.coordinateSystemIndex():
        axis.setLabelsColor(config22.color())
        axis.majorTicksLine().setColor(config22.color())
        axis.minorTicksLine().setColor(config22.color())
        axis.line().setColor(config22.color())

worksheet2.view().show()

###################################################################################################################################################################
###################################################################################################################################################################

worksheet3 = Worksheet("multi-range demo, alternative")
project.addChild(worksheet3)

worksheet3.setUseViewSize(False)
w3 = Worksheet.convertToSceneUnits(15, Worksheet.Unit.Centimeter)
h3 = Worksheet.convertToSceneUnits(10, Worksheet.Unit.Centimeter)
worksheet3.setPageRect(QRectF(0, 0, w3, h3))

worksheet3.setLayout(Worksheet.Layout.VerticalLayout)
worksheet3.setLayoutTopMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet3.setLayoutBottomMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet3.setLayoutLeftMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet3.setLayoutRightMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

worksheet3.setLayoutHorizontalSpacing(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter))
worksheet3.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter))

plotArea3 = CartesianPlot("IPCE3")
plotArea3.setType(CartesianPlot.Type.FourAxes)

plotArea3.setSymmetricPadding(False)
plotArea3.setHorizontalPadding(Worksheet.convertToSceneUnits(3.5, Worksheet.Unit.Centimeter))
plotArea3.setVerticalPadding(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
plotArea3.setRightPadding(Worksheet.convertToSceneUnits(0.6, Worksheet.Unit.Centimeter))
plotArea3.setBottomPadding(Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter))

# border3 = plotArea3.plotArea().borderType()
# border3.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border3.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border3.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border3.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea3.plotArea().setBorderType(border3)

# rangeX3 = plotArea3.range(CartesianCoordinateSystem.Dimension.X, 0)
# rangeX3.setRange(350, 800)
# plotArea3.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX3)

# rangeY31 = plotArea3.range(CartesianCoordinateSystem.Dimension.Y, 0)
# rangeY31.setRange(0, 80)
# plotArea3.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY31)

plotArea3.addYRange()

# rangeY32 = plotArea3.range(CartesianCoordinateSystem.Dimension.Y, 1)
# rangeY32.setRange(0, 12)
# plotArea3.setRange(CartesianCoordinateSystem.Dimension.Y, 1, rangeY32)

plotArea3.enableAutoScale(CartesianCoordinateSystem.Dimension.X, -1, False)
plotArea3.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, -1, False)

plotArea3.addCoordinateSystem()
plotArea3.setCoordinateSystemRangeIndex(1, CartesianCoordinateSystem.Dimension.Y, 1)

worksheet3.addChild(plotArea3)

for axis in plotArea3.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.title().setText("Wavelength (nm)")
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.title().setText("IPCE (%)")
        axis.setOffset(Worksheet.convertToSceneUnits(-0.5, Worksheet.Unit.Centimeter))
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Right:
        axis.title().setText("Current density (mA cm−2)")
        axis.setPosition(Axis.Position.Left)
        axis.setOffset(Worksheet.convertToSceneUnits(-2.2, Worksheet.Unit.Centimeter))
        axis.setTitleOffsetX(Worksheet.convertToSceneUnits(7, Worksheet.Unit.Point))
        # axis.setMajorTicksDirection(Axis.TicksDirection(Axis.TicksFlags.ticksIn))
        axis.setLabelsPosition(Axis.LabelsPosition.Out)
        axis.setLabelsOffset(Worksheet.convertToSceneUnits(6, Worksheet.Unit.Point))
        axis.setCoordinateSystemIndex(1)
    else:
        axis.title().setText("")
        axis.title().setVisible(False)
        axis.setVisible(False)

config31 = XYCurve("IPCE")
plotArea3.addChild(config31)
config31.setPlotType(Plot.PlotType.Line)
config31.setXColumn(spreadsheet1.column(0))
config31.setYColumn(spreadsheet1.column(1))
config31.setLineType(XYCurve.LineType.Line)
config31.symbol().setStyle(Symbol.Style.NoSymbols)
config31.setValuesType(XYCurve.ValuesType.NoValues)
config31.background().setType(Background.Type.Color)
config31.background().setPosition(Background.Position.Below)
config31.background().setColorStyle(Background.ColorStyle.VerticalLinearGradient)
config31.background().setFirstColor(config31.color())
config31.background().setSecondColor("white")
config31.background().setOpacity(0.4)
for axis in plotArea3.children(AspectType.Axis):
    if axis.coordinateSystemIndex() == config31.coordinateSystemIndex():
        axis.setLabelsColor(config31.color())
        axis.majorTicksLine().setColor(config31.color())
        axis.minorTicksLine().setColor(config31.color())
        axis.line().setColor(config31.color())

config32 = XYCurve("Current Density")
plotArea3.addChild(config32)
config32.setPlotType(Plot.PlotType.Line)
config32.setXColumn(spreadsheet2.column(0))
config32.setYColumn(spreadsheet2.column(1))
config32.setLineType(XYCurve.LineType.Line)
config32.symbol().setStyle(Symbol.Style.NoSymbols)
config32.setValuesType(XYCurve.ValuesType.NoValues)
config32.background().setType(Background.Type.Color)
config32.background().setPosition(Background.Position.Below)
config32.background().setColorStyle(Background.ColorStyle.VerticalLinearGradient)
config32.background().setFirstColor(config32.color())
config32.background().setSecondColor("white")
config32.background().setOpacity(0.4)
config32.setCoordinateSystemIndex(1)
for axis in plotArea3.children(AspectType.Axis):
    if axis.coordinateSystemIndex() == config32.coordinateSystemIndex():
        axis.setLabelsColor(config32.color())
        axis.majorTicksLine().setColor(config32.color())
        axis.minorTicksLine().setColor(config32.color())
        axis.line().setColor(config32.color())

Project.retransformElements(project)
