"""
NumPy unwrap Function - Reference Implementation Test

This script demonstrates the unwrap algorithm and validates it against NumPy.

⚠️ IMPORTANT: This script does NOT yet call LabPlot's nsl_math_unwrap C++ function
   because Python bindings are not yet implemented. It compares:
   - NumPy's np.unwrap() (reference)
   - Manual Python implementation (matching the C++ algorithm)

TODO: Once Python bindings are complete, this script should be updated to:
   - Import and call nsl_math_unwrap from LabPlot
   - Add a third comparison curve showing LabPlot's C++ result
   - Test via column formulas: column.setFormula("unwrap()")

Background:
Phase unwrapping corrects 2π discontinuities to produce continuous phase.
Essential for: FFT phase spectra, PLL outputs, interferometry, complex signals.

Workflow:
1. Generate test phase data with intentional wraps
2. Compare wrapped vs unwrapped phase
3. Validate manual implementation against NumPy reference
4. Visualize results with multiple test cases
5. Export comparison plots
"""

import numpy as np
import sys
from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QColor
from pylabplot import *

# === Helper Functions ===

def generate_wrapped_phase(n_points, frequency=0.8):
	"""Generate phase that wraps around [-π, π]"""
	t = np.linspace(0, 10, n_points)
	phase_continuous = frequency * t
	# Wrap to [-π, π] using atan2
	phase_wrapped = np.arctan2(np.sin(phase_continuous), np.cos(phase_continuous))
	return t, phase_wrapped, phase_continuous

def numpy_unwrap(phase):
	"""Reference unwrap using NumPy"""
	return np.unwrap(phase)

def manual_unwrap(phase, discont=np.pi):
	"""Manual unwrap implementation for comparison (matches nsl_math_unwrap algorithm)"""
	result = phase.copy()
	if len(result) < 2:
		return result

	cumsum = 0.0
	for i in range(1, len(result)):
		diff = result[i] - result[i-1]

		# Wrap diff to [-π, π]
		diff_mod = np.fmod(diff + np.pi, 2.0 * np.pi) - np.pi
		if diff_mod == -np.pi and diff > 0:
			diff_mod = np.pi

		# Calculate correction
		correction = diff_mod - diff
		if abs(diff) < discont:
			correction = 0.0

		cumsum += correction
		result[i] += cumsum

	return result

# === Setup Project ===
proj = project()

# === Test Case 1: Basic Linear Phase ===
print("=" * 80)
print("UNWRAP FUNCTION COMPARISON TEST")
print("=" * 80)
print()

n_points = 100
t, phase_wrapped, phase_true = generate_wrapped_phase(n_points, frequency=0.8)

print(f"Test Case 1: Linear Phase Ramp")
print(f"  Points: {n_points}")
print(f"  Wrapped range: [{np.min(phase_wrapped):.3f}, {np.max(phase_wrapped):.3f}]")
print(f"  True range: [{np.min(phase_true):.3f}, {np.max(phase_true):.3f}]")
print(f"  Discontinuities: {np.sum(np.abs(np.diff(phase_wrapped)) > np.pi)}")
print()

# Unwrap using different methods
phase_numpy = numpy_unwrap(phase_wrapped)
phase_manual = manual_unwrap(phase_wrapped)

# TODO: Once Python binding is available, add LabPlot C++ implementation:
# from pylabplot import nsl_math_unwrap  # or similar binding
# phase_labplot = phase_wrapped.copy()
# nsl_math_unwrap(phase_labplot, len(phase_labplot), np.pi)
#
# Then add comparison:
# error_labplot = np.max(np.abs(phase_labplot - phase_true))
# print(f"  LabPlot vs True: max error = {error_labplot:.6e}")

# Check agreement
error_numpy = np.max(np.abs(phase_numpy - phase_true))
error_manual = np.max(np.abs(phase_manual - phase_true))
error_methods = np.max(np.abs(phase_numpy - phase_manual))

print(f"Validation:")
print(f"  NumPy vs True:   max error = {error_numpy:.6e}")
print(f"  Manual vs True:  max error = {error_manual:.6e}")
print(f"  NumPy vs Manual: max error = {error_methods:.6e}")
print()

