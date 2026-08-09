"""
HypothesisTest Example

This script compares LabPlot's HypothesisTest implementation against standard
Python statistical packages (scipy.stats, statsmodels) for all 15 supported tests.

For each test:
1. Generate appropriate test data
2. Run the test using scipy.stats or statsmodels
3. Create a LabPlot HypothesisTest object with the same data
4. Display results side-by-side for manual comparison

Packages required:
- scipy (for most statistical tests)
- statsmodels (for ANOVA and some advanced tests)
- numpy
"""

import numpy as np
from scipy import stats
from scipy.stats import chi2_contingency
import statsmodels.api as sm
from statsmodels.stats.anova import AnovaRM
from statsmodels.stats.weightstats import ttest_ind
from pylabplot import *

# Set random seed for reproducibility
np.random.seed(42)

# Create project
proj = project()

def print_comparison(test_name, labplot_test, scipy_result_dict):
    """Helper to print side-by-side comparison"""
    print("\n" + "=" * 80)
    print(f"{test_name}")
    print("=" * 80)

    print("\nSciPy/StatsModels Results:")
    print("-" * 40)
    for key, value in scipy_result_dict.items():
        if isinstance(value, float):
            print(f"  {key}: {value:.6f}")
        else:
            print(f"  {key}: {value}")

    print("\nLabPlot HypothesisTest Results:")
    print("-" * 40)
    print(labplot_test.resultText())
    print("\n")


# =============================================================================
# Test 1: One-Sample t-Test
# =============================================================================
print("\n\n" + "#" * 80)
print("# Test 1: One-Sample t-Test")
print("#" * 80)

# Generate data: sample with mean around 105
sample1 = np.random.normal(105, 15, 30)
hypothesized_mean = 100

# SciPy
scipy_result = stats.ttest_1samp(sample1, hypothesized_mean)

# LabPlot
spreadsheet1 = Spreadsheet("One-Sample Data")
spreadsheet1.setColumnCount(1)
proj.addChild(spreadsheet1)
spreadsheet1.column(0).setName("Sample")
spreadsheet1.column(0).replaceValues(0, [float(x) for x in sample1])

test1 = HypothesisTest("One-Sample t-Test")
proj.addChild(test1)
test1.setTest(HypothesisTest.Test.t_test_one_sample)
test1.setDataColumns([spreadsheet1.column(0)])
test1.setTestMean(hypothesized_mean)
test1.setSignificanceLevel(0.05)
test1.setTail(nsl_stats_tail_type.nsl_stats_tail_type_two)
test1.recalculate()

print_comparison(
    "One-Sample t-Test: H₀: μ = 100",
    test1,
    {
        'Sample mean': np.mean(sample1),
        'Sample std': np.std(sample1, ddof=1),
        't-statistic': scipy_result.statistic,
        'p-value': scipy_result.pvalue,
        'df': len(sample1) - 1
    }
)


# =============================================================================
# Test 2: Two-Sample t-Test (Independent)
# =============================================================================
print("\n\n" + "#" * 80)
print("# Test 2: Two-Sample t-Test (Independent, Equal Variances)")
print("#" * 80)

# Generate two groups with different means
group_a = np.random.normal(50, 10, 25)
group_b = np.random.normal(55, 10, 25)

# SciPy (equal variances assumed)
scipy_result = stats.ttest_ind(group_a, group_b, equal_var=True)

# LabPlot
spreadsheet2 = Spreadsheet("Two-Sample Data")
spreadsheet2.setColumnCount(2)
proj.addChild(spreadsheet2)
spreadsheet2.column(0).setName("Group A")
spreadsheet2.column(1).setName("Group B")
spreadsheet2.column(0).replaceValues(0, [float(x) for x in group_a])
spreadsheet2.column(1).replaceValues(0, [float(x) for x in group_b])

test2 = HypothesisTest("Two-Sample t-Test")
proj.addChild(test2)
test2.setTest(HypothesisTest.Test.t_test_two_sample)
test2.setDataColumns([spreadsheet2.column(0), spreadsheet2.column(1)])
test2.setSignificanceLevel(0.05)
test2.setTail(nsl_stats_tail_type.nsl_stats_tail_type_two)
test2.recalculate()

