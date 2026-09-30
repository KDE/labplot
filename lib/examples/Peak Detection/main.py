#!/usr/bin/env python3
"""Runner for peak detection demo"""
import sys
import os

script_dir = os.path.dirname(os.path.abspath(__file__))
script_path = os.path.join(script_dir, 'script.py')

with open(script_path, 'r') as f:
    exec(f.read())
