"""
Statistical Comparison with Box Plots

This script demonstrates a complete workflow for comparing multiple datasets
with statistical analysis and visualization - a common task in experimental
science, clinical trials, and data analysis.

Workflow:
1. Import data from multiple CSV files (or generate synthetic data)
2. Compute descriptive statistics for each group
3. Create box plots with individual data points
4. Create summary statistics table
5. Export publication-ready figure and statistics

Use cases:
- Comparing treatment groups in experiments
- A/B testing analysis
- Quality control across batches
- Regional or temporal comparisons
"""

import numpy as np
import sys
from PySide6.QtCore import QRectF, QPointF
from PySide6.QtGui import QFont
from PySide6.QtWidgets import QTextEdit
from pylabplot import *

# === Generate Sample Data ===
# In real use, import from CSV files with:
# filter = AsciiFilter()
# filter.readDataFromFile("group1.csv", spreadsheet)

np.random.seed(42)

# Simulate experimental data: 4 groups with different distributions
groups = ['Control', 'Treatment A', 'Treatment B', 'Treatment C']

# Generate realistic experimental data (e.g., cell viability %)
data_dict = {
	'Control': np.random.normal(50, 8, 25),
	'Treatment A': np.random.normal(65, 10, 28),
	'Treatment B': np.random.normal(75, 9, 26),
	'Treatment C': np.random.normal(82, 7, 30)
}

# === Compute Statistics ===
def compute_stats(data):
	"""Calculate descriptive statistics for a dataset"""
	return {
		'n': len(data),
		'mean': np.mean(data),
		'median': np.median(data),
		'std': np.std(data, ddof=1),  # Sample std
		'sem': np.std(data, ddof=1) / np.sqrt(len(data)),  # Standard error
		'min': np.min(data),
		'max': np.max(data),
		'q25': np.percentile(data, 25),
		'q75': np.percentile(data, 75),
		'iqr': np.percentile(data, 75) - np.percentile(data, 25)
	}

stats_dict = {group: compute_stats(data) for group, data in data_dict.items()}

# === Setup Project ===
proj = project()

# === Create Spreadsheets for Each Group ===
spreadsheets = {}
for group_name, data in data_dict.items():
	ss = Spreadsheet(f"{group_name} Data")
	ss.setColumnCount(1)  # Single value column per group
	proj.addChild(ss)

	# Fill data column
	ss.column(0).setName("Value")
	ss.column(0).replaceValues(0, [float(x) for x in data])

	spreadsheets[group_name] = ss

# === Create Statistics Summary Spreadsheet ===
stats_spreadsheet = Spreadsheet("Summary Statistics")
stats_spreadsheet.setColumnCount(11)  # 11 columns for all statistics
proj.addChild(stats_spreadsheet)

# Setup columns
col_names = ['Group', 'N', 'Mean', 'Median', 'Std Dev', 'SEM', 'Min', 'Max', 'Q25', 'Q75', 'IQR']
for i, name in enumerate(col_names):
	stats_spreadsheet.column(i).setName(name)

# Fill statistics table
for row_idx, group_name in enumerate(groups):
	stats = stats_dict[group_name]

	# Group name (as index)
	stats_spreadsheet.column(0).replaceValues(row_idx, [float(row_idx + 1)])

	# Statistics
	stats_values = [
		stats['n'],
		stats['mean'],
		stats['median'],
		stats['std'],
		stats['sem'],
		stats['min'],
		stats['max'],
		stats['q25'],
		stats['q75'],
		stats['iqr']
	]

	for col_idx, value in enumerate(stats_values, start=1):
		stats_spreadsheet.column(col_idx).replaceValues(row_idx, [float(value)])

# === Create Worksheet ===
worksheet = Worksheet("Statistical Comparison")
proj.addChild(worksheet)

worksheet.setUseViewSize(False)
w = Worksheet.convertToSceneUnits(30, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
worksheet.setPageRect(QRectF(0, 0, w, h))

worksheet.setTheme("Bright")

# Vertical layout: box plot on top, statistics table below
worksheet.setLayout(Worksheet.Layout.VerticalLayout)
margin = Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter)
worksheet.setLayoutTopMargin(margin)
worksheet.setLayoutBottomMargin(margin)
worksheet.setLayoutLeftMargin(margin)
worksheet.setLayoutRightMargin(margin)
worksheet.setLayoutVerticalSpacing(Worksheet.convertToSceneUnits(1.0, Worksheet.Unit.Centimeter))

# === Create Box Plot ===
plot = CartesianPlot("Box Plot Comparison")
plot.setType(CartesianPlot.Type.FourAxes)
plot.title().setText("Treatment Group Comparison")
plot.setNiceExtend(True)  # Auto-extend ranges for cleaner appearance

worksheet.addChild(plot)

# Configure axes - do this AFTER adding to worksheet
x_axis = plot.horizontalAxis()
x_axis.title().setText("Treatment Group")
x_axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
x_axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

y_axis = plot.verticalAxis()
y_axis.title().setText("Response (%)")
y_axis.majorGridLine().setStyle(Qt.PenStyle.NoPen)
y_axis.minorGridLine().setStyle(Qt.PenStyle.NoPen)

# Create box plot with all groups
boxplot = BoxPlot("Group Comparison")

# Set data columns (one per group)
data_columns = [spreadsheets[group].column(0) for group in groups]
boxplot.setDataColumns(data_columns)

# Customize box plot appearance
boxplot.setOrientation(BoxPlot.Orientation.Vertical)
boxplot.setWhiskersType(BoxPlot.WhiskersType.IQR)
boxplot.setWhiskersRangeParameter(1.5)  # Standard 1.5×IQR rule
boxplot.setNotchesEnabled(True)  # Show confidence interval around median
boxplot.setVariableWidth(False)  # Equal width boxes

