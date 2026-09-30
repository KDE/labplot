"""
Publication-Ready Multi-Panel Figure Generator

This script demonstrates how to create a publication-quality figure with
multiple panels (A, B, C, D) suitable for scientific journals.

Features:
- Automatic panel layout (2×2 grid)
- Consistent styling across all panels
- Panel labels (a), (b), (c), (d) in proper position
- High-DPI export (300 DPI) for publication
- Professional appearance following journal standards

Use case: Creating "Figure 1" for your paper with multiple related plots
"""

import numpy as np
from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QFont
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

# === Generate Sample Data ===
# In real use, you would import your actual data files
np.random.seed(42)

# Panel A: Time series data
t = np.linspace(0, 10, 100)
signal_a = np.sin(2 * np.pi * 0.5 * t) * np.exp(-t / 10) + 0.1 * np.random.randn(100)

# Panel B: Dose-response curve
dose = np.logspace(-2, 2, 20)
response = 100 / (1 + (dose / 1.0)**(-2)) + 5 * np.random.randn(20)

# Panel C: Grouped bar chart data
groups = ['Control', 'Treatment A', 'Treatment B', 'Treatment C']
values_c = [45, 67, 58, 72]
errors_c = [5, 7, 6, 8]

# Panel D: Scatter plot with correlation
x_d = np.random.randn(50) * 2 + 5
y_d = 1.5 * x_d + 3 + np.random.randn(50) * 2

# === Setup Project ===
proj = project()

# Create spreadsheets for each panel
spreadsheet_a = Spreadsheet("Panel A Data")
spreadsheet_a.setColumnCount(2)  # time, signal
spreadsheet_b = Spreadsheet("Panel B Data")
spreadsheet_b.setColumnCount(2)  # dose, response
spreadsheet_c = Spreadsheet("Panel C Data")
spreadsheet_c.setColumnCount(3)  # group, value, error
spreadsheet_d = Spreadsheet("Panel D Data")
spreadsheet_d.setColumnCount(2)  # x, y

proj.addChild(spreadsheet_a)
proj.addChild(spreadsheet_b)
proj.addChild(spreadsheet_c)
proj.addChild(spreadsheet_d)

# === Fill Data ===
# Panel A
spreadsheet_a.column(0).setName("Time (s)")
spreadsheet_a.column(1).setName("Signal (mV)")
spreadsheet_a.column(0).replaceValues(0, [float(x) for x in t])
spreadsheet_a.column(1).replaceValues(0, [float(x) for x in signal_a])

# Panel B
spreadsheet_b.column(0).setName("Dose (μM)")
spreadsheet_b.column(1).setName("Response (%)")
spreadsheet_b.column(0).replaceValues(0, [float(x) for x in dose])
spreadsheet_b.column(1).replaceValues(0, [float(x) for x in response])

# Panel C
spreadsheet_c.column(0).setName("Group")
spreadsheet_c.column(1).setName("Value")
spreadsheet_c.column(2).setName("Error")
spreadsheet_c.column(0).replaceValues(0, [float(i+1) for i in range(len(groups))])
spreadsheet_c.column(1).replaceValues(0, [float(x) for x in values_c])
spreadsheet_c.column(2).replaceValues(0, [float(x) for x in errors_c])

# Panel D
spreadsheet_d.column(0).setName("X Variable")
spreadsheet_d.column(1).setName("Y Variable")
spreadsheet_d.column(0).replaceValues(0, [float(x) for x in x_d])
spreadsheet_d.column(1).replaceValues(0, [float(x) for x in y_d])

# === Create Worksheet ===
worksheet = Worksheet("Figure 1")
proj.addChild(worksheet)

