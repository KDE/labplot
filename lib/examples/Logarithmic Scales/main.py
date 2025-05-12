import sys

from PySide6.QtWidgets import QApplication
from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QFont
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

app = QApplication()

project = Project()

worksheet = Worksheet("Worksheet")
project.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(15.6, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(12.7, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setLayout(Worksheet.Layout.GridLayout)
worksheet.setLayoutRowCount(2)
worksheet.setLayoutColumnCount(2)

worksheet.setLayoutTopMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutBottomMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutLeftMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutRightMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

worksheet.setLayoutHorizontalSpacing(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

worksheet.setTheme("Solarized")

plotArea = CartesianPlot("xy-plot")
plotArea.setType(CartesianPlot.Type.FourAxes)

# border = plotArea.plotArea().borderType()
# border.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea.plotArea().setBorderType(border)

plotArea.setSymmetricPadding(True)
plotArea.setHorizontalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea.setVerticalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))

plotArea.title().setText("lin-lin")
plotArea.title().setHorizontalAlignment(WorksheetElement.HorizontalAlignment.Center)
plotArea.title().setVerticalAlignment(WorksheetElement.VerticalAlignment.Top)
plotArea.title().setPosition(QPointF(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter)))

worksheet.addChild(plotArea)

for axis in plotArea.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.title().setText("x")
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.title().setText("f(x)")

config11 = XYEquationCurve("f(x)=10^x")
plotArea.addChild(config11)
eqData11 = config11.equationData()
eqData11.type = XYEquationCurve.EquationType.Cartesian
eqData11.count = 1000
eqData11.min = "0"
eqData11.max = "10"
eqData11.expression1 = "10^x"
config11.setEquationData(eqData11)
config11.recalculate()

config12 = XYEquationCurve("f(x)=x")
plotArea.addChild(config12)
eqData12 = config12.equationData()
eqData12.type = XYEquationCurve.EquationType.Cartesian
eqData12.count = 2
eqData12.min = "0"
eqData12.max = "10"
eqData12.expression1 = "x"
config12.setEquationData(eqData12)
config12.recalculate()

config13 = XYEquationCurve("f(x)=log(x)")
plotArea.addChild(config13)
eqData13 = config13.equationData()
eqData13.type = XYEquationCurve.EquationType.Cartesian
eqData13.count = 100
eqData13.min = "0"
eqData13.max = "10"
eqData13.expression1 = "log(x)"
config13.setEquationData(eqData13)
config13.recalculate()

# plotArea.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)
# rangeX = plotArea.range(CartesianCoordinateSystem.Dimension.X, 0)
# rangeX.setRange(0, 10)
# plotArea.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX)

# plotArea.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
# rangeY = plotArea.range(CartesianCoordinateSystem.Dimension.Y, 0)
# rangeY.setRange(0, 10)
# plotArea.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY)

###################################################################################################################################################################
###################################################################################################################################################################

plotArea1 = CartesianPlot("xy-plot 1")
plotArea1.setType(CartesianPlot.Type.FourAxes)

# border1 = plotArea1.plotArea().borderType()
# border1.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border1.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border1.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border1.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea1.plotArea().setBorderType(border1)

plotArea1.setSymmetricPadding(True)
plotArea1.setHorizontalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea1.setVerticalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))

plotArea1.title().setText("log-lin")
plotArea1.title().setHorizontalAlignment(WorksheetElement.HorizontalAlignment.Center)
plotArea1.title().setVerticalAlignment(WorksheetElement.VerticalAlignment.Top)
plotArea1.title().setPosition(QPointF(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter)))

worksheet.addChild(plotArea1)

for axis in plotArea1.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.title().setText("x")
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.title().setText("f(x)")

config21 = XYEquationCurve("f(x)=10^x")
plotArea1.addChild(config21)
eqData21 = config21.equationData()
eqData21.type = XYEquationCurve.EquationType.Cartesian
eqData21.count = 100
eqData21.min = "0"
eqData21.max = "1"
eqData21.expression1 = "10^x"
config21.setEquationData(eqData21)
config21.recalculate()

config22 = XYEquationCurve("f(x)=x")
plotArea1.addChild(config22)
eqData22 = config22.equationData()
eqData22.type = XYEquationCurve.EquationType.Cartesian
eqData22.count = 100
eqData22.min = "0"
eqData22.max = "10"
eqData22.expression1 = "x"
config22.setEquationData(eqData22)
config22.recalculate()

config23 = XYEquationCurve("f(x)=log(x)")
plotArea1.addChild(config23)
eqData23 = config23.equationData()
eqData23.type = XYEquationCurve.EquationType.Cartesian
eqData23.count = 100
eqData23.min = "1"
eqData23.max = "1000"
eqData23.expression1 = "log(x)"
config23.setEquationData(eqData23)
config23.recalculate()

# plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)
# rangeX1 = plotArea1.range(CartesianCoordinateSystem.Dimension.X, 0)
# rangeX1.setScale(RangeT.Scale.Log10)
# rangeX1.setRange(0.1, 1000)
# plotArea1.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX1)

# plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
# rangeY1 = plotArea1.range(CartesianCoordinateSystem.Dimension.Y, 0)
# rangeY1.setRange(0, 10)
# plotArea1.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY1)

###################################################################################################################################################################
###################################################################################################################################################################

