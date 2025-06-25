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

filter.readDataFromFile("Basic Plots/data.txt", spreadsheet, AbstractFileFilter.ImportMode.Replace)

if filter.lastError():
	print(f"Import error: {filter.lastError()}")
	sys.exit(-1)

worksheet = Worksheet("Worksheet - Spreadsheet")
project.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setLayout(Worksheet.Layout.GridLayout)
worksheet.setLayoutRowCount(2)
worksheet.setLayoutColumnCount(2)

worksheet.setLayoutTopMargin(Worksheet.convertToSceneUnits(0.2, Worksheet.Unit.Centimeter))
worksheet.setLayoutBottomMargin(Worksheet.convertToSceneUnits(0.2, Worksheet.Unit.Centimeter))
worksheet.setLayoutLeftMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutRightMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

worksheet.setLayoutHorizontalSpacing(Worksheet.convertToSceneUnits(0.1, Worksheet.Unit.Centimeter))
worksheet.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(0.1, Worksheet.Unit.Centimeter))

plotArea1 = CartesianPlot("Plot - Spreadsheet")
plotArea1.setType(CartesianPlot.Type.FourAxes)

plotArea1.setSymmetricPadding(False)
plotArea1.setHorizontalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea1.setVerticalPadding(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
plotArea1.setRightPadding(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
plotArea1.setBottomPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))

for axis in plotArea1.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.title().setText("Frequency")
    else:
        axis.title().setText("")
        axis.title().setVisible(False)

worksheet.addChild(plotArea1)

config11 = Histogram("Lognormal(1,1)")
plotArea1.addChild(config11)
config11.setDataColumn(spreadsheet.column(1))
config11.setType(Histogram.Type.Ordinary)
config11.setOrientation(Histogram.Orientation.Vertical)
config11.setNormalization(Histogram.Normalization.Probability)
config11.setBinningMethod(Histogram.BinningMethod.SquareRoot)
config11.setAutoBinRanges(True)
config11.line().setHistogramLineType(Histogram.LineType.Bars)
config11.symbol().setStyle(Symbol.Style.NoSymbols)
config11.value().setType(Value.Type.NoValues)
config11.background().setType(Background.Type.Color)
config11.background().setColorStyle(Background.ColorStyle.SingleColor)
config11.background().setFirstColor(config11.color())
config11.background().setOpacity(0.5)
config11.background().setEnabled(True)
config11.setRugEnabled(True)
config11.setRugLength(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
config11.setRugWidth(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Point))
config11.setRugOffset(Worksheet.convertToSceneUnits(-5, Worksheet.Unit.Point))

fit11 = XYFitCurve("Distribution Fit to 'Lognormal(1,1)'")
plotArea1.addChild(fit11)

fit11.setDataSourceType(XYFitCurve.DataSourceType.Histogram)
fit11.setDataSourceHistogram(config11)

fitData11 = fit11.fitData()
fitData11.modelCategory = nsl_fit_model_category.nsl_fit_model_distribution
fitData11.modelType = nsl_sf_stats_distribution.nsl_sf_stats_gaussian

fitData11 = XYFitCurve.initFitData(fitData11)
fitData11 = fit11.initStartValues(fitData11)
fit11.setFitData(fitData11)
fit11.recalculate()

config12 = Histogram("Normal(0,1)")
plotArea1.addChild(config12)
config12.setDataColumn(spreadsheet.column(2))
config12.setType(Histogram.Type.Ordinary)
config12.setOrientation(Histogram.Orientation.Vertical)
config12.setNormalization(Histogram.Normalization.ProbabilityDensity)
config12.setBinningMethod(Histogram.BinningMethod.SquareRoot)
config12.setAutoBinRanges(True)
config12.line().setHistogramLineType(Histogram.LineType.Bars)
config12.symbol().setStyle(Symbol.Style.NoSymbols)
config12.value().setType(Value.Type.NoValues)
config12.background().setType(Background.Type.Color)
config12.background().setColorStyle(Background.ColorStyle.SingleColor)
config12.background().setFirstColor(config12.color())
config12.background().setOpacity(0.5)
config12.background().setEnabled(True)
config12.setRugEnabled(True)
config12.setRugLength(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
config12.setRugWidth(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Point))
config12.setRugOffset(Worksheet.convertToSceneUnits(-5, Worksheet.Unit.Point))

fit12 = XYFitCurve("Distribution Fit to 'Normal(0,1)'")
plotArea1.addChild(fit12)

fit12.setDataSourceType(XYFitCurve.DataSourceType.Histogram)
fit12.setDataSourceHistogram(config12)

