#!/usr/bin/env python3
"""Generate larger Parquet/Arrow/ORC test files for performance testing."""

import pyarrow as pa
import pyarrow.parquet as pq
import pyarrow.ipc as ipc
import numpy as np

# ~1M rows, mixed types — should be ~50-100MB depending on format
N = 1_000_000

print(f"Generating {N:,} rows...")

rng = np.random.default_rng(42)

table = pa.table({
    "id": pa.array(np.arange(N, dtype=np.int64)),
    "value_float": pa.array(rng.standard_normal(N).astype(np.float64)),
    "value_int": pa.array(rng.integers(-1000, 1000, N, dtype=np.int32)),
    "category": pa.array(rng.choice(["alpha", "beta", "gamma", "delta", "epsilon"], N)),
    "timestamp": pa.array(
        np.datetime64("2020-01-01") + rng.integers(0, 365 * 5 * 86400, N).astype("timedelta64[s]")
    ),
    "flag": pa.array(rng.choice([True, False], N)),
    "measurement": pa.array(rng.uniform(0, 100, N).astype(np.float32)),
    "label": pa.array([f"item_{i % 10000}" for i in range(N)]),
})

print(f"Table: {table.num_rows:,} rows x {table.num_columns} columns")

# Parquet (snappy compressed)
pq.write_table(table, "large_testdata.parquet", compression="snappy")
print("  -> large_testdata.parquet")

# Parquet (uncompressed, larger file)
pq.write_table(table, "large_testdata_uncompressed.parquet", compression="none")
print("  -> large_testdata_uncompressed.parquet")

# Arrow IPC
with pa.OSFile("large_testdata.arrow", "wb") as f:
    writer = ipc.new_file(f, table.schema)
    # Write in batches of 100k for realistic IPC structure
    for batch in table.to_batches(max_chunksize=100_000):
        writer.write_batch(batch)
    writer.close()
print("  -> large_testdata.arrow")

# ORC
try:
    from pyarrow import orc
    orc.write_table(table, "large_testdata.orc")
    print("  -> large_testdata.orc")
except Exception as e:
    print(f"  -> ORC skipped: {e}")

# Also a "wide" table (many columns, fewer rows)
N_WIDE = 100_000
N_COLS = 200
print(f"\nGenerating wide table: {N_WIDE:,} rows x {N_COLS} columns...")

wide_data = {f"col_{i:03d}": pa.array(rng.standard_normal(N_WIDE).astype(np.float64)) for i in range(N_COLS)}
wide_table = pa.table(wide_data)

pq.write_table(wide_table, "wide_testdata.parquet", compression="snappy")
print("  -> wide_testdata.parquet")

print("\nDone!")
