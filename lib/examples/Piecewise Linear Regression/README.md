# Piecewise Linear Regression

This tutorial demonstrates **piecewise linear regression** (also known as segmented regression or broken-line regression) - a powerful technique for analyzing data with distinct trend changes.

## What You'll Learn

- Automatic changepoint detection with LabPlot
- Comparison with the popular `pwlf` Python package
- Understanding different changepoint algorithms
- When to use continuous vs. discontinuous fits
- Interpreting statistical output (R², p-values, AIC/BIC)
- Real-world applications across multiple domains

## The Problem

**Many real-world datasets don't follow a single linear trend!**

Consider:
- **Economic data**: A company's sales might show rapid growth during launch, steady growth at maturity, then decline
- **Climate data**: Global temperature trends show different rates before and after industrialization
- **Industrial processes**: Manufacturing equipment operates through distinct phases (startup, steady-state, cooldown)
- **Medical data**: Human growth follows different rates (infancy, childhood, adolescence)

**Fitting a single line to such data masks the underlying phase changes.** Piecewise linear regression identifies where trends change and fits separate lines to each segment.

## What Is Piecewise Linear Regression?

Piecewise linear regression fits **multiple connected line segments** to data, where:
1. **Changepoints** (or breakpoints) mark where the trend changes
2. Each **segment** between changepoints has its own slope and intercept
3. Segments can be **continuous** (meet at changepoints) or **discontinuous** (independent)

### Example: Industrial Temperature Monitoring
```
Temperature (°C)
  80 │         Phase 3: Cooldown (slope = -0.3)
  70 │              ╱────────╲
  60 │         ╱────         ╲────
  50 │    Phase 2: Steady    ╲
  40 │  ╱─────  (slope = 0.5) ╲
  30 │ ╱                        ╲
  20 │╱ Phase 1: Startup
     │  (slope = 2.5)
     └────────────────────────────────→ Time (hours)
     0        20        50        70
          ↑ CP1       ↑ CP2
```

## LabPlot vs pwlf: Two Approaches

### LabPlot: Automatic Detection
```python
# Let LabPlot find changepoints automatically
fit_curve = XYPiecewiseLinearFitCurve("Auto Fit")
fit_data = fit_curve.fitData()
fit_data.changepointMethod = 0  # Binary Segmentation
fit_data.penalty = 50.0  # Controls sensitivity
fit_curve.setFitData(fit_data)
```

**Advantages:**
- No need to specify number of segments upfront
- Algorithmic changepoint detection (Binary Segmentation or PELT)
- Full statistical output per segment
- Integrated with LabPlot's visualization system

### pwlf: User-Specified Segments
```python
import pwlf

# User specifies 3 segments
model = pwlf.PiecewiseLinFit(x, y)
breaks = model.fit(3)  # Returns optimal breakpoint locations
```

**Advantages:**
- Simple API when you know the segment count
- Global optimization finds best breakpoints for given count
- Lightweight standalone library
- Easy prediction at new points

## The Changepoint Detection Problem

**Finding changepoints is computationally challenging** - testing all possible combinations is exponential in complexity!

### LabPlot's Algorithms

#### 1. Binary Segmentation (Faster, Greedy)
- **How it works**: Recursively finds the best single changepoint, splits data, repeats
- **Complexity**: O(n² log n)
- **Best for**: Quick detection, moderate number of changepoints
- **Limitation**: May miss optimal global solution

#### 2. PELT (Slower, Optimal)
- **How it works**: Dynamic programming with pruning, finds globally optimal set
- **Complexity**: O(n²) worst case, often O(n) in practice
- **Best for**: Finding true optimal changepoints
- **Limitation**: Slower on very large datasets

### pwlf's Approach
- **How it works**: Differential Evolution (global optimization)
- **User specifies**: Number of segments desired
- **Algorithm finds**: Best breakpoint positions for that count
- **Tradeoff**: Must know segment count in advance

## Continuous vs. Discontinuous Fitting

