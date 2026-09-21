# Automated Peak Detection and Annotation

This tutorial demonstrates automated peak finding and labeling in spectroscopic or chromatographic data - one of the most requested features in analytical chemistry software.

## What You'll Learn

- Automatic peak detection using scipy.signal
- Peak annotation with TextLabels
- Reference lines at peak positions
- Customizable detection parameters
- Exporting peak lists for further analysis

## The Problem

**Manual peak identification is tedious and error-prone!**

In analytical chemistry and materials science, you often need to:
1. Identify all peaks in a spectrum or chromatogram
2. Measure peak positions, heights, and widths
3. Label peaks for presentation/publication
4. Export peak data for database comparison

Doing this manually for dozens of spectra is time-consuming. **This script automates the entire workflow.**

## What It Does

### 1. Automatic Peak Detection
Uses `scipy.signal.find_peaks()` with configurable parameters:
- **Prominence**: Minimum height above baseline (filters noise)
- **Distance**: Minimum separation between peaks (merges shoulders)
- **Height**: Absolute minimum peak intensity

### 2. Peak Information Extraction
For each detected peak:
- Position (x-coordinate)
- Intensity (y-coordinate)
- Prominence (height above surrounding baseline)
- Width (FWHM - full width at half maximum)

### 3. Automatic Annotation
- Marks peaks with symbols
- Adds vertical reference lines
- Labels top N peaks with positions
- Creates exportable peak table

## Example: XRD Pattern Analysis

The demo analyzes an X-ray diffraction pattern showing:
- **Input**: 2θ angle vs intensity data
- **Output**: Annotated pattern with labeled peaks
- **Use**: Phase identification, crystallite size, lattice parameters

## Use Cases by Field

### Spectroscopy
```python
# Raman Spectroscopy
x = wavenumber  # cm⁻¹
y = intensity
# Find characteristic peaks for compound identification

# IR Spectroscopy
x = wavenumber  # cm⁻¹
y = transmittance  # %
# Identify functional groups

# UV-Vis
x = wavelength  # nm
y = absorbance
# Find absorption maxima
```

### Chromatography
```python
# HPLC/GC
x = retention_time  # minutes
y = absorbance or intensity
# Quantify compounds, calculate areas

# Mass Spectrometry
x = mz_ratio  # m/z
y = abundance  # %
# Identify molecular fragments
```

### Materials Science
```python
# XRD (this example)
x = two_theta  # degrees
y = intensity  # counts
# Phase identification, crystallinity

# XPS
x = binding_energy  # eV
y = counts
# Element identification
```

### Signal Processing
```python
# ECG/EEG
x = time  # seconds
y = voltage  # mV
# Detect R-peaks, spikes

# Audio
x = frequency  # Hz
y = amplitude
# Harmonic analysis
```

## Customizing Peak Detection

### Finding More Peaks (Lower Threshold)
```python
prominence_threshold = 50  # Lower value = more peaks
```

### Finding Fewer Peaks (Higher Threshold)
```python
prominence_threshold = 500  # Higher value = only major peaks
```

### Separate Close Peaks
```python
distance_threshold = 10  # Smaller = distinguish close peaks
```

### Merge Shoulders
```python
distance_threshold = 100  # Larger = treat shoulders as one peak
```

### Set Absolute Minimum Height
```python
peaks = find_peaks_with_info(
    x, y,
    prominence=200,
    height=100  # Ignore peaks below this intensity
)
```

## Advanced Techniques

### Baseline Correction First
```python
from scipy.signal import savgol_filter

# Smooth data
y_smooth = savgol_filter(y, window_length=51, polyorder=3)

# Estimate baseline (rolling minimum)
from scipy.ndimage import minimum_filter
baseline = minimum_filter(y_smooth, size=100)

# Subtract baseline
y_corrected = y - baseline

# Then find peaks in corrected data
peaks = find_peaks_with_info(x, y_corrected, ...)
```

### Peak Area Integration
```python
from scipy.integrate import simpson

for peak in peaks:
    # Define integration region (±3σ around peak)
    left_idx = peak['index'] - int(3 * peak['width'] / (x[1] - x[0]))
    right_idx = peak['index'] + int(3 * peak['width'] / (x[1] - x[0]))

    # Integrate
    x_region = x[left_idx:right_idx]
    y_region = y[left_idx:right_idx]
    area = simpson(y_region, x_region)

    peak['area'] = area
```

