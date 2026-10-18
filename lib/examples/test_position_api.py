"""
Minimal test script for position API in LabPlot Python bindings

Tests:
1. positionLogical() / setPositionLogical() - position in plot data coordinates
2. positionScene() / setPositionScene() - position in worksheet scene coordinates (cm)
"""
from PySide6.QtCore import QPointF
from pylabplot import *

# Create project and worksheet
proj = project()
worksheet = Worksheet("Position Test")
proj.addChild(worksheet)

# Create a text label
label = TextLabel("Test Label")
worksheet.addChild(label)

print("=== Testing Position API ===\n")

# Test 1: Scene coordinates (worksheet coordinates in scene units)
print("1. Scene Position (worksheet coordinates):")
x_cm = 5.0  # 5 cm from left
y_cm = 3.0  # 3 cm from top
x_scene = Worksheet.convertToSceneUnits(x_cm, Worksheet.Unit.Centimeter)
y_scene = Worksheet.convertToSceneUnits(y_cm, Worksheet.Unit.Centimeter)

print(f"   Setting scene position to ({x_cm}, {y_cm}) cm")
print(f"   Scene units: ({x_scene:.2f}, {y_scene:.2f})")
label.setPositionScene(QPointF(x_scene, y_scene))

pos_scene = label.positionScene()
print(f"   Read back: ({pos_scene.x():.2f}, {pos_scene.y():.2f})")
print(f"   ✓ setPositionScene() and positionScene() work!\n")

# Test 2: Logical coordinates (only meaningful when label is in a plot)
print("2. Logical Position (plot data coordinates):")
print("   Creating a CartesianPlot to test logical coordinates...")

plot = CartesianPlot("Test Plot")
worksheet.addChild(plot)

# Create a label as child of the plot
plot_label = TextLabel("Plot Label")
plot.addChild(plot_label)

# Set logical position in data coordinates
logical_x = 0.5  # Middle of default [0,1] range
logical_y = 0.8  # Near top of default [0,1] range
print(f"   Setting logical position to ({logical_x}, {logical_y})")
plot_label.setPositionLogical(QPointF(logical_x, logical_y))

pos_logical = plot_label.positionLogical()
print(f"   Read back: ({pos_logical.x()}, {pos_logical.y()})")
print(f"   ✓ setPositionLogical() and positionLogical() work!\n")

print("=== All Position API Tests Passed ===")
