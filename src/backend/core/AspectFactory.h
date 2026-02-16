/*
	File                 : AspectFactory.h
	Project              : LabPlot
	Description          : Factory to create an object instance from its type
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASPECTFACTORY_H

#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/timeseriesanalysis/SeasonalDecomposition.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/ScriptWorksheetElement.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"
#include "backend/worksheet/plots/cartesian/plots.h"
#ifndef SDK
#include "backend/core/Workbook.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/matrix/Matrix.h"
// #include "backend/datasources/LiveDataSource.h"
#include "backend/note/Note.h"
#endif

class AspectFactory {
public:
	static AbstractAspect* createAspect(AspectType type, AbstractAspect* parent) {
		if (type == AspectType::Folder)
			return new Folder(QString());

		/* worksheet and all its children */
		else if (type == AspectType::Worksheet)
			return new Worksheet(QString());
		else if (type == AspectType::CartesianPlot)
			return new CartesianPlot(QString(), true /*loading*/);
		else if (type == AspectType::TextLabel)
			return new TextLabel(QString());
		else if (type == AspectType::Image)
			return new Image(QString());
		else if (type == AspectType::ScriptWorksheetElement)
			return new ScriptWorksheetElement(QString());
		else if (type == AspectType::CustomPoint) {
			auto* plot = static_cast<CartesianPlot*>(parent);
			return new CustomPoint(plot, QString(), true /*loading*/);
		} else if (type == AspectType::ReferenceLine) {
			auto* plot = static_cast<CartesianPlot*>(parent);
			return new ReferenceLine(plot, QString(), true /*loading*/);
		} else if (type == AspectType::ReferenceRange) {
			auto* plot = static_cast<CartesianPlot*>(parent);
			return new ReferenceRange(plot, QString(), true /*loading*/);
		} else if (type == AspectType::InfoElement) {
			auto* plot = static_cast<CartesianPlot*>(parent);
			return new InfoElement(QString(), plot);
		}

		/* CartesianPlot's children */
		else if (type == AspectType::Axis)
			return new Axis(QString());
		else if (type == AspectType::XYCurve)
			return new XYCurve(QString());
		else if (type == AspectType::XYBaselineCorrectionCurve)
			return new XYBaselineCorrectionCurve(QString());
		else if (type == AspectType::XYEquationCurve)
			return new XYEquationCurve(QString());
		else if (type == AspectType::XYConvolutionCurve)
			return new XYConvolutionCurve(QString());
		else if (type == AspectType::XYCorrelationCurve)
			return new XYCorrelationCurve(QString());
		else if (type == AspectType::XYLineSimplificationCurve)
			return new XYLineSimplificationCurve(QString());
		else if (type == AspectType::XYDifferentiationCurve)
			return new XYDifferentiationCurve(QString());
		else if (type == AspectType::XYFitCurve)
			return new XYFitCurve(QString());
		else if (type == AspectType::XYFourierFilterCurve)
			return new XYFourierFilterCurve(QString());
		else if (type == AspectType::XYFourierTransformCurve)
			return new XYFourierTransformCurve(QString());
		else if (type == AspectType::XYIntegrationCurve)
			return new XYIntegrationCurve(QString());
		else if (type == AspectType::XYInterpolationCurve)
			return new XYInterpolationCurve(QString());
		else if (type == AspectType::XYSmoothCurve)
			return new XYSmoothCurve(QString());
		else if (type == AspectType::CartesianPlotLegend)
			return new CartesianPlotLegend(QString());

		/* statistical plots */
		else if (type == AspectType::BoxPlot)
			return new BoxPlot(QString(), true /*loading*/);
		else if (type == AspectType::Histogram)
			return new Histogram(QString(), true /*loading*/);
		else if (type == AspectType::KDEPlot)
			return new KDEPlot(QString());
		else if (type == AspectType::QQPlot)
			return new QQPlot(QString());

		/* bar plots */
		else if (type == AspectType::BarPlot)
			return new BarPlot(QString());
		else if (type == AspectType::LollipopPlot)
			return new LollipopPlot(QString());

		/* continuous improvement plots */
		else if (type == AspectType::ProcessBehaviorChart)
			return new ProcessBehaviorChart(QString(), true /*loading*/);
		else if (type == AspectType::RunChart)
			return new RunChart(QString());

		/* data containers */
		else if (type == AspectType::Spreadsheet)
			return new Spreadsheet(QString(), true /*loading*/);
		else if (type == AspectType::Column)
			return new Column(QString());

		/* time series analysis */
		else if (type == AspectType::SeasonalDecomposition)
			return new SeasonalDecomposition(QString(), true /*loading*/);
#ifndef SDK
		else if (type == AspectType::Matrix)
			return new Matrix(QString(), true /*loading*/);
		else if (type == AspectType::Datapicker)
			return new Datapicker(QString());
		else if (type == AspectType::Note)
			return new Note(QString());
		else if (type == AspectType::Workbook)
			return new Workbook(QString());
#endif
		return nullptr;
	}
};

#endif