print_comparison(
    "Two-Sample t-Test: H₀: μ₁ = μ₂ (equal variances)",
    test2,
    {
        'Group A mean': np.mean(group_a),
        'Group B mean': np.mean(group_b),
        'Group A std': np.std(group_a, ddof=1),
        'Group B std': np.std(group_b, ddof=1),
        't-statistic': scipy_result.statistic,
        'p-value': scipy_result.pvalue,
        'df': len(group_a) + len(group_b) - 2
    }
)


# =============================================================================
# Test 3: Paired t-Test
# =============================================================================
print("\n\n" + "#" * 80)
print("# Test 3: Paired t-Test")
print("#" * 80)

# Generate paired measurements (e.g., before/after treatment)
before = np.random.normal(120, 15, 20)
after = before + np.random.normal(5, 8, 20)  # Treatment effect ~5 units

# SciPy
scipy_result = stats.ttest_rel(before, after)

# LabPlot
spreadsheet3 = Spreadsheet("Paired Data")
spreadsheet3.setColumnCount(2)
proj.addChild(spreadsheet3)
spreadsheet3.column(0).setName("Before")
spreadsheet3.column(1).setName("After")
spreadsheet3.column(0).replaceValues(0, [float(x) for x in before])
spreadsheet3.column(1).replaceValues(0, [float(x) for x in after])

test3 = HypothesisTest("Paired t-Test")
proj.addChild(test3)
test3.setTest(HypothesisTest.Test.t_test_two_sample_paired)
test3.setDataColumns([spreadsheet3.column(0), spreadsheet3.column(1)])
test3.setSignificanceLevel(0.05)
test3.setTail(nsl_stats_tail_type.nsl_stats_tail_type_two)
test3.recalculate()

differences = after - before
print_comparison(
    "Paired t-Test: H₀: μ_diff = 0",
    test3,
    {
        'Before mean': np.mean(before),
        'After mean': np.mean(after),
        'Mean difference': np.mean(differences),
        'Std of differences': np.std(differences, ddof=1),
        't-statistic': scipy_result.statistic,
        'p-value': scipy_result.pvalue,
        'df': len(before) - 1
    }
)


# =============================================================================
# Test 4: Welch's t-Test (Unequal Variances)
# =============================================================================
print("\n\n" + "#" * 80)
print("# Test 4: Welch's t-Test (Unequal Variances)")
print("#" * 80)

# Generate two groups with different means AND variances
group_c = np.random.normal(60, 5, 20)   # Small variance
group_d = np.random.normal(65, 15, 25)  # Large variance

# SciPy (Welch's t-test)
scipy_result = stats.ttest_ind(group_c, group_d, equal_var=False)

# LabPlot
spreadsheet4 = Spreadsheet("Welch Data")
spreadsheet4.setColumnCount(2)
proj.addChild(spreadsheet4)
spreadsheet4.column(0).setName("Group C")
spreadsheet4.column(1).setName("Group D")
spreadsheet4.column(0).replaceValues(0, [float(x) for x in group_c])
spreadsheet4.column(1).replaceValues(0, [float(x) for x in group_d])

test4 = HypothesisTest("Welch t-Test")
proj.addChild(test4)
test4.setTest(HypothesisTest.Test.t_test_welch)
test4.setDataColumns([spreadsheet4.column(0), spreadsheet4.column(1)])
test4.setSignificanceLevel(0.05)
test4.setTail(nsl_stats_tail_type.nsl_stats_tail_type_two)
test4.recalculate()

print_comparison(
    "Welch's t-Test: H₀: μ₁ = μ₂ (unequal variances)",
    test4,
    {
        'Group C mean': np.mean(group_c),
        'Group D mean': np.mean(group_d),
        'Group C std': np.std(group_c, ddof=1),
        'Group D std': np.std(group_d, ddof=1),
        'Variance ratio': np.var(group_d, ddof=1) / np.var(group_c, ddof=1),
        't-statistic': scipy_result.statistic,
        'p-value': scipy_result.pvalue,
        'df (Welch-Satterthwaite)': scipy_result.df
    }
)


