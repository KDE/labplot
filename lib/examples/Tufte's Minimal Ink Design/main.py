import sys

from PySide6.QtWidgets import QApplication
from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QFont, QColor
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

app = QApplication()

project = Project()

filter = AsciiFilter()

p = filter.properties()
p.headerEnabled = False
filter.setProperties(p)

spreadsheet = Spreadsheet("Data")
project.addChild(spreadsheet)

filter.readDataFromFile("Tufte's Minimal Ink Design/data.txt", spreadsheet, AbstractFileFilter.ImportMode.Replace)

if filter.lastError():
	print(f"Import error: {filter.lastError()}")
	sys.exit(-1)

worksheet = Worksheet("Standard vs. Tufte's Minimal Ink Design")
project.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(15, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setLayout(Worksheet.Layout.GridLayout)
worksheet.setLayoutRowCount(2)
worksheet.setLayoutColumnCount(3)

worksheet.setLayoutTopMargin(Worksheet.convertToSceneUnits(1.2, Worksheet.Unit.Centimeter))
worksheet.setLayoutBottomMargin(Worksheet.convertToSceneUnits(0.3, Worksheet.Unit.Centimeter))
worksheet.setLayoutLeftMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutRightMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

worksheet.setLayoutHorizontalSpacing(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Centimeter))
worksheet.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(1.6, Worksheet.Unit.Centimeter))

###################################################################################################################################################################
###################################################################################################################################################################

plotArea1 = CartesianPlot("Scatterplot - Standard")
plotArea1.setType(CartesianPlot.Type.FourAxes)

plotArea1.setSymmetricPadding(True)
plotArea1.setHorizontalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))
plotArea1.setVerticalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))

for axis in plotArea1.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)

worksheet.addChild(plotArea1)

config1 = XYCurve("config1")
plotArea1.addChild(config1)
config1.setPlotType(Plot.PlotType.Scatter)
config1.setXColumn(spreadsheet.column(0))
config1.setYColumn(spreadsheet.column(1))
config1.symbol().setStyle(Symbol.Style.Circle)
config1.symbol().setSize(Worksheet.convertToSceneUnits(4, Worksheet.Unit.Point))
config1.setValuesType(XYCurve.ValuesType.NoValues)

# plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)
# rangeX1 = plotArea1.range(CartesianCoordinateSystem.Dimension.X, 0)
# rangeX1.setRange(0.5, 5.5)
# plotArea1.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX1)

# plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
# rangeY1 = plotArea1.range(CartesianCoordinateSystem.Dimension.Y, 0)
# rangeY1.setRange(-3, 3)
# plotArea1.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY1)

###################################################################################################################################################################
###################################################################################################################################################################

plotArea2 = CartesianPlot("Histograms - Standard")
plotArea2.setType(CartesianPlot.Type.FourAxes)

plotArea2.setSymmetricPadding(True)
plotArea2.setHorizontalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))
plotArea2.setVerticalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))

for axis in plotArea2.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)

worksheet.addChild(plotArea2)

config21 = Histogram("config21")
plotArea2.addChild(config21)
config21.setDataColumn(spreadsheet.column(0))
config21.setType(Histogram.Type.Ordinary)
config21.setOrientation(Histogram.Orientation.Vertical)
config21.setNormalization(Histogram.Normalization.Count)
config21.setBinningMethod(Histogram.BinningMethod.SquareRoot)
config21.setAutoBinRanges(True)
config21.line().setHistogramLineType(Histogram.LineType.Bars)
config21.symbol().setStyle(Symbol.Style.NoSymbols)
config21.value().setType(Value.Type.NoValues)
config21.background().setType(Background.Type.Color)
config21.background().setColorStyle(Background.ColorStyle.SingleColor)
config21.background().setFirstColor(config21.color())
config21.background().setOpacity(0.4)
config21.background().setEnabled(True)

config22 = Histogram("config22")
plotArea2.addChild(config22)
config22.setDataColumn(spreadsheet.column(1))
config22.setType(Histogram.Type.Ordinary)
config22.setOrientation(Histogram.Orientation.Vertical)
config22.setNormalization(Histogram.Normalization.Count)
config22.setBinningMethod(Histogram.BinningMethod.SquareRoot)
config22.setAutoBinRanges(True)
config22.line().setHistogramLineType(Histogram.LineType.Bars)
config22.symbol().setStyle(Symbol.Style.NoSymbols)
config22.value().setType(Value.Type.NoValues)
config22.background().setType(Background.Type.Color)
config22.background().setColorStyle(Background.ColorStyle.SingleColor)
config22.background().setFirstColor(config22.color())
config22.background().setOpacity(0.4)
config22.background().setEnabled(True)

###################################################################################################################################################################
###################################################################################################################################################################

plotArea3 = CartesianPlot("Boxplots - Standard")
plotArea3.setType(CartesianPlot.Type.FourAxes)

plotArea3.setSymmetricPadding(True)
plotArea3.setHorizontalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))
plotArea3.setVerticalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))

for axis in plotArea3.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)

worksheet.addChild(plotArea3)

config31 = BoxPlot("config31")
plotArea3.addChild(config31)
config31.setDataColumns([spreadsheet.column(0), spreadsheet.column(1)])
config31.setOrdering(BoxPlot.Ordering.None_)
config31.setWhiskersType(BoxPlot.WhiskersType.IQR)
config31.setWhiskersRangeParameter(1.5)
config31.setOrientation(BoxPlot.Orientation.Vertical)
config31.setVariableWidth(False)
config31.setNotchesEnabled(False)
config31.symbolMean().setStyle(Symbol.Style.Square)
config31.setJitteringEnabled(True)

