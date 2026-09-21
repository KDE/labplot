"""
Quantum Wave Packet Evolution Demo

This script demonstrates the time evolution of a Gaussian wave packet
in quantum mechanics, showing:
- Real and imaginary parts of the wave function ψ(x,t)
- Probability density |ψ(x,t)|²
- Wave packet propagation and dispersion

Physics:
A free particle Gaussian wave packet disperses as it propagates.
The wave function evolves according to the Schrödinger equation.
"""

import sys
import time
import numpy as np
from PySide6.QtCore import QCoreApplication
from pylabplot import *

# === Physical Parameters ===
hbar = 1.0  # Reduced Planck constant (natural units)
m = 1.0     # Particle mass (natural units)

# Wave packet initial parameters
x0 = 0.0          # Initial center position
k0 = 5.0          # Initial wave number (momentum p = hbar*k)
sigma0 = 1.0      # Initial width (standard deviation)

# Spatial grid
N_points = 512
x_min, x_max = -10.0, 10.0
x = np.linspace(x_min, x_max, N_points)

# Time parameters
N_steps = 100      # Number of animation frames
t_max = 3.0        # Maximum time
dt = t_max / N_steps

# === Wave Function Evolution ===
def psi(x, t):
	"""
	Analytical solution for free particle Gaussian wave packet.
	Returns complex wave function ψ(x,t).
	"""
	# Time-dependent width
	sigma_t = sigma0 * np.sqrt(1 + (hbar * t / (m * sigma0**2))**2)

	# Normalization
	norm = 1.0 / np.sqrt(sigma_t * np.sqrt(np.pi))

	# Phase factors
	phase1 = 1j * k0 * (x - x0)
	phase2 = -(x - x0 - hbar * k0 * t / m)**2 / (2 * sigma_t**2)
	phase3 = 1j * (hbar * k0**2 * t) / (2 * m)

	return norm * np.exp(phase1 + phase2 + phase3)

# === Setup LabPlot Project ===
proj = project()

# Check if objects already exist (script was run before)
existing_spreadsheet = None
existing_worksheet = None

for child in proj.children(AspectType.Spreadsheet):
	if child.name() == "Wave Data":
		existing_spreadsheet = child
		break

for child in proj.children(AspectType.Worksheet):
	if child.name() == "Quantum Wave Packet":
		existing_worksheet = child
		break

# Determine if we need to create objects or just update data
create_objects = (existing_spreadsheet is None or existing_worksheet is None)

