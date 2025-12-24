/*
	File             : XYBaselineCorrectionCurve.cpp
	Project          : LabPlot
	Description      : A xy-curve defined by baseline correction
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYBaselineCorrectionCurve.h"
#include "XYBaselineCorrectionCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"

#include <KLocalizedString>

#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>

/*
 * Implementation follows the patterns used in other XYAnalysisCurve derived classes
 */

XYBaselineCorrectionCurve::XYBaselineCorrectionCurve(const QString& name)
	: XYAnalysisCurve(name, new XYBaselineCorrectionCurvePrivate(this), AspectType::XYBaselineCorrectionCurve) {
}

XYBaselineCorrectionCurve::XYBaselineCorrectionCurve(const QString& name, XYBaselineCorrectionCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYBaselineCorrectionCurve) {
}

XYBaselineCorrectionCurve::~XYBaselineCorrectionCurve() = default;

const XYAnalysisCurve::Result& XYBaselineCorrectionCurve::result() const {
	Q_D(const XYBaselineCorrectionCurve);
	return d->baselineResult;
}

QIcon XYBaselineCorrectionCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-curve"));
}

BASIC_SHARED_D_READER_IMPL(XYBaselineCorrectionCurve, XYBaselineCorrectionCurve::BaselineData, baselineData, baselineData)

const XYBaselineCorrectionCurve::BaselineResult& XYBaselineCorrectionCurve::baselineResult() const {
	Q_D(const XYBaselineCorrectionCurve);
	return d->baselineResult;
}

STD_SETTER_CMD_IMPL_F_S(XYBaselineCorrectionCurve, SetBaselineData, XYBaselineCorrectionCurve::BaselineData, baselineData, recalculate)
void XYBaselineCorrectionCurve::setBaselineData(const XYBaselineCorrectionCurve::BaselineData& baselineData) {
	Q_D(XYBaselineCorrectionCurve);
	exec(new XYBaselineCorrectionCurveSetBaselineDataCmd(d, baselineData, ki18n("%1: set baseline options and perform correction")));
}

XYBaselineCorrectionCurvePrivate::XYBaselineCorrectionCurvePrivate(XYBaselineCorrectionCurve* owner)
	: XYAnalysisCurvePrivate(owner)
	, q(owner) {
}

XYBaselineCorrectionCurvePrivate::~XYBaselineCorrectionCurvePrivate() = default;

void XYBaselineCorrectionCurvePrivate::resetResults() {
	baselineResult = XYBaselineCorrectionCurve::BaselineResult();
}

bool XYBaselineCorrectionCurvePrivate::recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) {
	QElapsedTimer timer;
	timer.start();

	// copy y data (and x if available) into vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;

	double xmin, xmax;
	if (tmpXDataColumn && baselineData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = baselineData.xRange.first();
		xmax = baselineData.xRange.last();
	}

	if (tmpXDataColumn) {
		for (int row = 0; row < tmpXDataColumn->rowCount(); ++row) {
			if (tmpXDataColumn->isValid(row) && !tmpXDataColumn->isMasked(row) && tmpYDataColumn->isValid(row) && !tmpYDataColumn->isMasked(row)) {
				const double xv = tmpXDataColumn->valueAt(row);
				if (xv >= xmin && xv <= xmax) {
					xdataVector.append(xv);
					ydataVector.append(tmpYDataColumn->valueAt(row));
				}
			}
		}
	} else {
		for (int row = 0; row < tmpYDataColumn->rowCount(); ++row) {
			if (tmpYDataColumn->isValid(row) && !tmpYDataColumn->isMasked(row))
				ydataVector.append(tmpYDataColumn->valueAt(row));
		}
	}

	const size_t n = (size_t)ydataVector.size();
	if (n < 1) {
		baselineResult.available = true;
		baselineResult.valid = false;
		baselineResult.status = i18n("Not enough data points available.");
		return true;
	}

	double* ydata = ydataVector.data();

	int status = 0;

	switch (baselineData.method) {
	case nsl_diff_baseline_correction_minimum:
		nsl_baseline_remove_minimum(ydata, n);
		break;
	case nsl_diff_baseline_correction_maximum:
		nsl_baseline_remove_maximum(ydata, n);
		break;
	case nsl_diff_baseline_correction_mean:
		nsl_baseline_remove_mean(ydata, n);
		break;
	case nsl_diff_baseline_correction_median:
		nsl_baseline_remove_median(ydata, n);
		break;
	case nsl_diff_baseline_correction_endpoints:
		if (tmpXDataColumn)
			status = nsl_baseline_remove_endpoints(xdataVector.data(), ydata, n);
		else
			status = -1;
		break;
	case nsl_diff_baseline_correction_linear_regression:
		if (tmpXDataColumn)
			status = nsl_baseline_remove_linreg(xdataVector.data(), ydata, n);
		else
			status = -1;
		break;
	case nsl_diff_baseline_correction_arpls: {
		const double p = baselineData.arPLSTerminationRatio;
		const double lambda = pow(baselineData.arPLSSmoothness, 10);
		const int niter = baselineData.arPLSIterations;
		double tol = nsl_baseline_remove_arpls(ydata, n, p, lambda, niter);
		Q_UNUSED(tol);
		break;
	}
	}

	// write result into xVector/yVector (subtracted signal)
	xVector->resize((int)n);
	yVector->resize((int)n);
	if (tmpXDataColumn) {
		memcpy(xVector->data(), xdataVector.data(), n * sizeof(double));
	} else {
		for (size_t i = 0; i < n; ++i)
			xVector->data()[(int)i] = (double)i;
	}
	memcpy(yVector->data(), ydata, n * sizeof(double));

	baselineResult.available = true;
	baselineResult.valid = (status == 0);
	baselineResult.status = QString::number(status);
	baselineResult.elapsedTime = timer.elapsed();

	return true;
}

