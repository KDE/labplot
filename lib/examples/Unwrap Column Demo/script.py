"""
NumPy vs LabPlot Function Comparison

Part 1: Visual demonstration of unwrap function (from NumPy docs)
Part 2: Comprehensive comparison of all NumPy-like functions

Functions tested:
- Trigonometric: deg2rad, rad2deg
- Rounding: floor
- Arithmetic: exp2, square, reciprocal, fmod, remainder
- Clipping: clip
- Step function: heaviside
- Integer math: gcd, lcm
- Cell-based: unwrap, unwrapp, diff, cumsum, cumprod
"""

import numpy as np
from pylabplot import *
from PySide6.QtCore import QRectF, Qt

print("=" * 70)
print("NumPy vs LabPlot Function Comparison")
print("=" * 70)

proj = project()

# =============================================================================
# PART 1: Visual Unwrap Demonstrations
# =============================================================================

print("\n" + "=" * 70)
print("PART 1: Unwrap Visual Demonstrations")
print("=" * 70)

# -----------------------------------------------------------------------------
# Demo 1: Radian phase unwrapping (default period=2π)
# -----------------------------------------------------------------------------
print("\n--- Demo 1: Radian Phase (period=2π) ---")

folder1 = Folder("Unwrap Demo - Radian")
proj.addChild(folder1)

n1 = 200
t1 = np.linspace(0, 10, n1)
phase_true = 1.5 * t1
w1 = np.arctan2(np.sin(phase_true), np.cos(phase_true))

print(f"Generated {n1} data points")
print(f"Discontinuities: {np.sum(np.abs(np.diff(w1)) > np.pi)}")

ss1 = Spreadsheet("Radian Phase")
ss1.setColumnCount(4)
ss1.setRowCount(n1)
folder1.addChild(ss1)

col1_t = ss1.column(0)
col1_t.setName("t")
col1_w = ss1.column(1)
col1_w.setName("w")
col1_np = ss1.column(2)
col1_np.setName("numpy")
col1_lp = ss1.column(3)
col1_lp.setName("labplot")

np_unwrap1 = np.unwrap(w1)
for i in range(n1):
    col1_t.setValueAt(i, float(t1[i]))
    col1_w.setValueAt(i, float(w1[i]))
    col1_np.setValueAt(i, float(np_unwrap1[i]))

col1_lp.setFormula("unwrap(w)")
col1_lp.updateFormula()

max_err1 = max(abs(col1_lp.valueAt(i) - np_unwrap1[i]) for i in range(n1))
print(f"Max error: {max_err1:.2e} -> {'PASS' if max_err1 < 1e-10 else 'FAIL'}")

# Create visualization with 2 plots
ws1 = Worksheet("Radian Phase Unwrap")
folder1.addChild(ws1)
ws1.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(28, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
ws1.setPageRect(QRectF(0, 0, w, h))
ws1.setTheme("Bright")

# Plot 1: Wrapped signal
plot1a = CartesianPlot("Wrapped")
plot1a.setType(CartesianPlot.Type.FourAxes)
plot1a.title().setText("Wrapped Phase (Input)")
ws1.addChild(plot1a)

plot1a.horizontalAxis().title().setText("t")
plot1a.verticalAxis().title().setText("w (radians)")

c1a = XYCurve("w")
plot1a.addChild(c1a)
c1a.setXColumn(col1_t)
c1a.setYColumn(col1_w)
c1a.line().setWidth(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Point))
c1a.symbol().setStyle(Symbol.Style.NoSymbols)

# Plot 2: Unwrapped comparison
plot1b = CartesianPlot("Unwrapped")
plot1b.setType(CartesianPlot.Type.FourAxes)
plot1b.title().setText("Unwrapped: NumPy vs LabPlot")
ws1.addChild(plot1b)

plot1b.horizontalAxis().title().setText("t")
plot1b.verticalAxis().title().setText("unwrap(w)")

