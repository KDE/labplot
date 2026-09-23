#ifndef WRAPPEDCLASSES_H
#define WRAPPEDCLASSES_H

#ifndef SCRIPTING
#define SDK
#endif

#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QObject>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QTime>
#include <QtCore/QXmlStreamWriter>
#include <QtGui/QBrush>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QPen>

// abstract classes
#include "backend/core/AbstractAspect.h"
#include "backend/core/AbstractColumn.h"
#include "backend/worksheet/plots/AbstractCoordinateSystem.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/core/AbstractPart.h"
#include "backend/worksheet/plots/AbstractPlot.h"
#include "backend/worksheet/plots/cartesian/Plot.h"
#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/WorksheetElementContainer.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

// aspect containers
#include "backend/core/Folder.h" // inherits AbstractAspect, QObject
#include "backend/core/Project.h" // inherits Folder, AbstractAspect, QObject

// column
#include "backend/core/column/Column.h"

// data containers
#include "backend/spreadsheet/Spreadsheet.h" // inherits AbstractDataSource, AbstractPart, AbstractAspect, QObject
#include "backend/matrix/Matrix.h" // inherits AbstractDataSource, AbstractPart, AbstractAspect, QObject

// filters
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/filters/BinaryFilter.h"
#include "backend/datasources/filters/CANFilter.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/datasources/filters/HDF5Filter.h"
#include "backend/datasources/filters/ImageFilter.h"
#include "backend/datasources/filters/JsonFilter.h"
#include "backend/datasources/filters/MatioFilter.h"
#include "backend/datasources/filters/McapFilter.h"
#include "backend/datasources/filters/NetCDFFilter.h"
#include "backend/datasources/filters/OdsFilter.h"
#include "backend/datasources/filters/ReadStatFilter.h"
#include "backend/datasources/filters/ROOTFilter.h"
#include "backend/datasources/filters/SpiceFilter.h"
#include "backend/datasources/filters/VectorBLFFilter.h"
#include "backend/datasources/filters/XLSXFilter.h"
// helper classes
#include "backend/worksheet/Background.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianScale.h"
#include "backend/core/column/ColumnStringIO.h"
#include "backend/worksheet/plots/cartesian/ErrorBar.h"
#include "backend/worksheet/Line.h"
#include "backend/lib/Range.h"
#include "backend/spreadsheet/StatisticsSpreadsheet.h"
#include "backend/statistics/HypothesisTest.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/cartesian/Value.h"

// plot area elements
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"

// plots
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/KDEPlot.h"
#include "backend/worksheet/plots/cartesian/LollipopPlot.h"
#include "backend/worksheet/plots/cartesian/ProcessBehaviorChart.h"
#include "backend/worksheet/plots/cartesian/QQPlot.h"
#include "backend/worksheet/plots/cartesian/RunChart.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"

// analysis plots
#include "backend/worksheet/plots/cartesian/XYConvolutionCurve.h"
#include "backend/worksheet/plots/cartesian/XYCorrelationCurve.h"
#include "backend/worksheet/plots/cartesian/XYLineSimplificationCurve.h"
#include "backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "backend/worksheet/plots/cartesian/XYFourierTransformCurve.h"
#include "backend/worksheet/plots/cartesian/XYFunctionCurve.h"
#include "backend/worksheet/plots/cartesian/XYHilbertTransformCurve.h"
#include "backend/worksheet/plots/cartesian/XYIntegrationCurve.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "backend/worksheet/plots/cartesian/XYPiecewiseLinearFitCurve.h"
#include "backend/worksheet/plots/cartesian/XYSmoothCurve.h"

// worksheet
#include "backend/worksheet/Worksheet.h" // inherits AbstractPart, AbstractAspect, QObject

// worksheet element containers
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

// worksheet elements
#include "backend/worksheet/Image.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/TextLabel.h"

#ifdef SCRIPTING
// proxy for stdout/stderr
#include "backend/script/python/PythonLogger.h"
#endif

#endif // WRAPPEDCLASSES_H