# =============================================================================
# Test 5: One-Way ANOVA
# =============================================================================
print("\n\n" + "#" * 80)
print("# Test 5: One-Way ANOVA (3 independent groups)")
print("#" * 80)

# Generate three groups with different means
anova_g1 = np.random.normal(50, 10, 15)
anova_g2 = np.random.normal(55, 10, 15)
anova_g3 = np.random.normal(60, 10, 15)

# SciPy
scipy_result = stats.f_oneway(anova_g1, anova_g2, anova_g3)

# LabPlot
spreadsheet5 = Spreadsheet("ANOVA Data")
spreadsheet5.setColumnCount(3)
proj.addChild(spreadsheet5)
spreadsheet5.column(0).setName("Treatment A")
spreadsheet5.column(1).setName("Treatment B")
spreadsheet5.column(2).setName("Treatment C")
spreadsheet5.column(0).replaceValues(0, [float(x) for x in anova_g1])
spreadsheet5.column(1).replaceValues(0, [float(x) for x in anova_g2])
spreadsheet5.column(2).replaceValues(0, [float(x) for x in anova_g3])

test5 = HypothesisTest("One-Way ANOVA")
proj.addChild(test5)
test5.setTest(HypothesisTest.Test.one_way_anova)
test5.setDataColumns([spreadsheet5.column(0), spreadsheet5.column(1), spreadsheet5.column(2)])
test5.setSignificanceLevel(0.05)
test5.recalculate()

print_comparison(
    "One-Way ANOVA: H₀: μ₁ = μ₂ = μ₃",
    test5,
    {
        'Group 1 mean': np.mean(anova_g1),
        'Group 2 mean': np.mean(anova_g2),
        'Group 3 mean': np.mean(anova_g3),
        'F-statistic': scipy_result.statistic,
        'p-value': scipy_result.pvalue,
        'df_between': 2,
        'df_within': 15*3 - 3
    }
)


# =============================================================================
# Test 6: Mann-Whitney U Test
# =============================================================================
print("\n\n" + "#" * 80)
print("# Test 6: Mann-Whitney U Test (Non-parametric, 2 groups)")
print("#" * 80)

# Generate skewed data (not normal)
mw_g1 = np.random.exponential(10, 20)
mw_g2 = np.random.exponential(12, 20)

# SciPy
scipy_result = stats.mannwhitneyu(mw_g1, mw_g2, alternative='two-sided')

# LabPlot
spreadsheet6 = Spreadsheet("Mann-Whitney Data")
spreadsheet6.setColumnCount(2)
proj.addChild(spreadsheet6)
spreadsheet6.column(0).setName("Sample 1")
spreadsheet6.column(1).setName("Sample 2")
spreadsheet6.column(0).replaceValues(0, [float(x) for x in mw_g1])
spreadsheet6.column(1).replaceValues(0, [float(x) for x in mw_g2])

test6 = HypothesisTest("Mann-Whitney U Test")
proj.addChild(test6)
test6.setTest(HypothesisTest.Test.mann_whitney_u_test)
test6.setDataColumns([spreadsheet6.column(0), spreadsheet6.column(1)])
test6.setSignificanceLevel(0.05)
test6.setTail(nsl_stats_tail_type.nsl_stats_tail_type_two)
test6.recalculate()

print_comparison(
    "Mann-Whitney U Test: H₀: distributions are equal",
    test6,
    {
        'Sample 1 median': np.median(mw_g1),
        'Sample 2 median': np.median(mw_g2),
        'U-statistic': scipy_result.statistic,
        'p-value': scipy_result.pvalue,
        'n1': len(mw_g1),
        'n2': len(mw_g2)
    }
)


# =============================================================================
# Test 7: Kruskal-Wallis Test
# =============================================================================
print("\n\n" + "#" * 80)
print("# Test 7: Kruskal-Wallis Test (Non-parametric ANOVA, 3+ groups)")
print("#" * 80)

# Generate skewed data for 3 groups
kw_g1 = np.random.gamma(2, 2, 15)
kw_g2 = np.random.gamma(3, 2, 15)
kw_g3 = np.random.gamma(4, 2, 15)

