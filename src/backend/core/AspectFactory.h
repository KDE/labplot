/*
    File                 : AbstractAspect.h
    Project              : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007-2009 Tilman Benkert (thzs@gmx.net)
    SPDX-FileCopyrightText: 2007-2010 Knut Franke (knut.franke@gmx.de)
    SPDX-FileCopyrightText: 2011-2015 Alexander Semke (alexander.semke@web.de)
    Description          : Base class for all objects in a Project.
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASPECTFACTORY_H

#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/note/Note.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/worksheet/plots/cartesian/XYConvolutionCurve.h"
#include "backend/worksheet/plots/cartesian/XYCorrelationCurve.h"
#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"
#include "backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "backend/worksheet/plots/cartesian/XYFourierTransformCurve.h"
#include "backend/worksheet/plots/cartesian/XYIntegrationCurve.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "backend/worksheet/plots/cartesian/XYSmoothCurve.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/datapicker/DatapickerCurve.h"

class AspectFactory {
public:
	static AbstractAspect* createAspect(AspectType type, AbstractAspect* parent) {
		if (type == AspectType::Folder)
			return new Folder(QString());
		if (type == AspectType::Datapicker)
			return new Datapicker(QString());

		else if (type == AspectType::Note)
			return new Note(QString());
		/* worksheet and all its children */
		else if (type == AspectType::Worksheet)
			return new Worksheet(QString());
		else if (type == AspectType::CartesianPlot)
			return new CartesianPlot(QString());
		else if (type == AspectType::TextLabel)
			return new TextLabel(QString());
		else if (type == AspectType::Image)
			return new Image(QString());
		else if (type == AspectType::CustomPoint) {
			auto* plot = static_cast<CartesianPlot*>(parent);
			return new CustomPoint(plot, QString());
		} else if (type == AspectType::ReferenceLine) {
			auto* plot = static_cast<CartesianPlot*>(parent);
			return new ReferenceLine(plot, QString());
		} else if (type == AspectType::InfoElement) {
			auto* plot = static_cast<CartesianPlot*>(parent);
			return new InfoElement(QString(), plot);
		}

		/* CartesianPlot's children */
		else if (type == AspectType::Axis)
			return new Axis(QString());
		else if (type == AspectType::XYCurve)
			return new XYCurve(QString());
		else if (type == AspectType::XYEquationCurve)
			return new XYEquationCurve(QString());
		else if (type == AspectType::XYConvolutionCurve)
			return new XYConvolutionCurve(QString());
		else if (type == AspectType::XYCorrelationCurve)
			return new XYCorrelationCurve(QString());
		else if (type == AspectType::XYDataReductionCurve)
			return new XYDataReductionCurve(QString());
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
		else if (type == AspectType::Histogram)
			return new Histogram(QString());
		else if (type == AspectType::BoxPlot)
			return new BoxPlot(QString());
		else if (type == AspectType::CartesianPlotLegend)
			return new CartesianPlotLegend(QString());

		/* spreadsheet and its children */
		else if (type == AspectType::Spreadsheet)
			return new Spreadsheet(QString(), true /*loading*/);
		else if (type == AspectType::Column)
			return new Column(QString());

		else if (type == AspectType::Matrix)
			return new Matrix(QString());
		else if (type == AspectType::Workbook)
			return new Workbook(QString());

		return nullptr;
	}
};

#endif
