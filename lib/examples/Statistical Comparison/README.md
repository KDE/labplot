# Statistical Comparison with Box Plots

This tutorial demonstrates a complete statistical comparison workflow - one of the most common tasks in experimental science, clinical research, and data analysis.

## What You'll Learn

- Importing and organizing multiple datasets
- Computing descriptive statistics (mean, median, std, SEM, etc.)
- Creating publication-quality box plots
- Visualizing individual data points with jittering
- Generating summary statistics tables
- Interpreting box plot elements

## The Complete Workflow

```
Data Import → Statistical Analysis → Visualization → Export
```

### Step 1: Data Import
- Load data from multiple groups/conditions
- Can use CSV files, Excel, or generate synthetic data
- Each group gets its own spreadsheet

### Step 2: Statistical Analysis
Computes for each group:
- **N** - Sample size
- **Mean ± SEM** - Average and standard error
- **Median** - 50th percentile
- **Std Dev** - Standard deviation (variability)
- **IQR** - Interquartile range (25th to 75th percentile)
- **Min/Max** - Range of values

### Step 3: Visualization
Creates a box plot showing:
- **Box** - Interquartile range (middle 50% of data)
- **Line in box** - Median value
- **Diamond** - Mean value
- **Notches** - 95% confidence interval around median
- **Whiskers** - Extend to 1.5 × IQR
- **Individual points** - All raw data (jittered for visibility)
- **Outliers** - Points beyond whiskers
- **Rug marks** - Data density at bottom

### Step 4: Export
- High-resolution figure for publication
- Statistics table as spreadsheet
- Summary statistics in console

## Why Box Plots?

Box plots are **ideal for comparing groups** because they show:

1. **Central tendency** (median and mean)
2. **Spread** (IQR, range)
3. **Skewness** (asymmetric box)
4. **Outliers** (unusual values)
5. **Statistical significance** (via notches)

They're much more informative than just bar charts with error bars!

## Use Cases

### Biological Sciences
```python
# Compare drug treatments
groups = ['Vehicle', 'Drug 10μM', 'Drug 50μM', 'Drug 100μM']
# Response: cell viability, gene expression, etc.
```

### Clinical Research
```python
# Compare patient groups
groups = ['Healthy', 'Stage I', 'Stage II', 'Stage III']
# Response: biomarker levels, symptoms scores, etc.
```

### Quality Control
```python
# Compare production batches
groups = ['Batch A', 'Batch B', 'Batch C', 'Batch D']
# Response: purity, yield, defect rate, etc.
```

### A/B Testing
```python
# Compare website versions
groups = ['Control', 'Variant A', 'Variant B']
# Response: conversion rate, time on page, etc.
```

## Interpreting the Results

### Visual Assessment

**Non-overlapping boxes** → Groups likely different
```
  Control:  [----]
Treatment A:          [-----]
```

**Overlapping notches** → No significant difference
```
  Group 1: [---(===)---]
  Group 2:   [---(===)---]
           Notches overlap
```

**Many outliers** → Check data quality or use robust methods
```
  Group X: [-----] o  o     oo  (many outliers)
```

### Statistical Testing

The box plot is **exploratory** - follow up with formal tests:

**For normally distributed data:**
- **ANOVA** (3+ groups) or **t-test** (2 groups)
- Post-hoc: Tukey HSD, Bonferroni

**For non-normal data:**
- **Kruskal-Wallis** (3+ groups) or **Mann-Whitney** (2 groups)
- Post-hoc: Dunn's test with correction

**Check assumptions:**
```python
# Normality: Shapiro-Wilk test
# Equal variances: Levene's test
```

## Customization

### Import Real Data from CSV
Replace the data generation section:
```python
import pandas as pd

data_dict = {}
for group in ['control', 'treatment_a', 'treatment_b']:
    df = pd.read_csv(f"{group}.csv")
    data_dict[group] = df['value'].values
```

### Change Box Plot Style

**Show only medians (no individual points):**
```python
boxplot.symbolData().setStyle(Symbol.Style.NoSymbols)
boxplot.setJitteringEnabled(False)
```

**Horizontal orientation:**
```python
boxplot.setOrientation(BoxPlot.Orientation.Horizontal)
```

**Colored boxes by group:**
```python
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']
for i, color in enumerate(colors):
    boxplot.backgroundAt(i).setFirstColor(color)
    boxplot.backgroundAt(i).setOpacity(0.3)
```

### Add Statistical Annotations

```python
# Add significance markers (*, **, ***) between groups
# Requires implementing statistical tests first
sig_label = TextLabel("*** p < 0.001")
plot.addChild(sig_label)
# Position above compared groups
```

## Best Practices

### Sample Size
- **N < 10** per group: Show all individual points
- **N = 10-50**: Box plot with jittered points (this example)
- **N > 50**: Box plot without individual points (too crowded)

### Reporting
Always report:
- Sample sizes (N)
- Central tendency (mean or median)
- Variability (SEM or SD)
- Statistical test used and p-values

### Publication
- Use **median ± IQR** for skewed data
- Use **mean ± SEM** for symmetric data
- Report **outliers** but don't automatically exclude them
- Mention **normality tests** if using parametric statistics

## Common Mistakes to Avoid

❌ **Using only mean ± SD without showing distribution**
✅ Show box plots to reveal skewness, outliers, distribution shape

❌ **Excluding outliers without justification**
✅ Report outliers, investigate causes, use robust statistics

❌ **Claiming significance from overlapping notches alone**
✅ Perform proper statistical tests (ANOVA, t-test)

❌ **Bar charts for non-normal data**
✅ Box plots show distribution shape, not just mean

## Related Examples

- See `../Basic Plots/script.py` for basic box plot creation
- See `../Same Data Different Boxplots/script.py` for style variations
- See `../Publication Multi-Panel Figure/script.py` for combining with other plots

## Further Reading

- Tukey, J.W. (1977). "Exploratory Data Analysis"
- McGill et al. (1978). "Variations of Box Plots" (notched boxes)
- Wickham, H. (2009). "ggplot2: Elegant Graphics for Data Analysis"