if create_objects:
	print("First run - creating project structure...")

	# Create spreadsheet with columns
	spreadsheet = Spreadsheet("Wave Data")
	spreadsheet.setColumnCount(4)  # We need 4 columns: x, real(ψ), imag(ψ), |ψ|²
	proj.addChild(spreadsheet)

	# Get column references
	col_x = spreadsheet.column(0)
	col_real = spreadsheet.column(1)
	col_imag = spreadsheet.column(2)
	col_prob = spreadsheet.column(3)

	# Set column names
	col_x.setName("x")
	col_real.setName("Re(ψ)")
	col_imag.setName("Im(ψ)")
	col_prob.setName("|ψ|²")

	# Initialize with t=0 data
	psi_t0 = psi(x, 0)
	col_x.replaceValues(0, [float(xi) for xi in x])
	col_real.replaceValues(0, [float(val.real) for val in psi_t0])
	col_imag.replaceValues(0, [float(val.imag) for val in psi_t0])
	col_prob.replaceValues(0, [float(abs(val)**2) for val in psi_t0])

	# === Create Worksheet ===
	worksheet = Worksheet("Quantum Wave Packet")
	proj.addChild(worksheet)

	# Set worksheet size
	worksheet.setUseViewSize(False)
	w = Worksheet.convertToSceneUnits(20, Worksheet.Unit.Centimeter)
	h = Worksheet.convertToSceneUnits(15, Worksheet.Unit.Centimeter)
	from PySide6.QtCore import QRectF
	worksheet.setPageRect(QRectF(0, 0, w, h))

	# Apply a nice theme
	worksheet.setTheme("Tufte")

	# Setup layout
	worksheet.setLayout(Worksheet.Layout.VerticalLayout)
	ms = Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter)
	worksheet.setLayoutTopMargin(ms)
	worksheet.setLayoutBottomMargin(ms)
	worksheet.setLayoutLeftMargin(ms)
	worksheet.setLayoutRightMargin(ms)
	worksheet.setLayoutVerticalSpacing(ms)

	# === Create Plot Area ===
	plotArea = CartesianPlot("Wave Function Plot")
	plotArea.setType(CartesianPlot.Type.FourAxes)
	plotArea.title().setText("Quantum Wave Packet Evolution")

	# Configure axes
	plotArea.horizontalAxis().title().setText("Position x")
	plotArea.verticalAxis().title().setText("ψ(x,t)")

	worksheet.addChild(plotArea)

	# Set fixed Y range to reduce flickering during animation
	plotArea.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
	rangeY = plotArea.range(CartesianCoordinateSystem.Dimension.Y, 0)
	rangeY.setRange(-1.0, 1.0)
	plotArea.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY)

	# === Create Curves ===
	# Real part
	curve_real = XYCurve("Re(ψ)")
	curve_real.setXColumn(col_x)
	curve_real.setYColumn(col_real)
	curve_real.setLineType(XYCurve.LineType.Line)
	curve_real.symbol().setStyle(Symbol.Style.NoSymbols)
	curve_real.line().setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
	plotArea.addChild(curve_real)

	# Imaginary part
	curve_imag = XYCurve("Im(ψ)")
	curve_imag.setXColumn(col_x)
	curve_imag.setYColumn(col_imag)
	curve_imag.setLineType(XYCurve.LineType.Line)
	curve_imag.symbol().setStyle(Symbol.Style.NoSymbols)
	curve_imag.line().setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))
	plotArea.addChild(curve_imag)

	# Probability density
	curve_prob = XYCurve("|ψ|²")
	curve_prob.setXColumn(col_x)
	curve_prob.setYColumn(col_prob)
	curve_prob.setLineType(XYCurve.LineType.Line)
	curve_prob.symbol().setStyle(Symbol.Style.NoSymbols)
	curve_prob.line().setWidth(Worksheet.convertToSceneUnits(3, Worksheet.Unit.Point))
	plotArea.addChild(curve_prob)

	# Add legend
	plotArea.addLegend()
else:
	print("Objects already exist - reusing and resetting animation...")

	# Reuse existing objects
	spreadsheet = existing_spreadsheet
	worksheet = existing_worksheet

	# Get columns
	col_x = spreadsheet.column(0)
	col_real = spreadsheet.column(1)
	col_imag = spreadsheet.column(2)
	col_prob = spreadsheet.column(3)

	# Get plot area (first CartesianPlot child of worksheet)
	plotArea = None
	for child in worksheet.children(AspectType.CartesianPlot):
		if child.name() == "Wave Function Plot":
			plotArea = child
			break

	if plotArea is None:
		print("Warning: Could not find plot area!")

# Reset to initial state (t=0) for animation
psi_t0 = psi(x, 0)
col_real.replaceValues(0, [float(val.real) for val in psi_t0])
col_imag.replaceValues(0, [float(val.imag) for val in psi_t0])
col_prob.replaceValues(0, [float(abs(val)**2) for val in psi_t0])

# === Animation Loop ===
print("Starting wave packet evolution animation...")
print(f"Time step: {dt:.3f}, Total frames: {N_steps}")

for i in range(N_steps):
	t = i * dt

	# Compute wave function at time t
	psi_t = psi(x, t)

	# Update column data
	col_real.replaceValues(0, [float(val.real) for val in psi_t])
	col_imag.replaceValues(0, [float(val.imag) for val in psi_t])
	col_prob.replaceValues(0, [float(abs(val)**2) for val in psi_t])

	# Process Qt events to update the UI
	QCoreApplication.processEvents()

	# Small delay for animation effect
	time.sleep(0.05)

	# Progress indicator
	if (i + 1) % 10 == 0:
		print(f"Frame {i+1}/{N_steps} (t = {t:.2f})")

print("Animation complete!")
print(f"Final time: t = {t_max:.2f}")
print(f"Wave packet has dispersed by factor: {np.sqrt(1 + (hbar * t_max / (m * sigma0**2))**2):.2f}x")
