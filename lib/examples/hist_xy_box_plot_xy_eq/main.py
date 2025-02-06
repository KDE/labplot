import sys
import random

from PySide6.QtWidgets import QApplication, QWidget, QHBoxLayout

from pylabplot import Column, Worksheet, CartesianPlot, BoxPlot, Histogram, XYCurve, XYEquationCurve, Symbol

def main():
    app = QApplication()

    mainWidget = QWidget()
    layout = QHBoxLayout()

    mainWidget.setLayout(layout)

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
        randValue = 10 * random.uniform(0, 1)
        value = a * pow(i, 2) + b * i + c + randValue
        y.setValueAt(i, value)

    curve = XYCurve("raw data")
    curve.setXColumn(x)
    curve.setYColumn(y)
    curve.setLineType(XYCurve.LineType.NoLine)
    curve.symbol().setStyle(Symbol.Style.Circle)
    plot.addChild(curve)

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
    randomData = Column("x")

    for i in range(1000):
        randomData.setValueAt(i, 100 * random.uniform(0, 1))

    hist.setDataColumn(randomData)
    plot2.addChild(hist)

    plot3 = CartesianPlot("plot 3")
    plot3.setType(CartesianPlot.Type.FourAxes)
    worksheet.addChild(plot3)

    boxPlot = BoxPlot("boxplot")
    columns = [randomData]
    boxPlot.setDataColumns(columns)
    plot3.addChild(boxPlot)

    layout.addWidget(worksheet.view())
    mainWidget.show()

    app.exec()

main()