fitData12 = fit12.fitData()
fitData12.modelCategory = nsl_fit_model_category.nsl_fit_model_distribution
fitData12.modelType = nsl_sf_stats_distribution.nsl_sf_stats_gaussian

fitData12 = XYFitCurve.initFitData(fitData12)
fitData12 = fit12.initStartValues(fitData12)
fit12.setFitData(fitData12)
fit12.recalculate()

###################################################################################################################################################################
###################################################################################################################################################################

plotArea2 = CartesianPlot("Plot - Spreadsheet 1")
plotArea2.setType(CartesianPlot.Type.FourAxes)

plotArea2.setSymmetricPadding(False)
plotArea2.setHorizontalPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))
plotArea2.setVerticalPadding(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
plotArea2.setRightPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))
plotArea2.setBottomPadding(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Centimeter))

for axis in plotArea2.children(AspectType.Axis):
    axis.title().setText("")
    axis.title().setVisible(False)

worksheet.addChild(plotArea2)

config2 = BoxPlot("config2")
plotArea2.addChild(config2)
config2.setDataColumns([spreadsheet.column(1), spreadsheet.column(2)])
config2.setOrdering(BoxPlot.Ordering.None_)
config2.setWhiskersType(BoxPlot.WhiskersType.IQR)
config2.setWhiskersRangeParameter(1.5)
config2.setOrientation(BoxPlot.Orientation.Horizontal)
config2.setVariableWidth(False)
config2.setNotchesEnabled(True)
config2.symbolMean().setStyle(Symbol.Style.Square)
config2.symbolOutlier().setStyle(Symbol.Style.Circle)
config2.setJitteringEnabled(True)
config2.setRugEnabled(True)
config2.setRugLength(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
config2.setRugWidth(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Point))
config2.setRugOffset(Worksheet.convertToSceneUnits(-5, Worksheet.Unit.Point))
config2.setWhiskersCapSize(Worksheet.convertToSceneUnits(20, Worksheet.Unit.Point))
config2.recalc()

###################################################################################################################################################################
###################################################################################################################################################################

plotArea3 = CartesianPlot("Plot - Spreadsheet 2")
plotArea3.setType(CartesianPlot.Type.FourAxes)

plotArea3.setSymmetricPadding(False)
plotArea3.setHorizontalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea3.setVerticalPadding(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
plotArea3.setRightPadding(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
plotArea3.setBottomPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))

for axis in plotArea1.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.title().setText("Time")
    else:
        axis.title().setText("")
        axis.title().setVisible(False)

worksheet.addChild(plotArea3)

config31 = XYCurve("Normal(-1,2)")
plotArea3.addChild(config31)
config31.setPlotType(Plot.PlotType.Scatter)
config31.setXColumn(spreadsheet.column(0))
config31.setYColumn(spreadsheet.column(1))
config31.setLineType(XYCurve.LineType.NoLine)
config31.symbol().setStyle(Symbol.Style.Circle)
config31.symbol().setSize(Worksheet.convertToSceneUnits(6, Worksheet.Unit.Point))
config31.symbol().setOpacity(0.7)
config31.setValuesType(XYCurve.ValuesType.NoValues)
config31.background().setPosition(Background.Position.No)
config31.setRugEnabled(True)
config31.setRugLength(Worksheet.convertToSceneUnits(15, Worksheet.Unit.Point))
config31.setRugWidth(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Point))
config31.setRugOffset(Worksheet.convertToSceneUnits(-30, Worksheet.Unit.Point))
config31.setRugOrientation(WorksheetElement.Orientation.Vertical)
config31.recalc()

config32 = XYCurve("Normal(0,1)")
plotArea3.addChild(config32)
config32.setPlotType(Plot.PlotType.Scatter)
config32.setXColumn(spreadsheet.column(0))
config32.setYColumn(spreadsheet.column(2))
config32.setLineType(XYCurve.LineType.NoLine)
config32.symbol().setStyle(Symbol.Style.Circle)
config32.symbol().setSize(Worksheet.convertToSceneUnits(6, Worksheet.Unit.Point))
config32.symbol().setOpacity(0.7)
config32.setValuesType(XYCurve.ValuesType.NoValues)
config32.background().setPosition(Background.Position.No)
config32.setRugEnabled(True)
config32.setRugLength(Worksheet.convertToSceneUnits(15, Worksheet.Unit.Point))
config32.setRugWidth(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Point))
config32.setRugOffset(Worksheet.convertToSceneUnits(-30, Worksheet.Unit.Point))
config32.setRugOrientation(WorksheetElement.Orientation.Vertical)
config32.recalc()

###################################################################################################################################################################
###################################################################################################################################################################

worksheet.view().show()

app.exec()
