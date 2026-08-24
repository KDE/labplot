# 3D Visualization Implementation

## Overview

The 3D visualization subsystem wraps **Qt Graphs** (formerly Qt DataVisualization) into LabPlot's aspect-based architecture. It provides surface plots, scatter plots, and bar charts with standard LabPlot features: undo/redo, XML serialization, and data source binding.

**Location:** `src/backend/worksheet/plots/3d/`

**External Dependency:** Qt Graphs module (part of Qt 6)



⏺ The change from Graphs to DataVisualization was required because Qt 6.11 ships with TWO completely different 3D APIs:

  1. QtDataVisualization (what LabPlot uses):
     - Classes: Q3DSurface, Q3DScatter, Q3DBars
     - Inherits from QWindow (needs createWindowContainer())
     - Stable, mature API
     - Camera accessed via scene()->activeCamera()
  2. QtGraphs (new in Qt 6.11):
     - Classes: Q3DSurfaceWidgetItem, Q3DScatterWidgetItem
     - QML/Quick-based architecture
     - Completely different API surface
     - Not backward compatible with QtDataVisualization

  When the CMakeLists.txt said find_package(Qt6 COMPONENTS Graphs), it was linking the wrong module. The code compiled before because:
  - Either QtGraphs wasn't installed yet, so CMake fell back
  - Or the includes were finding DataVisualization headers through different paths
  - Or previous Qt versions had different module names

  The errors you hit ('QAbstract3DInputHandler' file not found, no member named 'window') were symptoms of the module mismatch. Fixing it back to DataVisualization restored the correct API.

  

  
## Architecture

### Class Hierarchy

```
AbstractAspect
    └─ WorksheetElementContainer
           └─ Base3DArea (abstract base for all 3D plots)
                  ├─ Surface3DPlot (3D surface plots)
                  ├─ Scatter3DPlot (3D scatter plots)
                  └─ Bar3DPlot (3D bar charts)

AbstractAspect
    └─ Axis3D (3D axis configuration)

AbstractPlot
    └─ Plot3DArea (container/manager for 3D plots)
```

### Core Components

#### Base3DArea (Base3DArea.h/cpp, 7.6KB)

Abstract base class for all 3D plot types. Manages common properties and delegates to Qt Graphs objects.

**Properties:**
- `theme` — 7 predefined themes (Qt, PrimaryColors, StoneMoss, ArmyBlue, Retro, Ebony, Isabelle)
- `shadowQuality` — None, Low, Medium, High, SoftLow, SoftMedium, SoftHigh
- `xRotation`, `yRotation` — camera rotation angles
- `zoomLevel` — camera zoom (0-100+)

**Qt Graphs objects held (one per plot type):**
- `Q3DSurface* m_surface`
- `Q3DScatter* m_scatter`
- `Q3DBars* m_bar`

**Note:** All three pointers exist per instance, but only one is used based on plot type. This wastes two pointers per plot.

**Pattern:** All setters use LabPlot's undo command pattern via `STD_SETTER_CMD_IMPL_F_S` macro.

#### Surface3DPlot (Surface3DPlot.h/cpp, 18KB)

Renders 3D surface meshes from matrix data, spreadsheet columns, or demo data.

**Data Sources:**
- `DataSource_Spreadsheet` — X, Y, Z columns (assumes √N × √N grid)
- `DataSource_Matrix` — Matrix object with xStart/xEnd/yStart/yEnd ranges
- `DataSource_Empty` — Generates demo sphere (50×50 parametric surface)

**Draw Modes:**
- `DrawWireframe` — wireframe only
- `DrawSurface` — filled surface only
- `DrawWireframeSurface` — both wireframe and surface

**Properties:**
- `flatShading` — enable/disable flat shading
- `smooth` — enable/disable smooth mesh
- `color` — base surface color

**Implementation Details:**
- Matrix mode: `generateMatrixData()` reads `Matrix::cell<double>(x, y)`, maps to `QVector3D(x_val, z_val, y_val)`
- Spreadsheet mode: `generateSpreadsheetData()` requires `xRowCount == yRowCount == zRowCount` and `√numPoints` must be an integer (no validation error shown)
- Demo mode: `generateDemoData()` creates parametric sphere using spherical coordinates
- Data flows: Column/Matrix → `recalc()` → `QSurfaceDataArray` → `QSurface3DSeries` → `Q3DSurface`

**Known Issues:**
- TODO comment (line 16): "get rid of this include, use forward declaration only"
- Series accumulation: `recalc()` only removes first series if list is non-empty; repeated calls can accumulate series
- No error message when spreadsheet data isn't a perfect square grid

#### Scatter3DPlot (Scatter3DPlot.h/cpp, 10KB)

Renders 3D scatter points from three columns (X, Y, Z).

**Point Styles:**
- Sphere, Cube, Cone, Pyramid

**Properties:**
- `xColumn`, `yColumn`, `zColumn` — data sources
- `pointStyle` — mesh shape for points
- `color` — base point color