###################################################################################################################################################################
###################################################################################################################################################################

plotArea4 = CartesianPlot("Scatterplot - Tufte")
plotArea4.setType(CartesianPlot.Type.FourAxes)

plotArea4.setSymmetricPadding(True)
plotArea4.setHorizontalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))
plotArea4.setVerticalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))

for axis in plotArea4.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)

worksheet.addChild(plotArea4)

config4 = XYCurve("config4")
plotArea4.addChild(config4)
config4.setPlotType(Plot.PlotType.Scatter)
config4.setXColumn(spreadsheet.column(0))
config4.setYColumn(spreadsheet.column(1))
config4.symbol().setStyle(Symbol.Style.Circle)
config4.symbol().setSize(Worksheet.convertToSceneUnits(4, Worksheet.Unit.Point))
config4.symbol().setColor("black")
config4.setValuesType(XYCurve.ValuesType.NoValues)

# plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)
# rangeX4 = plotArea1.range(CartesianCoordinateSystem.Dimension.X, 0)
# rangeX4.setRange(0.5, 5.5)
# plotArea1.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX4)

# plotArea1.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
# rangeY4 = plotArea1.range(CartesianCoordinateSystem.Dimension.Y, 0)
# rangeY4.setRange(-3, 3)
# plotArea1.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY4)

###################################################################################################################################################################
###################################################################################################################################################################

plotArea5 = CartesianPlot("Histograms - Tufte")
plotArea5.setType(CartesianPlot.Type.FourAxes)

plotArea5.setSymmetricPadding(True)
plotArea5.setHorizontalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))
plotArea5.setVerticalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))

for axis in plotArea5.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)

worksheet.addChild(plotArea5)

config51 = Histogram("config51")
plotArea5.addChild(config51)
config51.setDataColumn(spreadsheet.column(0))
config51.setType(Histogram.Type.Ordinary)
config51.setOrientation(Histogram.Orientation.Vertical)
config51.setNormalization(Histogram.Normalization.Count)
config51.setBinningMethod(Histogram.BinningMethod.SquareRoot)
config51.setAutoBinRanges(True)
config51.line().setHistogramLineType(Histogram.LineType.HalfBars)
config51.line().setColor("black")
config51.symbol().setStyle(Symbol.Style.NoSymbols)
config51.value().setType(Value.Type.NoValues)
config51.background().setEnabled(False)

config52 = Histogram("config52")
plotArea5.addChild(config52)
config52.setDataColumn(spreadsheet.column(1))
config52.setType(Histogram.Type.Ordinary)
config52.setOrientation(Histogram.Orientation.Vertical)
config52.setNormalization(Histogram.Normalization.Count)
config52.setBinningMethod(Histogram.BinningMethod.SquareRoot)
config52.setAutoBinRanges(True)
config52.line().setHistogramLineType(Histogram.LineType.HalfBars)
config52.line().setColor("black")
config52.symbol().setStyle(Symbol.Style.NoSymbols)
config52.value().setType(Value.Type.NoValues)
config52.background().setEnabled(False)

###################################################################################################################################################################
###################################################################################################################################################################

plotArea6 = CartesianPlot("Boxplots - Tufte")
plotArea6.setType(CartesianPlot.Type.FourAxes)

plotArea6.setSymmetricPadding(True)
plotArea6.setHorizontalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))
plotArea6.setVerticalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))

for axis in plotArea6.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)

worksheet.addChild(plotArea6)

config61 = BoxPlot("config61")
plotArea6.addChild(config61)
config61.setDataColumns([spreadsheet.column(0), spreadsheet.column(1)])
config61.setOrdering(BoxPlot.Ordering.None_)
config61.setWhiskersType(BoxPlot.WhiskersType.IQR)
config61.setWhiskersRangeParameter(1.5)
config61.setOrientation(BoxPlot.Orientation.Vertical)
config61.setVariableWidth(False)
config61.setNotchesEnabled(False)
config61.symbolMean().setStyle(Symbol.Style.Circle)
config61.symbolMean().setSize(Worksheet.convertToSceneUnits(4, Worksheet.Unit.Point))
config61.symbolMean().setColor("black")
config61.setWhiskersCapSize(0)
config61.whiskersCapLine().setColor("black")
config61.setJitteringEnabled(True)

for i in range(spreadsheet.columnCount()):
    config61.medianLineAt(i).setStyle(Qt.PenStyle.NoPen)
    config61.borderLineAt(i).setStyle(Qt.PenStyle.NoPen)
    config61.backgroundAt(i).setEnabled(False)

###################################################################################################################################################################
###################################################################################################################################################################

textLabel1 = TextLabel("Tufte's Minimal Ink Design")
textLabel1.setText("Tufte's Minimal Ink Design")
worksheet.addChild(textLabel1)
textLabel1.setPosition(QPointF(Worksheet.convertToSceneUnits(0.1, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-0.7, Worksheet.Unit.Centimeter)))

textLabel2 = TextLabel("Standard Design")
textLabel2.setText("Standard Design")
worksheet.addChild(textLabel2)
textLabel2.setPosition(QPointF(Worksheet.convertToSceneUnits(0.1, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(6.8, Worksheet.Unit.Centimeter)))

###################################################################################################################################################################
###################################################################################################################################################################

worksheet.view().show()

app.exec()