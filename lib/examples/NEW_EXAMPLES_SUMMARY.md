# New Python Scripting Tutorial Examples

This document summarizes the four new tutorial examples created for LabPlot's Python scripting feature.

## Overview

These tutorials demonstrate real-world scientific workflows that are commonly requested by users and heavily promoted by competing software (Origin, Igor Pro, GraphPad Prism).

## 1. Quantum Wave Packet Evolution 🌊
**Location:** `lib/examples/Quantum Wave Packet/`

**What it demonstrates:**
- Dynamic data generation and real-time updates
- Animation through column data updates
- Scientific computing with NumPy
- Two variants: free particle dispersion + quantum tunneling

**Use cases:**
- Physics education (quantum mechanics visualization)
- Demonstrating LabPlot's animation capabilities
- Scientific simulations with real-time plotting

**Dependencies:** NumPy, SciPy
**Complexity:** Advanced
**Target audience:** Physics students, researchers, educators

---

## 2. Publication Multi-Panel Figures 📊
**Location:** `lib/examples/Publication Multi-Panel Figure/`

**What it demonstrates:**
- Creating multi-panel layouts (2×2, 2×3 grids)
- Consistent styling across panels
- Panel labels (a), (b), (c), (d)
- Different plot types in one figure
- High-DPI export (300+ DPI)
- Journal-standard dimensions

**Use cases:**
- Creating "Figure 1" for scientific papers
- Presentation slides with multiple plots
- Reports with comparative visualizations
- Any multi-panel figure requirement

**Dependencies:** NumPy (for demo data only)
**Complexity:** Intermediate
**Target audience:** ALL scientists publishing papers

**Why this matters:**
- Most requested workflow for academic users
- Every paper needs multi-panel figures
- Competing software charges $500-1000+ for this
- LabPlot does it FREE

---

## 3. Statistical Comparison with Box Plots 📈
**Location:** `lib/examples/Statistical Comparison/`

**What it demonstrates:**
- Importing multiple datasets
- Computing descriptive statistics (mean, median, SEM, IQR)
- Creating comprehensive box plots
- Individual data points with jittering
- Notched boxes (confidence intervals)
- Statistical summary table generation
- Interpretation guide

**Use cases:**
- Comparing experimental groups (control vs treatments)
- Clinical trials analysis
- Quality control across batches
- A/B testing
- Regional/temporal comparisons

**Dependencies:** NumPy
**Complexity:** Intermediate
**Target audience:** Biologists, medical researchers, QC analysts

**Why this matters:**
- GraphPad Prism's #1 use case
- Standard workflow in biology/medicine
- Shows LabPlot's statistical capabilities
- More informative than bar charts

---

## 4. Peak Detection and Annotation 🔍
**Location:** `lib/examples/Peak Detection/`

**What it demonstrates:**
- Automated peak finding with scipy.signal
- Peak annotation with TextLabels
- Reference lines at peak positions
- Customizable detection parameters
- Peak information extraction (position, prominence, width)
- Exportable peak tables

**Use cases:**
- X-ray diffraction (XRD) pattern analysis
- Raman/IR spectroscopy
- Chromatography (HPLC, GC-MS)
- Mass spectrometry
- Any 1D signal with peaks

**Dependencies:** NumPy, SciPy
**Complexity:** Intermediate
**Target audience:** Analytical chemists, materials scientists

**Why this matters:**
- OriginLab charges $1000+ for Peak Analyzer
- Extremely common in analytical chemistry
- Saves hours of manual work
- LabPlot provides it FREE

---

## Common Features Across All Examples

### Well-Documented
- Comprehensive README.md with:
  - What you'll learn
  - Use cases by field
  - Customization guide
  - Best practices
  - Troubleshooting
  - Literature references

### Multiple Entry Points
- `script.py` - Python implementation for LabPlot
- `main.py` - Standalone Python runner
- `main.cpp` - C++ SDK implementation (when applicable)
- `CMakeLists.txt` - Build configuration

### Production-Ready
- Clean, well-commented code
- Error handling
- Console output with progress/results
- Export instructions
- Follows existing example patterns

### Educational
- Explains the "why" not just the "how"
- Scientific background where relevant
- Comparison with competitors
- Tips for publication/reporting

---

## Integration

All examples are integrated into:
- `lib/examples/CMakeLists.txt` (build system)
- Ready to be featured in documentation
- Can be run directly from LabPlot's Script editor

---

## Impact

### For Users
- **Time savings:** Automate tedious workflows
- **Cost savings:** FREE alternatives to expensive commercial tools
- **Quality:** Publication-ready outputs
- **Learning:** Educational tutorials with scientific context

### For LabPlot
- **Competitive advantage:** Match/exceed Origin, Igor Pro features
- **User adoption:** Address most requested workflows
- **Documentation:** Living examples users can adapt
- **Marketing:** Concrete demonstrations of Python scripting power

---

## Recommended Documentation Flow

1. **docs.labplot.org/en/scripting.html**
   - Add section: "Tutorial Examples"
   - Brief description of each
   - Links to GitHub/source

2. **docs.labplot.org/en/tutorials/** (new section)
   - Dedicated page for each tutorial
   - Screenshots/GIFs of results
   - Step-by-step walkthrough
   - Download links

3. **Landing page highlight**
   - "Featured Examples" section
   - Show visual results
   - "Compare with Origin/Igor" table

---

## Future Extension Ideas

Based on this foundation, easy to add:
- **Data normalization tutorial** (min-max, z-score, reference)
- **Calibration curves** (linear, polynomial, with CI)
- **Batch export** (loop through worksheets)
- **Template-based plotting** (lab standard styles)
- **Time series analysis** (moving average, trend removal)
- **Heatmap generation** (2D data visualization)
- **Error propagation** (uncertainty calculations)
- **Fourier analysis** (FFT, filtering, reconstruction)

---

## Dependencies Summary

All examples use only standard scientific Python packages:
- **NumPy** - Numerical arrays (ubiquitous in science)
- **SciPy** - Scientific algorithms (signal processing, stats)
- **PySide6** - Qt bindings (already required by LabPlot)

No exotic dependencies, all are:
- ✅ Available via pip/conda
- ✅ Included in Anaconda
- ✅ Well-maintained and stable
- ✅ Industry standard

---

## Testing Recommendations

1. **Syntax check:** Run each script.py in LabPlot
2. **Visual check:** Verify plots look correct
3. **Export check:** Test PDF/PNG export
4. **Parameter sweep:** Try different detection thresholds, sizes, etc.
5. **Real data:** Test with actual user data files
6. **Documentation:** Verify README instructions are clear

---

## Success Metrics

How to measure impact:
- Downloads/views of example scripts
- User forum discussions referencing these tutorials
- GitHub issues requesting similar examples
- Citations in publications ("Figures created with LabPlot")
- Feature comparisons favorable vs Origin/Igor

---

## Conclusion

These four tutorials cover the most impactful workflows across multiple scientific disciplines:

1. **Quantum Wave Packet** - Physics education, dynamic visualization
2. **Publication Figures** - Universal need for all scientists
3. **Statistical Comparison** - Biology, medicine, quality control
4. **Peak Detection** - Analytical chemistry, materials science

Together they demonstrate LabPlot's Python scripting as a **powerful, free alternative to expensive commercial software** while providing **practical, ready-to-use solutions** for real scientific work.
