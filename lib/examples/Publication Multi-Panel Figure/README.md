# Publication-Ready Multi-Panel Figures

This tutorial demonstrates how to create professional, publication-quality figures with multiple panels suitable for scientific journals.

## What You'll Learn

- Creating multi-panel layouts (2×2, 2×3, etc.)
- Consistent styling across all panels
- Adding panel labels (a), (b), (c), (d)
- Different plot types in one figure
- High-DPI export for journals (300+ DPI)
- Proper sizing for journal requirements

## The Result

A complete "Figure 1" with four panels showing:
- **(a)** Time series data with scatter plot
- **(b)** Dose-response curve with sigmoid fit (log scale X-axis)
- **(c)** Bar chart with error bars comparing treatment groups
- **(d)** Scatter plot with linear regression and correlation

## Why This Matters

**Every scientific paper needs multi-panel figures!** Journals require:
- Multiple related plots in one figure
- Consistent styling (fonts, colors, sizes)
- Proper panel labels for reference in text
- Specific dimensions (single column ≈ 8.5 cm, double column ≈ 17 cm)
- High resolution (≥300 DPI for print)

This script automates the entire process, ensuring consistency and saving hours of manual work.

## Key Features Demonstrated

### Layout Control
```python
worksheet.setLayout(Worksheet.Layout.GridLayout)
worksheet.setLayoutRowCount(2)
worksheet.setLayoutColumnCount(2)
```

### Panel Labels
Automatically positioned TextLabels with bold font:
```python
label_texts = ['(a)', '(b)', '(c)', '(d)']
# Positioned at top-left of each panel
```

### Multiple Plot Types
- Line plots with symbols (time series)
- Scatter plots with log scale (dose-response)
- Bar charts with error bars (grouped data)
- Scatter with regression line (correlation)

### High-Quality Export
```python
# PDF for print journals
worksheet.exportToFile("Figure1.pdf", Worksheet.ExportFormat.PDF)

# PNG at 300 DPI for online submissions
worksheet.exportToFile("Figure1.png", Worksheet.ExportFormat.PNG, 300)

# SVG for infinite scalability
worksheet.exportToFile("Figure1.svg", Worksheet.ExportFormat.SVG)
```

## Usage

1. **Run the script** - It generates synthetic data for demonstration
2. **Customize for your data** - Replace the data generation section with your actual data import
3. **Adjust layout** - Change row/column counts for different arrangements
4. **Export** - Uncomment the export lines at the end

## Customization Tips

### Change Layout to 1×3 (horizontal strip)
```python
worksheet.setLayoutRowCount(1)
worksheet.setLayoutColumnCount(3)
```

### Adjust Figure Size for Single Column
```python
w = Worksheet.convertToSceneUnits(8.5, Worksheet.Unit.Centimeter)
h = Worksheet.convertToSceneUnits(12, Worksheet.Unit.Centimeter)
```

### Add Statistical Annotations
Use `TextLabel` to add p-values, R², or other stats directly on plots:
```python
stat_label = TextLabel("R² = 0.92, p < 0.001")
plot_d.addChild(stat_label)
```

### Consistent Colors Across Panels
Define colors once and reuse:
```python
color1 = "#1f77b4"  # Blue
color2 = "#ff7f0e"  # Orange
curve_a.line().setColor(color1)
bar_c.borderLineAt(0).setColor(color1)
```

## Journal Requirements

Common specifications:
- **Nature**: 89 mm (single) or 183 mm (double) width, 300 DPI minimum
- **Science**: 5.5 cm (single) or 12 cm (double) width, CMYK or RGB
- **PLOS**: 8.30 cm or 17.35 cm width, 300-600 DPI
- **Cell**: 8.5 cm or 17.4 cm width, RGB, 300 DPI minimum

This script uses standard double-column width (17 cm) that works for most journals.

## Real-World Applications

Replace the synthetic data with your actual experiments:

### Example: Drug Study
- Panel A: Cell viability over time
- Panel B: Dose-response relationship
- Panel C: Comparison across cell lines
- Panel D: Drug A vs Drug B correlation

### Example: Materials Science
- Panel A: XRD pattern
- Panel B: TEM image
- Panel C: Particle size distribution
- Panel D: Mechanical properties

### Example: Climate Data
- Panel A: Temperature time series
- Panel B: Precipitation histogram
- Panel C: Regional comparison
- Panel D: Temperature vs CO₂ correlation

## Tips for Publication Success

1. **Use vector formats** (PDF, SVG) when possible - they scale perfectly
2. **Check journal guidelines** before finalizing dimensions
3. **Maintain consistent fonts** across all panels (usually Arial or Helvetica)
4. **Keep panel labels bold** and positioned consistently
5. **Test print** your figure at actual size before submission
6. **Use colorblind-friendly palettes** for accessibility

## Related Examples

- See `../Demo/script.py` for data import and fitting basics
- See `../Basic Plots/script.py` for more plot type examples
- See `../Tufte's Minimal Ink Design/script.py` for minimalist publication style