# Set size appropriate for publication (single column = 8.5 cm, double column = 17.4 cm)
# Using double column width
worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(17, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(17, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

# Use publication-appropriate theme (clean, high contrast)
worksheet.setTheme("Tufte")

# Setup 2×2 grid layout
worksheet.setLayout(Worksheet.Layout.GridLayout)
worksheet.setLayoutRowCount(2)
worksheet.setLayoutColumnCount(2)

# Tight margins for publication (journals prefer compact figures)
margin = Worksheet.convertToSceneUnits(0.3, Worksheet.Unit.Centimeter)
worksheet.setLayoutTopMargin(margin)
worksheet.setLayoutBottomMargin(margin)
worksheet.setLayoutLeftMargin(margin)
worksheet.setLayoutRightMargin(margin)
worksheet.setLayoutHorizontalSpacing(Worksheet.convertToSceneUnits(0.8, Worksheet.Unit.Centimeter))
worksheet.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(0.8, Worksheet.Unit.Centimeter))

# === Panel A: Time Series ===
plot_a = CartesianPlot("Panel A")
plot_a.setType(CartesianPlot.Type.FourAxes)
plot_a.setNiceExtend(True)

worksheet.addChild(plot_a)

# Configure axes
x_axis = plot_a.horizontalAxis()
x_axis.title().setText("Time (s)")
x_axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
x_axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

y_axis = plot_a.verticalAxis()
y_axis.title().setText("Signal (mV)")
y_axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
y_axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

curve_a = XYCurve("Time Series")
curve_a.setXColumn(spreadsheet_a.column(0))
curve_a.setYColumn(spreadsheet_a.column(1))
curve_a.setLineType(XYCurve.LineType.Line)
curve_a.symbol().setStyle(Symbol.Style.Circle)
curve_a.symbol().setSize(Worksheet.convertToSceneUnits(3, Worksheet.Unit.Point))
plot_a.addChild(curve_a)

# === Panel B: Dose-Response (log scale) ===
plot_b = CartesianPlot("Panel B")
plot_b.setType(CartesianPlot.Type.FourAxes)
plot_b.setNiceExtend(True)

worksheet.addChild(plot_b)

# Configure axis titles
x_axis = plot_b.horizontalAxis()
x_axis.title().setText("Dose (μM)")
x_axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
x_axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

y_axis = plot_b.verticalAxis()
y_axis.title().setText("Response (%)")
y_axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
y_axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

# Set X axis to logarithmic scale (via Range object)
plot_b.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)
rangeX_b = plot_b.range(CartesianCoordinateSystem.Dimension.X, 0)
rangeX_b.setScale(RangeT.Scale.Log10)
rangeX_b.setRange(0.01, 100)  # Set appropriate range for log scale
plot_b.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX_b)

curve_b = XYCurve("Dose Response")
curve_b.setXColumn(spreadsheet_b.column(0))
curve_b.setYColumn(spreadsheet_b.column(1))
curve_b.setLineType(XYCurve.LineType.NoLine)
curve_b.symbol().setStyle(Symbol.Style.Square)
curve_b.symbol().setSize(Worksheet.convertToSceneUnits(5, Worksheet.Unit.Point))
plot_b.addChild(curve_b)

# Add sigmoid fit
fit_b = XYFitCurve("Sigmoid Fit")
fit_b.setXDataColumn(spreadsheet_b.column(0))
fit_b.setYDataColumn(spreadsheet_b.column(1))

fitData_b = fit_b.fitData()
fitData_b.modelCategory = nsl_fit_model_category.nsl_fit_model_growth
fitData_b.modelType = nsl_fit_model_type_growth.nsl_fit_model_sigmoid
fitData_b = XYFitCurve.initFitData(fitData_b)
fitData_b = fit_b.initStartValues(fitData_b)
fit_b.setFitData(fitData_b)
fit_b.recalculate()
plot_b.addChild(fit_b)

# === Panel C: Grouped Bar Chart with Error Bars ===
plot_c = CartesianPlot("Panel C")
plot_c.setType(CartesianPlot.Type.FourAxes)
plot_c.setNiceExtend(True)

worksheet.addChild(plot_c)

# Configure axes
x_axis = plot_c.horizontalAxis()
x_axis.title().setText("Treatment Group")
x_axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
x_axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)
# Set custom labels
x_axis.setMajorTicksType(Axis.TicksType.CustomColumn)
x_axis.setMajorTicksColumn(spreadsheet_c.column(0))

y_axis = plot_c.verticalAxis()
y_axis.title().setText("Effect Size (%)")
y_axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
y_axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

bar_c = BarPlot("Treatment Effects")
bar_c.setDataColumns([spreadsheet_c.column(1)])
bar_c.setOrientation(BarPlot.Orientation.Vertical)
bar_c.setType(BarPlot.Type.Grouped)

plot_c.addChild(bar_c)