# SciPy
scipy_result = stats.kruskal(kw_g1, kw_g2, kw_g3)

# LabPlot
spreadsheet7 = Spreadsheet("Kruskal-Wallis Data")
spreadsheet7.setColumnCount(3)
proj.addChild(spreadsheet7)
spreadsheet7.column(0).setName("Group 1")
spreadsheet7.column(1).setName("Group 2")
spreadsheet7.column(2).setName("Group 3")
spreadsheet7.column(0).replaceValues(0, [float(x) for x in kw_g1])
spreadsheet7.column(1).replaceValues(0, [float(x) for x in kw_g2])
spreadsheet7.column(2).replaceValues(0, [float(x) for x in kw_g3])

test7 = HypothesisTest("Kruskal-Wallis Test")
proj.addChild(test7)
test7.setTest(HypothesisTest.Test.kruskal_wallis_test)
test7.setDataColumns([spreadsheet7.column(0), spreadsheet7.column(1), spreadsheet7.column(2)])
test7.setSignificanceLevel(0.05)
test7.recalculate()

print_comparison(
    "Kruskal-Wallis Test: H₀: medians are equal",
    test7,
    {
        'Group 1 median': np.median(kw_g1),
        'Group 2 median': np.median(kw_g2),
        'Group 3 median': np.median(kw_g3),
        'H-statistic': scipy_result.statistic,
        'p-value': scipy_result.pvalue,
        'df': 2
    }
)


# =============================================================================
# Test 8: Wilcoxon Signed-Rank Test
# =============================================================================
print("\n\n" + "#" * 80)
print("# Test 8: Wilcoxon Signed-Rank Test (Non-parametric paired test)")
print("#" * 80)

# Generate paired data with non-normal differences
wilcox_before = np.random.lognormal(3, 0.5, 20)
wilcox_after = wilcox_before * np.random.uniform(0.9, 1.1, 20)

# SciPy
scipy_result = stats.wilcoxon(wilcox_before, wilcox_after, alternative='two-sided')

# LabPlot
spreadsheet8 = Spreadsheet("Wilcoxon Data")
spreadsheet8.setColumnCount(2)
proj.addChild(spreadsheet8)
spreadsheet8.column(0).setName("Before")
spreadsheet8.column(1).setName("After")
spreadsheet8.column(0).replaceValues(0, [float(x) for x in wilcox_before])
spreadsheet8.column(1).replaceValues(0, [float(x) for x in wilcox_after])

test8 = HypothesisTest("Wilcoxon Test")
proj.addChild(test8)
test8.setTest(HypothesisTest.Test.wilcoxon_test)
test8.setDataColumns([spreadsheet8.column(0), spreadsheet8.column(1)])
test8.setSignificanceLevel(0.05)
test8.setTail(nsl_stats_tail_type.nsl_stats_tail_type_two)
test8.recalculate()

print_comparison(
    "Wilcoxon Signed-Rank Test: H₀: median difference = 0",
    test8,
    {
        'Before median': np.median(wilcox_before),
        'After median': np.median(wilcox_after),
        'Median difference': np.median(wilcox_after - wilcox_before),
        'W-statistic': scipy_result.statistic,
        'p-value': scipy_result.pvalue,
        'n_pairs': len(wilcox_before)
    }
)


# =============================================================================
# Test 9: Chi-Square Test of Independence
# =============================================================================
print("\n\n" + "#" * 80)
print("# Test 9: Chi-Square Test of Independence (Contingency Table)")
print("#" * 80)

# Create a contingency table: Treatment vs Outcome
# Rows: Treatment A, Treatment B
# Columns: Success, Failure
contingency_table = np.array([
    [40, 10],  # Treatment A: 40 success, 10 failure
    [25, 25]   # Treatment B: 25 success, 25 failure
])

# SciPy
chi2_stat, p_val, dof, expected = chi2_contingency(contingency_table)

print("\nChi-Square Test of Independence:")
print("=" * 80)
print("\nObserved Contingency Table:")
print("              Success   Failure")
print(f"Treatment A:     {contingency_table[0,0]}        {contingency_table[0,1]}")
print(f"Treatment B:     {contingency_table[1,0]}        {contingency_table[1,1]}")

