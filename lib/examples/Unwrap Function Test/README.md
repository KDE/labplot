# Unwrap Function Test

This example demonstrates and validates the phase unwrapping function, comparing NumPy's `np.unwrap()` with LabPlot's implementation.

## What is Phase Unwrapping?

Phase unwrapping corrects 2π discontinuities in wrapped phase data to produce continuous phase values. This is essential in:

- **Signal Processing**: FFT/DFT phase spectra
- **Control Systems**: Phase-locked loops (PLL)
- **Optics**: Interferometry measurements
- **Communications**: Complex signal analysis

## What This Example Does

1. **Generates Test Data**: Creates phase data with intentional 2π wraps
2. **Compares Implementations**: Tests NumPy unwrap vs manual implementation
3. **Validates Results**: Checks accuracy against true continuous phase
4. **Visualizes**: Creates comparison plots showing:
   - Wrapped vs unwrapped phase
   - Error analysis on log scale
   - Summary statistics table

## Running the Example

### In LabPlot GUI
1. Open LabPlot
2. Go to **Project → Import → Python Script**
3. Select `script.py` from this directory
4. View the generated plots and data

### From Command Line
```bash
# Run directly
python3 main.py

# Or with LabPlot
labplot --script script.py
```

## Expected Output

### Console Output
```
================================================================================
UNWRAP FUNCTION COMPARISON TEST
================================================================================

Test Case 1: Linear Phase Ramp
  Points: 100
  Wrapped range: [-3.142, 3.142]
  True range: [0.000, 8.000]
  Discontinuities: 4

Validation:
  NumPy vs True:   max error = 2.220446e-16
  Manual vs True:  max error = 2.220446e-16
  NumPy vs Manual: max error = 0.000000e+00

  ✓ Implementations match (within 1e-10 tolerance)

[... additional test cases ...]

================================================================================
SUMMARY
================================================================================

Test Results:
  ✓ 4 test cases passed
  ✓ NumPy and manual implementations match
  ✓ Maximum error: 2.22e-16 (machine precision)
```

### Visual Output
- **Top Plot**: Comparison showing:
  - Wrapped phase (blue circles) - jumps at ±π
  - True phase (black dashed) - continuous reference
  - NumPy unwrapped (red) - NumPy's np.unwrap result
  - Manual unwrapped (green dotted) - Algorithm implementation
  
- **Bottom Plot**: Error analysis on logarithmic scale showing machine precision errors

## Test Cases Included

1. **Linear Phase Ramp**: 100 points with multiple 2π wraps
2. **No Unwrap Needed**: Small phase steps, no correction required
3. **Discontinuity at π**: Actual wrap from π to -π
4. **Single Element**: Edge case with one data point

## Algorithm

The unwrap algorithm (matching NumPy):

```python
for i in range(1, n):
    diff = phase[i] - phase[i-1]
    diff_wrapped = wrap_to_pi(diff)
    
    if abs(diff) >= discont:
        correction = diff_wrapped - diff
        cumsum += correction
    
    phase[i] += cumsum
```

## Files

- `script.py` - Main LabPlot script with test cases
- `main.py` - Standalone runner
- `README.md` - This file

## Implementation Status

✅ **C++ Implementation**: `nsl_math_unwrap()` in `src/backend/nsl/nsl_math.{h,c}`
⏳ **Python Binding**: TODO - direct Python access to nsl_math_unwrap
⏳ **GUI Integration**: TODO - column operation and XYUnwrapCurve

## Related Examples

- **Statistical Comparison** - Demonstrates data analysis workflow
- **Basic Plots** - Shows spreadsheet and plotting basics

## Learn More

- NumPy documentation: https://numpy.org/doc/stable/reference/generated/numpy.unwrap.html
- Phase unwrapping theory: https://en.wikipedia.org/wiki/Phase_unwrapping