c1b_np = XYCurve("NumPy")
plot1b.addChild(c1b_np)
c1b_np.setXColumn(col1_t)
c1b_np.setYColumn(col1_np)
c1b_np.line().setWidth(Worksheet.convertToSceneUnits(2.5, Worksheet.Unit.Point))
c1b_np.symbol().setStyle(Symbol.Style.NoSymbols)

c1b_lp = XYCurve("LabPlot")
plot1b.addChild(c1b_lp)
c1b_lp.setXColumn(col1_t)
c1b_lp.setYColumn(col1_lp)
c1b_lp.line().setStyle(Line.Style.DashLine)
c1b_lp.line().setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
c1b_lp.symbol().setStyle(Symbol.Style.NoSymbols)

plot1b.addLegend()

ws1.setLayout(Worksheet.Layout.VerticalLayout)

# -----------------------------------------------------------------------------
# Demo 2: Custom period unwrapping (from NumPy docs)
# -----------------------------------------------------------------------------
print("\n--- Demo 2: Custom Period (period=2, from NumPy docs) ---")

folder2 = Folder("Unwrap Demo - Custom Period")
proj.addChild(folder2)

n2 = 801
t2 = np.linspace(0, 25, n2)
w2 = np.mod(1.5 * np.sin(1.1 * t2 + 0.26) * (1 - t2 / 6 + (t2 / 23) ** 3), 2.0) - 1

print(f"Generated {n2} data points")
print("Signal: w = mod(1.5*sin(1.1*t + 0.26)*(1 - t/6 + (t/23)^3), 2) - 1")

ss2 = Spreadsheet("Custom Period")
ss2.setColumnCount(4)
ss2.setRowCount(n2)
folder2.addChild(ss2)

col2_t = ss2.column(0)
col2_t.setName("t")
col2_w = ss2.column(1)
col2_w.setName("w")
col2_np = ss2.column(2)
col2_np.setName("numpy")
col2_lp = ss2.column(3)
col2_lp.setName("labplot")

np_unwrap2 = np.unwrap(w2, period=2.0)
for i in range(n2):
    col2_t.setValueAt(i, float(t2[i]))
    col2_w.setValueAt(i, float(w2[i]))
    col2_np.setValueAt(i, float(np_unwrap2[i]))

col2_lp.setFormula("unwrapp(2; w)")
col2_lp.updateFormula()

max_err2 = max(abs(col2_lp.valueAt(i) - np_unwrap2[i]) for i in range(n2))
print(f"Max error: {max_err2:.2e} -> {'PASS' if max_err2 < 1e-10 else 'FAIL'}")

# Create visualization with 2 plots
ws2 = Worksheet("Custom Period Unwrap")
folder2.addChild(ws2)
ws2.setUseViewSize(False)
ws2.setPageRect(QRectF(0, 0, w, h))
ws2.setTheme("Bright")

# Plot 1: Wrapped signal
plot2a = CartesianPlot("Wrapped")
plot2a.setType(CartesianPlot.Type.FourAxes)
plot2a.title().setText("Wrapped Signal (period=2)")
ws2.addChild(plot2a)

plot2a.horizontalAxis().title().setText("t")
plot2a.verticalAxis().title().setText("w")

c2a = XYCurve("w")
plot2a.addChild(c2a)
c2a.setXColumn(col2_t)
c2a.setYColumn(col2_w)
c2a.line().setWidth(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Point))
c2a.symbol().setStyle(Symbol.Style.NoSymbols)

# Plot 2: Unwrapped comparison
plot2b = CartesianPlot("Unwrapped")
plot2b.setType(CartesianPlot.Type.FourAxes)
plot2b.title().setText("Unwrapped: NumPy vs LabPlot (period=2)")
ws2.addChild(plot2b)

plot2b.horizontalAxis().title().setText("t")
plot2b.verticalAxis().title().setText("unwrapp(2, w)")