# Add error bars via ErrorBar object
error_bar = bar_c.errorBarAt(0)  # Get error bar for first (only) data series
error_bar.setYErrorType(ErrorBar.ErrorType.Symmetric)
error_bar.setYPlusColumn(spreadsheet_c.column(2))

# === Panel D: Scatter with Linear Regression ===
plot_d = CartesianPlot("Panel D")
plot_d.setType(CartesianPlot.Type.FourAxes)
plot_d.setNiceExtend(True)

worksheet.addChild(plot_d)

x_axis = plot_d.horizontalAxis()
x_axis.title().setText("X Variable (a.u.)")
x_axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
x_axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

y_axis = plot_d.verticalAxis()
y_axis.title().setText("Y Variable (a.u.)")
y_axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
y_axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

scatter_d = XYCurve("Data Points")
scatter_d.setXColumn(spreadsheet_d.column(0))
scatter_d.setYColumn(spreadsheet_d.column(1))
scatter_d.setLineType(XYCurve.LineType.NoLine)
scatter_d.symbol().setStyle(Symbol.Style.Circle)
scatter_d.symbol().setSize(Worksheet.convertToSceneUnits(4, Worksheet.Unit.Point))
scatter_d.symbol().setOpacity(0.6)
plot_d.addChild(scatter_d)

# Linear regression
fit_d = XYFitCurve("Linear Fit")
fit_d.setXDataColumn(spreadsheet_d.column(0))
fit_d.setYDataColumn(spreadsheet_d.column(1))

fitData_d = fit_d.fitData()
fitData_d.modelCategory = nsl_fit_model_category.nsl_fit_model_basic
fitData_d.modelType = nsl_fit_model_type_basic.nsl_fit_model_polynomial
fitData_d.degree = 1  # Linear
fitData_d = XYFitCurve.initFitData(fitData_d)
fitData_d = fit_d.initStartValues(fitData_d)
fit_d.setFitData(fitData_d)
fit_d.recalculate()
plot_d.addChild(fit_d)

# === Add Panel Labels (a), (b), (c), (d) ===
# These are positioned at top-left of each panel
label_font = QFont()
label_font.setPointSize(12)
label_font.setBold(True)

label_positions = [
	(-0.5, 8.0),   # Panel (a) - top left
	(8.0, 8.0),    # Panel (b) - top right
	(-0.5, -0.5),  # Panel (c) - bottom left
	(8.0, -0.5)    # Panel (d) - bottom right
]

label_texts = ['(a)', '(b)', '(c)', '(d)']

te = QTextEdit()
for i, (label_text, (x, y)) in enumerate(zip(label_texts, label_positions)):
	label = TextLabel(f"Label {label_text}")
	worksheet.addChild(label)

	te.clear()
	te.setFont(label_font)
	te.setPlainText(label_text)
	label.setText(te.toHtml())

	x_pos = Worksheet.convertToSceneUnits(x, Worksheet.Unit.Centimeter)
	y_pos = Worksheet.convertToSceneUnits(y, Worksheet.Unit.Centimeter)
	label.setPositionScene(QPointF(x_pos, y_pos))

print("=" * 60)
print("Publication Figure Created Successfully!")
print("=" * 60)
print()
print("Figure Layout: 2×2 grid with panels (a), (b), (c), (d)")
print()
print("Panel (a): Time series with noise")
print("Panel (b): Dose-response curve with sigmoid fit")
print("Panel (c): Grouped bar chart with error bars")
print("Panel (d): Scatter plot with linear regression")
print()
print("=" * 60)
print("Export Options for Publication:")
print("=" * 60)
print()
print("Use worksheet.exportToFile() for high-quality export:")
print('  worksheet.exportToFile("Figure1.pdf", Worksheet.ExportFormat.PDF)')
print('  worksheet.exportToFile("Figure1.png", Worksheet.ExportFormat.PNG, 300)  # 300 DPI')
print('  worksheet.exportToFile("Figure1.svg", Worksheet.ExportFormat.SVG)')
print()
print("Most journals prefer:")
print("  - PDF or TIFF for print")
print("  - 300 DPI minimum resolution")
print("  - RGB color mode")
print("  - Single or double column width (8.5 or 17.4 cm)")
print()

# Uncomment to auto-export:
# worksheet.exportToFile("Figure1.pdf", Worksheet.ExportFormat.PDF)
# worksheet.exportToFile("Figure1_300dpi.png", Worksheet.ExportFormat.PNG, 300)
