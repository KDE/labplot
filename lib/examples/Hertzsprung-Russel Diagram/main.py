import sys

from PySide6.QtWidgets import QApplication
from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QFont
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

app = QApplication()

project = Project()

spreadsheet = Spreadsheet("HYG data")
project.addChild(spreadsheet)

filter = AsciiFilter()

p = filter.properties()
p.headerEnabled = False
filter.setProperties(p)

filter.readDataFromFile("Hertzsprung-Russel Diagram/HYG_data.txt", spreadsheet, AbstractFileFilter.ImportMode.Replace)

if filter.lastError():
	print(f"Import error: {filter.lastError()}")
	sys.exit(-1)

spreadsheet2 = Spreadsheet("temp to ci mapping")
project.addChild(spreadsheet2)

filter2 = AsciiFilter()

p2 = filter2.properties()
p2.headerEnabled = False
filter2.setProperties(p2)

filter2.readDataFromFile("Hertzsprung-Russel Diagram/temp_to_ci_mapping.txt", spreadsheet2, AbstractFileFilter.ImportMode.Replace)

if filter2.lastError():
	print(f"Import error: {filter2.lastError()}")
	sys.exit(-1)

worksheet = Worksheet("Worksheet")
project.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(25, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setLayout(Worksheet.Layout.VerticalLayout)
worksheet.setLayoutTopMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutBottomMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutLeftMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutRightMargin(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

worksheet.setLayoutHorizontalSpacing(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))
worksheet.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter))

plotArea = CartesianPlot("xy-plot")
plotArea.setType(CartesianPlot.Type.FourAxes)
plotArea.title().setText("Hertzsprung-Russell diagram")
border = plotArea.borderType()
border = CartesianPlot.BorderTypeFlags.BorderLeft | CartesianPlot.BorderTypeFlags.BorderTop | CartesianPlot.BorderTypeFlags.BorderRight | CartesianPlot.BorderTypeFlags.BorderBottom
plotArea.setBorderType(border)
plotArea.setSymmetricPadding(False)
plotArea.setHorizontalPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea.setVerticalPadding(Worksheet.convertToSceneUnits(2.8, Worksheet.Unit.Centimeter))
plotArea.setRightPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))
plotArea.setBottomPadding(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Centimeter))

worksheet.addChild(plotArea)

for axis in plotArea.children(AspectType.Axis):
    if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
        axis.title().setText("Color Index (B-V)")
    elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
        axis.title().setText("Absolute Magnitude")
    elif axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Top:
        axis.title().setText("Temperature [K]")
        axis.setTitleOffsetX(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
        axis.setTitleOffsetY(Worksheet.convertToSceneUnits(43, Worksheet.Unit.Point))
        axis.setLabelsOffset(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
        axis.setLabelsPosition(Axis.LabelsPosition.In)

config1 = XYCurve("config1")
plotArea.addChild(config1)
config1.setPlotType(Plot.PlotType.Scatter)
config1.setXColumn(spreadsheet.column(1))
config1.setYColumn(spreadsheet.column(0))
config1.symbol().setStyle(Symbol.Style.Circle)
config1.symbol().setSize(Worksheet.convertToSceneUnits(1, Worksheet.Unit.Point))
config1.setValuesType(XYCurve.ValuesType.NoValues)

plotArea.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)
rangeX = plotArea.range(CartesianCoordinateSystem.Dimension.X, 0)
rangeX.setRange(-0.5, 2.5)
plotArea.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX)

plotArea.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
rangeY = plotArea.range(CartesianCoordinateSystem.Dimension.Y, 0)
rangeY.setRange(20, -20)
plotArea.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY)

textLabel = TextLabel("White Dwarfs")
textLabel.setText("White Dwarfs")
worksheet.addChild(textLabel)
textLabel.setPosition(QPointF(Worksheet.convertToSceneUnits(-1.8, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-8.7, Worksheet.Unit.Centimeter)))

textLabel1 = TextLabel("Main Sequence")
textLabel1.setText("Main Sequence")
worksheet.addChild(textLabel1)
textLabel1.setPosition(QPointF(Worksheet.convertToSceneUnits(6.3, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(-6.8, Worksheet.Unit.Centimeter)))

textLabel2 = TextLabel("Giants")
textLabel2.setText("Giants")
worksheet.addChild(textLabel2)
textLabel2.setPosition(QPointF(Worksheet.convertToSceneUnits(5.4, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(2.1, Worksheet.Unit.Centimeter)))

textLabel3 = TextLabel("Super Giants")
textLabel3.setText("Super Giants")
worksheet.addChild(textLabel3)
textLabel3.setPosition(QPointF(Worksheet.convertToSceneUnits(5.5, Worksheet.Unit.Centimeter), Worksheet.convertToSceneUnits(7.5, Worksheet.Unit.Centimeter)))

worksheet.setTheme("Monokai")

worksheet.view().show()

app.exec()
