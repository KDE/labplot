"""
Debug script to inspect Worksheet API
"""
from pylabplot import *

proj = project()
worksheet = Worksheet("Test")
proj.addChild(worksheet)

print("=== Worksheet methods containing 'page' ===")
for attr in dir(worksheet):
    if 'page' in attr.lower():
        print(f"  {attr}")

print("\n=== Checking specific methods ===")
print(f"  hasattr(worksheet, 'setPageRect'): {hasattr(worksheet, 'setPageRect')}")
print(f"  hasattr(worksheet, 'pageRect'): {hasattr(worksheet, 'pageRect')}")
print(f"  type(worksheet): {type(worksheet)}")
