#!/usr/bin/env python3
"""
Generate small test data files for LabPlot's ParquetFilter unit tests.

Requires: pip install pyarrow
Generates: testdata.parquet, testdata.arrow, testdata.orc,
           testdata_types.parquet, testdata_nulls.parquet, testdata_empty.parquet
"""

import datetime
import pyarrow as pa
import pyarrow.parquet as pq
import pyarrow.feather as feather
import pyarrow.orc as orc

OUTPUT_DIR = "data"

# ---------------------------------------------------------------------------
# 1. Basic test table: int, double, string, timestamp — 5 rows, with nulls
# ---------------------------------------------------------------------------
basic_table = pa.table({
    "id":    pa.array([1, 2, 3, 4, 5], type=pa.int32()),
    "value": pa.array([1.1, 2.2, None, 4.4, 5.5], type=pa.float64()),
    "name":  pa.array(["Alice", "Bob", None, "Dave", "Eve"], type=pa.utf8()),
    "ts":    pa.array([
        datetime.datetime(2025, 1, 1, 0, 0, 0),
        datetime.datetime(2025, 6, 15, 12, 30, 0),
        datetime.datetime(2025, 12, 31, 23, 59, 59),
        None,
        datetime.datetime(2024, 3, 14, 9, 26, 53),
    ], type=pa.timestamp("us")),
})

pq.write_table(basic_table, f"{OUTPUT_DIR}/testdata.parquet")
feather.write_feather(basic_table, f"{OUTPUT_DIR}/testdata.arrow")
orc.write_table(basic_table, f"{OUTPUT_DIR}/testdata.orc")

print("Created testdata.parquet, testdata.arrow, testdata.orc")

# ---------------------------------------------------------------------------
# 2. Extended types: bool, int8, int16, int32, int64, float, double, date, string
# ---------------------------------------------------------------------------
types_table = pa.table({
    "col_int8":    pa.array([1, -2, 3], type=pa.int8()),
    "col_int16":   pa.array([100, -200, 300], type=pa.int16()),
    "col_int32":   pa.array([10000, -20000, 30000], type=pa.int32()),
    "col_int64":   pa.array([100000000000, -200000000000, 300000000000], type=pa.int64()),
    "col_float":   pa.array([1.5, -2.5, 3.5], type=pa.float32()),
    "col_double":  pa.array([1.123456789, -2.987654321, 3.141592654], type=pa.float64()),
    "col_string":  pa.array(["hello", "world", "test"], type=pa.utf8()),
    "col_bool":    pa.array([True, False, True], type=pa.bool_()),
})

pq.write_table(types_table, f"{OUTPUT_DIR}/testdata_types.parquet")
print("Created testdata_types.parquet")

# ---------------------------------------------------------------------------
# 3. All-null columns (edge case)
# ---------------------------------------------------------------------------
nulls_table = pa.table({
    "int_col":    pa.array([None, None, None], type=pa.int32()),
    "double_col": pa.array([None, None, None], type=pa.float64()),
    "text_col":   pa.array([None, None, None], type=pa.utf8()),
})

pq.write_table(nulls_table, f"{OUTPUT_DIR}/testdata_nulls.parquet")
print("Created testdata_nulls.parquet")

# ---------------------------------------------------------------------------
# 4. Empty table (schema only, 0 rows)
# ---------------------------------------------------------------------------
schema = pa.schema([
    ("id", pa.int32()),
    ("value", pa.float64()),
    ("name", pa.utf8()),
])
empty_table = pa.table({"id": pa.array([], type=pa.int32()),
                         "value": pa.array([], type=pa.float64()),
                         "name": pa.array([], type=pa.utf8())})

pq.write_table(empty_table, f"{OUTPUT_DIR}/testdata_empty.parquet")
print("Created testdata_empty.parquet")

print("\nAll test data files generated successfully.")