c2b_np = XYCurve("NumPy")
plot2b.addChild(c2b_np)
c2b_np.setXColumn(col2_t)
c2b_np.setYColumn(col2_np)
c2b_np.line().setWidth(Worksheet.convertToSceneUnits(2.5, Worksheet.Unit.Point))
c2b_np.symbol().setStyle(Symbol.Style.NoSymbols)

c2b_lp = XYCurve("LabPlot")
plot2b.addChild(c2b_lp)
c2b_lp.setXColumn(col2_t)
c2b_lp.setYColumn(col2_lp)
c2b_lp.line().setStyle(Line.Style.DashLine)
c2b_lp.line().setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
c2b_lp.symbol().setStyle(Symbol.Style.NoSymbols)

plot2b.addLegend()

ws2.setLayout(Worksheet.Layout.VerticalLayout)

# =============================================================================
# PART 2: Comprehensive Function Comparison Tests
# =============================================================================

print("\n" + "=" * 70)
print("PART 2: Comprehensive Function Tests")
print("=" * 70)

def create_comparison(folder_name, col_names, input_data, np_func, lp_formula, description):
    """
    Create a comparison folder with spreadsheet and plot.

    Returns (folder, max_error, passed)
    """
    folder = Folder(folder_name)
    proj.addChild(folder)

    n = len(input_data[0]) if isinstance(input_data, list) else len(input_data)

    # Create spreadsheet
    ss = Spreadsheet(folder_name)
    num_cols = len(col_names) + 2  # inputs + numpy + labplot
    ss.setColumnCount(num_cols)
    ss.setRowCount(n)
    folder.addChild(ss)

    # Fill input columns
    col_idx = 0
    input_cols = []
    if isinstance(input_data, list):
        for i, (name, data) in enumerate(zip(col_names, input_data)):
            col = ss.column(col_idx)
            col.setName(name)
            for j, v in enumerate(data):
                col.setValueAt(j, float(v))
            input_cols.append(col)
            col_idx += 1
    else:
        col = ss.column(col_idx)
        col.setName(col_names[0])
        for j, v in enumerate(input_data):
            col.setValueAt(j, float(v))
        input_cols.append(col)
        col_idx += 1

    # NumPy result
    col_np = ss.column(col_idx)
    col_np.setName("numpy")
    np_result = np_func(*input_data) if isinstance(input_data, list) else np_func(input_data)
    for j, v in enumerate(np_result):
        col_np.setValueAt(j, float(v))
    col_idx += 1

    # LabPlot result
    col_lp = ss.column(col_idx)
    col_lp.setName("labplot")
    col_lp.setFormula(lp_formula)
    col_lp.updateFormula()

    # Compare
    max_err = 0.0
    for j in range(n):
        np_val = col_np.valueAt(j)
        lp_val = col_lp.valueAt(j)
        if np.isnan(np_val) and np.isnan(lp_val):
            continue
        if np.isnan(np_val) or np.isnan(lp_val):
            max_err = float('inf')
            break
        err = abs(np_val - lp_val)
        if err > max_err:
            max_err = err

    passed = max_err < 1e-10

    # Create plot
    ws = Worksheet(folder_name)
    folder.addChild(ws)
    ws.setUseViewSize(False)
    w = Worksheet.convertToSceneUnits(24, Worksheet.Unit.Centimeter)
    h = Worksheet.convertToSceneUnits(12, Worksheet.Unit.Centimeter)
    ws.setPageRect(QRectF(0, 0, w, h))
    ws.setTheme("Bright")

    plot = CartesianPlot(folder_name)
    plot.setType(CartesianPlot.Type.FourAxes)
    plot.title().setText(f"{folder_name}: {description}")
    ws.addChild(plot)

    plot.horizontalAxis().title().setText(col_names[0])
    plot.verticalAxis().title().setText("Result")

    # NumPy curve
    c_np = XYCurve("NumPy")
    plot.addChild(c_np)
    c_np.setXColumn(input_cols[0])
    c_np.setYColumn(col_np)
    c_np.line().setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
    c_np.symbol().setStyle(Symbol.Style.NoSymbols)

    # LabPlot curve
    c_lp = XYCurve("LabPlot")
    plot.addChild(c_lp)
    c_lp.setXColumn(input_cols[0])
    c_lp.setYColumn(col_lp)
    c_lp.line().setStyle(Line.Style.DashLine)
    c_lp.line().setWidth(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Point))
    c_lp.symbol().setStyle(Symbol.Style.NoSymbols)

    plot.addLegend()

    return folder, max_err, passed

