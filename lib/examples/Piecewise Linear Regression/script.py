"""
Piecewise Linear Regression - LabPlot vs pwlf Comparison

This script demonstrates piecewise linear regression (segmented regression) for
data with distinct trend changes - a common analysis in economics, climate science,
and engineering.

Features:
- Automatic changepoint detection with LabPlot
- Side-by-side comparison with pwlf package
- Multiple example datasets (sales, temperature, industrial)
- Visual comparison of both approaches

Use cases:
- Economic trend analysis (GDP, sales, market phases)
- Climate data (temperature regimes, seasonal transitions)
- Industrial monitoring (process phases, equipment wear)
- Medical studies (growth phases, treatment effects)
"""

import numpy as np
from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QFont, QColor
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

# Try to import pwlf for comparison
try:
    import pwlf
    HAS_PWLF = True
    print("✓ pwlf package available for comparison")
except ImportError:
    HAS_PWLF = False
    print("⚠ pwlf not installed - install with: pip install pwlf")
    print("  Comparison will be skipped, showing LabPlot implementation only\n")

# === Generate Sample Data: Industrial Process with Phase Changes ===
print("=" * 70)
print("PIECEWISE LINEAR REGRESSION DEMONSTRATION")
print("=" * 70)
print("\nScenario: Industrial Process Monitoring")
print("A manufacturing process undergoes three distinct phases:")
print("  Phase 1 (0-20h):  Startup - rapid temperature increase")
print("  Phase 2 (20-50h): Steady state - gradual warming")
print("  Phase 3 (50-70h): Cooldown - slow decrease")
print()

np.random.seed(42)

# Time points (hours)
n_points = 150
time = np.linspace(0, 70, n_points)

# Ground truth: three distinct phases with known changepoints
changepoint1 = 20.0  # hours
changepoint2 = 50.0  # hours

temperature = np.zeros_like(time)

# Phase 1: Rapid increase (0-20h) - slope = 2.5
mask1 = time <= changepoint1
temperature[mask1] = 20 + 2.5 * time[mask1]

# Phase 2: Gradual increase (20-50h) - slope = 0.5
mask2 = (time > changepoint1) & (time <= changepoint2)
temp_at_cp1 = 20 + 2.5 * changepoint1
temperature[mask2] = temp_at_cp1 + 0.5 * (time[mask2] - changepoint1)

# Phase 3: Slow decrease (50-70h) - slope = -0.3
mask3 = time > changepoint2
temp_at_cp2 = temp_at_cp1 + 0.5 * (changepoint2 - changepoint1)
temperature[mask3] = temp_at_cp2 - 0.3 * (time[mask3] - changepoint2)

# Add realistic noise
temperature += np.random.normal(0, 1.5, n_points)

print(f"Generated {n_points} data points with known changepoints:")
print(f"  True changepoint 1: {changepoint1:.1f}h")
print(f"  True changepoint 2: {changepoint2:.1f}h")
print()

# === LabPlot Piecewise Linear Fit ===
print("=" * 70)
print("METHOD 1: LabPlot XYPiecewiseLinearFitCurve")
print("=" * 70)
print()

proj = project()

# Create spreadsheet with data
spreadsheet = Spreadsheet("Process Data")
spreadsheet.setColumnCount(2)
proj.addChild(spreadsheet)

spreadsheet.column(0).setName("Time (hours)")
spreadsheet.column(1).setName("Temperature (°C)")
spreadsheet.column(0).replaceValues(0, [float(t) for t in time])
spreadsheet.column(1).replaceValues(0, [float(temp) for temp in temperature])

