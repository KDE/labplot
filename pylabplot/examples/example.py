import sys
import random

from PySide2.QtWidgets import QApplication, QWidget, QHBoxLayout
from pylabplot import Column, Worksheet, PlotArea, Axis, CartesianPlot, CartesianPlotLegend, BoxPlot, Histogram, XYCurve, XYEquationCurve, XYFitCurve, CartesianCoordinateSystem, Symbol

app = QApplication()

main_widget = QWidget()
layout = QHBoxLayout()

main_widget.setLayout(layout)

worksheet = Worksheet("test")

plot = CartesianPlot("plot")
plot.setType(CartesianPlot.Type.FourAxes)

worksheet.addChild(plot)

count = 10
a = 1.27
b = 4.375
c = -6.692

x = Column("x")
y = Column("y")

for i in range(count):
    x.setValueAt(i, i)
    rand_value = 10 * random.uniform(0, 1)
    value = a*pow(i, 2) + b*i + c + rand_value
    y.setValueAt(i, value)

curve = XYCurve("raw data")
curve.setXColumn(x)
curve.setYColumn(y)
curve.setLineType(XYCurve.LineType.NoLine)
curve.symbol().setStyle(Symbol.Style.Circle)
plot.addChild(curve)
plot.autoScale(CartesianCoordinateSystem.Dimension.X)

fitCurve = XYFitCurve("fit")
fitCurve.recalculate()
plot.addChild(fitCurve)

eqCurve = XYEquationCurve("eq")
plot.addChild(eqCurve)
data = XYEquationCurve.EquationData()
data.expression1 = "50*sin(x)"
data.min = "0.0"
data.max = "10.0"
eqCurve.setEquationData(data)
eqCurve.recalculate()

plot.addLegend()

plot2 = CartesianPlot("plot 2")
plot2.setType(CartesianPlot.Type.FourAxes)
worksheet.addChild(plot2)

hist = Histogram("histogram")
random_data = Column("x")

for i in range(1000):
    random_data.setValueAt(i, 100 * random.uniform(0, 1))

hist.setDataColumn(random_data)
plot2.addChild(hist)

plot3 = CartesianPlot("plot 3")
plot3.setType(CartesianPlot.Type.FourAxes)
worksheet.addChild(plot3)

boxPlot = BoxPlot("boxplot")
boxPlot.setDataColumns([random_data])
plot3.addChild(boxPlot)

layout.addWidget(worksheet.view())

main_widget.show()

sys.exit(app.exec_())