**Implementation Details:**
- Simple 1:1 mapping: column row index → 3D point
- Uses `std::min()` of three column row counts
- Data flows: Columns → `recalc()` → `QScatterDataArray` → `QScatter3DSeries` → `Q3DScatter`

#### Bar3DPlot (Bar3DPlot.h/cpp, 9KB)

Renders 3D bar charts from multiple columns.

**Properties:**
- `dataColumns` — vector of columns (each column = one bar series)
- `color` — base bar color

**Implementation Details:**
- Each column in `dataColumns` becomes one bar series
- Number of rows determined by `std::min()` of all column row counts
- Data flows: Columns → `recalc()` → `QBarDataArray` → `QBar3DSeries` → `Q3DBars`

#### Axis3D (Axis3D.h/cpp, 5.8KB)

Wraps `QValue3DAxis` from Qt Graphs. Three axes (X, Y, Z) created per plot.

**Properties:**
- `title` — axis label
- `minRange`, `maxRange` — axis range
- `segmentCount` — number of major tick segments
- `subSegmentCount` — number of minor ticks per segment
- `axisFormat` — Decimal, Scientific, PowerOf10, PowerOf2, PowerOfE, MultiplierOfPi

**Known Issues:**
- `save()` and `load()` are stubs (lines 27-31) — axis configuration is not persisted to XML

#### MouseInteractor (MouseInteractor.h/cpp, 1.6KB)

Custom input handler for 3D plot interaction.

**Controls:**
- Left mouse drag: rotate camera (updates `cameraXRotation`, `cameraYRotation`)
- Mouse wheel: zoom in/out
- Triggers re-render via `scene()->needRender()`

Subclasses `QAbstract3DInputHandler` from Qt Graphs.

#### Plot3DArea (Plot3DArea.h/cpp, 5KB)

Container/manager for 3D plots. Subclasses `AbstractPlot`.

**Purpose:**
- Not a plot itself — holds multiple 3D plots as children
- Provides context menu actions: Add Surface, Add Scatter, Add Bar
- Manages plot lifecycle

## Data Flow

```
Column/Matrix data
    ↓
recalc() triggered by data change signal
    ↓
generate{Spreadsheet|Matrix|Demo}Data()
    ↓
QSurfaceDataArray / QScatterDataArray / QBarDataArray
    ↓
QSurface3DSeries / QScatter3DSeries / QBar3DSeries
    ↓
Q3DSurface / Q3DScatter / Q3DBars (Qt Graphs rendering)
```

## Key Design Patterns

### Undo/Redo Integration

All property setters follow LabPlot's standard command pattern:

```cpp
STD_SETTER_CMD_IMPL_F_S(Surface3DPlot, SetColor, QColor, color, updateColor)
void Surface3DPlot::setColor(QColor color) {
    Q_D(Surface3DPlot);
    if (color != d->color)
        exec(new Surface3DPlotSetColorCmd(d, color, ki18n("%1: color changed")));
}
```

This generates a `QUndoCommand` subclass that:
- Swaps old/new values on redo/undo
- Calls `updateColor()` to apply changes to Qt Graphs objects
- Emits `changed()` signal for UI updates

### Data Source Binding

Plots connect to data sources via Qt signals:

```cpp
void Surface3DPlot::setXColumn(const AbstractColumn* xCol) {
    if (xCol) {
        connect(xCol, &AbstractColumn::dataChanged, this, &Surface3DPlot::recalc);
        connect(xCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved,
                this, &Surface3DPlot::xColumnAboutToBeRemoved);
    }
}
```

When column data changes or column is deleted → `recalc()` updates Qt Graphs objects.

### XML Serialization

Standard LabPlot pattern:
- `save()` writes column paths (not pointers)
- `load()` reads paths, resolution happens after full project load
- Uses `WRITE_COLUMN` / `READ_COLUMN` macros

Example:
```xml
<surface3dplot>
    <general color="#00ff00" sourceType="0" drawMode="3" .../>
    <spreadsheet>
        <column xColumn="Spreadsheet/Column1"/>
        <column yColumn="Spreadsheet/Column2"/>
        <column zColumn="Spreadsheet/Column3"/>
    </spreadsheet>
</surface3dplot>
```

## Qt DataVisualization vs Qt Graphs

**LabPlot uses Qt DataVisualization** (`QtDataVisualization` module) for 3D plots.

### Historical Context

- **Aug 2024:** 3D visualization implemented using `QtDataVisualization` with `Q3DSurface`, `Q3DScatter`, `Q3DBars`
- **Qt 6.x:** Qt renamed `QtDataVisualization` → `QtGraphs` (module rename, API compatible)
- **Qt 6.11:** Qt introduced **new** `QtGraphs` + `QtGraphsWidgets` with completely different QML-based architecture

### Two Separate APIs in Qt 6.11

Qt 6.11 ships with **both** APIs:

