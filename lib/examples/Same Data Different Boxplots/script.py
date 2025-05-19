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

spreadsheet1 = Spreadsheet("Data")
project.addChild(spreadsheet1)

filter1.readDataFromFile("lib/examples/Same Data Different Boxplots/data.txt", spreadsheet1, AbstractFileFilter.ImportMode.Replace)

if filter1.lastError():
	print(f"Import error: {filter1.lastError()}")
	sys.exit(-1)

filter2 = AsciiFilter()

p2 = filter2.properties()
p2.headerEnabled = False
filter2.setProperties(p2)

spreadsheet2 = Spreadsheet("Labels")
project.addChild(spreadsheet2)

filter2.readDataFromFile("lib/examples/Same Data Different Boxplots/labels.txt", spreadsheet2, AbstractFileFilter.ImportMode.Replace)

if filter2.lastError():
	print(f"Import error: {filter2.lastError()}")
	sys.exit(-1)

worksheet = Worksheet("Boxplots")
project.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setLayout(Worksheet.Layout.GridLayout)
worksheet.setLayoutRowCount(3)
worksheet.setLayoutColumnCount(3)

worksheet.setLayoutTopMargin(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutBottomMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutLeftMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutRightMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

worksheet.setLayoutHorizontalSpacing(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter))
worksheet.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter))

plotArea1 = CartesianPlot("IPCE1")
plotArea1.setType(CartesianPlot.Type.FourAxes)

plotArea1.setSymmetricPadding(False)
plotArea1.setHorizontalPadding(Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter))
plotArea1.setVerticalPadding(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
plotArea1.setRightPadding(Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter))
plotArea1.setBottomPadding(Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter))

fo7 = QFont()
fo7.setPointSize(Worksheet.convertToSceneUnits(7, Worksheet.Unit.Point))

plotArea1 = CartesianPlot("Version 1")
plotArea1.setType(CartesianPlot.Type.FourAxes)

plotArea1.setSymmetricPadding(True)
plotArea1.setHorizontalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))
plotArea1.setVerticalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))

for axis in plotArea1.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)
    # axis.setMinorTicksDirection(Axis.TicksDirection.enum_type.noTicks)
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.setMajorTicksType(Axis.TicksType.CustomColumn)
        axis.setMajorTicksColumn(spreadsheet2.column(0))
        axis.setLabelsTextType(Axis.LabelsTextType.CustomValues)
        axis.setLabelsTextColumn(spreadsheet2.column(1))
        axis.setLabelsFont(fo7)
        axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.setLabelsFont(fo7)

plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, True)
plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, True)

worksheet.addChild(plotArea1)

boxPlot1 = BoxPlot("BoxPlot")
plotArea1.addChild(boxPlot1)
boxPlot1.setDataColumns([spreadsheet1.column(0), spreadsheet1.column(1)])
boxPlot1.setOrdering(BoxPlot.Ordering.None_)
boxPlot1.setWhiskersType(BoxPlot.WhiskersType.IQR)
boxPlot1.setWhiskersRangeParameter(1.5)
boxPlot1.setOrientation(BoxPlot.Orientation.Vertical)
boxPlot1.setVariableWidth(False)
boxPlot1.setNotchesEnabled(False)
boxPlot1.symbolMean().setStyle(Symbol.Style.NoSymbols)
boxPlot1.symbolMedian().setStyle(Symbol.Style.NoSymbols)
boxPlot1.symbolOutlier().setStyle(Symbol.Style.NoSymbols)
boxPlot1.symbolFarOut().setStyle(Symbol.Style.Plus)
boxPlot1.symbolFarOut().setSize(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
boxPlot1.symbolData().setStyle(Symbol.Style.NoSymbols)
boxPlot1.symbolWhiskerEnd().setStyle(Symbol.Style.NoSymbols)
boxPlot1.setJitteringEnabled(True)
boxPlot1.setRugEnabled(False)
boxPlot1.whiskersLine().setStyle(Qt.PenStyle.DashLine)
boxPlot1.whiskersCapLine().setStyle(Qt.PenStyle.NoPen)
boxPlot1.backgroundAt(0).setEnabled(False)
boxPlot1.backgroundAt(1).setEnabled(False)
boxPlot1.recalc()

plotArea2 = CartesianPlot("Version 2")
plotArea2.setType(CartesianPlot.Type.FourAxes)

plotArea2.setSymmetricPadding(True)
plotArea2.setHorizontalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))
plotArea2.setVerticalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))