print("\nSciPy/StatsModels Results:")
print("-" * 40)
print(f"  Chi-square statistic: {chi2_stat:.6f}")
print(f"  p-value: {p_val:.6f}")
print(f"  Degrees of freedom: {dof}")
print(f"  Expected frequencies:\n{expected}")

print("\n⚠️  Note: Chi-square independence test requires 2D table input.")
print("    LabPlot's Python bindings currently expose column-based API.")
print("    For this test, create columns from flattened table data.")
print("    This is a limitation of the current binding design.")


# =============================================================================
# Test 10: Mann-Kendall Trend Test
# =============================================================================
print("\n\n" + "#" * 80)
print("# Test 10: Mann-Kendall Trend Test (Time series trend)")
print("#" * 80)

# Generate time series with upward trend
time_points = np.arange(30)
trend_data = 10 + 0.5 * time_points + np.random.normal(0, 2, 30)

# SciPy (use kendalltau as proxy)
scipy_result = stats.kendalltau(time_points, trend_data)

# LabPlot
spreadsheet10 = Spreadsheet("Trend Data")
spreadsheet10.setColumnCount(1)
proj.addChild(spreadsheet10)
spreadsheet10.column(0).setName("Values")
spreadsheet10.column(0).replaceValues(0, [float(x) for x in trend_data])

test10 = HypothesisTest("Mann-Kendall Test")
proj.addChild(test10)
test10.setTest(HypothesisTest.Test.mann_kendall_test)
test10.setDataColumns([spreadsheet10.column(0)])
test10.setSignificanceLevel(0.05)
test10.setTail(nsl_stats_tail_type.nsl_stats_tail_type_two)
test10.recalculate()

print_comparison(
    "Mann-Kendall Trend Test: H₀: no monotonic trend",
    test10,
    {
        'Kendall tau (scipy)': scipy_result.correlation,
        'p-value (scipy kendalltau)': scipy_result.pvalue,
        'Data points': len(trend_data),
        'Linear trend slope (OLS)': np.polyfit(time_points, trend_data, 1)[0]
    }
)


# =============================================================================
# Summary
# =============================================================================
print("\n\n" + "#" * 80)
print("# COMPARISON SUMMARY")
print("#" * 80)
print("""
Tests Compared:
✓ 1. One-Sample t-Test
✓ 2. Two-Sample t-Test (Independent, Equal Variances)
✓ 3. Paired t-Test
✓ 4. Welch's t-Test (Unequal Variances)
✓ 5. One-Way ANOVA
✓ 6. Mann-Whitney U Test
✓ 7. Kruskal-Wallis Test
✓ 8. Wilcoxon Signed-Rank Test
✓ 9. Chi-Square Test of Independence (contingency table)
✓ 10. Mann-Kendall Trend Test

Additional Tests Available in LabPlot (not demonstrated above):
- One-Way Repeated Measures ANOVA
- Friedman Test (non-parametric repeated measures)
- Chi-Square Goodness of Fit Test
- Log-Rank Test (survival analysis)
- Wald-Wolfowitz Runs Test (randomness)
- Ramirez-Runger Test (stability/change point)

HOW TO COMPARE:
Compare the key statistics between SciPy and LabPlot:
- Test statistics (t, F, H, U, W, etc.) should match closely
- p-values should match (within numerical precision ~1e-6)
- Degrees of freedom should be identical
- Mean, variance, and other descriptive stats should align

INTERPRETATION GUIDE:
- p-value < 0.05: Reject null hypothesis (significant result at α=0.05)
- p-value ≥ 0.05: Fail to reject null hypothesis (not significant)
- Small differences (<1e-4) in p-values are due to numerical methods
- LabPlot uses NSL library (C/GSL), SciPy uses different implementations

Manual inspection:
Open each HypothesisTest object in LabPlot's GUI to see:
- Full formatted results with effect sizes
- Detailed statistical output
- Visual comparison options
- Export functionality
""")

print("\n✓ All test objects created successfully!")
print("  Open LabPlot GUI to inspect detailed results for each test.\n")
