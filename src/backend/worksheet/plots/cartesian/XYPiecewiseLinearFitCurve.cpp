/*
	File                 : XYPiecewiseLinearFitCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve defined by piecewise linear regression
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYPiecewiseLinearFitCurve.h"
#include "XYPiecewiseLinearFitCurvePrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QIcon>

using namespace std;

XYPiecewiseLinearFitCurve::XYPiecewiseLinearFitCurve(const QString& name)
	: XYAnalysisCurve(name, new XYPiecewiseLinearFitCurvePrivate(this), AspectType::XYPiecewiseLinearFitCurve) {
}

XYPiecewiseLinearFitCurve::XYPiecewiseLinearFitCurve(const QString& name, XYPiecewiseLinearFitCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYPiecewiseLinearFitCurve) {
}

XYPiecewiseLinearFitCurve::~XYPiecewiseLinearFitCurve() = default;

const XYAnalysisCurve::Result& XYPiecewiseLinearFitCurve::result() const {
	Q_D(const XYPiecewiseLinearFitCurve);
	return d->fitResult;
}

const XYPiecewiseLinearFitCurve::FitResult& XYPiecewiseLinearFitCurve::fitResult() const {
	Q_D(const XYPiecewiseLinearFitCurve);
	return d->fitResult;
}

QIcon XYPiecewiseLinearFitCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve"));
}

BASIC_D_READER_IMPL(XYPiecewiseLinearFitCurve, XYPiecewiseLinearFitCurve::FitData, fitData, fitData)

STD_SETTER_CMD_IMPL_S(XYPiecewiseLinearFitCurve, SetFitData, XYPiecewiseLinearFitCurve::FitData, fitData)
void XYPiecewiseLinearFitCurve::setFitData(const XYPiecewiseLinearFitCurve::FitData& fitData) {
	Q_D(XYPiecewiseLinearFitCurve);
	exec(new XYPiecewiseLinearFitCurveSetFitDataCmd(d, fitData, ki18n("%1: set fit options and perform the fit")));
}

void XYPiecewiseLinearFitCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYPiecewiseLinearFitCurve);

	writer->writeStartElement(QStringLiteral("xyPiecewiseLinearFitCurve"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement(QStringLiteral("fitData"));
	writer->writeAttribute(QStringLiteral("changepointMethod"), QString::number(d->fitData.changepointMethod));
	writer->writeAttribute(QStringLiteral("penalty"), QString::number(d->fitData.penalty));
	writer->writeAttribute(QStringLiteral("minSegmentSize"), QString::number(d->fitData.minSegmentSize));
	writer->writeAttribute(QStringLiteral("maxChangepoints"), QString::number(d->fitData.maxChangepoints));
	writer->writeAttribute(QStringLiteral("autoRange"), QString::number(d->fitData.autoRange));
	writer->writeAttribute(QStringLiteral("autoEvalRange"), QString::number(d->fitData.autoEvalRange));
	writer->writeAttribute(QStringLiteral("fitRangeMin"), QString::number(d->fitData.fitRange.start()));
	writer->writeAttribute(QStringLiteral("fitRangeMax"), QString::number(d->fitData.fitRange.end()));
	writer->writeAttribute(QStringLiteral("evalRangeMin"), QString::number(d->fitData.evalRange.start()));
	writer->writeAttribute(QStringLiteral("evalRangeMax"), QString::number(d->fitData.evalRange.end()));
	writer->writeAttribute(QStringLiteral("evaluatedPoints"), QString::number(d->fitData.evaluatedPoints));
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("fitResult"));
	writer->writeAttribute(QStringLiteral("available"), QString::number(d->fitResult.available));
	writer->writeAttribute(QStringLiteral("valid"), QString::number(d->fitResult.valid));
	writer->writeAttribute(QStringLiteral("status"), d->fitResult.status);
	writer->writeAttribute(QStringLiteral("numSegments"), QString::number(d->fitResult.numSegments));
	writer->writeAttribute(QStringLiteral("overallRsquare"), QString::number(d->fitResult.overallRsquare));
	writer->writeAttribute(QStringLiteral("sse"), QString::number(d->fitResult.sse));
	writer->writeEndElement();

	XYAnalysisCurve::save(writer);
	writer->writeEndElement();
}

bool XYPiecewiseLinearFitCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYPiecewiseLinearFitCurve);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyPiecewiseLinearFitCurve"))
			break;

		if (reader->isStartElement()) {
			if (reader->name() == QLatin1String("fitData")) {
				attribs = reader->attributes();
				d->fitData.changepointMethod = static_cast<nsl_changepoint_method>(attribs.value(QStringLiteral("changepointMethod")).toInt());
				d->fitData.penalty = attribs.value(QStringLiteral("penalty")).toDouble();
				d->fitData.minSegmentSize = attribs.value(QStringLiteral("minSegmentSize")).toULongLong();
				d->fitData.maxChangepoints = attribs.value(QStringLiteral("maxChangepoints")).toULongLong();
				d->fitData.autoRange = attribs.value(QStringLiteral("autoRange")).toInt();
				d->fitData.autoEvalRange = attribs.value(QStringLiteral("autoEvalRange")).toInt();
				d->fitData.fitRange.setStart(attribs.value(QStringLiteral("fitRangeMin")).toDouble());
				d->fitData.fitRange.setEnd(attribs.value(QStringLiteral("fitRangeMax")).toDouble());
				d->fitData.evalRange.setStart(attribs.value(QStringLiteral("evalRangeMin")).toDouble());
				d->fitData.evalRange.setEnd(attribs.value(QStringLiteral("evalRangeMax")).toDouble());
				d->fitData.evaluatedPoints = attribs.value(QStringLiteral("evaluatedPoints")).toULongLong();
			} else if (reader->name() == QLatin1String("fitResult")) {
				attribs = reader->attributes();
				d->fitResult.available = attribs.value(QStringLiteral("available")).toInt();
				d->fitResult.valid = attribs.value(QStringLiteral("valid")).toInt();
				d->fitResult.status = attribs.value(QStringLiteral("status")).toString();
				d->fitResult.numSegments = attribs.value(QStringLiteral("numSegments")).toULongLong();
				d->fitResult.overallRsquare = attribs.value(QStringLiteral("overallRsquare")).toDouble();
				d->fitResult.sse = attribs.value(QStringLiteral("sse")).toDouble();
			} else if (!XYAnalysisCurve::load(reader, preview)) {
				return false;
			}
		}
	}

	return true;
}