for axis in plotArea2.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)
    # axis.setMinorTicksDirection(Axis.TicksDirection.enum_type.noTicks)
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.setMajorTicksType(Axis.TicksType.CustomColumn)
        axis.setMajorTicksColumn(spreadsheet2.column(0))
        axis.setLabelsTextType(Axis.LabelsTextType.CustomValues)
        axis.setLabelsTextColumn(spreadsheet2.column(1))
        axis.setLabelsFont(fo7)
        axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.setLabelsFont(fo7)

plotArea2.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, True)
plotArea2.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, True)

worksheet.addChild(plotArea2)

boxPlot2 = BoxPlot("BoxPlot")
plotArea2.addChild(boxPlot2)
boxPlot2.setDataColumns([spreadsheet1.column(0), spreadsheet1.column(1)])
boxPlot2.setOrdering(BoxPlot.Ordering.None_)
boxPlot2.setWhiskersType(BoxPlot.WhiskersType.IQR)
boxPlot2.setWhiskersRangeParameter(1.5)
boxPlot2.setOrientation(BoxPlot.Orientation.Vertical)
boxPlot2.setVariableWidth(False)
boxPlot2.setNotchesEnabled(False)
boxPlot2.symbolMean().setStyle(Symbol.Style.Circle)
boxPlot2.symbolMean().setColor("white")
boxPlot2.symbolMean().setSize(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
boxPlot2.symbolMedian().setStyle(Symbol.Style.NoSymbols)
boxPlot2.symbolOutlier().setStyle(Symbol.Style.Circle)
boxPlot2.symbolOutlier().setColor("black")
boxPlot2.symbolOutlier().setSize(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
boxPlot2.symbolFarOut().setStyle(Symbol.Style.Plus)
boxPlot2.symbolFarOut().setColor("black")
boxPlot2.symbolFarOut().setSize(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
boxPlot2.symbolData().setStyle(Symbol.Style.NoSymbols)
boxPlot2.symbolWhiskerEnd().setStyle(Symbol.Style.NoSymbols)
boxPlot2.setJitteringEnabled(True)
boxPlot2.setRugEnabled(False)
boxPlot2.whiskersLine().setStyle(Qt.PenStyle.DashLine)
boxPlot2.whiskersCapLine().setStyle(Qt.PenStyle.SolidLine)
boxPlot2.setWhiskersCapSize(Worksheet.convertToSceneUnits(25, Worksheet.Unit.Point))
boxPlot2.backgroundAt(0).setEnabled(True)
boxPlot2.backgroundAt(0).setFirstColor("black")
boxPlot2.backgroundAt(0).setOpacity(1)
boxPlot2.backgroundAt(1).setEnabled(True)
boxPlot2.backgroundAt(1).setFirstColor("black")
boxPlot2.backgroundAt(1).setOpacity(1)
boxPlot2.recalc()

plotArea3 = CartesianPlot("Version 3")
plotArea3.setType(CartesianPlot.Type.FourAxes)

plotArea3.setSymmetricPadding(True)
plotArea3.setHorizontalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))
plotArea3.setVerticalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))

for axis in plotArea3.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)
    # axis.setMinorTicksDirection(Axis.TicksDirection.enum_type.noTicks)
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.setMajorTicksType(Axis.TicksType.CustomColumn)
        axis.setMajorTicksColumn(spreadsheet2.column(0))
        axis.setLabelsTextType(Axis.LabelsTextType.CustomValues)
        axis.setLabelsTextColumn(spreadsheet2.column(1))
        axis.setLabelsFont(fo7)
        axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.setLabelsFont(fo7)

plotArea3.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, True)
plotArea3.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, True)

worksheet.addChild(plotArea3)

