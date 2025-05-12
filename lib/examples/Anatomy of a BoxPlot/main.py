import sys

from PySide6.QtWidgets import QApplication
from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QFont
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

app = QApplication()

project = Project()

filter = AsciiFilter()

p = filter.properties()
p.headerEnabled = False
filter.setProperties(p)

spreadsheet = Spreadsheet("Spreadsheet")
project.addChild(spreadsheet)

filter.readDataFromFile("Anatomy of a BoxPlot/data.txt", spreadsheet, AbstractFileFilter.ImportMode.Replace)

if filter.lastError():
	print(f"Import error: {filter.lastError()}")
	sys.exit(-1)

worksheet = Worksheet("Anatomy of a Box Plot")
project.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(10, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(10, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setLayout(Worksheet.Layout.VerticalLayout)

worksheet.setLayoutTopMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutBottomMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutLeftMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutRightMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

worksheet.setLayoutHorizontalSpacing(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

plotArea = CartesianPlot("Plot")
plotArea.setType(CartesianPlot.Type.FourAxes)

plotArea.setSymmetricPadding(True)
plotArea.setHorizontalPadding(Worksheet.convertToSceneUnits(2.4, Worksheet.Unit.Centimeter))
plotArea.setVerticalPadding(Worksheet.convertToSceneUnits(1.3, Worksheet.Unit.Centimeter))

for axis in plotArea.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)
    axis.setVisible(False)

worksheet.addChild(plotArea)

boxPlot = BoxPlot("BoxPlot")
plotArea.addChild(boxPlot)
boxPlot.setDataColumns([spreadsheet.column(0)])
boxPlot.setOrdering(BoxPlot.Ordering.None_)
boxPlot.setWhiskersType(BoxPlot.WhiskersType.IQR)
boxPlot.setWhiskersRangeParameter(1.5)
boxPlot.setWidthFactor(0.7)
boxPlot.setOrientation(BoxPlot.Orientation.Vertical)
boxPlot.setVariableWidth(False)
boxPlot.setNotchesEnabled(False)
boxPlot.symbolMean().setStyle(Symbol.Style.NoSymbols)
boxPlot.symbolOutlier().setStyle(Symbol.Style.Circle)
boxPlot.symbolOutlier().setSize(Worksheet.convertToSceneUnits(3, Worksheet.Unit.Point))
boxPlot.symbolFarOut().setStyle(Symbol.Style.Circle)
boxPlot.symbolFarOut().setSize(Worksheet.convertToSceneUnits(6, Worksheet.Unit.Point))
boxPlot.setJitteringEnabled(False)
boxPlot.setRugEnabled(False)
boxPlot.setWhiskersCapSize(Worksheet.convertToSceneUnits(40.5, Worksheet.Unit.Point))
boxPlot.recalc()

###################################################################################################################################################################
###################################################################################################################################################################

f = QFont()
f.setPointSize(4)
te = QTextEdit()
te.setFont(f)

textLabel1 = TextLabel("Lower inner fence - label")
worksheet.addChild(textLabel1)
te.setPlainText("Inner fence (not shown)")
textLabel1.setText(te.toHtml())
textLabel1.setPosition(QPointF(Worksheet.convertToSceneUnits(0.1, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-0.7, Worksheet.Unit.Centimeter)))

textLabel2 = TextLabel("Lower outer fence - label")
worksheet.addChild(textLabel2)
te.setPlainText("Outer fence (not shown)")
textLabel2.setText(te.toHtml())
textLabel2.setPosition(QPointF(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-2.1, Worksheet.Unit.Centimeter)))

textLabel3 = TextLabel("Lower outliers - label")
worksheet.addChild(textLabel3)
te.setPlainText("Lower outliers")
textLabel3.setText(te.toHtml())
textLabel3.setPosition(QPointF(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-1, Worksheet.Unit.Centimeter)))

textLabel4 = TextLabel("Lower far outliers - label")
worksheet.addChild(textLabel4)
te.setPlainText("Lower far outliers")
textLabel4.setText(te.toHtml())
textLabel4.setPosition(QPointF(Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-2.5, Worksheet.Unit.Centimeter)))

textLabel5 = TextLabel("Lower adjacent value - label")
worksheet.addChild(textLabel5)
te.setPlainText("Lower adjacent value")
textLabel5.setText(te.toHtml())
textLabel5.setPosition(QPointF(Worksheet.convertToSceneUnits(1.8, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-0.4, Worksheet.Unit.Centimeter)))