# =============================================================================
# Test data
# =============================================================================
n = 50
x = np.linspace(0, 10, n)
x_int = np.arange(1, n + 1, dtype=float)
x_deg = np.linspace(0, 360, n)
x_phase = np.arctan2(np.sin(1.5 * x), np.cos(1.5 * x))

results = []

# =============================================================================
# 1. deg2rad
# =============================================================================
print("\n1. deg2rad...")
_, err, passed = create_comparison(
    "deg2rad",
    ["degrees"],
    x_deg,
    np.deg2rad,
    "deg2rad(degrees)",
    "Degrees to radians"
)
results.append(("deg2rad", err, passed))
print(f"   Max error: {err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 2. rad2deg
# =============================================================================
print("2. rad2deg...")
x_rad = np.linspace(0, 2*np.pi, n)
_, err, passed = create_comparison(
    "rad2deg",
    ["radians"],
    x_rad,
    np.rad2deg,
    "rad2deg(radians)",
    "Radians to degrees"
)
results.append(("rad2deg", err, passed))
print(f"   Max error: {err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 3. floor
# =============================================================================
print("3. floor...")
x_floor = np.linspace(-2.5, 2.5, n)
_, err, passed = create_comparison(
    "floor",
    ["x"],
    x_floor,
    np.floor,
    "floor(x)",
    "Floor function"
)
results.append(("floor", err, passed))
print(f"   Max error: {err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 4. exp2
# =============================================================================
print("4. exp2...")
x_exp = np.linspace(0, 5, n)
_, err, passed = create_comparison(
    "exp2",
    ["x"],
    x_exp,
    np.exp2,
    "exp2(x)",
    "2^x"
)
results.append(("exp2", err, passed))
print(f"   Max error: {err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 5. square
# =============================================================================
print("5. square...")
_, err, passed = create_comparison(
    "square",
    ["x"],
    x,
    np.square,
    "square(x)",
    "x^2"
)
results.append(("square", err, passed))
print(f"   Max error: {err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 6. reciprocal
# =============================================================================
print("6. reciprocal...")
x_recip = np.linspace(0.5, 5, n)  # Avoid zero
_, err, passed = create_comparison(
    "reciprocal",
    ["x"],
    x_recip,
    np.reciprocal,
    "reciprocal(x)",
    "1/x"
)
results.append(("reciprocal", err, passed))
print(f"   Max error: {err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 7. clip
# =============================================================================
print("7. clip...")
folder = Folder("clip")
proj.addChild(folder)

ss = Spreadsheet("clip")
ss.setColumnCount(4)
ss.setRowCount(n)
folder.addChild(ss)

col_x = ss.column(0)
col_x.setName("x")
for i, v in enumerate(x):
    col_x.setValueAt(i, float(v))

col_np = ss.column(1)
col_np.setName("numpy")
np_clip = np.clip(x, 2, 8)
for i, v in enumerate(np_clip):
    col_np.setValueAt(i, float(v))

col_lp = ss.column(2)
col_lp.setName("labplot")
col_lp.setFormula("clip(x; 2; 8)")
col_lp.updateFormula()

max_err = max(abs(col_lp.valueAt(i) - np_clip[i]) for i in range(n))
passed = max_err < 1e-10
results.append(("clip", max_err, passed))
print(f"   Max error: {max_err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 8. fmod
# =============================================================================
print("8. fmod...")
folder = Folder("fmod")
proj.addChild(folder)

ss = Spreadsheet("fmod")
ss.setColumnCount(4)
ss.setRowCount(n)
folder.addChild(ss)

col_x = ss.column(0)
col_x.setName("x")
for i, v in enumerate(x):
    col_x.setValueAt(i, float(v))

col_np = ss.column(1)
col_np.setName("numpy")
np_fmod = np.fmod(x, 3.0)
for i, v in enumerate(np_fmod):
    col_np.setValueAt(i, float(v))

col_lp = ss.column(2)
col_lp.setName("labplot")
col_lp.setFormula("fmod(x; 3)")
col_lp.updateFormula()

max_err = max(abs(col_lp.valueAt(i) - np_fmod[i]) for i in range(n))
passed = max_err < 1e-10
results.append(("fmod", max_err, passed))
print(f"   Max error: {max_err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 9. remainder (IEEE 754 - differs from NumPy's floor-based remainder)
# =============================================================================
print("9. remainder (IEEE 754)...")
folder = Folder("remainder")
proj.addChild(folder)

ss = Spreadsheet("remainder")
ss.setColumnCount(4)
ss.setRowCount(n)
folder.addChild(ss)

col_x = ss.column(0)
col_x.setName("x")
for i, v in enumerate(x):
    col_x.setValueAt(i, float(v))

col_np = ss.column(1)
col_np.setName("numpy")
# Use C++ std::remainder semantics (IEEE 754) for comparison
import math
np_rem = np.array([math.remainder(v, 3.0) for v in x])
for i, v in enumerate(np_rem):
    col_np.setValueAt(i, float(v))

col_lp = ss.column(2)
col_lp.setName("labplot")
col_lp.setFormula("remainder(x; 3)")
col_lp.updateFormula()

max_err = max(abs(col_lp.valueAt(i) - np_rem[i]) for i in range(n))
passed = max_err < 1e-10
results.append(("remainder", max_err, passed))
print(f"   Max error: {max_err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 10. heaviside
# =============================================================================
print("10. heaviside...")
folder = Folder("heaviside")
proj.addChild(folder)

x_heavi = np.linspace(-2, 2, n)
ss = Spreadsheet("heaviside")
ss.setColumnCount(4)
ss.setRowCount(n)
folder.addChild(ss)

col_x = ss.column(0)
col_x.setName("x")
for i, v in enumerate(x_heavi):
    col_x.setValueAt(i, float(v))

col_np = ss.column(1)
col_np.setName("numpy")
np_heavi = np.heaviside(x_heavi, 0.5)
for i, v in enumerate(np_heavi):
    col_np.setValueAt(i, float(v))

col_lp = ss.column(2)
col_lp.setName("labplot")
col_lp.setFormula("heaviside(x; 0.5)")
col_lp.updateFormula()

max_err = max(abs(col_lp.valueAt(i) - np_heavi[i]) for i in range(n))
passed = max_err < 1e-10
results.append(("heaviside", max_err, passed))
print(f"   Max error: {max_err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 11. gcd
# =============================================================================
print("11. gcd...")
folder = Folder("gcd")
proj.addChild(folder)

a_gcd = np.array([12, 15, 21, 35, 48, 64, 81, 100, 121, 144], dtype=float)
b_gcd = np.array([8, 10, 14, 25, 36, 48, 27, 75, 77, 108], dtype=float)
n_gcd = len(a_gcd)

ss = Spreadsheet("gcd")
ss.setColumnCount(5)
ss.setRowCount(n_gcd)
folder.addChild(ss)

col_a = ss.column(0)
col_a.setName("a")
col_b = ss.column(1)
col_b.setName("b")
for i in range(n_gcd):
    col_a.setValueAt(i, float(a_gcd[i]))
    col_b.setValueAt(i, float(b_gcd[i]))

col_np = ss.column(2)
col_np.setName("numpy")
np_gcd = np.gcd(a_gcd.astype(int), b_gcd.astype(int)).astype(float)
for i, v in enumerate(np_gcd):
    col_np.setValueAt(i, float(v))

col_lp = ss.column(3)
col_lp.setName("labplot")
col_lp.setFormula("gcd(a; b)")
col_lp.updateFormula()

max_err = max(abs(col_lp.valueAt(i) - np_gcd[i]) for i in range(n_gcd))
passed = max_err < 1e-10
results.append(("gcd", max_err, passed))
print(f"   Max error: {max_err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 12. lcm
# =============================================================================
print("12. lcm...")
folder = Folder("lcm")
proj.addChild(folder)

ss = Spreadsheet("lcm")
ss.setColumnCount(5)
ss.setRowCount(n_gcd)
folder.addChild(ss)

col_a = ss.column(0)
col_a.setName("a")
col_b = ss.column(1)
col_b.setName("b")
for i in range(n_gcd):
    col_a.setValueAt(i, float(a_gcd[i]))
    col_b.setValueAt(i, float(b_gcd[i]))

col_np = ss.column(2)
col_np.setName("numpy")
np_lcm = np.lcm(a_gcd.astype(int), b_gcd.astype(int)).astype(float)
for i, v in enumerate(np_lcm):
    col_np.setValueAt(i, float(v))

col_lp = ss.column(3)
col_lp.setName("labplot")
col_lp.setFormula("lcm(a; b)")
col_lp.updateFormula()

max_err = max(abs(col_lp.valueAt(i) - np_lcm[i]) for i in range(n_gcd))
passed = max_err < 1e-10
results.append(("lcm", max_err, passed))
print(f"   Max error: {max_err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 13. unwrap (default period=2π)
# =============================================================================
print("13. unwrap...")
folder = Folder("unwrap")
proj.addChild(folder)

ss = Spreadsheet("unwrap")
ss.setColumnCount(4)
ss.setRowCount(n)
folder.addChild(ss)

col_x = ss.column(0)
col_x.setName("x")
col_w = ss.column(1)
col_w.setName("w")
for i in range(n):
    col_x.setValueAt(i, float(x[i]))
    col_w.setValueAt(i, float(x_phase[i]))

col_np = ss.column(2)
col_np.setName("numpy")
np_unwrap = np.unwrap(x_phase)
for i, v in enumerate(np_unwrap):
    col_np.setValueAt(i, float(v))

col_lp = ss.column(3)
col_lp.setName("labplot")
col_lp.setFormula("unwrap(w)")
col_lp.updateFormula()

max_err = max(abs(col_lp.valueAt(i) - np_unwrap[i]) for i in range(n))
passed = max_err < 1e-10
results.append(("unwrap", max_err, passed))
print(f"   Max error: {max_err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 14. unwrapp (custom period=2)
# =============================================================================
print("14. unwrapp (period=2)...")
folder = Folder("unwrapp")
proj.addChild(folder)

n2 = 100
t2 = np.linspace(0, 25, n2)
w2 = np.mod(1.5 * np.sin(1.1 * t2 + 0.26) * (1 - t2 / 6 + (t2 / 23) ** 3), 2.0) - 1

ss = Spreadsheet("unwrapp")
ss.setColumnCount(4)
ss.setRowCount(n2)
folder.addChild(ss)

col_t = ss.column(0)
col_t.setName("t")
col_w = ss.column(1)
col_w.setName("w")
for i in range(n2):
    col_t.setValueAt(i, float(t2[i]))
    col_w.setValueAt(i, float(w2[i]))

col_np = ss.column(2)
col_np.setName("numpy")
np_unwrap2 = np.unwrap(w2, period=2.0)
for i, v in enumerate(np_unwrap2):
    col_np.setValueAt(i, float(v))

col_lp = ss.column(3)
col_lp.setName("labplot")
col_lp.setFormula("unwrapp(2; w)")
col_lp.updateFormula()

max_err = max(abs(col_lp.valueAt(i) - np_unwrap2[i]) for i in range(n2))
passed = max_err < 1e-10
results.append(("unwrapp", max_err, passed))
print(f"   Max error: {max_err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 15. diff
# =============================================================================
print("15. diff...")
folder = Folder("diff")
proj.addChild(folder)

ss = Spreadsheet("diff")
ss.setColumnCount(4)
ss.setRowCount(n)
folder.addChild(ss)

col_x = ss.column(0)
col_x.setName("x")
col_v = ss.column(1)
col_v.setName("values")
values = x ** 2  # quadratic for interesting diff
for i in range(n):
    col_x.setValueAt(i, float(x[i]))
    col_v.setValueAt(i, float(values[i]))

col_np = ss.column(2)
col_np.setName("numpy")
np_diff = np.diff(values, prepend=np.nan)
for i, v in enumerate(np_diff):
    col_np.setValueAt(i, float(v))

col_lp = ss.column(3)
col_lp.setName("labplot")
col_lp.setFormula("diff(values)")
col_lp.updateFormula()

# Skip first row (NaN)
max_err = 0.0
for i in range(1, n):
    err = abs(col_lp.valueAt(i) - np_diff[i])
    if err > max_err:
        max_err = err
passed = max_err < 1e-10
results.append(("diff", max_err, passed))
print(f"   Max error: {max_err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 16. cumsum
# =============================================================================
print("16. cumsum...")
folder = Folder("cumsum")
proj.addChild(folder)

ss = Spreadsheet("cumsum")
ss.setColumnCount(4)
ss.setRowCount(n)
folder.addChild(ss)

col_x = ss.column(0)
col_x.setName("x")
col_v = ss.column(1)
col_v.setName("values")
for i in range(n):
    col_x.setValueAt(i, float(x[i]))
    col_v.setValueAt(i, float(x_int[i]))

col_np = ss.column(2)
col_np.setName("numpy")
np_cumsum = np.cumsum(x_int)
for i, v in enumerate(np_cumsum):
    col_np.setValueAt(i, float(v))

col_lp = ss.column(3)
col_lp.setName("labplot")
col_lp.setFormula("cumsum(values)")
col_lp.updateFormula()

max_err = max(abs(col_lp.valueAt(i) - np_cumsum[i]) for i in range(n))
passed = max_err < 1e-10
results.append(("cumsum", max_err, passed))
print(f"   Max error: {max_err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# 17. cumprod
# =============================================================================
print("17. cumprod...")
folder = Folder("cumprod")
proj.addChild(folder)

# Use small values to avoid overflow
x_small = np.linspace(1.0, 1.5, 20)
n_cp = len(x_small)

ss = Spreadsheet("cumprod")
ss.setColumnCount(4)
ss.setRowCount(n_cp)
folder.addChild(ss)

col_x = ss.column(0)
col_x.setName("x")
col_v = ss.column(1)
col_v.setName("values")
for i in range(n_cp):
    col_x.setValueAt(i, float(i))
    col_v.setValueAt(i, float(x_small[i]))

col_np = ss.column(2)
col_np.setName("numpy")
np_cumprod = np.cumprod(x_small)
for i, v in enumerate(np_cumprod):
    col_np.setValueAt(i, float(v))

col_lp = ss.column(3)
col_lp.setName("labplot")
col_lp.setFormula("cumprod(values)")
col_lp.updateFormula()

max_err = max(abs(col_lp.valueAt(i) - np_cumprod[i]) for i in range(n_cp))
passed = max_err < 1e-10
results.append(("cumprod", max_err, passed))
print(f"   Max error: {max_err:.2e} -> {'PASS' if passed else 'FAIL'}")

# =============================================================================
# Summary
# =============================================================================
print("\n" + "=" * 70)
print("SUMMARY")
print("=" * 70)
print()
passed_count = sum(1 for _, _, p in results if p)
total_count = len(results)
print(f"{'Function':<15} {'Max Error':<15} {'Status'}")
print("-" * 40)
for name, err, passed in results:
    status = "PASS" if passed else "FAIL"
    print(f"{name:<15} {err:<15.2e} {status}")
print("-" * 40)
print(f"\nTotal: {passed_count}/{total_count} tests passed")
print("=" * 70)
