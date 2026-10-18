"""
Test script to debug grid line visibility
"""
from PySide6.QtCore import Qt
from pylabplot import *

proj = project()
worksheet = Worksheet("Grid Test")
proj.addChild(worksheet)

plot = CartesianPlot("Test Plot")
plot.setType(CartesianPlot.Type.FourAxes)
worksheet.addChild(plot)

# Get axes
x_axis = plot.horizontalAxis()
y_axis = plot.verticalAxis()

print("=== Before disabling grid lines ===")
print(f"X axis major grid style: {x_axis.majorGridLine().style()}")
print(f"X axis minor grid style: {x_axis.minorGridLine().style()}")
print(f"Y axis major grid style: {y_axis.majorGridLine().style()}")
print(f"Y axis minor grid style: {y_axis.minorGridLine().style()}")

# Try to disable grid lines
x_axis.majorGridLine().setStyle(Line.Style.NoPen)
x_axis.minorGridLine().setStyle(Line.Style.NoPen)
y_axis.majorGridLine().setStyle(Line.Style.NoPen)
y_axis.minorGridLine().setStyle(Line.Style.NoPen)

print("\n=== After disabling grid lines ===")
print(f"X axis major grid style: {x_axis.majorGridLine().style()}")
print(f"X axis minor grid style: {x_axis.minorGridLine().style()}")
print(f"Y axis major grid style: {y_axis.majorGridLine().style()}")
print(f"Y axis minor grid style: {y_axis.minorGridLine().style()}")

print(f"\nLine.Style.NoPen value: {Line.Style.NoPen}")
print(f"Line.Style.SolidLine value: {Line.Style.SolidLine}")