| Module | Classes | Architecture | Status |
|--------|---------|--------------|--------|
| `QtDataVisualization` | `Q3DSurface`, `Q3DScatter`, `Q3DBars` | QWidget-based, direct rendering | **Stable, LabPlot uses this** |
| `QtGraphs` + `QtGraphsWidgets` | `Q3DSurfaceWidgetItem`, etc. | QML/Quick-based, needs `QQuickWidget` | New in 6.11, different API |

### Why LabPlot Uses QtDataVisualization

1. **Stable widget-based API** - integrates directly with LabPlot's QWidget architecture
2. **No QML dependency** - pure C++/Qt Widgets
3. **Custom input handling** - `MouseInteractor` provides LabPlot-specific interaction (drag to rotate, wheel to zoom)
4. **Working code** - original implementation is correct and requires no changes
5. **Qt documentation claims** "no breaking changes" but the new `QtGraphs` API is completely different

### If Migrating to QtGraphs (Not Recommended Now)

The new `QtGraphs` API would require:
- Creating `QQuickWidget` containers for each 3D plot
- Wrapping `Q3DSurfaceWidgetItem` objects with `setWidget(QQuickWidget*)`
- Integrating Quick widgets into QGraphicsScene-based frontend
- Losing custom `MouseInteractor` (built-in handlers are Quick-based)
- Major architectural changes with unclear benefits

**Recommendation:** Stay with `QtDataVisualization` until Qt deprecates it or provides a clear migration path.

## Known Issues & TODOs

### Bugs
1. **Series accumulation:** `recalc()` removes only first series from list; repeated calls pile up series
2. **No validation:** Spreadsheet surface mode requires perfect square grid but doesn't validate or show error
3. **Axis persistence:** Axis3D save/load are stubs — axis configuration lost on save/reload

### Design Issues
1. **Memory waste:** Base3DArea holds three Qt Graphs pointers (surface/scatter/bar), only one used per instance
2. **TODO comment:** Surface3DPlot.h:16 wants forward declaration instead of `#include <Q3DSurface>`
3. **Headless rendering:** Qt Graphs requires graphics context — CLI batch export needs Xvfb or offscreen rendering workaround

### Missing Features
1. No axis label format persistence
2. No multi-series support (scatter/surface plots show only one data series)
3. Limited theme customization (can't define custom themes)
4. No export to 3D formats (STL, OBJ, etc.)

## Qt Graphs Dependency

**Why Qt Graphs:**
- Mature OpenGL/RHI rendering pipeline
- Camera controls, lighting, shadows, anti-aliasing
- Touch/gesture input support
- ~50K+ lines of battle-tested 3D rendering code

**Risks:**
- Qt Graphs is relatively young (renamed from Qt DataVisualization in Qt 6)
- API stability not guaranteed across Qt versions
- GPL/Commercial license (could block future LGPL licensing)
- Requires graphics context (problematic for headless/CLI rendering)

**Alternative Considered:** Custom OpenGL implementation would give full control but require months of development for feature parity.

## Files Overview

| File | Size | Purpose |
|------|------|---------|
| Base3DArea.h/cpp | 7.6KB | Abstract base for 3D plots, common properties |
| Surface3DPlot.h/cpp | 18KB | Surface plot implementation |
| Scatter3DPlot.h/cpp | 10KB | Scatter plot implementation |
| Bar3DPlot.h/cpp | 9KB | Bar chart implementation |
| Axis3D.h/cpp | 5.8KB | 3D axis wrapper |
| MouseInteractor.h/cpp | 1.6KB | Custom input handler |
| Plot3DArea.h/cpp | 5KB | Container/manager |
| *Private.h | ~700B each | Private implementation headers |

**Total:** ~60KB of code wrapping Qt Graphs into LabPlot's architecture.

## Future Considerations

### Short Term (Bug Fixes)
1. Fix series accumulation: clear all series, not just first
2. Implement Axis3D persistence
3. Add validation + error message for surface grid requirements
4. Optimize: use single `QAbstract3DGraph*` pointer instead of three typed pointers

### Medium Term (Architecture)
1. Abstract backend interface:
   ```cpp
   class Plot3DBackend {
       virtual void setSeries(data) = 0;
       virtual void render(target) = 0;
   };
   ```
2. Keep Qt Graphs for GUI, add headless backend for CLI export
3. Support multi-series plots

### Long Term (If Qt Graphs Becomes Limiting)
1. Custom OpenGL/Vulkan backend for full control
2. Support for massive datasets (>10M points)
3. VR/AR rendering
4. Custom shaders and materials
5. Export to 3D file formats

## Testing

Tests should be added under `tests/backend/worksheet/plots/3d/`:
- Surface plot: matrix mode, spreadsheet mode, demo mode
- Scatter plot: basic rendering, point style changes
- Bar plot: multi-column rendering
- Axis configuration and persistence
- Undo/redo for all property changes
- Data source removal handling

Currently no dedicated 3D plot tests exist.