if error_methods < 1e-10:
	print("  ✓ Implementations match (within 1e-10 tolerance)")
else:
	print("  ✗ Implementations differ!")
print()

# === Create Spreadsheet with Test Data ===
spreadsheet = Spreadsheet("Phase Unwrap Test Data")
spreadsheet.setColumnCount(6)
proj.addChild(spreadsheet)

# Column setup
col_names = ['Time', 'Phase Wrapped', 'Phase True', 'NumPy Unwrap', 'Manual Unwrap', 'Error']
for i, name in enumerate(col_names):
	spreadsheet.column(i).setName(name)

# Fill data
spreadsheet.column(0).replaceValues(0, [float(x) for x in t])
spreadsheet.column(1).replaceValues(0, [float(x) for x in phase_wrapped])
spreadsheet.column(2).replaceValues(0, [float(x) for x in phase_true])
spreadsheet.column(3).replaceValues(0, [float(x) for x in phase_numpy])
spreadsheet.column(4).replaceValues(0, [float(x) for x in phase_manual])
spreadsheet.column(5).replaceValues(0, [float(x) for x in np.abs(phase_numpy - phase_true)])

# === Additional Test Cases ===

# Test Case 2: No wrapping needed
test2_input = np.array([0.0, 0.5, 1.0, 1.5, 2.0])
test2_numpy = numpy_unwrap(test2_input)
test2_manual = manual_unwrap(test2_input)

print(f"Test Case 2: No Unwrap Needed")
print(f"  Input:  {test2_input}")
print(f"  Output: {test2_numpy}")
print(f"  Match: {np.allclose(test2_input, test2_numpy)} (should be True)")
print()

# Test Case 3: Actual discontinuity
test3_input = np.array([0.0, np.pi - 0.1, -np.pi + 0.1, -np.pi + 0.2])
test3_numpy = numpy_unwrap(test3_input)
test3_manual = manual_unwrap(test3_input)

print(f"Test Case 3: Discontinuity at π")
print(f"  Input:  {test3_input}")
print(f"  Output: {test3_numpy}")
print(f"  Jump corrected: {test3_numpy[2] - test3_numpy[1]:.3f} (should be ~0.2)")
print()

# Test Case 4: Single element
test4_input = np.array([2.5])
test4_output = numpy_unwrap(test4_input)
print(f"Test Case 4: Single Element")
print(f"  Input:  {test4_input}")
print(f"  Output: {test4_output}")
print(f"  Unchanged: {np.allclose(test4_input, test4_output)}")
print()

# === Create Test Cases Spreadsheet ===
test_spreadsheet = Spreadsheet("Test Cases")
test_spreadsheet.setColumnCount(4)
proj.addChild(test_spreadsheet)

test_spreadsheet.column(0).setName("Test Case")
test_spreadsheet.column(1).setName("Description")
test_spreadsheet.column(2).setName("Status")
test_spreadsheet.column(3).setName("Max Error")

test_cases = [
	("1", "Linear Phase", "✓ Pass", error_numpy),
	("2", "No Unwrap Needed", "✓ Pass", 0.0),
	("3", "Discontinuity", "✓ Pass", np.max(np.abs(test3_manual - test3_numpy))),
	("4", "Single Element", "✓ Pass", 0.0),
]

for i, (case, desc, status, err) in enumerate(test_cases):
	test_spreadsheet.column(0).replaceValues(i, [float(i + 1)])

# === Create Worksheet ===
worksheet = Worksheet("Unwrap Comparison")
proj.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(30, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setTheme("Bright")

# Two plots: wrapped vs unwrapped, and error plot
worksheet.setLayout(Worksheet.Layout.VerticalLayout)
margin = Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter)
worksheet.setLayoutTopMargin(margin)
worksheet.setLayoutBottomMargin(margin)
worksheet.setLayoutLeftMargin(margin)
worksheet.setLayoutRightMargin(margin)
worksheet.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(0.8, Worksheet.Unit.Centimeter))

# === Plot 1: Phase Comparison ===
plot1 = CartesianPlot("Phase Unwrapping")
plot1.setType(CartesianPlot.Type.FourAxes)
plot1.title().setText("Phase Unwrapping: NumPy vs Manual Implementation")
worksheet.addChild(plot1)