### Continuous Mode (Default)
Segments **must meet** at changepoints - each segment is constrained to pass through the endpoint of the previous segment.

```
     ╱────
    ╱     ╲────
   ╱           ╲
  ╱             ╲
```

**Use when:**
- Physical continuity is expected (temperature, altitude, cumulative metrics)
- Trend changes are smooth transitions
- Prevents unrealistic jumps

### Discontinuous Mode
Segments are fit **independently** - gaps allowed at changepoints.

```
     ╱────
    ╱     
   ╱      ─────
  ╱             ╲
```

**Use when:**
- Data has genuine jumps (policy changes, equipment failures)
- Comparing independent regimes
- Testing if continuity assumption is valid

## Understanding the Output

### Per-Segment Statistics
For each line segment, LabPlot provides:

| Metric | Meaning | Interpretation |
|--------|---------|----------------|
| **Slope ± SE** | Rate of change with uncertainty | How steep the trend is |
| **t-value, p-value** | Statistical significance | Is the slope really non-zero? |
| **R²** | Goodness of fit | How well does the line fit this segment? |
| **DoF** | Degrees of freedom | Sample size - parameters |
| **AIC, BIC** | Information criteria | Lower = better fit (penalized for complexity) |

### Overall Statistics
- **Overall R²**: How well all segments together explain the data
- **Total SSE**: Sum of squared errors across all segments
- **Number of segments**: How many distinct phases detected

### Interpreting p-values
- **p < 0.001**: Very strong evidence of a trend
- **p < 0.01**: Strong evidence
- **p < 0.05**: Moderate evidence (traditional threshold)
- **p > 0.05**: Weak evidence (trend might be noise)

## Example Output

```
LabPlot XYPiecewiseLinearFitCurve
✓ Fit successful!
  Segments detected: 3
  Changepoints detected: 2
  Overall R²: 0.9987
  Total SSE: 251.34

  Detected changepoint positions:
    Changepoint 1: 19.87h (true: 20.0h, error: 0.13h)
    Changepoint 2: 50.14h (true: 50.0h, error: 0.14h)

  Segment details:
    Segment 1: slope =   2.513, intercept =   19.82, R² = 0.9991
    Segment 2: slope =   0.487, intercept =   69.49, R² = 0.9623
    Segment 3: slope =  -0.304, intercept =  109.92, R² = 0.9841
```

## Real-World Use Cases

### Economics & Finance
```python
# Sales lifecycle analysis
x = months_since_launch
y = monthly_revenue
# Detect: launch phase, growth, maturity, decline

# Market regime detection
x = trading_days
y = stock_price
# Detect: bull markets, bear markets, transitions
```

### Climate Science
```python
# Temperature trend analysis
x = year (1850-2024)
y = global_mean_temperature
# Detect: pre-industrial, industrial, modern warming

# Sea level rise
x = year
y = sea_level_mm
# Detect: acceleration points
```

### Engineering & Manufacturing
```python
# Stress-strain curve
x = strain_percent
y = stress_MPa
# Detect: elastic region, yield point, plastic region

# Equipment degradation
x = operating_hours
y = efficiency_percent
# Detect: normal operation, accelerated wear, failure
```

### Medicine & Biology
```python
# Growth curves
x = age_years
y = height_cm
# Detect: infancy, childhood, puberty, adulthood

# Drug response
x = dose_mg
y = response_units
# Detect: subtherapeutic, therapeutic, toxic
```

### Social Sciences
```python
# Population growth
x = year
y = population_millions
# Detect: slow growth, demographic transition, aging

# Learning curves
x = practice_hours
y = test_score
# Detect: initial learning, plateau, mastery
```

## Tips for Good Results

### 1. Choose the Right Algorithm
- **Binary Segmentation**: Start here, works well for most cases
- **PELT**: Use when you need provably optimal changepoints