boxPlot3 = BoxPlot("BoxPlot")
plotArea3.addChild(boxPlot3)
boxPlot3.setDataColumns([spreadsheet1.column(0), spreadsheet1.column(1)])
boxPlot3.setOrdering(BoxPlot.Ordering.None_)
boxPlot3.setWidthFactor(0.8)
boxPlot3.setWhiskersType(BoxPlot.WhiskersType.IQR)
boxPlot3.setWhiskersRangeParameter(1.5)
boxPlot3.setOrientation(BoxPlot.Orientation.Vertical)
boxPlot3.setVariableWidth(True)
boxPlot3.setNotchesEnabled(True)
boxPlot3.symbolMean().setStyle(Symbol.Style.Square)
boxPlot3.symbolMean().setSize(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
boxPlot3.symbolMedian().setStyle(Symbol.Style.NoSymbols)
boxPlot3.symbolOutlier().setStyle(Symbol.Style.NoSymbols)
boxPlot3.symbolFarOut().setStyle(Symbol.Style.Plus)
boxPlot3.symbolFarOut().setSize(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
boxPlot3.symbolData().setStyle(Symbol.Style.NoSymbols)
boxPlot3.symbolWhiskerEnd().setStyle(Symbol.Style.NoSymbols)
boxPlot3.setJitteringEnabled(True)
boxPlot3.setRugEnabled(False)
boxPlot3.whiskersLine().setStyle(Qt.PenStyle.SolidLine)
boxPlot3.whiskersCapLine().setStyle(Qt.PenStyle.NoPen)
boxPlot3.backgroundAt(0).setEnabled(True)
boxPlot3.backgroundAt(0).setOpacity(0.3)
boxPlot3.backgroundAt(1).setEnabled(True)
boxPlot3.backgroundAt(1).setOpacity(0.3)
boxPlot3.recalc()

plotArea4 = CartesianPlot("Version 4")
plotArea4.setType(CartesianPlot.Type.FourAxes)

plotArea4.setSymmetricPadding(True)
plotArea4.setHorizontalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))
plotArea4.setVerticalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))

for axis in plotArea4.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)
    # axis.setMinorTicksDirection(Axis.TicksDirection.enum_type.noTicks)
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.setMajorTicksType(Axis.TicksType.CustomColumn)
        axis.setMajorTicksColumn(spreadsheet2.column(0))
        axis.setLabelsTextType(Axis.LabelsTextType.CustomValues)
        axis.setLabelsTextColumn(spreadsheet2.column(1))
        axis.setLabelsFont(fo7)
        axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.setLabelsFont(fo7)

plotArea4.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, True)
plotArea4.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, True)

worksheet.addChild(plotArea4)

boxPlot4 = BoxPlot("BoxPlot")
plotArea4.addChild(boxPlot4)
boxPlot4.setDataColumns([spreadsheet1.column(0), spreadsheet1.column(1)])
boxPlot4.setOrdering(BoxPlot.Ordering.None_)
boxPlot4.setWidthFactor(0.8)
boxPlot4.setWhiskersType(BoxPlot.WhiskersType.IQR)
boxPlot4.setWhiskersRangeParameter(1.5)
boxPlot4.setOrientation(BoxPlot.Orientation.Vertical)
boxPlot4.setVariableWidth(True)
boxPlot4.setNotchesEnabled(False)
boxPlot4.symbolMean().setStyle(Symbol.Style.Square)
boxPlot4.symbolMean().setSize(Worksheet.convertToSceneUnits(6, Worksheet.Unit.Point))
boxPlot4.symbolMedian().setStyle(Symbol.Style.NoSymbols)
boxPlot4.symbolOutlier().setStyle(Symbol.Style.Diamond)
boxPlot4.symbolOutlier().setSize(Worksheet.convertToSceneUnits(7, Worksheet.Unit.Point))
boxPlot4.symbolFarOut().setStyle(Symbol.Style.Diamond)
boxPlot4.symbolFarOut().setSize(Worksheet.convertToSceneUnits(7, Worksheet.Unit.Point))
boxPlot4.symbolData().setStyle(Symbol.Style.Circle)
boxPlot4.symbolData().setSize(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
boxPlot4.symbolData().setOpacity(0.15)
boxPlot4.symbolData().setColor("black")
boxPlot4.symbolWhiskerEnd().setStyle(Symbol.Style.NoSymbols)
boxPlot4.setJitteringEnabled(True)
boxPlot4.setRugEnabled(False)
boxPlot4.whiskersLine().setStyle(Qt.PenStyle.SolidLine)
boxPlot4.whiskersCapLine().setStyle(Qt.PenStyle.NoPen)
boxPlot4.backgroundAt(0).setEnabled(False)
boxPlot4.backgroundAt(1).setEnabled(False)
boxPlot4.recalc()

plotArea5 = CartesianPlot("Version 5")
plotArea5.setType(CartesianPlot.Type.FourAxes)

plotArea5.setSymmetricPadding(True)
plotArea5.setHorizontalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))
plotArea5.setVerticalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))

