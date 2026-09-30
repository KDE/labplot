"""
Quantum Tunneling Demo - Wave Packet Encounters a Potential Barrier

This script demonstrates quantum tunneling by simulating a Gaussian wave packet
encountering a rectangular potential barrier.

Physics:
- A wave packet with definite momentum approaches a barrier
- Classical: particle reflects if E < V₀
- Quantum: finite probability of tunneling through the barrier
- Shows transmission and reflection coefficients visually

This is a more advanced demo showing:
- Potential energy visualization
- Split-operator method for time evolution
- Complex quantum phenomena (tunneling)
"""

import sys
import time
import numpy as np
from PySide6.QtCore import QCoreApplication
from pylabplot import *

# === Physical Parameters ===
hbar = 1.0  # Reduced Planck constant
m = 1.0     # Particle mass

# Wave packet initial parameters
x0 = -5.0         # Initial center position (left side)
k0 = 8.0          # Initial wave number (momentum)
sigma0 = 0.8      # Initial width
E = (hbar * k0)**2 / (2 * m)  # Energy

# Potential barrier
V0 = 0.7 * E      # Barrier height (< E for partial tunneling)
barrier_x1 = 0.0  # Barrier start
barrier_x2 = 2.0  # Barrier end

# Spatial grid
N_points = 1024
x_min, x_max = -15.0, 15.0
x = np.linspace(x_min, x_max, N_points)
dx = x[1] - x[0]

# Potential energy function
def V(x_val):
	"""Rectangular potential barrier"""
	return V0 if (barrier_x1 <= x_val <= barrier_x2) else 0.0

V_array = np.array([V(xi) for xi in x])

# Time parameters
N_steps = 200
dt = 0.02
t_max = N_steps * dt

# === Split-Operator Method for Time Evolution ===
# This method is more accurate than analytical free-particle solution
# when a potential is present

def evolve_split_operator(psi, V_array, dt):
	"""
	Evolve wave function by one time step using split-operator method.
	U(dt) ≈ exp(-iV dt/2ℏ) · exp(-iT dt/ℏ) · exp(-iV dt/2ℏ)
	where T is kinetic energy operator.
	"""
	# Momentum space grid
	dk = 2 * np.pi / (N_points * dx)
	k_vals = np.fft.fftfreq(N_points, dx) * 2 * np.pi

	# Half step in position space (potential energy)
	psi = psi * np.exp(-1j * V_array * dt / (2 * hbar))

	# Full step in momentum space (kinetic energy)
	psi_k = np.fft.fft(psi)
	psi_k = psi_k * np.exp(-1j * hbar * k_vals**2 * dt / (2 * m))
	psi = np.fft.ifft(psi_k)

	# Half step in position space (potential energy)
	psi = psi * np.exp(-1j * V_array * dt / (2 * hbar))

	return psi

# === Initial Wave Function ===
# Gaussian wave packet with momentum k0
psi = np.exp(1j * k0 * (x - x0)) * np.exp(-(x - x0)**2 / (2 * sigma0**2))
psi = psi / np.sqrt(np.sum(np.abs(psi)**2) * dx)  # Normalize

# === Setup LabPlot Project ===
proj = project()

# Check if objects already exist (script was run before)
existing_spreadsheet = None
existing_worksheet = None

for child in proj.children(AspectType.Spreadsheet):
	if child.name() == "Tunneling Data":
		existing_spreadsheet = child
		break

for child in proj.children(AspectType.Worksheet):
	if child.name() == "Quantum Tunneling":
		existing_worksheet = child
		break

# Determine if we need to create objects or just update data
create_objects = (existing_spreadsheet is None or existing_worksheet is None)