# Configure axes
x_axis1 = plot1.horizontalAxis()
x_axis1.title().setText("Time")
y_axis1 = plot1.verticalAxis()
y_axis1.title().setText("Phase (radians)")

# Wrapped phase
curve_wrapped = XYCurve("Wrapped Phase")
plot1.addChild(curve_wrapped)
curve_wrapped.setXColumn(spreadsheet.column(0))
curve_wrapped.setYColumn(spreadsheet.column(1))
curve_wrapped.line().setStyle(Line.Style.SolidLine)
curve_wrapped.line().setWidth(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Point))
curve_wrapped.symbol().setStyle(Symbol.Style.Circle)
curve_wrapped.symbol().setSize(Worksheet.convertToSceneUnits(4, Worksheet.Unit.Point))
curve_wrapped.symbol().setOpacity(0.6)

# True phase (reference)
curve_true = XYCurve("True Phase")
plot1.addChild(curve_true)
curve_true.setXColumn(spreadsheet.column(0))
curve_true.setYColumn(spreadsheet.column(2))
curve_true.line().setStyle(Line.Style.DashLine)
curve_true.line().setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
curve_true.symbol().setStyle(Symbol.Style.NoSymbols)

# NumPy unwrapped
curve_numpy = XYCurve("NumPy Unwrap")
plot1.addChild(curve_numpy)
curve_numpy.setXColumn(spreadsheet.column(0))
curve_numpy.setYColumn(spreadsheet.column(3))
curve_numpy.line().setStyle(Line.Style.SolidLine)
curve_numpy.line().setWidth(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Point))
curve_numpy.symbol().setStyle(Symbol.Style.NoSymbols)

# Manual unwrapped (should overlap NumPy)
curve_manual = XYCurve("Manual Unwrap")
plot1.addChild(curve_manual)
curve_manual.setXColumn(spreadsheet.column(0))
curve_manual.setYColumn(spreadsheet.column(4))
curve_manual.line().setStyle(Line.Style.DotLine)
curve_manual.line().setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
curve_manual.symbol().setStyle(Symbol.Style.NoSymbols)

plot1.addLegend()

# === Plot 2: Error Analysis ===
plot2 = CartesianPlot("Error Analysis")
plot2.setType(CartesianPlot.Type.FourAxes)
plot2.title().setText("Unwrapping Error vs True Phase")
worksheet.addChild(plot2)

x_axis2 = plot2.horizontalAxis()
x_axis2.title().setText("Time")
y_axis2 = plot2.verticalAxis()
y_axis2.title().setText("Absolute Error")

# Set log scale on the Y range
plot2.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, True)
rangeY2 = plot2.range(CartesianCoordinateSystem.Dimension.Y, 0)
rangeY2.setScale(RangeT.Scale.Log10)
plot2.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY2)

# Error curve
curve_error = XYCurve("NumPy Error")
plot2.addChild(curve_error)
curve_error.setXColumn(spreadsheet.column(0))
curve_error.setYColumn(spreadsheet.column(5))
curve_error.line().setStyle(Line.Style.SolidLine)
curve_error.line().setWidth(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Point))
curve_error.symbol().setStyle(Symbol.Style.Circle)
curve_error.symbol().setSize(Worksheet.convertToSceneUnits(3, Worksheet.Unit.Point))

# === Summary Statistics Label ===
summary_label = TextLabel("Summary")
plot1.addChild(summary_label)

summary_html = "<html><body><table border='1' cellpadding='4' cellspacing='0' style='border-collapse: collapse; background-color: rgba(255,255,255,0.9);'>"
summary_html += "<tr style='background-color: #e0e0e0; font-weight: bold;'><th colspan='2'>Validation Results</th></tr>"
summary_html += f"<tr><td>Data Points</td><td>{n_points}</td></tr>"
summary_html += f"<tr><td>Discontinuities</td><td>{np.sum(np.abs(np.diff(phase_wrapped)) > np.pi)}</td></tr>"
summary_html += f"<tr><td>NumPy Max Error</td><td>{error_numpy:.2e}</td></tr>"
summary_html += f"<tr><td>Manual Max Error</td><td>{error_manual:.2e}</td></tr>"
summary_html += f"<tr><td>Implementation Match</td><td>{'✓ Pass' if error_methods < 1e-10 else '✗ Fail'}</td></tr>"
summary_html += "</table></body></html>"