for axis in plotArea5.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)
    # axis.setMinorTicksDirection(Axis.TicksDirection.enum_type.noTicks)
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.setMajorTicksType(Axis.TicksType.CustomColumn)
        axis.setMajorTicksColumn(spreadsheet2.column(0))
        axis.setLabelsTextType(Axis.LabelsTextType.CustomValues)
        axis.setLabelsTextColumn(spreadsheet2.column(1))
        axis.setLabelsFont(fo7)
        axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
        axis.majorTicksLine().setStyle(Qt.PenStyle.NoPen)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.setLabelsFont(fo7)
        axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
        axis.majorTicksLine().setStyle(Qt.PenStyle.NoPen)
    else:
        axis.setVisible(False)

plotArea5.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, True)
plotArea5.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, True)

worksheet.addChild(plotArea5)

boxPlot5 = BoxPlot("BoxPlot")
plotArea5.addChild(boxPlot5)
boxPlot5.setDataColumns([spreadsheet1.column(0), spreadsheet1.column(1)])
boxPlot5.setOrdering(BoxPlot.Ordering.None_)
boxPlot5.setWhiskersType(BoxPlot.WhiskersType.IQR)
boxPlot5.setWhiskersRangeParameter(1.5)
boxPlot5.setOrientation(BoxPlot.Orientation.Vertical)
boxPlot5.setVariableWidth(False)
boxPlot5.setNotchesEnabled(False)
boxPlot5.symbolMean().setStyle(Symbol.Style.Circle)
boxPlot5.symbolMean().setSize(Worksheet.convertToSceneUnits(4, Worksheet.Unit.Point))
boxPlot5.symbolMean().setColor("black")
boxPlot5.setWhiskersCapSize(0)
boxPlot5.whiskersCapLine().setColor("black")
boxPlot5.setJitteringEnabled(True)
for i in range(spreadsheet1.columnCount()):
    boxPlot5.medianLineAt(i).setStyle(Qt.PenStyle.NoPen)
    boxPlot5.borderLineAt(i).setStyle(Qt.PenStyle.NoPen)
    boxPlot5.backgroundAt(i).setEnabled(False)

plotArea6 = CartesianPlot("Version 6")
plotArea6.setType(CartesianPlot.Type.FourAxes)

plotArea6.setSymmetricPadding(True)
plotArea6.setHorizontalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))
plotArea6.setVerticalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))

for axis in plotArea6.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)
    # axis.setMinorTicksDirection(Axis.TicksDirection.enum_type.noTicks)
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.setMajorTicksType(Axis.TicksType.CustomColumn)
        axis.setMajorTicksColumn(spreadsheet2.column(0))
        axis.setLabelsTextType(Axis.LabelsTextType.CustomValues)
        axis.setLabelsTextColumn(spreadsheet2.column(1))
        axis.setLabelsFont(fo7)
        axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.setLabelsFont(fo7)

plotArea6.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, True)
plotArea6.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, True)

worksheet.addChild(plotArea6)