# Show individual data points with jittering
boxplot.symbolData().setStyle(Symbol.Style.Circle)
boxplot.symbolData().setSize(Worksheet.convertToSceneUnits(3, Worksheet.Unit.Point))
boxplot.symbolData().setOpacity(0.4)
boxplot.setJitteringEnabled(True)

# Show mean as well as median
boxplot.symbolMean().setStyle(Symbol.Style.Diamond)
boxplot.symbolMean().setSize(Worksheet.convertToSceneUnits(6, Worksheet.Unit.Point))

# Highlight outliers
boxplot.symbolOutlier().setStyle(Symbol.Style.Circle)
boxplot.symbolOutlier().setSize(Worksheet.convertToSceneUnits(4, Worksheet.Unit.Point))

# Enable rug plot for better data distribution visualization
boxplot.setRugEnabled(True)
boxplot.setRugLength(Worksheet.convertToSceneUnits(8, Worksheet.Unit.Point))

plot.addChild(boxplot)
boxplot.recalc()

# Add legend
plot.addLegend()

# === Create Statistics Table as Text ===
# Create formatted statistics table as child of plot (bottom right corner)
table_label = TextLabel("Statistics Table")
plot.addChild(table_label)

# Build HTML table
#  font-family: Arial; font-size: 6pt;
html_table = "<html><body><table border='1' cellpadding='3' cellspacing='0' style='border-collapse: collapse;'>"
html_table += "<tr style='background-color: #e0e0e0; font-weight: bold;'>"
html_table += "<th>Group</th><th>N</th><th>Mean ± SEM</th><th>Median</th><th>Std Dev</th><th>Range</th></tr>"

for group_name in groups:
	stats = stats_dict[group_name]
	html_table += "<tr>"
	html_table += f"<td>{group_name}</td>"
	html_table += f"<td>{stats['n']}</td>"
	html_table += f"<td>{stats['mean']:.2f} ± {stats['sem']:.2f}</td>"
	html_table += f"<td>{stats['median']:.2f}</td>"
	html_table += f"<td>{stats['std']:.2f}</td>"
	html_table += f"<td>{stats['min']:.1f} - {stats['max']:.1f}</td>"
	html_table += "</tr>"

html_table += "</table></body></html>"

table_label.setText(html_table)

# Position in bottom right corner of plot using logical coordinates
table_label.setCoordinateBindingEnabled(True)
# Get plot data range to position in bottom right
rangeX = plot.range(CartesianCoordinateSystem.Dimension.X, 0)
rangeY = plot.range(CartesianCoordinateSystem.Dimension.Y, 0)
# Position at 95% of X range and 5% of Y range (bottom right)
table_label.setPositionLogical(QPointF(rangeX.end() * 0.95, rangeY.start() + (rangeY.end() - rangeY.start()) * 0.05))
table_label.setHorizontalAlignment(WorksheetElement.HorizontalAlignment.Right)
table_label.setVerticalAlignment(WorksheetElement.VerticalAlignment.Bottom)

# === Print Statistics Summary ===
print("=" * 80)
print("STATISTICAL COMPARISON ANALYSIS")
print("=" * 80)
print()

for group_name in groups:
	stats = stats_dict[group_name]
	print(f"{group_name}:")
	print(f"  N = {stats['n']}")
	print(f"  Mean ± SEM = {stats['mean']:.2f} ± {stats['sem']:.2f}")
	print(f"  Median (IQR) = {stats['median']:.2f} ({stats['q25']:.2f} - {stats['q75']:.2f})")
	print(f"  Std Dev = {stats['std']:.2f}")
	print(f"  Range = {stats['min']:.2f} - {stats['max']:.2f}")
	print()

print("=" * 80)
print("INTERPRETATION GUIDE")
print("=" * 80)
print()
print("Box Plot Elements:")
print("  • Box = Interquartile range (IQR, 25th to 75th percentile)")
print("  • Line in box = Median")
print("  • Diamond symbol = Mean")
print("  • Notches = 95% confidence interval around median")
print("  • Whiskers = 1.5 × IQR (standard Tukey method)")
print("  • Individual points = Raw data with jitter for visibility")
print("  • Circles beyond whiskers = Outliers")
print("  • Rug marks at bottom = Data density")
print()
print("Statistical Notes:")
print("  • Use SEM (Standard Error of Mean) for comparing means")
print("  • Use Std Dev for describing variability within groups")
print("  • Notched boxes: non-overlapping notches suggest significant difference")
print("  • For formal testing, use ANOVA or Kruskal-Wallis test")
print()
print("=" * 80)
print("NEXT STEPS")
print("=" * 80)
print()
print("1. Visual Assessment:")
print("   - Do the boxes overlap? (suggests similarity)")
print("   - Do the notches overlap? (suggests no significant difference)")
print("   - Are outliers present? (check data quality)")
print()
print("2. Statistical Testing:")
print("   - Run ANOVA for normally distributed data")
print("   - Use Kruskal-Wallis for non-normal data")
print("   - Perform post-hoc tests (Tukey HSD, Dunn's test)")
print()
print("3. Export:")
print("   - Save figure: worksheet.exportToFile('comparison.pdf', ...)")
print("   - Save statistics: export stats_spreadsheet to CSV")
print()

# Optional: Export
# worksheet.exportToFile("statistical_comparison.pdf", Worksheet.ExportFormat.PDF)
# worksheet.exportToFile("statistical_comparison.png", Worksheet.ExportFormat.PNG, 300)
