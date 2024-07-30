// <!-- src/backend/core -->
#include "./backend/core/AbstractAspect.h"
#include "./backend/core/AbstractColumn.h"
#include "./backend/core/AbstractFilter.h"
#include "./backend/core/AbstractSimpleFilter.h"
#include "./backend/core/AbstractPart.h"

// <!-- src/backend/core/column -->
#include "./backend/core/column/Column.h"

// <!-- src/backend/datasources -->
#include "./backend/datasources/AbstractDataSource.h"

// <!-- src/backend/datasources/filters -->
#include "./backend/datasources/filters/AbstractFileFilter.h"
#include "./backend/datasources/filters/AsciiFilter.h"
#include "./backend/datasources/filters/BinaryFilter.h"
#include "./backend/datasources/filters/CANFilter.h"
#include "./backend/datasources/filters/FITSFilter.h"
#include "./backend/datasources/filters/HDF5Filter.h"
#include "./backend/datasources/filters/ImageFilter.h"
#include "./backend/datasources/filters/JsonFilter.h"
#include "./backend/datasources/filters/MatioFilter.h"
#include "./backend/datasources/filters/NetCDFFilter.h"
#include "./backend/datasources/filters/OdsFilter.h"
#include "./backend/datasources/filters/ReadStatFilter.h"
#include "./backend/datasources/filters/ROOTFilter.h"
#include "./backend/datasources/filters/SpiceFilter.h"
#include "./backend/datasources/filters/VectorBLFFilter.h"
#include "./backend/datasources/filters/XLSXFilter.h"

// <!-- src/backend/lib -->
#include "./backend/lib/XmlStreamReader.h"

// <!-- src/backend/spreadsheet -->
#include "./backend/spreadsheet/Spreadsheet.h"
#include "./backend/spreadsheet/StatisticsSpreadsheet.h"

// <!-- src/backend/worksheet -->
#include "./backend/worksheet/Background.h"
#include "./backend/worksheet/Image.h"
#include "./backend/worksheet/InfoElement.h"
#include "./backend/worksheet/Line.h"
#include "./backend/worksheet/ResizeItem.h"
#include "./backend/worksheet/TextLabel.h"
#include "./backend/worksheet/Worksheet.h"
#include "./backend/worksheet/WorksheetElement.h"
#include "./backend/worksheet/WorksheetElementContainer.h"
#include "./backend/worksheet/WorksheetElementGroup.h"

// <!-- src/backend/worksheet/plots -->
#include "./backend/worksheet/plots/AbstractCoordinateSystem.h"
#include "./backend/worksheet/plots/AbstractPlot.h"
#include "./backend/worksheet/plots/PlotArea.h"

// <!-- src/backend/worksheet/plots/cartesian -->
#include "./backend/worksheet/plots/cartesian/Axis.h"
#include "./backend/worksheet/plots/cartesian/BarPlot.h"
#include "./backend/worksheet/plots/cartesian/BoxPlot.h"
#include "./backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "./backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "./backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "./backend/worksheet/plots/cartesian/CartesianScale.h"
#include "./backend/worksheet/plots/cartesian/CustomPoint.h"
#include "./backend/worksheet/plots/cartesian/ErrorBar.h"
#include "./backend/worksheet/plots/cartesian/Histogram.h"
#include "./backend/worksheet/plots/cartesian/KDEPlot.h"
#include "./backend/worksheet/plots/cartesian/LollipopPlot.h"
#include "./backend/worksheet/plots/cartesian/Plot.h"
#include "./backend/worksheet/plots/cartesian/QQPlot.h"
#include "./backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "./backend/worksheet/plots/cartesian/ReferenceRange.h"
#include "./backend/worksheet/plots/cartesian/Symbol.h"
#include "./backend/worksheet/plots/cartesian/Value.h"
#include "./backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
#include "./backend/worksheet/plots/cartesian/XYConvolutionCurve.h"
#include "./backend/worksheet/plots/cartesian/XYCorrelationCurve.h"
#include "./backend/worksheet/plots/cartesian/XYCurve.h"
#include "./backend/worksheet/plots/cartesian/XYDataReductionCurve.h"
#include "./backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"
#include "./backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "./backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "./backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "./backend/worksheet/plots/cartesian/XYFourierTransformCurve.h"
#include "./backend/worksheet/plots/cartesian/XYHilbertTransformCurve.h"
#include "./backend/worksheet/plots/cartesian/XYIntegrationCurve.h"
#include "./backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "./backend/worksheet/plots/cartesian/XYSmoothCurve.h"

// src/backend/core
#include "src/backend/core/AbstractAspect.h"
#include "src/backend/core/AbstractPart.h"
#include "src/backend/core/Folder.h" // inherits AbstractAspect, QObject
#include "src/backend/core/Project.h" // inherits Folder, AbstractAspect, QObject
#include "src/backend/core/Workbook.h" // inherits AbstractPart, AbstractAspect, QObject

// src/backend/datasources
#include "src/backend/datasources/AbstractDataSource.h"

// src/backend/spreadsheet
#include "src/backend/spreadsheet/Spreadsheet.h" // inherits AbstractDataSource, AbstractPart, AbstractAspect, QObject

// src/backend/matrix
#include "src/backend/matrix/Matrix.h" // inherits AbstractDataSource, AbstractPart, AbstractAspect, QObject

// src/backend/worksheet
#include "src/backend/worksheet/Worksheet.h" // inherits AbstractPart, AbstractAspect, QObject

// src/backend/note
#include "src/backend/note/Note.h" // inherits AbstractPart, AbstractAspect, QObject

// src/backend/datapicker
#include "src/backend/datapicker/Datapicker.h" // inherits AbstractPart, AbstractAspect, QObject