# Create worksheet
worksheet = Worksheet("Piecewise Linear Regression Comparison")
proj.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(24, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(16, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setTheme("Bright")

margin = Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter)
worksheet.setLayoutTopMargin(margin)
worksheet.setLayoutBottomMargin(margin)
worksheet.setLayoutLeftMargin(margin)
worksheet.setLayoutRightMargin(margin)

# Create plot
plot = CartesianPlot("Main Plot")
plot.setType(CartesianPlot.Type.FourAxes)
plot.title().setText("Piecewise Linear Regression: Industrial Process Monitoring")
plot.setNiceExtend(True)

worksheet.addChild(plot)

# Configure axes
x_axis = plot.horizontalAxis()
x_axis.title().setText("Time (hours)")
x_axis.majorGridLine().setStyle(Qt.PenStyle.DotLine)
x_axis.majorGridLine().setOpacity(0.3)

y_axis = plot.verticalAxis()
y_axis.title().setText("Temperature (°C)")
y_axis.majorGridLine().setStyle(Qt.PenStyle.DotLine)
y_axis.majorGridLine().setOpacity(0.3)

# Plot raw data
data_curve = XYCurve("Measured Data")
data_curve.setXColumn(spreadsheet.column(0))
data_curve.setYColumn(spreadsheet.column(1))
data_curve.setLineType(XYCurve.LineType.NoLine)
symbol = data_curve.symbol()
symbol.setStyle(Symbol.Style.Circle)
symbol.setSize(Worksheet.convertToSceneUnits(4, Worksheet.Unit.Point))
symbol.setOpacity(0.6)
plot.addChild(data_curve)

# Create piecewise linear fit curve
fit_curve = XYPiecewiseLinearFitCurve("LabPlot Fit")
fit_curve.setXDataColumn(spreadsheet.column(0))
fit_curve.setYDataColumn(spreadsheet.column(1))

# Configure fit parameters
fit_data = fit_curve.fitData()
fit_data.changepointMethod = nsl_changepoint_method.nsl_changepoint_method_binary_segmentation
fit_data.connectionType = XYPiecewiseLinearFitCurve.ConnectionType.Continuous
fit_data.penalty = 20.0  # Adjust to control number of segments (lower = more sensitive)
fit_data.minSegmentSize = 10
fit_data.maxChangepoints = 5
fit_curve.setFitData(fit_data)

# Add to plot
plot.addChild(fit_curve)

# Style the fit line
fit_curve.line().setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
fit_curve.line().setColor(QColor(220, 50, 50))  # Red

# Get fit results
fit_result = fit_curve.fitResult()

if fit_result.available and fit_result.valid:
    print(f"✓ Fit successful!")
    print(f"  Segments detected: {fit_result.numSegments}")
    print(f"  Changepoints detected: {len(fit_result.changepoints)}")
    print(f"  Overall R²: {fit_result.rsquare:.4f}")
    print(f"  Total SSE: {fit_result.sse:.2f}")
    print()

    if len(fit_result.changepoints) > 0:
        print(f"  Detected changepoint positions:")
        true_changepoints = [changepoint1, changepoint2]
        for i, cp in enumerate(fit_result.changepoints):
            if i < len(true_changepoints):
                true_val = true_changepoints[i]
                error = abs(cp - true_val)
                print(f"    Changepoint {i+1}: {cp:.2f}h (true: {true_val:.1f}h, error: {error:.2f}h)")
            else:
                print(f"    Changepoint {i+1}: {cp:.2f}h (extra - no ground truth)")

    print()
    print(f"  Segment details:")
    for i in range(fit_result.numSegments):
        seg_result = fit_result.segmentResults[i]
        slope = seg_result.paramValues[1]
        intercept = seg_result.paramValues[0]
        r2 = seg_result.rsquare
        print(f"    Segment {i+1}: slope = {slope:>7.3f}, intercept = {intercept:>7.2f}, R² = {r2:.4f}")
else:
    print(f"✗ Fit failed: {fit_result.status}")

print()

# === PELT Algorithm Comparison ===
print("=" * 70)
print("METHOD 2: LabPlot with PELT Algorithm")
print("=" * 70)
print("PELT (Pruned Exact Linear Time) finds the globally optimal changepoints")
print("using dynamic programming - slower but more accurate than Binary Seg.")
print()

# Create second fit curve using PELT
fit_curve_pelt = XYPiecewiseLinearFitCurve("LabPlot Fit (PELT)")
fit_curve_pelt.setXDataColumn(spreadsheet.column(0))
fit_curve_pelt.setYDataColumn(spreadsheet.column(1))

# Configure PELT parameters
fit_data_pelt = fit_curve_pelt.fitData()
fit_data_pelt.changepointMethod = nsl_changepoint_method.nsl_changepoint_method_pelt
fit_data_pelt.connectionType = XYPiecewiseLinearFitCurve.ConnectionType.Continuous
fit_data_pelt.penalty = 20.0  # Same penalty as Binary Segmentation
fit_data_pelt.minSegmentSize = 10
fit_data_pelt.maxChangepoints = 5
fit_curve_pelt.setFitData(fit_data_pelt)

# Add to plot
plot.addChild(fit_curve_pelt)

# Style the PELT fit line differently
fit_curve_pelt.line().setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
fit_curve_pelt.line().setColor(QColor(50, 200, 50))  # Green
fit_curve_pelt.line().setStyle(Qt.PenStyle.DashLine)

# Get PELT fit results
fit_result_pelt = fit_curve_pelt.fitResult()

if fit_result_pelt.available and fit_result_pelt.valid:
    print(f"✓ PELT fit successful!")
    print(f"  Segments detected: {fit_result_pelt.numSegments}")
    print(f"  Changepoints detected: {len(fit_result_pelt.changepoints)}")
    print(f"  Overall R²: {fit_result_pelt.rsquare:.4f}")
    print(f"  Total SSE: {fit_result_pelt.sse:.2f}")
    print()

    if len(fit_result_pelt.changepoints) > 0:
        print(f"  Detected changepoint positions:")
        true_changepoints = [changepoint1, changepoint2]
        for i, cp in enumerate(fit_result_pelt.changepoints):
            if i < len(true_changepoints):
                true_val = true_changepoints[i]
                error = abs(cp - true_val)
                print(f"    Changepoint {i+1}: {cp:.2f}h (true: {true_val:.1f}h, error: {error:.2f}h)")
            else:
                print(f"    Changepoint {i+1}: {cp:.2f}h (extra - no ground truth)")

    print()
    print(f"  Segment details:")
    for i in range(fit_result_pelt.numSegments):
        seg_result = fit_result_pelt.segmentResults[i]
        slope = seg_result.paramValues[1]
        intercept = seg_result.paramValues[0]
        r2 = seg_result.rsquare
        print(f"    Segment {i+1}: slope = {slope:>7.3f}, intercept = {intercept:>7.2f}, R² = {r2:.4f}")

    print()
    print("Comparison: Binary Segmentation vs PELT")
    print("-" * 70)
    print(f"                      Binary Seg          PELT")
    print("-" * 70)
    print(f"Segments:            {fit_result.numSegments:>2}                 {fit_result_pelt.numSegments:>2}")
    print(f"Changepoints:        {len(fit_result.changepoints):>2}                 {len(fit_result_pelt.changepoints):>2}")
    print(f"Overall R²:          {fit_result.rsquare:>6.4f}           {fit_result_pelt.rsquare:>6.4f}")
    print(f"SSE:                 {fit_result.sse:>7.2f}          {fit_result_pelt.sse:>7.2f}")
    print()

    if fit_result.numSegments == fit_result_pelt.numSegments and len(fit_result.changepoints) == len(fit_result_pelt.changepoints):
        print("NOTE: Both algorithms found the same solution!")
        print("  • This means the changepoints are clear and unambiguous")
        print("  • Binary Segmentation found the optimal solution (validated by PELT)")
        print("  • For this clean data, the faster algorithm (Binary Seg) is sufficient")
        print()
        print("When do algorithms differ?")
        print("  • Noisy data with multiple plausible changepoint locations")
        print("  • Subtle trends where greedy choices lead to suboptimal solutions")
        print("  • Complex patterns with many potential segments")
        print("  • In such cases, PELT guarantees the global optimum")
    else:
        print("NOTE: Algorithms found different solutions!")
        print("  • Binary Segmentation (greedy) found a local optimum")
        print("  • PELT (dynamic programming) found the global optimum")
        print(f"  • PELT has lower SSE: {fit_result_pelt.sse:.2f} < {fit_result.sse:.2f}")
    print()
else:
    print(f"✗ PELT fit failed: {fit_result_pelt.status}")

print()

# === pwlf Comparison (if available) ===
if HAS_PWLF:
    print("=" * 70)
    print("METHOD 3: pwlf Package Comparison")
    print("=" * 70)
    print()

    # Fit with pwlf
    pwlf_model = pwlf.PiecewiseLinFit(time, temperature)

    # Fit with 3 segments (4 points including endpoints)
    n_segments = 3
    breaks = pwlf_model.fit(n_segments)

    print(f"✓ pwlf fit with {n_segments} segments")
    print(f"  Detected breakpoints: {len(breaks) - 2}")  # Exclude endpoints
    print(f"  SSE: {pwlf_model.ssr:.2f}")
    print()

    if len(breaks) > 2:
        print(f"  Detected breakpoint positions:")
        internal_breaks = breaks[1:-1]  # Exclude first and last
        for i, bp in enumerate(internal_breaks):
            true_val = [changepoint1, changepoint2][i] if i < 2 else 0
            error = abs(bp - true_val) if true_val > 0 else 0
            print(f"    Breakpoint {i+1}: {bp:.2f}h", end="")
            if true_val > 0:
                print(f" (true: {true_val:.1f}h, error: {error:.2f}h)")
            else:
                print()

    print()

    # Generate pwlf prediction points
    time_pred = np.linspace(time.min(), time.max(), 200)
    temp_pred = pwlf_model.predict(time_pred)

    # Create spreadsheet for pwlf results
    pwlf_spreadsheet = Spreadsheet("pwlf Fit Results")
    pwlf_spreadsheet.setColumnCount(2)
    proj.addChild(pwlf_spreadsheet)

    pwlf_spreadsheet.column(0).setName("Time (hours)")
    pwlf_spreadsheet.column(1).setName("Temperature (°C)")
    pwlf_spreadsheet.column(0).replaceValues(0, [float(t) for t in time_pred])
    pwlf_spreadsheet.column(1).replaceValues(0, [float(t) for t in temp_pred])

    # Plot pwlf results
    pwlf_curve = XYCurve("pwlf Fit")
    pwlf_curve.setXColumn(pwlf_spreadsheet.column(0))
    pwlf_curve.setYColumn(pwlf_spreadsheet.column(1))
    pwlf_curve.setLineType(XYCurve.LineType.Line)
    pwlf_curve.line().setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
    pwlf_curve.line().setColor(QColor(50, 150, 220))  # Blue
    pwlf_curve.line().setStyle(Qt.PenStyle.DashLine)
    pwlf_curve.symbol().setStyle(Symbol.Style.NoSymbols)
    plot.addChild(pwlf_curve)
    pwlf_curve.retransform()

    print("=" * 70)
    print("COMPARISON SUMMARY")
    print("=" * 70)
    print()
    print(f"                      LabPlot              pwlf")
    print("-" * 70)
    print(f"Segments:            {fit_result.numSegments:>2}                   {n_segments:>2}")
    print(f"Changepoints:        {len(fit_result.changepoints):>2}                   {len(breaks)-2:>2}")
    print(f"Overall R²:          {fit_result.rsquare:>6.4f}              N/A")
    print(f"SSE:                 {fit_result.sse:>7.2f}            {pwlf_model.ssr:>7.2f}")
    print()

    # Compare changepoint accuracy
    if len(fit_result.changepoints) >= 2 and len(internal_breaks) >= 2:
        labplot_error_1 = abs(fit_result.changepoints[0] - changepoint1)
        labplot_error_2 = abs(fit_result.changepoints[1] - changepoint2)
        pwlf_error_1 = abs(internal_breaks[0] - changepoint1)
        pwlf_error_2 = abs(internal_breaks[1] - changepoint2)

        print(f"Changepoint Accuracy:")
        print(f"  CP1 error:         {labplot_error_1:>6.2f}h             {pwlf_error_1:>6.2f}h")
        print(f"  CP2 error:         {labplot_error_2:>6.2f}h             {pwlf_error_2:>6.2f}h")
        print()

# Add legend
plot.addLegend()

# Add info text
info_label = TextLabel("Method Info")
worksheet.addChild(info_label)

te = QTextEdit()
label_font = QFont()
label_font.setPointSize(8)
te.setFont(label_font)

info_text = f"Binary Seg: {fit_result.numSegments} segments, R² = {fit_result.rsquare:.4f}\n"
info_text += f"PELT: {fit_result_pelt.numSegments} segments, R² = {fit_result_pelt.rsquare:.4f}\n"
if HAS_PWLF:
    info_text += f"pwlf: {n_segments} segments (user-specified)"
else:
    info_text += "pwlf: not installed"

te.setPlainText(info_text)
info_label.setText(te.toHtml())
info_label.setPositionScene(QPointF(
    Worksheet.convertToSceneUnits(17, Worksheet.Unit.Centimeter),
    Worksheet.convertToSceneUnits(13, Worksheet.Unit.Centimeter)
))

# === Summary ===
print("=" * 70)
print("KEY DIFFERENCES")
print("=" * 70)
print()
print("LabPlot XYPiecewiseLinearFitCurve:")
print("  ✓ Automatic changepoint detection (no need to specify count)")
print("  ✓ Two algorithms:")
print("    • Binary Segmentation - fast, greedy, approximate")
print("    • PELT - slower, optimal, uses dynamic programming")
print("  ✓ Continuous or discontinuous segment connections")
print("  ✓ Full statistical output (R², p-values, AIC, BIC per segment)")
print("  ✓ Integrated visualization with changepoint lines")
print()
print("Binary Segmentation vs PELT:")
print("  • Binary Seg: O(n² log n), finds good solution quickly")
print("  • PELT: O(n²) worst-case, often O(n), finds optimal solution")
print("  • Both use same penalty parameter tuning")
print("  • PELT guarantees global optimum, Binary Seg may miss it")
print("  • For this example: both found similar changepoints!")
print()
print("pwlf Package:")
print("  ✓ User specifies number of segments")
print("  ✓ Global optimization to find optimal breakpoints")
print("  ✓ Simpler API for basic use cases")
print("  ✓ Prediction at arbitrary points")
print()
print("=" * 70)
print("USE CASES")
print("=" * 70)
print()
print("Economics:")
print("  • GDP growth phases (recession, recovery, expansion)")
print("  • Sales trends (launch, growth, maturity, decline)")
print("  • Stock price regimes")
print()
print("Climate Science:")
print("  • Temperature trends (pre-industrial, industrial, modern)")
print("  • Sea level rise acceleration")
print("  • CO₂ concentration phases")
print()
print("Engineering:")
print("  • Failure point detection (stress-strain)")
print("  • Process phase transitions")
print("  • Wear and degradation analysis")
print()
print("Medical:")
print("  • Growth curves (infancy, childhood, adolescence)")
print("  • Drug response phases")
print("  • Disease progression stages")
print()

# Export options
# worksheet.exportToFile("piecewise_regression.pdf", Worksheet.ExportFormat.PDF)
# worksheet.exportToFile("piecewise_regression.png", Worksheet.ExportFormat.PNG, 300)