boxPlot6 = BoxPlot("BoxPlot")
plotArea6.addChild(boxPlot6)
boxPlot6.setDataColumns([spreadsheet1.column(0), spreadsheet1.column(1)])
boxPlot6.setOrdering(BoxPlot.Ordering.None_)
boxPlot6.setWhiskersType(BoxPlot.WhiskersType.IQR)
boxPlot6.setWhiskersRangeParameter(1.5)
boxPlot6.setOrientation(BoxPlot.Orientation.Vertical)
boxPlot6.setVariableWidth(False)
boxPlot6.setNotchesEnabled(False)
boxPlot6.symbolMean().setStyle(Symbol.Style.NoSymbols)
boxPlot6.symbolOutlier().setStyle(Symbol.Style.Circle)
boxPlot6.symbolOutlier().setSize(Worksheet.convertToSceneUnits(3, Worksheet.Unit.Point))
boxPlot6.symbolFarOut().setStyle(Symbol.Style.Circle)
boxPlot6.symbolFarOut().setSize(Worksheet.convertToSceneUnits(6, Worksheet.Unit.Point))
boxPlot6.setJitteringEnabled(False)
boxPlot6.setRugEnabled(False)
boxPlot6.setWhiskersCapSize(Worksheet.convertToSceneUnits(40.5, Worksheet.Unit.Point))
boxPlot6.recalc()

plotArea7 = CartesianPlot("Version 7")
plotArea7.setType(CartesianPlot.Type.FourAxes)

plotArea7.setSymmetricPadding(True)
plotArea7.setHorizontalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))
plotArea7.setVerticalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))

for axis in plotArea7.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)
    # axis.setMinorTicksDirection(Axis.TicksDirection.enum_type.noTicks)
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.setMajorTicksType(Axis.TicksType.CustomColumn)
        axis.setMajorTicksColumn(spreadsheet2.column(0))
        axis.setLabelsTextType(Axis.LabelsTextType.CustomValues)
        axis.setLabelsTextColumn(spreadsheet2.column(1))
        axis.setLabelsFont(fo7)
        axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.setLabelsFont(fo7)

plotArea7.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, True)
plotArea7.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, True)

worksheet.addChild(plotArea7)

boxPlot7 = BoxPlot("BoxPlot")
plotArea7.addChild(boxPlot7)
boxPlot7.setDataColumns([spreadsheet1.column(0), spreadsheet1.column(1)])
boxPlot7.setOrdering(BoxPlot.Ordering.None_)
boxPlot7.setWhiskersType(BoxPlot.WhiskersType.IQR)
boxPlot7.setWhiskersRangeParameter(1.5)
boxPlot7.setOrientation(BoxPlot.Orientation.Vertical)
boxPlot7.setVariableWidth(False)
boxPlot7.setNotchesEnabled(False)
boxPlot7.symbolMean().setStyle(Symbol.Style.NoSymbols)
boxPlot7.symbolOutlier().setStyle(Symbol.Style.Circle)
boxPlot7.symbolOutlier().setSize(Worksheet.convertToSceneUnits(3, Worksheet.Unit.Point))
boxPlot7.symbolFarOut().setStyle(Symbol.Style.Circle)
boxPlot7.symbolFarOut().setSize(Worksheet.convertToSceneUnits(6, Worksheet.Unit.Point))
boxPlot7.setJitteringEnabled(False)
boxPlot7.setRugEnabled(False)
boxPlot7.setWhiskersCapSize(Worksheet.convertToSceneUnits(40.5, Worksheet.Unit.Point))
boxPlot7.recalc()

plotArea8 = CartesianPlot("Version 8")
plotArea8.setType(CartesianPlot.Type.FourAxes)

plotArea8.setSymmetricPadding(True)
plotArea8.setHorizontalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))
plotArea8.setVerticalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))

for axis in plotArea8.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)
    # axis.setMinorTicksDirection(Axis.TicksDirection.enum_type.noTicks)
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.setMajorTicksType(Axis.TicksType.CustomColumn)
        axis.setMajorTicksColumn(spreadsheet2.column(0))
        axis.setLabelsTextType(Axis.LabelsTextType.CustomValues)
        axis.setLabelsTextColumn(spreadsheet2.column(1))
        axis.setLabelsFont(fo7)
        axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.setLabelsFont(fo7)

plotArea8.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, True)
plotArea8.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, True)

worksheet.addChild(plotArea8)

