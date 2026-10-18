#!/usr/bin/env python3
"""
Simple unwrap test using LabPlot

Demonstrates how to use the unwrap function in LabPlot once
the Python binding to nsl_math_unwrap is available.
"""

import numpy as np
from pylabplot import *

# Create project and spreadsheet
proj = project()
spreadsheet = Spreadsheet("Phase Data")
proj.addChild(spreadsheet)

# Generate wrapped phase data
n_points = 50
t = np.linspace(0, 10, n_points)
phase_continuous = 0.8 * t
phase_wrapped = np.arctan2(np.sin(phase_continuous), np.cos(phase_continuous))

# Setup spreadsheet
spreadsheet.setColumnCount(3)
spreadsheet.setRowCount(n_points)

col_t = spreadsheet.column(0)
col_t.setName("Time")

col_wrapped = spreadsheet.column(1)
col_wrapped.setName("Phase Wrapped")

col_unwrapped = spreadsheet.column(2)
col_unwrapped.setName("Phase Unwrapped")

# Fill data
for i in range(n_points):
    col_t.setValueAt(i, float(t[i]))
    col_wrapped.setValueAt(i, float(phase_wrapped[i]))

print("=== LabPlot Unwrap Test ===")
print(f"Generated {n_points} phase samples")
print(f"Wrapped phase range: [{np.min(phase_wrapped):.3f}, {np.max(phase_wrapped):.3f}]")

# TODO: Once C++ binding is available, unwrap can be used like this:
#
# Method 1: Using nsl_math_unwrap directly
# phase_array = [col_wrapped.valueAt(i) for i in range(n_points)]
# nsl_math_unwrap(phase_array, len(phase_array), np.pi)
# for i, val in enumerate(phase_array):
#     col_unwrapped.setValueAt(i, val)
#
# Method 2: Using a column operation/formula
# col_unwrapped.setFormula("unwrap(Phase Wrapped)")
#
# Method 3: Using XY analysis curve
# unwrap_curve = XYUnwrapCurve("Unwrapped Phase")
# unwrap_curve.setDataSourceColumns(col_t, col_wrapped)
# unwrap_data = unwrap_curve.unwrapData()
# unwrap_data.discontinuity = np.pi
# unwrap_curve.setUnwrapData(unwrap_data)
# unwrap_curve.recalculate()

# For now, use NumPy to demonstrate expected result
phase_unwrapped_numpy = np.unwrap(phase_wrapped)
for i, val in enumerate(phase_unwrapped_numpy):
    col_unwrapped.setValueAt(i, float(val))

print(f"Unwrapped phase range: [{np.min(phase_unwrapped_numpy):.3f}, {np.max(phase_unwrapped_numpy):.3f}]")

# Create a simple plot
worksheet = Worksheet("Phase Unwrap Demo")
proj.addChild(worksheet)

plot = CartesianPlot("Phase Plot")
plot.setType(CartesianPlot.Type.FourAxes)
worksheet.addChild(plot)

# Wrapped phase curve
curve_wrapped = XYCurve("Wrapped")
plot.addChild(curve_wrapped)
curve_wrapped.setXColumn(col_t)
curve_wrapped.setYColumn(col_wrapped)
curve_wrapped.line().setStyle(Line.Style.SolidLine)
curve_wrapped.symbol().setStyle(Symbol.Style.Circle)
curve_wrapped.symbol().setSize(5)

# Unwrapped phase curve
curve_unwrapped = XYCurve("Unwrapped")
plot.addChild(curve_unwrapped)
curve_unwrapped.setXColumn(col_t)
curve_unwrapped.setYColumn(col_unwrapped)
curve_unwrapped.line().setStyle(Line.Style.SolidLine)
curve_unwrapped.symbol().setStyle(Symbol.Style.NoSymbols)

print("\nSpreadsheet and plot created successfully")
print("Check the LabPlot GUI to see the wrapped vs unwrapped phase")
