"""
Automated Peak Detection and Annotation

This script demonstrates automated peak finding and labeling in spectroscopic
or chromatographic data - a common task in analytical chemistry, materials
science, and signal processing.

Features:
- Automatic peak detection with prominence threshold
- Peak annotation with TextLabels and arrows
- Customizable detection parameters
- Works with any signal: XRD, Raman, IR, HPLC, mass spec, etc.

Use cases:
- X-ray diffraction (XRD) pattern analysis
- Raman/IR spectroscopy peak identification
- Chromatography (HPLC, GC) peak labeling
- Mass spectrometry peak annotation
- Any 1D signal with peaks!
"""

import numpy as np
from scipy import signal
from PySide6.QtCore import QRectF, QPointF, Qt
from PySide6.QtGui import QFont
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

# === Peak Detection Function ===
def find_peaks_with_info(x, y, prominence=None, distance=None, height=None):
	"""
	Find peaks in signal using scipy.signal.find_peaks

	Parameters:
	-----------
	x : array
		X-axis values (e.g., wavelength, retention time, 2-theta)
	y : array
		Y-axis values (intensity, absorbance, counts)
	prominence : float
		Minimum prominence (height above surrounding baseline)
	distance : int
		Minimum distance between peaks (in samples)
	height : float
		Minimum peak height

	Returns:
	--------
	peak_info : list of dict
		Each dict contains: index, x_pos, y_pos, prominence, width
	"""
	# Find peaks
	if prominence is None:
		prominence = 0.1 * (np.max(y) - np.min(y))

	peaks, properties = signal.find_peaks(
		y,
		prominence=prominence,
		distance=distance,
		height=height,
		width=1  # Also calculate peak widths
	)

	# Extract peak information
	peak_info = []
	for i, peak_idx in enumerate(peaks):
		info = {
			'index': peak_idx,
			'x_pos': x[peak_idx],
			'y_pos': y[peak_idx],
			'prominence': properties['prominences'][i],
			'width': properties['widths'][i] * (x[1] - x[0]) if len(x) > 1 else 0,
			'height': y[peak_idx]
		}
		peak_info.append(info)

	return peak_info

# === Generate Sample XRD Pattern ===
# Simulated X-ray diffraction pattern with multiple peaks
np.random.seed(42)

# X-axis: 2-theta angle (degrees)
two_theta = np.linspace(10, 80, 2000)

# Background (polynomial baseline)
baseline = 50 + 0.1 * two_theta + 0.001 * two_theta**2

# Add multiple Gaussian peaks at different positions
peak_positions = [22.5, 31.8, 38.4, 45.6, 48.2, 56.8, 62.3, 68.5, 74.2]
peak_heights = [800, 1200, 600, 950, 400, 700, 500, 350, 450]
peak_widths = [0.8, 0.6, 0.7, 0.5, 0.6, 0.7, 0.8, 0.6, 0.7]

intensity = baseline.copy()
for pos, height, width in zip(peak_positions, peak_heights, peak_widths):
	intensity += height * np.exp(-((two_theta - pos) / width)**2)

# Add noise
intensity += np.random.normal(0, 20, len(intensity))

# Ensure non-negative
intensity = np.maximum(intensity, 0)

# === Detect Peaks ===
# Adjust prominence threshold to find significant peaks only
prominence_threshold = 200  # Minimum peak prominence
distance_threshold = 50     # Minimum distance between peaks (in samples)

peaks = find_peaks_with_info(
	two_theta,
	intensity,
	prominence=prominence_threshold,
	distance=distance_threshold
)

print("=" * 70)
print("AUTOMATED PEAK DETECTION")
print("=" * 70)
print(f"\nFound {len(peaks)} peaks in the spectrum\n")
print(f"{'Peak #':<8} {'Position':<12} {'Intensity':<12} {'Prominence':<12} {'Width':<10}")
print("-" * 70)
for i, peak in enumerate(peaks, 1):
	print(f"{i:<8} {peak['x_pos']:>10.2f}° {peak['y_pos']:>11.1f} {peak['prominence']:>11.1f} {peak['width']:>9.3f}°")
print()

# === Setup Project ===
proj = project()

# Create spreadsheet
spreadsheet = Spreadsheet("XRD Pattern Data")
spreadsheet.setColumnCount(2)  # x and y data
proj.addChild(spreadsheet)

# Fill data
spreadsheet.column(0).setName("2θ (degrees)")
spreadsheet.column(1).setName("Intensity (counts)")
spreadsheet.column(0).replaceValues(0, [float(x) for x in two_theta])
spreadsheet.column(1).replaceValues(0, [float(x) for x in intensity])

# Create peak positions spreadsheet
peaks_spreadsheet = Spreadsheet("Detected Peaks")
peaks_spreadsheet.setColumnCount(3)  # position, intensity, prominence
proj.addChild(peaks_spreadsheet)

peaks_spreadsheet.column(0).setName("2θ (degrees)")
peaks_spreadsheet.column(1).setName("Intensity")
peaks_spreadsheet.column(2).setName("Prominence")

peak_x = [peak['x_pos'] for peak in peaks]
peak_y = [peak['y_pos'] for peak in peaks]
peak_prom = [peak['prominence'] for peak in peaks]

peaks_spreadsheet.column(0).replaceValues(0, [float(x) for x in peak_x])
peaks_spreadsheet.column(1).replaceValues(0, [float(x) for x in peak_y])
peaks_spreadsheet.column(2).replaceValues(0, [float(x) for x in peak_prom])

