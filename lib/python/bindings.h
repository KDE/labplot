#ifndef WRAPPEDCLASSES_H
#define WRAPPEDCLASSES_H

#ifndef SCRIPTING
#define SDK
#endif

#include <QXmlStreamWriter>

// abstract classes
#include "src/backend/core/AbstractAspect.h"
#include "src/backend/core/AbstractColumn.h"
#include "src/backend/worksheet/plots/AbstractCoordinateSystem.h"
#include "src/backend/datasources/AbstractDataSource.h"
#include "src/backend/datasources/filters/AbstractFileFilter.h"
#include "src/backend/core/AbstractPart.h"
#include "src/backend/worksheet/plots/AbstractPlot.h"
#include "src/backend/worksheet/plots/cartesian/Plot.h"
#include "src/backend/worksheet/WorksheetElement.h"
#include "src/backend/worksheet/WorksheetElementContainer.h"
#include "src/backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

// aspect containers
#include "src/backend/core/Folder.h" // inherits AbstractAspect, QObject
#include "src/backend/core/Project.h" // inherits Folder, AbstractAspect, QObject

// column
#include "src/backend/core/column/Column.h"

// data containers
#include "src/backend/spreadsheet/Spreadsheet.h" // inherits AbstractDataSource, AbstractPart, AbstractAspect, QObject
#include "src/backend/matrix/Matrix.h" // inherits AbstractDataSource, AbstractPart, AbstractAspect, QObject

// filters
#include "src/backend/datasources/filters/AsciiFilter.h"
#include "src/backend/datasources/filters/BinaryFilter.h"
#include "src/backend/datasources/filters/CANFilter.h"
#include "src/backend/datasources/filters/FITSFilter.h"
#include "src/backend/datasources/filters/HDF5Filter.h"
#include "src/backend/datasources/filters/ImageFilter.h"
#include "src/backend/datasources/filters/JsonFilter.h"
#include "src/backend/datasources/filters/MatioFilter.h"
#include "src/backend/datasources/filters/McapFilter.h"
#include "src/backend/datasources/filters/NetCDFFilter.h"
#include "src/backend/datasources/filters/OdsFilter.h"
#include "src/backend/datasources/filters/ReadStatFilter.h"
#include "src/backend/datasources/filters/ROOTFilter.h"
#include "src/backend/datasources/filters/SpiceFilter.h"
#include "src/backend/datasources/filters/VectorBLFFilter.h"
#include "src/backend/datasources/filters/XLSXFilter.h"

// helper classes
#include "src/backend/worksheet/Background.h"
#include "src/backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "src/backend/worksheet/plots/cartesian/CartesianScale.h"
#include "src/backend/core/column/ColumnStringIO.h"
#include "src/backend/worksheet/plots/cartesian/ErrorBar.h"
#include "src/backend/worksheet/Line.h"
#include "src/backend/lib/Range.h"
#include "src/backend/spreadsheet/StatisticsSpreadsheet.h"
#include "src/backend/worksheet/plots/cartesian/Symbol.h"
#include "src/backend/worksheet/plots/cartesian/Value.h"

// plot area elements
#include "src/backend/worksheet/plots/cartesian/Axis.h"
#include "src/backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "src/backend/worksheet/plots/cartesian/CustomPoint.h"
#include "src/backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "src/backend/worksheet/plots/cartesian/ReferenceRange.h"

// plots
#include "src/backend/worksheet/plots/cartesian/BarPlot.h"
#include "src/backend/worksheet/plots/cartesian/BoxPlot.h"
#include "src/backend/worksheet/plots/cartesian/Histogram.h"
#include "src/backend/worksheet/plots/cartesian/KDEPlot.h"
#include "src/backend/worksheet/plots/cartesian/LollipopPlot.h"
#include "src/backend/worksheet/plots/cartesian/ProcessBehaviorChart.h"
#include "src/backend/worksheet/plots/cartesian/QQPlot.h"
#include "src/backend/worksheet/plots/cartesian/RunChart.h"
#include "src/backend/worksheet/plots/cartesian/XYCurve.h"
#include "src/backend/worksheet/plots/cartesian/XYEquationCurve.h"

// analysis plots
#include "src/backend/worksheet/plots/cartesian/XYConvolutionCurve.h"
#include "src/backend/worksheet/plots/cartesian/XYCorrelationCurve.h"
#include "src/backend/worksheet/plots/cartesian/XYLineSimplificationCurve.h"
#include "src/backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"
#include "src/backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "src/backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "src/backend/worksheet/plots/cartesian/XYFourierTransformCurve.h"
#include "src/backend/worksheet/plots/cartesian/XYFunctionCurve.h"
#include "src/backend/worksheet/plots/cartesian/XYHilbertTransformCurve.h"
#include "src/backend/worksheet/plots/cartesian/XYIntegrationCurve.h"
#include "src/backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "src/backend/worksheet/plots/cartesian/XYSmoothCurve.h"

// worksheet
#include "src/backend/worksheet/Worksheet.h" // inherits AbstractPart, AbstractAspect, QObject

// worksheet element containers
#include "src/backend/worksheet/plots/cartesian/CartesianPlot.h"

// worksheet elements
#include "src/backend/worksheet/Image.h"
#include "src/backend/worksheet/InfoElement.h"
#include "src/backend/worksheet/TextLabel.h"

#ifdef SCRIPTING
// proxy for stdout/stderr
#include "src/backend/script/python/PythonLogger.h"
#endif

#endif // WRAPPEDCLASSES_H