// Serialization/Deserialization
void XYBaselineCorrectionCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYBaselineCorrectionCurve);
	writer->writeStartElement(QStringLiteral("xyBaselineCorrectionCurve"));
	XYAnalysisCurve::save(writer);

	writer->writeStartElement(QStringLiteral("baselineData"));
	writer->writeAttribute(QStringLiteral("method"), QString::number((int)d->baselineData.method));
	writer->writeAttribute(QStringLiteral("autoRange"), QString::number(d->baselineData.autoRange));
	writer->writeAttribute(QStringLiteral("xRangeMin"), QString::number(d->baselineData.xRange.first()));
	writer->writeAttribute(QStringLiteral("xRangeMax"), QString::number(d->baselineData.xRange.last()));
	writer->writeAttribute(QStringLiteral("arPLSSmoothness"), QString::number(d->baselineData.arPLSSmoothness));
	writer->writeAttribute(QStringLiteral("arPLSTerminationRatio"), QString::number(d->baselineData.arPLSTerminationRatio));
	writer->writeAttribute(QStringLiteral("arPLSIterations"), QString::number(d->baselineData.arPLSIterations));
	writer->writeEndElement(); // baselineData

	qDebug()<<"in write " << d->baselineData.arPLSTerminationRatio;

	writer->writeStartElement(QStringLiteral("baselineResult"));
	writer->writeAttribute(QStringLiteral("available"), QString::number(d->baselineResult.available));
	writer->writeAttribute(QStringLiteral("valid"), QString::number(d->baselineResult.valid));
	writer->writeAttribute(QStringLiteral("status"), d->baselineResult.status);
	writer->writeAttribute(QStringLiteral("time"), QString::number(d->baselineResult.elapsedTime));
	if (saveCalculations() && d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); // baselineResult

	writer->writeEndElement(); // xyBaselineCorrectionCurve
}

bool XYBaselineCorrectionCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYBaselineCorrectionCurve);

	QXmlStreamAttributes attribs;
    QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyBaselineCorrectionCurve"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyAnalysisCurve")) {
			if (!XYAnalysisCurve::load(reader, preview))
				return false;
		} else if (!preview && reader->name() == QLatin1String("baselineData")) {
			attribs = reader->attributes();
			READ_INT_VALUE("method", baselineData.method, nsl_baseline_correction_method);
			READ_INT_VALUE("autoRange", baselineData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", baselineData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", baselineData.xRange.last());
			READ_INT_VALUE("arPLSSmoothness", baselineData.arPLSSmoothness, int);
			READ_DOUBLE_VALUE("arPLSTerminationRatio", baselineData.arPLSTerminationRatio);
			READ_INT_VALUE("arPLSIterations", baselineData.arPLSIterations, int);
		} else if (!preview && reader->name() == QLatin1String("baselineResult")) {
			attribs = reader->attributes();
			READ_INT_VALUE("available", baselineResult.available, int);
			READ_INT_VALUE("valid", baselineResult.valid, int);
			READ_STRING_VALUE("status", baselineResult.status);
			READ_INT_VALUE("time", baselineResult.elapsedTime, int);
		} else if (!preview && reader->name() == QLatin1String("column")) {
			auto* column = new Column(QString(), AbstractColumn::ColumnMode::Double);
			if (!column->load(reader, preview)) {
				delete column;
				return false;
			}
			if (column->name() == QLatin1String("x"))
				d->xColumn = column;
			else if (column->name() == QLatin1String("y"))
				d->yColumn = column;
		} else {
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	if (preview)
		return true;

	QThreadPool::globalInstance()->waitForDone();

	if (d->xColumn && d->yColumn) {
		d->xColumn->setHidden(true);
		addChild(d->xColumn);

		d->yColumn->setHidden(true);
		addChild(d->yColumn);

		d->xVector = static_cast<QVector<double>*>(d->xColumn->data());
		d->yVector = static_cast<QVector<double>*>(d->yColumn->data());

		static_cast<XYCurvePrivate*>(d_ptr)->xColumn = d->xColumn;
		static_cast<XYCurvePrivate*>(d_ptr)->yColumn = d->yColumn;

		recalc();
	}

	return true;
}