# === Create Worksheet ===
worksheet = Worksheet("XRD Pattern with Peak Labels")
proj.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(22, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(14, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setTheme("Bright")

margin = Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter)
worksheet.setLayoutTopMargin(margin)
worksheet.setLayoutBottomMargin(margin)
worksheet.setLayoutLeftMargin(margin)
worksheet.setLayoutRightMargin(margin)

# === Create Plot ===
plot = CartesianPlot("XRD Pattern")
plot.setType(CartesianPlot.Type.FourAxes)
plot.title().setText("X-Ray Diffraction Pattern with Automatic Peak Detection")
plot.setNiceExtend(True)  # Auto-extend ranges for cleaner appearance

worksheet.addChild(plot)

# Configure axes - do this AFTER adding to worksheet
x_axis = plot.horizontalAxis()
x_axis.title().setText("2θ (degrees)")
x_axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
x_axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

y_axis = plot.verticalAxis()
y_axis.title().setText("Intensity (counts)")
y_axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
y_axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

# === Plot Spectrum ===
curve = XYCurve("XRD Pattern")
curve.setXColumn(spreadsheet.column(0))
curve.setYColumn(spreadsheet.column(1))
curve.setLineType(XYCurve.LineType.Line)
curve.line().setWidth(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Point))
curve.symbol().setStyle(Symbol.Style.NoSymbols)
plot.addChild(curve)

# === Mark Peaks with Symbols ===
peaks_curve = XYCurve("Detected Peaks")
peaks_curve.setXColumn(peaks_spreadsheet.column(0))
peaks_curve.setYColumn(peaks_spreadsheet.column(1))
peaks_curve.setLineType(XYCurve.LineType.NoLine)
symbol = peaks_curve.symbol()
symbol.setStyle(Symbol.Style.Circle)
symbol.setSize(Worksheet.convertToSceneUnits(8, Worksheet.Unit.Point))
pen = symbol.pen()
pen.setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
symbol.setPen(pen)
plot.addChild(peaks_curve)

# === Add Peak Labels ===
# Create TextLabels for each peak
label_font = QFont()
label_font.setPointSize(9)

te = QTextEdit()
te.setFont(label_font)

# Add reference lines and labels for major peaks (top N by prominence)
# Sort peaks by prominence
peaks_sorted = sorted(peaks, key=lambda p: p['prominence'], reverse=True)
n_labels = min(10, len(peaks_sorted))  # Label top 10 peaks

for i, peak in enumerate(peaks_sorted[:n_labels]):
	# Add vertical reference line at peak position
	ref_line = ReferenceLine(plot, f"Peak {i+1}")
	plot.addChild(ref_line)
	ref_line.setOrientation(ReferenceLine.Orientation.Vertical)
	ref_line.setPositionLogical(QPointF(peak['x_pos'], 0))
	ref_line.line().setStyle(Qt.PenStyle.DashLine)
	ref_line.line().setWidth(Worksheet.convertToSceneUnits(0, Worksheet.Unit.Point))
	ref_line.line().setOpacity(0.5)
	ref_line.retransform()  # Make line visible

	# Add text label above peak - as child of plot, using logical coordinates
	label = TextLabel(f"Peak Label {i+1}")
	plot.addChild(label)

	te.clear()
	te.setPlainText(f"{peak['x_pos']:.1f}°")
	label.setText(te.toHtml())

	# Enable coordinate binding and position label above the peak in logical (data) coordinates
	label.setCoordinateBindingEnabled(True)
	label.setPositionLogical(QPointF(peak['x_pos'], peak['y_pos'] * 1.05))  # 5% above peak

# === Add Summary Information ===
info_label = TextLabel("Peak Detection Info")
worksheet.addChild(info_label)

te.clear()
te.setFont(label_font)
te.setPlainText(f"Detected {len(peaks)} peaks\nProminence threshold: {prominence_threshold}\nTop {n_labels} peaks labeled")
info_label.setText(te.toHtml())
info_label.setPositionScene(QPointF(
	Worksheet.convertToSceneUnits(16, Worksheet.Unit.Centimeter),
	Worksheet.convertToSceneUnits(11, Worksheet.Unit.Centimeter)
))

# Add legend
plot.addLegend()

# === Print Summary ===
print("=" * 70)
print("PEAK ANNOTATION COMPLETE")
print("=" * 70)
print()
print(f"✓ Plotted spectrum with {len(two_theta)} data points")
print(f"✓ Detected {len(peaks)} peaks automatically")
print(f"✓ Labeled top {n_labels} peaks by prominence")
print(f"✓ Added reference lines for major peaks")
print()
print("Customization options:")
print("  • Adjust 'prominence_threshold' to find more/fewer peaks")
print("  • Adjust 'distance_threshold' to merge/separate nearby peaks")
print("  • Change 'n_labels' to annotate more/fewer peaks")
print()
print("Export options:")
print("  • worksheet.exportToFile('xrd_pattern.pdf', Worksheet.ExportFormat.PDF)")
print("  • peaks_spreadsheet can be exported to CSV for further analysis")
print()
print("=" * 70)
print("TIP: This workflow works for ANY 1D signal!")
print("=" * 70)
print("Just replace the data with:")
print("  • Raman/IR spectrum (wavenumber vs intensity)")
print("  • HPLC chromatogram (time vs absorbance)")
print("  • Mass spectrum (m/z vs abundance)")
print("  • EEG/ECG signal (time vs voltage)")
print()

# Optional: Export
# worksheet.exportToFile("peak_detection.pdf", Worksheet.ExportFormat.PDF)
# worksheet.exportToFile("peak_detection.png", Worksheet.ExportFormat.PNG, 300)