boxPlot8 = BoxPlot("BoxPlot")
plotArea8.addChild(boxPlot8)
boxPlot8.setDataColumns([spreadsheet1.column(0), spreadsheet1.column(1)])
boxPlot8.setOrdering(BoxPlot.Ordering.None_)
boxPlot8.setWhiskersType(BoxPlot.WhiskersType.IQR)
boxPlot8.setWhiskersRangeParameter(1.5)
boxPlot8.setOrientation(BoxPlot.Orientation.Vertical)
boxPlot8.setVariableWidth(False)
boxPlot8.setNotchesEnabled(False)
boxPlot8.symbolMean().setStyle(Symbol.Style.NoSymbols)
boxPlot8.symbolOutlier().setStyle(Symbol.Style.Circle)
boxPlot8.symbolOutlier().setSize(Worksheet.convertToSceneUnits(3, Worksheet.Unit.Point))
boxPlot8.symbolFarOut().setStyle(Symbol.Style.Circle)
boxPlot8.symbolFarOut().setSize(Worksheet.convertToSceneUnits(6, Worksheet.Unit.Point))
boxPlot8.setJitteringEnabled(False)
boxPlot8.setRugEnabled(False)
boxPlot8.setWhiskersCapSize(Worksheet.convertToSceneUnits(40.5, Worksheet.Unit.Point))
boxPlot8.recalc()

plotArea9 = CartesianPlot("Version 9")
plotArea9.setType(CartesianPlot.Type.FourAxes)

plotArea9.setSymmetricPadding(True)
plotArea9.setHorizontalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))
plotArea9.setVerticalPadding(Worksheet.convertToSceneUnits(0.7, Worksheet.Unit.Centimeter))

for axis in plotArea9.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)
    # axis.setMinorTicksDirection(Axis.TicksDirection.enum_type.noTicks)
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.setMajorTicksType(Axis.TicksType.CustomColumn)
        axis.setMajorTicksColumn(spreadsheet2.column(0))
        axis.setLabelsTextType(Axis.LabelsTextType.CustomValues)
        axis.setLabelsTextColumn(spreadsheet2.column(1))
        axis.setLabelsFont(fo7)
        axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.setLabelsFont(fo7)

plotArea9.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, True)
plotArea9.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, True)

worksheet.addChild(plotArea9)

boxPlot9 = BoxPlot("BoxPlot")
plotArea9.addChild(boxPlot9)
boxPlot9.setDataColumns([spreadsheet1.column(0), spreadsheet1.column(1)])
boxPlot9.setOrdering(BoxPlot.Ordering.None_)
boxPlot9.setWhiskersType(BoxPlot.WhiskersType.IQR)
boxPlot9.setWhiskersRangeParameter(1.5)
boxPlot9.setOrientation(BoxPlot.Orientation.Vertical)
boxPlot9.setVariableWidth(False)
boxPlot9.setNotchesEnabled(False)
boxPlot9.symbolMean().setStyle(Symbol.Style.NoSymbols)
boxPlot9.symbolOutlier().setStyle(Symbol.Style.Circle)
boxPlot9.symbolOutlier().setSize(Worksheet.convertToSceneUnits(3, Worksheet.Unit.Point))
boxPlot9.symbolFarOut().setStyle(Symbol.Style.Circle)
boxPlot9.symbolFarOut().setSize(Worksheet.convertToSceneUnits(6, Worksheet.Unit.Point))
boxPlot9.setJitteringEnabled(False)
boxPlot9.setRugEnabled(False)
boxPlot9.setWhiskersCapSize(Worksheet.convertToSceneUnits(40.5, Worksheet.Unit.Point))
boxPlot9.recalc()

###################################################################################################################################################################
###################################################################################################################################################################

textLabel1 = TextLabel("Text Label")
worksheet.addChild(textLabel1)

te = QTextEdit()
te.clear()
te.setFontPointSize(14)
te.append("Same Data, Different Boxplots")

textLabel1.setText(te.toHtml())
textLabel1.setPosition(QPointF(Worksheet.convertToSceneUnits(-0.3, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(9.2, Worksheet.Unit.Centimeter)))

Project.retransformElements(project)
