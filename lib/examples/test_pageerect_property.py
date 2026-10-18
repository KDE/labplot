"""
Test setting pageRect as property
"""
from PySide6.QtCore import QRectF
from pylabplot import *

proj = project()
worksheet = Worksheet("Test")
proj.addChild(worksheet)

print("=== Testing pageRect property ===")

# Try to read it
try:
    current = worksheet.pageRect
    print(f"  Current pageRect: {current}")
    print(f"  Type: {type(current)}")
except Exception as e:
    print(f"  Error reading: {e}")

# Try to set it
try:
    worksheet.setUseViewSize(False)
    w = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
    h = Worksheet.convertToSceneUnits(15, Worksheet.Unit.Centimeter)
    new_rect = QRectF(0, 0, w, h)

    worksheet.pageRect = new_rect
    print(f"  Successfully set pageRect to: {new_rect}")
    print(f"  New value: {worksheet.pageRect}")
except Exception as e:
    print(f"  Error setting: {e}")
