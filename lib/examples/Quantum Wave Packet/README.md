# Quantum Wave Packet Evolution

This example demonstrates the dynamic capabilities of LabPlot's Python scripting by simulating the time evolution of quantum mechanical wave packets.

**Two demos are included:**
1. **script.py** - Free particle wave packet dispersion
2. **script_tunneling.py** - Quantum tunneling through a potential barrier

## What it demonstrates

**Physics:**
- Gaussian wave packet propagating as a free particle
- Time evolution according to the Schrödinger equation
- Wave packet dispersion (spreading over time)

**LabPlot Scripting Features:**
- Creating project structure (Worksheet, CartesianPlot, Spreadsheet) via Python
- Dynamic data generation with NumPy
- Real-time column data updates for animation
- Multiple synchronized curves on a single plot
- Theme application and layout control

## The Physics

A free particle described by a Gaussian wave packet:

```
ψ(x,0) = (1/√(2πσ₀²))^(1/2) exp(ik₀x) exp(-(x-x₀)²/(4σ₀²))
```

evolves according to the Schrödinger equation. The analytical solution shows:
- The wave packet center moves with group velocity v = ℏk₀/m
- The width increases as σ(t) = σ₀√(1 + (ℏt/mσ₀²)²)
- The probability density |ψ(x,t)|² spreads out over time

## What you'll see

### Demo 1: Free Particle (script.py)

The animation shows three curves:
- **Re(ψ)** - Real part of the wave function (blue)
- **Im(ψ)** - Imaginary part of the wave function (orange)  
- **|ψ|²** - Probability density (green, bold)

As time progresses, you'll observe:
1. The wave packet moves to the right (positive momentum k₀ > 0)
2. The oscillations maintain constant phase velocity
3. The envelope spreads out (quantum dispersion)
4. The peak of |ψ|² decreases (probability conservation with spreading)

### Demo 2: Quantum Tunneling (script_tunneling.py)

The animation shows:
- **|ψ|²** - Probability density (approaching the barrier)
- **Re(ψ)** - Real part showing interference
- **Barrier (V/E)** - Potential energy normalized to particle energy (shaded region)

You'll observe the fascinating quantum phenomenon:
1. Wave packet approaches barrier from the left
2. Partial reflection at the barrier entrance
3. **Tunneling** - part of the wave appears beyond the barrier despite E < V₀!
4. Transmitted and reflected components separate
5. Final transmission/reflection probabilities printed at end

## Parameters

**Demo 1 (Free particle)** - Edit script.py to explore:
- `k0`: Initial momentum (try negative for leftward motion)
- `sigma0`: Initial width (smaller = faster dispersion)
- `N_steps`: Number of frames (more = smoother animation)
- `t_max`: Total simulation time

**Demo 2 (Tunneling)** - Edit script_tunneling.py to explore:
- `V0`: Barrier height relative to energy (try `V0 = 0.5*E` or `V0 = 1.2*E`)
- `barrier_x2 - barrier_x1`: Barrier width (wider = less tunneling)
- `k0`: Particle momentum (higher energy = more tunneling)
- Watch the transmission/reflection probabilities!

## Running the scripts

**Method 1: Within LabPlot (Recommended)**
1. Open LabPlot
2. Create a new Script (File → New → Script)
3. Select Python as the language
4. Copy the contents of `script.py` or `script_tunneling.py`
5. Click "Run" and watch the evolution of the quantum wave packet!

**Method 2: Standalone C++ executable**
1. Build with CMake (if examples are enabled)
2. Run `./QuantumWavePacket` from the build directory

**Method 3: Python script (if pylabplot installed)**
```bash
python3 main.py  # Runs script.py
python3 script_tunneling.py  # Runs tunneling demo
```

## Educational value

This demo is perfect for:
- Quantum mechanics courses (visualizing abstract wave functions)
- Demonstrating LabPlot's scientific computing capabilities
- Teaching students about wave-particle duality
- Showing how to create interactive scientific visualizations

## References

- Griffiths, "Introduction to Quantum Mechanics", Chapter 2
- The blog post that inspired this: https://ben.land/post/2022/03/09/quantum-mechanics-simulation/