textLabel6 = TextLabel("Median")
worksheet.addChild(textLabel6)
te.setPlainText("Median")
textLabel6.setText(te.toHtml())
textLabel6.setPosition(QPointF(Worksheet.convertToSceneUnits(1.2, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(0.6, Worksheet.Unit.Centimeter)))

textLabel7 = TextLabel("3rd Quartile")
worksheet.addChild(textLabel7)
te.setPlainText("3rd Quartile")
textLabel7.setText(te.toHtml())
textLabel7.setPosition(QPointF(Worksheet.convertToSceneUnits(1.4, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(0.9, Worksheet.Unit.Centimeter)))

textLabel8 = TextLabel("1st Quartile")
worksheet.addChild(textLabel8)
te.setPlainText("1st Quartile")
textLabel8.setText(te.toHtml())
textLabel8.setPosition(QPointF(Worksheet.convertToSceneUnits(1.4, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(0.4, Worksheet.Unit.Centimeter)))

textLabel9 = TextLabel("Upper adjacent value - label")
worksheet.addChild(textLabel9)
te.setPlainText("Upper adjacent value")
textLabel9.setText(te.toHtml())
textLabel9.setPosition(QPointF(Worksheet.convertToSceneUnits(1.8, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(1.7, Worksheet.Unit.Centimeter)))

textLabel10 = TextLabel("Upper inner fence - label")
worksheet.addChild(textLabel10)
te.setPlainText("Upper inner fence (not shown)")
textLabel10.setText(te.toHtml())
textLabel10.setPosition(QPointF(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(1.9, Worksheet.Unit.Centimeter)))

textLabel11 = TextLabel("Upper outer fence - label")
worksheet.addChild(textLabel11)
te.setPlainText("Upper outer fence (not shown)")
textLabel11.setText(te.toHtml())
textLabel11.setPosition(QPointF(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(2.8, Worksheet.Unit.Centimeter)))

textLabel12 = TextLabel("Upper far outliers - label")
worksheet.addChild(textLabel12)
te.setPlainText("Upper far outliers")
textLabel12.setText(te.toHtml())
textLabel12.setPosition(QPointF(Worksheet.convertToSceneUnits(1.6, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(3.2, Worksheet.Unit.Centimeter)))

textLabel13 = TextLabel("Upper outliers")
worksheet.addChild(textLabel13)
te.setPlainText("Upper outliers")
textLabel13.setText(te.toHtml())
textLabel13.setPosition(QPointF(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(2.3, Worksheet.Unit.Centimeter)))

textLabel14 = TextLabel("Upper whisker - label")
worksheet.addChild(textLabel14)
te.setPlainText("Upper whisker")
textLabel14.setText(te.toHtml())
textLabel14.setPosition(QPointF(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(1.3, Worksheet.Unit.Centimeter)))

textLabel15 = TextLabel("Lower whisker - label")
worksheet.addChild(textLabel15)
te.setPlainText("Lower whisker")
textLabel15.setText(te.toHtml())
textLabel15.setPosition(QPointF(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter)))

###################################################################################################################################################################
###################################################################################################################################################################

rl1 = ReferenceLine(plotArea, "Lower inner fence")
plotArea.addChild(rl1)
rl1.setOrientation(ReferenceLine.Orientation.Horizontal)
rl1.setPositionLogical(QPointF(0, 0.2))
rl1.line().setStyle(Qt.PenStyle.DashLine)
rl1.line().setWidth(0)
rl1.line().setOpacity(0.8)

rl2 = ReferenceLine(plotArea, "Lower outer fence")
plotArea.addChild(rl2)
rl2.setOrientation(ReferenceLine.Orientation.Horizontal)
rl2.setPositionLogical(QPointF(0, -10))
rl2.line().setStyle(Qt.PenStyle.DashLine)
rl2.line().setWidth(0)
rl2.line().setOpacity(0.8)

rl3 = ReferenceLine(plotArea, "Upper inner fence")
plotArea.addChild(rl3)
rl3.setOrientation(ReferenceLine.Orientation.Horizontal)
rl3.setPositionLogical(QPointF(0, 19))
rl3.line().setStyle(Qt.PenStyle.DashLine)
rl3.line().setWidth(0)
rl3.line().setOpacity(0.8)

rl4 = ReferenceLine(plotArea, "Upper outer fence")
plotArea.addChild(rl4)
rl4.setOrientation(ReferenceLine.Orientation.Horizontal)
rl4.setPositionLogical(QPointF(0, 25.5))
rl4.line().setStyle(Qt.PenStyle.DashLine)
rl4.line().setWidth(0)
rl4.line().setOpacity(0.8)

###################################################################################################################################################################
###################################################################################################################################################################

# plotArea.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)
# rangeX = plotArea.range(CartesianCoordinateSystem.Dimension.X, 0)
# rangeX.setRange(0.5, 1.5)
# plotArea.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX)

# plotArea.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
# rangeY = plotArea.range(CartesianCoordinateSystem.Dimension.Y, 0)
# rangeY.setRange(-20, 30)
# plotArea.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY)

###################################################################################################################################################################
###################################################################################################################################################################


worksheet.view().show()

app.exec()
