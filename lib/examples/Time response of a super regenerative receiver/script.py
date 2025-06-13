import sys

from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QFont, QColor
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

project = project()

filter = AsciiFilter()

p = filter.properties()
p.headerEnabled = False
filter.setProperties(p)

spreadsheet = Spreadsheet("Q=1.25, tr=5ps")
project.addChild(spreadsheet)

filter.readDataFromFile("lib/examples/Time response of a super regenerative receiver/data.txt", spreadsheet, AbstractFileFilter.ImportMode.Replace)

if filter.lastError():
	print(f"Import error: {filter.lastError()}")
	sys.exit(-1)

worksheet = Worksheet("Worksheet")
project.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(13.2, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(12.7, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setLayout(Worksheet.Layout.VerticalLayout)
worksheet.setLayoutTopMargin(Worksheet.convertToSceneUnits(0.3, Worksheet.Unit.Centimeter))
worksheet.setLayoutBottomMargin(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter))
worksheet.setLayoutLeftMargin(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter))
worksheet.setLayoutRightMargin(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter))

worksheet.setLayoutHorizontalSpacing(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

###################################################################################################################################################################
###################################################################################################################################################################

plotArea = CartesianPlot("Response plot")
plotArea.setType(CartesianPlot.Type.FourAxes)

plotArea.setSymmetricPadding(False)
plotArea.setHorizontalPadding(Worksheet.convertToSceneUnits(3, Worksheet.Unit.Centimeter))
plotArea.setVerticalPadding(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Centimeter))
plotArea.setRightPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea.setBottomPadding(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Centimeter))

rangeX = plotArea.range(CartesianCoordinateSystem.Dimension.X, 0)
rangeX.setRange(2E-09, 5E-09)
plotArea.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX)

rangeY = plotArea.range(CartesianCoordinateSystem.Dimension.Y, 0)
rangeY.setRange(-0.00035, 0.00035)
plotArea.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY)

plotArea.enableAutoScale(CartesianCoordinateSystem.Dimension.X, -1, False)
plotArea.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, -1, False)

te = QTextEdit()
te.setText("Time response of SuperRegenerative receiverQ=1,25  ·  tr=5ps  ·  wid=343,8ps  ·  delay=0ps")
te.setHtml(te.toHtml().replace("receiver", "receiver<br>"))
plotArea.title().setText(te.toHtml())

fo1 = QFont()
fo1.setPointSizeF(Worksheet.convertToSceneUnits(7, Worksheet.Unit.Point))

for axis in plotArea.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.title().setText("Time [s]")
        axis.setRangeType(Axis.RangeType.Custom)
        axis.setRange(2E-09, 5E-09)
        axis.setScalingFactor(1E+09)
        axis.setLabelsSuffix("ns")
        axis.setMinorTicksAutoNumber(False)
        axis.setMinorTicksNumber(4)
        axis.majorGridLine().setStyle(Qt.PenStyle.SolidLine)
        axis.minorGridLine().setStyle(Qt.PenStyle.DotLine)
        axis.setLabelsFont(fo1)
        axis.setLabelsAutoPrecision(False)
        axis.setLabelsPrecision(1)
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.title().setText("Voltage [V]")
        axis.setRangeType(Axis.RangeType.Custom)
        axis.setRange(-0.0004, 0.0004)
        axis.setScalingFactor(1000)
        axis.setLabelsSuffix("mV")
        axis.setMinorTicksAutoNumber(False)
        axis.setMinorTicksNumber(4)
        axis.majorGridLine().setStyle(Qt.PenStyle.SolidLine)
        axis.minorGridLine().setStyle(Qt.PenStyle.DotLine)
        axis.setLabelsFont(fo1)

worksheet.addChild(plotArea)

config1 = XYCurve("Received pulse (x100)")
plotArea.addChild(config1)
config1.setPlotType(Plot.PlotType.Line)
config1.setXColumn(spreadsheet.column(0))
config1.setYColumn(spreadsheet.column(1))
config1.setLineType(XYCurve.LineType.Line)
config1.symbol().setStyle(Symbol.Style.NoSymbols)
config1.setValuesType(XYCurve.ValuesType.NoValues)

config2 = XYCurve("Response")
plotArea.addChild(config2)
config2.setPlotType(Plot.PlotType.Line)
config2.setXColumn(spreadsheet.column(0))
config2.setYColumn(spreadsheet.column(2))
config2.setLineType(XYCurve.LineType.Line)
config2.symbol().setStyle(Symbol.Style.NoSymbols)
config2.setValuesType(XYCurve.ValuesType.NoValues)

config3 = XYHilbertTransformCurve("Envelope")
plotArea.addChild(config3)
config3.setXDataColumn(spreadsheet.column(0))
config3.setYDataColumn(spreadsheet.column(2))
config3.setLineInterpolationPointsCount(1)
configData = config3.transformData()
configData.type = nsl_hilbert_result_type.nsl_hilbert_result_envelope
config3.setTransformData(configData)

cp = CustomPoint(plotArea, "Gmax")
plotArea.addChild(cp)
cp.setCoordinateSystemIndex(plotArea.defaultCoordinateSystemIndex())
cp.setCoordinateBindingEnabled(True)
cp.setPositionLogical(QPointF(3.355E-09, 0.000312))

rl1 = ReferenceLine(plotArea, "MaxTimeLine")
plotArea.addChild(rl1)
rl1.setOrientation(ReferenceLine.Orientation.Vertical)
rl1.setPositionLogical(QPointF(3.355E-09, 0))
rl1.line().setStyle(Qt.PenStyle.DashLine)
rl1.line().setWidth(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Point))
rl1.line().setOpacity(1)

rl2 = ReferenceLine(plotArea, "MaxAmpLine")
plotArea.addChild(rl2)
rl2.setOrientation(ReferenceLine.Orientation.Horizontal)
rl2.setPositionLogical(QPointF(0, 0.000312493))
rl2.line().setStyle(Qt.PenStyle.DashLine)
rl2.line().setWidth(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Point))
rl2.line().setOpacity(1)

textLabel1 = TextLabel("MaxAmpText")
worksheet.addChild(textLabel1)

fo21 = QFont()
fo21.setPointSize(8)
te1 = QTextEdit()
te1.setFont(fo21)
te1.setTextColor(rl1.line().color())

te1.setPlainText("0,312 mV")
textLabel1.setText(te1.toHtml())
textLabel1.setPosition(QPointF(Worksheet.convertToSceneUnits(4.1, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(3.3, Worksheet.Unit.Centimeter)))

textLabel2 = TextLabel("MaxTimeText")
worksheet.addChild(textLabel2)

fo22 = QFont()
fo22.setPointSize(8)
te2 = QTextEdit()
te2.setFont(fo22)
te2.setTextColor(rl1.line().color())

te2.setPlainText("3,355 ns")
textLabel2.setText(te2.toHtml())
textLabel2.setPosition(QPointF(Worksheet.convertToSceneUnits(1.3, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-3, Worksheet.Unit.Centimeter)))

plotArea.retransform()

###################################################################################################################################################################
###################################################################################################################################################################

Project.retransformElements(project)
