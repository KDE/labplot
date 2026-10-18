#!/usr/bin/env python3
"""
Test cases for unwrap function validation

This script provides reference test cases that can be used to validate
LabPlot's nsl_math_unwrap implementation against NumPy's np.unwrap.
"""

import numpy as np

def print_test_case(name, input_data, expected_output, description=""):
    """Print a test case in a clear format"""
    print(f"\n{'=' * 60}")
    print(f"Test Case: {name}")
    print(f"{'=' * 60}")
    if description:
        print(f"Description: {description}")
    print(f"\nInput:    {input_data}")
    print(f"Expected: {expected_output}")
    print(f"Match: {np.allclose(np.unwrap(input_data), expected_output)}")


def main():
    print("=" * 60)
    print("NumPy unwrap() Reference Test Cases")
    print("For validating LabPlot nsl_math_unwrap implementation")
    print("=" * 60)

    # Test 1: Basic wrapping at π
    test1_input = np.array([0.0, np.pi, 2.0 * np.pi, 3.0 * np.pi])
    test1_output = np.unwrap(test1_input)
    print_test_case(
        "Basic Sequential",
        test1_input,
        test1_output,
        "Sequential values with no actual discontinuities when wrapped to [-π, π]"
    )

    # Test 2: Actual discontinuity correction
    test2_input = np.array([0.0, np.pi - 0.1, -np.pi + 0.1, -np.pi + 0.2])
    test2_output = np.unwrap(test2_input)
    print_test_case(
        "Discontinuity Correction",
        test2_input,
        test2_output,
        "Phase wraps from π to -π, should unwrap to continuous"
    )

    # Test 3: No unwrapping needed (small steps)
    test3_input = np.array([0.0, 0.5, 1.0, 1.5, 2.0])
    test3_output = np.unwrap(test3_input)
    print_test_case(
        "No Unwrap Needed",
        test3_input,
        test3_output,
        "All steps < π, no correction needed"
    )

    # Test 4: Edge case - single element
    test4_input = np.array([2.5])
    test4_output = np.unwrap(test4_input)
    print_test_case(
        "Single Element",
        test4_input,
        test4_output,
        "Single element should remain unchanged"
    )

    # Test 5: Negative wrapping
    test5_input = np.array([0.0, -np.pi + 0.1, np.pi - 0.1, np.pi - 0.2])
    test5_output = np.unwrap(test5_input)
    print_test_case(
        "Negative Wrap",
        test5_input,
        test5_output,
        "Phase wraps from -π to π, should unwrap downward"
    )

    # Test 6: Multiple wraps
    test6_input = np.array([0, 3.1, -3.0, 3.2, -2.9])
    test6_output = np.unwrap(test6_input)
    print_test_case(
        "Multiple Wraps",
        test6_input,
        test6_output,
        "Multiple 2π jumps in both directions"
    )

    # Test 7: Exact ±π values
    test7_input = np.array([0.0, np.pi, -np.pi, 0.0])
    test7_output = np.unwrap(test7_input)
    print_test_case(
        "Exact ±π Values",
        test7_input,
        test7_output,
        "Values exactly at ±π boundaries"
    )

    # Test 8: Real-world scenario - FFT phase
    print(f"\n{'=' * 60}")
    print("Test Case: FFT Phase Spectrum")
    print(f"{'=' * 60}")
    print("Description: Simulated phase response from FFT")

    # Simulate phase from a frequency response
    freq = np.linspace(0, 10, 100)
    phase_continuous = -0.5 * freq  # Group delay
    phase_wrapped = np.arctan2(np.sin(phase_continuous), np.cos(phase_continuous))
    phase_unwrapped = np.unwrap(phase_wrapped)

    print(f"\nSamples: {len(phase_wrapped)}")
    print(f"Wrapped range: [{np.min(phase_wrapped):.3f}, {np.max(phase_wrapped):.3f}]")
    print(f"Unwrapped range: [{np.min(phase_unwrapped):.3f}, {np.max(phase_unwrapped):.3f}]")
    print(f"Number of discontinuities detected: {np.sum(np.abs(np.diff(phase_wrapped)) > np.pi)}")
    print(f"Recovery accuracy: {np.max(np.abs(phase_unwrapped - phase_continuous)):.6e}")

    # Test 9: Custom discontinuity threshold
    print(f"\n{'=' * 60}")
    print("Test Case: Custom Discontinuity Threshold")
    print(f"{'=' * 60}")
    print("Description: Using discont=π/2 instead of default π")

    test9_input = np.array([0.0, 0.8, -0.7, 0.9])
    test9_output_default = np.unwrap(test9_input)
    test9_output_custom = np.unwrap(test9_input, discont=np.pi/2)

    print(f"\nInput: {test9_input}")
    print(f"Output (discont=π):   {test9_output_default}")
    print(f"Output (discont=π/2): {test9_output_custom}")
    print("Note: With smaller discont, more jumps are corrected")

    # Test 10: Zero crossing
    print(f"\n{'=' * 60}")
    print("Test Case: Zero Crossing")
    print(f"{'=' * 60}")
    print("Description: Phase oscillating around zero")

    test10_input = np.array([0.5, -0.5, 0.6, -0.6, 0.7])
    test10_output = np.unwrap(test10_input)
    print(f"\nInput:    {test10_input}")
    print(f"Expected: {test10_output}")
    print("Note: No unwrapping since all jumps < π")

    # Summary statistics
    print(f"\n{'=' * 60}")
    print("Implementation Notes")
    print(f"{'=' * 60}")
    print("""
1. Algorithm: Cumulative correction approach
   - Calculate diff between consecutive points
   - Wrap diff to [-π, π]
   - If abs(diff) < discont: no correction
   - Accumulate corrections and add to original values

2. Edge cases to handle:
   - Empty array (n=0): return immediately
   - Single element (n=1): return unchanged
   - Exact ±π values: handle the diff_mod == -π && diff > 0 case

3. Default parameters:
   - discont = π (matches NumPy default)

4. In-place operation:
   - Original array is modified
   - First element never changes
   - Only subsequent elements accumulate corrections

5. Precision:
   - Use double precision (64-bit float)
   - NumPy and LabPlot results should match to machine precision (~1e-15)
    """)

    print("\n" + "=" * 60)
    print("Test cases completed")
    print("=" * 60)


if __name__ == "__main__":
    main()