if create_objects:
	print("First run - creating project structure...")

	# Create spreadsheet
	spreadsheet = Spreadsheet("Tunneling Data")
	spreadsheet.setColumnCount(5)
	proj.addChild(spreadsheet)

	# Columns: x, Re(ψ), Im(ψ), |ψ|², V(x)
	col_x = spreadsheet.column(0)
	col_real = spreadsheet.column(1)
	col_imag = spreadsheet.column(2)
	col_prob = spreadsheet.column(3)
	col_potential = spreadsheet.column(4)

	col_x.setName("x")
	col_real.setName("Re(ψ)")
	col_imag.setName("Im(ψ)")
	col_prob.setName("|ψ|²")
	col_potential.setName("V(x)/E")

	# Initialize x and potential (these don't change)
	col_x.replaceValues(0, [float(xi) for xi in x])
	col_potential.replaceValues(0, [float(V(xi) / E) for xi in x])

	# === Create Worksheet ===
	worksheet = Worksheet("Quantum Tunneling")
	proj.addChild(worksheet)

	worksheet.setUseViewSize(False)
	w = Worksheet.convertToSceneUnits(22, Worksheet.Unit.Centimeter)
	h = Worksheet.convertToSceneUnits(15, Worksheet.Unit.Centimeter)
	from PySide6.QtCore import QRectF
	worksheet.setPageRect(QRectF(0, 0, w, h))

	worksheet.setTheme("Bright")

	ms = Worksheet.convertToSceneUnits(0.5, Worksheet.Unit.Centimeter)
	worksheet.setLayoutTopMargin(ms)
	worksheet.setLayoutBottomMargin(ms)
	worksheet.setLayoutLeftMargin(ms)
	worksheet.setLayoutRightMargin(ms)

	# === Create Plot Area ===
	plotArea = CartesianPlot("Tunneling Simulation")
	plotArea.setType(CartesianPlot.Type.FourAxes)
	plotArea.title().setText(f"Quantum Tunneling (E = {E:.2f}, V₀ = {V0:.2f})")
	plotArea.setNiceExtend(True)  # Auto-extend ranges for cleaner appearance

	# Set fixed Y range to reduce flickering during animation
	plotArea.enableAutoScale(CartesianCoordinateSystem.Dimension.Y, 0, False)
	rangeY = plotArea.range(CartesianCoordinateSystem.Dimension.Y, 0)
	rangeY.setRange(-1.0, 1.0)
	plotArea.setRange(CartesianCoordinateSystem.Dimension.Y, 0, rangeY)

	# Configure axes
	for axis in plotArea.children(AspectType.Axis):
		if axis.orientation() == WorksheetElement.Orientation.Horizontal and axis.position() == Axis.Position.Bottom:
			axis.title().setText("Position x")
		elif axis.orientation() == WorksheetElement.Orientation.Vertical and axis.position() == Axis.Position.Left:
			axis.title().setText("Wave Function / Potential")

	worksheet.addChild(plotArea)

	# === Create Potential Barrier Visualization ===
	# Use a filled area to show the barrier
	curve_potential = XYCurve("Barrier (V/E)")
	curve_potential.setXColumn(col_x)
	curve_potential.setYColumn(col_potential)
	curve_potential.setLineType(XYCurve.LineType.Line)
	curve_potential.symbol().setStyle(Symbol.Style.NoSymbols)
	curve_potential.line().setWidth(Worksheet.convertToSceneUnits(2, Worksheet.Unit.Point))

	# Fill under the potential curve
	from PySide6.QtCore import Qt
	curve_potential.background().setEnabled(True)
	curve_potential.background().setType(Background.Type.Color)
	curve_potential.background().setPosition(Background.Position.Below)
	curve_potential.background().setOpacity(0.3)

	plotArea.addChild(curve_potential)

	# === Create Wave Function Curves ===
	curve_prob = XYCurve("|ψ|²")
	curve_prob.setXColumn(col_x)
	curve_prob.setYColumn(col_prob)
	curve_prob.setLineType(XYCurve.LineType.Line)
	curve_prob.symbol().setStyle(Symbol.Style.NoSymbols)
	curve_prob.line().setWidth(Worksheet.convertToSceneUnits(3, Worksheet.Unit.Point))
	plotArea.addChild(curve_prob)

	curve_real = XYCurve("Re(ψ)")
	curve_real.setXColumn(col_x)
	curve_real.setYColumn(col_real)
	curve_real.setLineType(XYCurve.LineType.Line)
	curve_real.symbol().setStyle(Symbol.Style.NoSymbols)
	curve_real.line().setWidth(Worksheet.convertToSceneUnits(1.5, Worksheet.Unit.Point))
	curve_real.line().setOpacity(0.6)
	plotArea.addChild(curve_real)

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
	col_potential = spreadsheet.column(4)

	# Get plot area (first CartesianPlot child of worksheet)
	plotArea = None
	for child in worksheet.children(AspectType.CartesianPlot):
		if child.name() == "Tunneling Simulation":
			plotArea = child
			break

	if plotArea is None:
		print("Warning: Could not find plot area!")

# Reset wave function to initial state for animation
psi = np.exp(1j * k0 * (x - x0)) * np.exp(-(x - x0)**2 / (2 * sigma0**2))
psi = psi / np.sqrt(np.sum(np.abs(psi)**2) * dx)  # Normalize

# Initialize wave function data
col_real.replaceValues(0, [float(val.real) for val in psi])
col_imag.replaceValues(0, [float(val.imag) for val in psi])
col_prob.replaceValues(0, [float(abs(val)**2) for val in psi])

# === Animation Loop ===
print("Starting quantum tunneling simulation...")
print(f"Energy: E = {E:.3f}")
print(f"Barrier: V₀ = {V0:.3f} ({V0/E*100:.1f}% of E)")
print(f"Barrier width: {barrier_x2 - barrier_x1:.2f}")
print(f"Time step: {dt:.4f}, Total frames: {N_steps}")
print()

for i in range(N_steps):
	t = i * dt

	# Evolve wave function
	psi = evolve_split_operator(psi, V_array, dt)

	# Update column data
	col_real.replaceValues(0, [float(val.real) for val in psi])
	col_imag.replaceValues(0, [float(val.imag) for val in psi])
	col_prob.replaceValues(0, [float(abs(val)**2) for val in psi])

	# Process Qt events
	QCoreApplication.processEvents()

	# Animation delay
	time.sleep(0.03)

	# Progress and analysis
	if (i + 1) % 20 == 0:
		# Compute transmission and reflection
		prob_transmitted = np.sum(np.abs(psi[x > barrier_x2])**2) * dx
		prob_reflected = np.sum(np.abs(psi[x < barrier_x1])**2) * dx
		prob_barrier = np.sum(np.abs(psi[(x >= barrier_x1) & (x <= barrier_x2)])**2) * dx

		print(f"Frame {i+1}/{N_steps} (t={t:.2f}) | "
			  f"R={prob_reflected:.3f}, T={prob_transmitted:.3f}, "
			  f"B={prob_barrier:.3f}")

print()
print("Simulation complete!")
print()
print("=== Final Analysis ===")
prob_transmitted = np.sum(np.abs(psi[x > barrier_x2])**2) * dx
prob_reflected = np.sum(np.abs(psi[x < barrier_x1])**2) * dx
total_prob = np.sum(np.abs(psi)**2) * dx
print(f"Transmitted: {prob_transmitted:.4f} ({prob_transmitted/total_prob*100:.1f}%)")
print(f"Reflected:   {prob_reflected:.4f} ({prob_reflected/total_prob*100:.1f}%)")
print(f"Total probability: {total_prob:.4f} (should be ≈1.0)")
print()
print("Quantum tunneling observed! The particle has a finite probability")
print("of appearing beyond the barrier despite having insufficient classical energy.")