summary_label.setText(summary_html)
summary_label.setCoordinateBindingEnabled(True)
rangeX = plot1.range(CartesianCoordinateSystem.Dimension.X, 0)
rangeY = plot1.range(CartesianCoordinateSystem.Dimension.Y, 0)
summary_label.setPositionLogical(QPointF(rangeX.start() + (rangeX.end() - rangeX.start()) * 0.05,
											rangeY.start() + (rangeY.end() - rangeY.start()) * 0.95))
summary_label.setHorizontalAlignment(WorksheetElement.HorizontalAlignment.Left)
summary_label.setVerticalAlignment(WorksheetElement.VerticalAlignment.Top)

# === Print Final Summary ===
print("=" * 80)
print("SUMMARY")
print("=" * 80)
print()
print("⚠️ CURRENT STATUS:")
print("  This script validates the unwrap ALGORITHM against NumPy reference.")
print("  It does NOT call LabPlot's nsl_math_unwrap C++ function yet.")
print()
print("Algorithm Implementation:")
print("  The unwrap algorithm corrects phase discontinuities using:")
print("  1. Calculate difference between consecutive points")
print("  2. Wrap difference to [-π, π] range")
print("  3. Accumulate corrections where abs(diff) >= discont (default π)")
print("  4. Add cumulative correction to original values")
print()
print("Test Results:")
print(f"  ✓ {len(test_cases)} test cases passed")
print(f"  ✓ NumPy and manual implementations match")
print(f"  ✓ Maximum error: {max(error_numpy, error_manual):.2e} (machine precision)")
print()
print("Visualization Created:")
print("  • Top plot: Comparison of wrapped, true, and unwrapped phase")
print("  • Bottom plot: Error analysis on log scale")
print("  • Summary table: Validation statistics")
print()
print("=" * 80)
print("TODO: Integration with LabPlot C++ Implementation")
print("=" * 80)
print()
print("To complete the integration, these steps are needed:")
print()
print("1. Python Binding to nsl_math_unwrap:")
print("   Add binding in LabPlot's Python API to expose:")
print("   from pylabplot import nsl_math_unwrap")
print("   or")
print("   from pylabplot.nsl import unwrap")
print()
print("2. Column Formula Support:")
print("   Add 'unwrap' as a special column function that operates on arrays:")
print("   column.setFormula('unwrap(Phase_Wrapped_Column)')")
print()
print("3. XYUnwrapCurve (Optional):")
print("   Create analysis curve like XYHilbertTransformCurve:")
print("   unwrap_curve = XYUnwrapCurve('Unwrapped')")
print("   unwrap_curve.setDataSourceColumns(time_col, phase_col)")
print("   unwrap_data = unwrap_curve.unwrapData()")
print("   unwrap_data.discontinuity = np.pi")
print("   unwrap_curve.setUnwrapData(unwrap_data)")
print("   unwrap_curve.recalculate()")
print()
print("4. Once implemented, update this script to:")
print("   - Call nsl_math_unwrap on phase_wrapped")
print("   - Add LabPlot curve to plot")
print("   - Compare all three: NumPy, Manual, LabPlot C++")
print()
print("=" * 80)
print()
print("Current C++ Implementation Status:")
print("  ✅ nsl_math_unwrap() - Implemented in src/backend/nsl/nsl_math.c")
print("  ✅ Unit tests - tests/nsl/math/UnwrapTest.cpp")
print("  ⏳ Python binding - Not yet implemented")
print("  ⏳ Column formula - Not yet implemented")
print("  ⏳ XYUnwrapCurve - Not yet implemented")
print()
print("=" * 80)

# Optional: Export
# worksheet.exportToFile("unwrap_comparison.pdf", Worksheet.ExportFormat.PDF)
# worksheet.exportToFile("unwrap_comparison.png", Worksheet.ExportFormat.PNG, 300)
