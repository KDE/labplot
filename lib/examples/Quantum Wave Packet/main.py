#!/usr/bin/env python3
"""
Standalone runner for the Quantum Wave Packet demo.

This script can be executed directly with Python if LabPlot's Python
bindings are installed in your Python environment.
"""

import sys
import os

# Add the path to pylabplot if needed
# sys.path.insert(0, '/path/to/pylabplot')

# Import the script
script_dir = os.path.dirname(os.path.abspath(__file__))
script_path = os.path.join(script_dir, 'script.py')

with open(script_path, 'r') as f:
    script_code = f.read()

# Execute the script
exec(script_code)