plotArea2 = CartesianPlot("xy-plot 2")
plotArea2.setType(CartesianPlot.Type.FourAxes)

# border2 = plotArea2.plotArea().borderType()
# border2.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border2.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border2.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border2.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea2.plotArea().setBorderType(border2)

plotArea2.setSymmetricPadding(True)
plotArea2.setHorizontalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea2.setVerticalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))

plotArea2.title().setText("lin-log")
plotArea2.title().setHorizontalAlignment(WorksheetElement.HorizontalAlignment.Center)
plotArea2.title().setVerticalAlignment(WorksheetElement.VerticalAlignment.Top)
plotArea2.title().setPosition(QPointF(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter)))

worksheet.addChild(plotArea2)

for axis in plotArea2.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.title().setText("x")
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.title().setText("f(x)")

config31 = XYEquationCurve("f(x)=10^x")
plotArea2.addChild(config31)
eqData31 = config31.equationData()
eqData31.type = XYEquationCurve.EquationType.Cartesian
eqData31.count = 1000
eqData31.min = "0"
eqData31.max = "10"
eqData31.expression1 = "10^x"
config31.setEquationData(eqData31)
config31.recalculate()

config32 = XYEquationCurve("f(x)=x")
plotArea2.addChild(config32)
eqData32 = config32.equationData()
eqData32.type = XYEquationCurve.EquationType.Cartesian
eqData32.count = 100
eqData32.min = "0"
eqData32.max = "10"
eqData32.expression1 = "x"
config32.setEquationData(eqData32)
config32.recalculate()

config33 = XYEquationCurve("f(x)=log(x)")
plotArea2.addChild(config33)
eqData33 = config33.equationData()
eqData33.type = XYEquationCurve.EquationType.Cartesian
eqData33.count = 1000
eqData33.min = "0"
eqData33.max = "10"
eqData33.expression1 = "log(x)"
config33.setEquationData(eqData33)
config33.recalculate()

# plotArea2.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)
# rangeX2 = plotArea2.range(CartesianCoordinateSystem.Dimension.X, 0)
# rangeX2.setRange(0, 10)
# plotArea2.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX2)

# plotArea2.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
# rangeY2 = plotArea2.range(CartesianCoordinateSystem.Dimension.Y, 0)
# rangeY2.setRange(0.1, 1000)
# rangeY2.setScale(RangeT.Scale.Log10)
# plotArea2.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY2)

###################################################################################################################################################################
###################################################################################################################################################################

plotArea3 = CartesianPlot("xy-plot 3")
plotArea3.setType(CartesianPlot.Type.FourAxes)

# border3 = plotArea3.plotArea().borderType()
# border3.setFlag(PlotArea.BorderTypeFlags.BorderLeft, True)
# border3.setFlag(PlotArea.BorderTypeFlags.BorderTop, True)
# border3.setFlag(PlotArea.BorderTypeFlags.BorderRight, True)
# border3.setFlag(PlotArea.BorderTypeFlags.BorderBottom, True)
# plotArea3.plotArea().setBorderType(border3)

plotArea3.setSymmetricPadding(True)
plotArea3.setHorizontalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea3.setVerticalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))

plotArea3.title().setText("log-log")
plotArea3.title().setHorizontalAlignment(WorksheetElement.HorizontalAlignment.Center)
plotArea3.title().setVerticalAlignment(WorksheetElement.VerticalAlignment.Top)
plotArea3.title().setPosition(QPointF(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter)))

worksheet.addChild(plotArea3)

for axis in plotArea3.children(AspectType.Axis):
    axis.setMinorTicksLength(0)
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.title().setText("x")
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.title().setText("f(x)")

config41 = XYEquationCurve("f(x)=10^x")
plotArea3.addChild(config41)
eqData41 = config41.equationData()
eqData41.type = XYEquationCurve.EquationType.Cartesian
eqData41.count = 1000
eqData41.min = "0"
eqData41.max = "10"
eqData41.expression1 = "10^x"
config41.setEquationData(eqData41)
config41.recalculate()

config42 = XYEquationCurve("f(x)=x")
plotArea3.addChild(config42)
eqData42 = config42.equationData()
eqData42.type = XYEquationCurve.EquationType.Cartesian
eqData42.count = 1000
eqData42.min = "0.1"
eqData42.max = "1000"
eqData42.expression1 = "x"
config42.setEquationData(eqData42)
config42.recalculate()

config43 = XYEquationCurve("f(x)=log(x)")
plotArea3.addChild(config43)
eqData43 = config43.equationData()
eqData43.type = XYEquationCurve.EquationType.Cartesian
eqData43.count = 10000
eqData43.min = "0.1"
eqData43.max = "1000"
eqData43.expression1 = "log(x)"
config43.setEquationData(eqData43)
config43.recalculate()

# plotArea3.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)
# rangeX3 = plotArea3.range(CartesianCoordinateSystem.Dimension.X, 0)
# rangeX3.setScale(RangeT.Scale.Log10)
# rangeX3.setRange(0.1, 1000)
# plotArea3.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX3)

# plotArea3.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
# rangeY3 = plotArea3.range(CartesianCoordinateSystem.Dimension.Y, 0)
# rangeY3.setScale(RangeT.Scale.Log10)
# rangeY3.setRange(0.1, 1000)
# plotArea3.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY3)

worksheet.view().show()

app.exec()
