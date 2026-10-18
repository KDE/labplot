"""
Test ReferenceLine visibility
"""
from PySide6.QtCore import QPointF, Qt
from pylabplot import *

proj = project()
worksheet = Worksheet("ReferenceLine Test")
proj.addChild(worksheet)

# Create a simple plot
plot = CartesianPlot("Test Plot")
plot.setType(CartesianPlot.Type.FourAxes)
worksheet.addChild(plot)

# Set plot range
plot.enableAutoScale(CartesianCoordinateSystem.Dimension.X, 0, False)
plot.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
rangeX = plot.range(CartesianCoordinateSystem.Dimension.X, 0)
rangeX.setRange(0, 100)
plot.setRange(CartesianCoordinateSystem.Dimension.X, 0, rangeX)
rangeY = plot.range(CartesianCoordinateSystem.Dimension.Y, 0)
rangeY.setRange(0, 1000)
plot.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY)

print("=== Testing ReferenceLine ===")

# Test 1: Add child first, then configure
ref_line1 = ReferenceLine(plot, "Test Line 1")
plot.addChild(ref_line1)
print(f"Line 1 added to plot")

# IMPORTANT: Set coordinate system index (this was missing!)
ref_line1.setCoordinateSystemIndex(plot.defaultCoordinateSystemIndex())
print(f"Line 1 coordinate system index set")

ref_line1.setOrientation(ReferenceLine.Orientation.Vertical)
print(f"Line 1 orientation set to Vertical")

ref_line1.setPositionLogical(QPointF(25, 0))
print(f"Line 1 position set to x=25")

line1 = ref_line1.line()
print(f"Line 1 style before: {line1.style()}")
line1.setStyle(Line.Style.DashLine)
print(f"Line 1 style after: {line1.style()}")

print(f"Line 1 width before: {line1.width()}")
line1.setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
print(f"Line 1 width after: {line1.width()}")

print(f"Line 1 opacity: {line1.opacity()}")

# IMPORTANT: Retransform to make it visible!
ref_line1.retransform()
print("Line 1 retransformed")

# Test 2: Try solid line at different position
ref_line2 = ReferenceLine(plot, "Test Line 2")
plot.addChild(ref_line2)
ref_line2.setCoordinateSystemIndex(plot.defaultCoordinateSystemIndex())
ref_line2.setOrientation(ReferenceLine.Orientation.Vertical)
ref_line2.setPositionLogical(QPointF(75, 0))
ref_line2.line().setStyle(Line.Style.SolidLine)
ref_line2.line().setWidth(Worksheet.convertToSceneUnits(3, Worksheet.Unit.Point))
ref_line2.retransform()

print("\nCreated 2 reference lines - check if they're visible in the plot")
print("Line 1: x=25, DashLine, 2pt")
print("Line 2: x=75, SolidLine, 3pt")
