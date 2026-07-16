#ifndef LABPLOT_SDK
#define LABPLOT_SDK

#define SDK

#include "backend/datasources/filters/filters.h"

#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include "backend/core/Folder.h"
#include "backend/core/Project.h"

#include "backend/core/column/Column.h"

#include "backend/worksheet/Worksheet.h"

#include "backend/worksheet/Image.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/TextLabel.h"

#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"

#include "backend/worksheet/plots/cartesian/plots.h"

#include "backend/core/AbstractAspect.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/AbstractPart.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/WorksheetElementContainer.h"
#include "backend/worksheet/plots/AbstractCoordinateSystem.h"
#include "backend/worksheet/plots/AbstractPlot.h"
#include "backend/worksheet/plots/cartesian/Plot.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

#include "backend/core/column/ColumnStringIO.h"
#include "backend/lib/Range.h"
#include "backend/spreadsheet/StatisticsSpreadsheet.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/ErrorBar.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/cartesian/Value.h"

#endif