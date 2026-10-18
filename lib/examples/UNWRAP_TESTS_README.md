# Unwrap Function Test Scripts

This directory contains Python scripts for testing and comparing the `unwrap` function between NumPy and LabPlot.

## Scripts

### 1. `test_unwrap_reference.py`
**Purpose:** Reference test cases for validation

**What it does:**
- Provides 10+ test cases with known inputs and expected outputs
- Documents edge cases and algorithm details
- Validates NumPy's unwrap behavior
- Can be used to verify LabPlot's nsl_math_unwrap implementation

**Usage:**
```bash
python3 test_unwrap_reference.py
```

**Output:** Console output with all test cases and implementation notes

---

### 2. `test_unwrap_comparison.py`
**Purpose:** Full comparison with visualization

**What it does:**
- Generates realistic wrapped phase data
- Tests both NumPy and LabPlot unwrap implementations
- Calculates error metrics
- Creates comparison plots (saved as `unwrap_comparison.png`)
- Tests edge cases

**Usage:**
```bash
python3 test_unwrap_comparison.py
```

**Output:**
- Console statistics comparing implementations
- `unwrap_comparison.png` - three-panel comparison plot

**Requirements:**
- numpy
- matplotlib
- pylabplot (LabPlot Python bindings)

---

### 3. `test_unwrap_simple.py`
**Purpose:** Simple LabPlot usage demonstration

**What it does:**
- Creates a LabPlot project with spreadsheet
- Generates and unwraps phase data
- Creates visualization plot
- Shows how to use unwrap in LabPlot Python API

**Usage:**
```bash
labplot --script test_unwrap_simple.py
```

**Output:**
- LabPlot project with spreadsheet and plot
- Console log of operations

---

## Test Data

All scripts use realistic phase data that simulates:
- FFT/DFT phase responses
- Phase from atan2 calculations
- Wrapped angles from sensor measurements

The wrapped phase intentionally contains 2π discontinuities that must be corrected.

## Expected Results

### Validation Criteria
✅ **Pass:** NumPy and LabPlot results match within machine precision (~1e-15)
✅ **Pass:** Unwrapped phase is continuous (no jumps > discont)
✅ **Pass:** First element unchanged
✅ **Pass:** Edge cases handled correctly

### Example Output
```
=== Unwrap Comparison Results ===
NumPy max error:     2.220446e-16
NumPy mean error:    1.234567e-16
LabPlot max error:   2.220446e-16
LabPlot mean error:  1.234567e-16

✓ Results match within tolerance (1e-10)
```

## Algorithm Reference

The unwrap algorithm (matching NumPy):

```python
def unwrap(phase, discont=π):
    result = phase.copy()
    cumsum = 0.0

    for i in range(1, len(phase)):
        diff = phase[i] - phase[i-1]

        # Wrap diff to [-π, π]
        diff_mod = (diff + π) % (2*π) - π
        if diff_mod == -π and diff > 0:
            diff_mod = π

        # Calculate correction
        correction = diff_mod - diff
        if abs(diff) < discont:
            correction = 0.0

        cumsum += correction
        result[i] += cumsum

    return result
```

## TODO

- [ ] Add Python binding to `nsl_math_unwrap` C++ function
- [ ] Expose unwrap as column operation in LabPlot GUI
- [ ] Add unwrap to XY analysis curves (like XYHilbertTransformCurve)
- [ ] Support custom discontinuity threshold in GUI

## Related Files

- **C++ Implementation:** `src/backend/nsl/nsl_math.{h,c}`
- **Function Registration:** `src/backend/gsl/functions.{h,cpp}`
- **Tests:** `tests/nsl/math/UnwrapTest.{h,cpp}`
- **Documentation:** `doc/nsl_math_unwrap.md`

## Contact

For questions about these scripts or the unwrap implementation, see:
- NumPy documentation: https://numpy.org/doc/stable/reference/generated/numpy.unwrap.html
- LabPlot documentation: (upcoming)