### 2. Tune the Penalty Parameter
- **Too low**: Over-segmentation (too many changepoints)
- **Too high**: Under-segmentation (misses real changes)
- **Rule of thumb**: Start with penalty ≈ variance of y-values
- **Adjust**: If you get too many/few segments, increase/decrease penalty

### 3. Set Minimum Segment Size
- Prevents detecting tiny segments (likely noise)
- Typical: 5-10% of total data points
- Depends on: noise level, data resolution

### 4. Validate Results
- Do detected changepoints make **domain sense**?
- Are segments **statistically significant** (check p-values)?
- Compare **AIC/BIC** - lower is better
- Try **both continuous and discontinuous** - which makes sense?

### 5. Visual Inspection
- Always plot the fit!
- Do segments follow data well?
- Are changepoints at reasonable locations?
- Any obvious missed transitions?

## Common Pitfalls

### Over-fitting
**Problem**: Too many segments fit noise, not real trends

**Signs**:
- Many short segments
- Similar slopes in adjacent segments
- High segment count but only marginal R² improvement

**Solutions**:
- Increase penalty parameter
- Increase minimum segment size
- Check if some "changepoints" are really just noise

### Under-fitting
**Problem**: Missing real trend changes

**Signs**:
- Low R² despite obvious phases in plot
- One long segment spans multiple regimes
- Large residuals in specific regions

**Solutions**:
- Decrease penalty parameter
- Check data for outliers skewing detection
- Try different algorithm

### False Changepoints at Outliers
**Problem**: Single outliers detected as changepoints

**Solutions**:
- Clean outliers before fitting
- Increase minimum segment size
- Increase penalty
- Visual inspection to identify spurious points

## Comparison Summary

| Feature | LabPlot | pwlf |
|---------|---------|------|
| **Changepoint Detection** | Automatic | Manual (specify count) |
| **Algorithm** | Binary Seg / PELT | Differential Evolution |
| **User Input Required** | Penalty parameter | Number of segments |
| **Statistical Output** | Full (per-segment) | Basic (overall) |
| **Visualization** | Integrated | External plotting |
| **Speed** | Fast | Moderate |
| **Optimal Solution** | PELT: yes, BinSeg: approximate | For given segment count |
| **Continuous/Discontinuous** | Both options | Continuous only |
| **Best For** | Exploratory analysis, unknown phases | Known number of phases |

## When to Use Each

### Use LabPlot when:
- ✓ You don't know how many segments to expect
- ✓ You want full statistical analysis per segment
- ✓ You need integrated visualization
- ✓ You want to compare continuous vs. discontinuous fits
- ✓ You're exploring the data structure

### Use pwlf when:
- ✓ You know the number of segments
- ✓ You want a simple standalone solution
- ✓ You need predictions at arbitrary points
- ✓ You're implementing in a pipeline
- ✓ LabPlot is not available

### Use both when:
- ✓ Cross-validating results
- ✓ Exploring different segment counts
- ✓ Writing a methods comparison paper
- ✓ Teaching/demonstrating the approach

## Running the Example

```bash
# Run with LabPlot Python SDK
python3 main.py

# Or if you want to compare with pwlf:
pip install pwlf
python3 main.py
```

The script will:
1. Generate synthetic industrial process data with 3 phases
2. Detect changepoints automatically with LabPlot
3. (If pwlf installed) Compare with pwlf's approach
4. Create an annotated visualization
5. Print detailed comparison statistics

## Further Reading

### LabPlot Documentation
- XYPiecewiseLinearFitCurve API reference
- Binary Segmentation algorithm
- PELT algorithm

### Academic Background
- **Killick et al. (2012)**: "Optimal Detection of Changepoints" (PELT algorithm)
- **Scott & Knott (1974)**: "A Cluster Analysis Method" (Binary Segmentation)

### Related Packages
- **pwlf**: Piecewise linear fit library (Python)
- **ruptures**: General changepoint detection (Python)
- **segmented**: Segmented regression (R)
- **strucchange**: Structural change tests (R)

## License

This example is part of LabPlot and follows the project's GPL-2.0-or-later license.