### Peak Fitting (Gaussian, Lorentzian)
```python
from scipy.optimize import curve_fit

def gaussian(x, amp, center, width):
    return amp * np.exp(-(x - center)**2 / (2 * width**2))

for peak in peaks:
    # Extract region around peak
    window = 50
    idx = peak['index']
    x_fit = x[idx-window:idx+window]
    y_fit = y[idx-window:idx+window]

    # Fit Gaussian
    params, _ = curve_fit(
        gaussian, x_fit, y_fit,
        p0=[peak['height'], peak['x_pos'], peak['width']]
    )

    peak['fitted_center'] = params[1]
    peak['fitted_width'] = params[2]
```

### Multi-Component Deconvolution
```python
# For overlapping peaks, fit sum of Gaussians
def multi_gaussian(x, *params):
    # params = [amp1, center1, width1, amp2, center2, width2, ...]
    y = np.zeros_like(x)
    for i in range(0, len(params), 3):
        y += gaussian(x, params[i], params[i+1], params[i+2])
    return y

# Initial guess from detected peaks
p0 = []
for peak in peaks:
    p0.extend([peak['height'], peak['x_pos'], peak['width']])

# Fit all peaks simultaneously
params_fitted, _ = curve_fit(multi_gaussian, x, y, p0=p0)
```

## Exporting Peak Data

### To Spreadsheet/CSV
```python
# The peaks_spreadsheet contains all detected peaks
# Export via LabPlot's UI or:
# Save as CSV for database comparison, literature matching
```

### To Text Report
```python
with open('peak_report.txt', 'w') as f:
    f.write("Peak Detection Report\n")
    f.write("=" * 50 + "\n\n")
    for i, peak in enumerate(peaks, 1):
        f.write(f"Peak {i}:\n")
        f.write(f"  Position: {peak['x_pos']:.2f}\n")
        f.write(f"  Intensity: {peak['y_pos']:.1f}\n")
        f.write(f"  Prominence: {peak['prominence']:.1f}\n")
        f.write(f"  Width: {peak['width']:.3f}\n\n")
```

## Comparison with Competitors

**OriginLab** - Peak Analyzer tool (commercial, $1000+)
**PeakFit** - Dedicated peak fitting software (commercial, $500+)
**Igor Pro** - Peak finding procedures (commercial, $700+)

**LabPlot + This Script** - FREE and open source! 🎉

## Validation and Quality Control

### Check for False Positives
```python
# Visual inspection: do marked peaks look real?
# Compare with known reference patterns
# Check peak widths (too narrow = noise spike)
```

### Check for Missed Peaks
```python
# Lower prominence threshold gradually
# Visually inspect spectrum for unmarked features
```

### Reproducibility
```python
# Run detection multiple times with same parameters
# Should give identical results (deterministic algorithm)
```

## Best Practices

### Data Preprocessing
1. **Remove spikes** (cosmic rays in Raman, electrical noise)
2. **Smooth if noisy** (Savitzky-Golay filter)
3. **Correct baseline** (polynomial, rolling minimum)
4. **Normalize** (to internal standard or max peak)

### Parameter Selection
1. **Start conservative** (high prominence, large distance)
2. **Lower thresholds gradually** until false positives appear
3. **Document final parameters** for reproducibility
4. **Use same parameters** across sample series

### Annotation Strategy
1. **Label major peaks** only (top 5-10 by prominence)
2. **Use consistent formatting** (font, color, position)
3. **Include units** (degrees, cm⁻¹, ppm)
4. **Reference literature** values if identifying phases

## Troubleshooting

### "Too many peaks detected (noise)"
→ Increase `prominence_threshold`
→ Smooth data first (`savgol_filter`)
→ Apply baseline correction

### "Missing real peaks"
→ Decrease `prominence_threshold`
→ Decrease `distance_threshold`
→ Check for baseline drift

### "Peaks too close together"
→ Increase `distance_threshold`
→ Or use peak deconvolution for genuine overlap

### "Labels overlap"
→ Reduce `n_labels` (only top N peaks)
→ Adjust label positions manually
→ Use leader lines/arrows

## Related Examples

- See `../NIST - Linear Regression/script.py` for calibration curves from peak areas
- See `../Publication Multi-Panel Figure/script.py` for including annotated spectra in figures

## Further Reading

- scipy.signal.find_peaks documentation
- "Automated Spectral Peak Finding" - Applied Spectroscopy reviews
- NIST Chemistry WebBook for reference peak positions
- International Centre for Diffraction Data (ICDD) for XRD patterns
