/*
	File                 : XYEquationCurve2.cpp
	Project              : LabPlot
	Description          : A xy-curve defined by a mathematical equation from other curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class XYEquationCurve2
  \brief A xy-curve defined by a mathematical equation

  \ingroup worksheet
*/

#include "XYEquationCurve2.h"
#include "XYEquationCurve2Private.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Folder.h"
#include "backend/core/column/Column.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <KLocalizedString>
#include <QIcon>

XYEquationCurve2::XYEquationCurve2(const QString& name)
	: XYAnalysisCurve(name, new XYEquationCurve2Private(this), AspectType::XYEquationCurve2) {
	init();
}

XYEquationCurve2::XYEquationCurve2(const QString& name, XYEquationCurve2Private* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYEquationCurve2) {
	init();
}

void XYEquationCurve2::init() {
	Q_D(XYEquationCurve2);

	d->resultColumn->setHidden(true);
	addChildFast(d->resultColumn);

	// TODO: read from the saved settings for XYEquationCurve2?
	d->lineType = XYCurve::LineType::Line;
	d->symbol->setStyle(Symbol::Style::NoSymbols);

	setUndoAware(false);
	setSuppressRetransform(true);
	setYColumn(d->resultColumn); // Currently only y column as result column supported
	setSuppressRetransform(false);
	setUndoAware(true);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYEquationCurve2::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-equation-curve"));
}

const XYAnalysisCurve::Result& XYEquationCurve2::result() const {
	Q_D(const XYEquationCurve2);
	return d->m_result;
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
// BASIC_SHARED_D_READER_IMPL(XYEquationCurve2, XYEquationCurve2::EquationData, equationData, equationData)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
// STD_SETTER_CMD_IMPL_F_S(XYEquationCurve2, SetEquationData, XYEquationCurve2::EquationData, equationData, recalculate)
// void XYEquationCurve2::setEquationData(const XYEquationCurve2::EquationData& equationData) {
// 	Q_D(XYEquationCurve2);/*
// 	if (equationData.expression != d->equationData.expression)
// 		exec(new XYEquationCurve2SetEquationDataCmd(d, equationData, ki18n("%1: set equation")));*/
// }

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################
class CurveSetGlobalEquationCmd : public QUndoCommand {
public:
	explicit CurveSetGlobalEquationCmd(XYEquationCurve2Private* curve,
									   QString equation,
									   QStringList variableNames,
									   QVector<XYCurve*> variableCurves,
									   QUndoCommand* parent = nullptr)	: QUndoCommand(parent)
		, m_curve(curve)
		, m_newEquation(std::move(equation))
		, m_newVariableNames(std::move(variableNames))
		, m_newVariableCurves(std::move(variableCurves)) {
		setText(i18n("%1: set equation", curve->name()));
	}

	void redo() override {
		if (!m_copied) {
			m_equation = m_curve->equation();
			for (auto& d : m_curve->equationData()) {
				m_variableNames << d.variableName();
				m_variableCurves << d.m_curve;
			}
			m_copied = true;
		}

		QVector<XYEquationCurve2::EquationData> equationData;
		for (int i = 0; i < m_newVariableNames.count(); i++)
			if (i < m_newVariableCurves.size()) // names may be defined but without column
				equationData << XYEquationCurve2::EquationData(m_newVariableNames.at(i), m_newVariableCurves.at(i));

		m_curve->setEquation(m_newEquation, equationData);
	}

	void undo() override {
		QVector<XYEquationCurve2::EquationData> equationData;
		for (int i = 0; i < m_variableNames.count(); i++)
			equationData << XYEquationCurve2::EquationData(m_variableNames.at(i), m_variableCurves.at(i));
		m_curve->setEquation(m_equation, equationData);
	}

private:
	XYEquationCurve2Private* m_curve;
	QString m_equation;
	QStringList m_variableNames;
	QVector<XYCurve*> m_variableCurves;
	QString m_newEquation;
	QStringList m_newVariableNames;
	QVector<XYCurve*> m_newVariableCurves;
	bool m_copied{false};
};


void XYEquationCurve2::setEquation(const QString& equation, const QStringList& variableNames, const QVector<XYCurve*>& curves) {
	Q_D(XYEquationCurve2);
	exec(new CurveSetGlobalEquationCmd(d, equation, variableNames, curves));
}

/**
 * \brief Clears the equation used to generate column values
 */
void XYEquationCurve2::clearEquation() {
	setEquation(QString(), QStringList(), QVector<XYCurve*>());
}

QString XYEquationCurve2::equation() const {
	Q_D(const XYEquationCurve2);
	return d->equation();
}

const QVector<XYEquationCurve2::EquationData>& XYEquationCurve2::equationData() const {
	Q_D(const XYEquationCurve2);
	return d->equationData();
}

void XYEquationCurve2::setEquationVariableCurve(XYCurve* c) {
	Q_D(XYEquationCurve2);
	d->setEquationVariableCurve(c);
}

void XYEquationCurve2::setEquationVariableCurvesPath(int index, const QString& path) {
	Q_D(XYEquationCurve2);
	d->setEquationVariableCurvesPath(index, path);
}

void XYEquationCurve2::setEquationVariableCurve(int index, XYCurve* curve) {
	Q_D(XYEquationCurve2);
	d->setEquationVariableCurve(index, curve);
}

void XYEquationCurve2::equationVariableCurveRemoved(const AbstractAspect* aspect) {
	Q_D(XYEquationCurve2);
	d->equationVariableCurveRemoved(aspect);
}

void XYEquationCurve2::equationVariableCurveAdded(const AbstractAspect* aspect) {
	Q_D(XYEquationCurve2);
	d->equationVariableCurveAdded(aspect);
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYEquationCurve2Private::XYEquationCurve2Private(XYEquationCurve2* owner)
	: XYAnalysisCurvePrivate(owner)
	, resultColumn(new Column(QStringLiteral("result"), AbstractColumn::ColumnMode::Double))
	, resultVector(static_cast<QVector<double>*>(resultColumn->data()))
	, q(owner) {
}

QString XYEquationCurve2Private::equation() const {
	return m_equation;
}

const QVector<XYEquationCurve2::EquationData>& XYEquationCurve2Private::equationData() const {
	return m_equationData;
}

void XYEquationCurve2Private::setEquation(const QString& equation,
							   const QStringList& variableNames,
							   const QStringList& variableCurvePaths) {
	m_equation = equation;
	m_equationData.clear();
	for (int i = 0; i < variableNames.count(); i++)
		m_equationData.append(XYEquationCurve2::EquationData(variableNames.at(i), variableCurvePaths.at(i)));

}

void XYEquationCurve2Private::setEquationVariableCurvesPath(int index, const QString& path) {
	if (!m_equationData[index].setCurvePath(path))
		DEBUG(Q_FUNC_INFO << ": For some reason, there was already a curve assigned");
}

void XYEquationCurve2Private::setEquationVariableCurve(int index, XYCurve *curve) {
	if (m_equationData.at(index).curve()) // if there exists already a valid curve, disconnect it first
		q->disconnect(m_equationData.at(index).m_curve, nullptr, q, nullptr);
	m_equationData[index].setCurve(curve);
	connectEquationCurve(curve);
}

void XYEquationCurve2Private::setEquationVariableCurve(XYCurve* c) {
	for (auto& d : m_equationData) {
		if (d.curveName() == c->path()) {
			d.setCurve(c);
			break;
		}
	}
}

void XYEquationCurve2Private::connectEquationCurve(const XYCurve* curve) {
	if (!curve)
		return;

   // avoid circular dependencies - the current curve cannot be part of the variable curve.
   // this should't actually happen because of the checks done when the equation is defined,
   // but in case we have bugs somewhere or somebody manipulated the project xml file we add
   // a sanity check to avoid recursive calls here and crash because of the stack overflow.
	if (curve == q)
		return;

	DEBUG(Q_FUNC_INFO)
	m_connectionsUpdateEquation << q->connect(curve, &XYCurve::changed, q, &XYEquationCurve2::recalculate);
	m_connectionsUpdateEquation << q->connect(curve, &AbstractAspect::aspectAboutToBeRemoved,q,&XYEquationCurve2::equationVariableCurveRemoved);
	m_connectionsUpdateEquation << q->connect(curve->parentAspect(), &AbstractAspect::childAspectAdded, q, &XYEquationCurve2::equationVariableCurveAdded);
}

void XYEquationCurve2Private::equationVariableCurveRemoved(const AbstractAspect* aspect) {
	const XYCurve* curve = dynamic_cast<const XYCurve*>(aspect);
	if (!curve)
		return;
	q->disconnect(curve, nullptr, q, nullptr);
	int index = -1;
	for (int i = 0; i < equationData().count(); i++) {
		auto& d = equationData().at(i);
		if (d.curve() == curve) {
			index = i;
			break;
		}
	}
	if (index != -1) {
		m_equationData[index].setCurve(nullptr);
		DEBUG(Q_FUNC_INFO << ", calling updateEquation()")
		recalculate();
	}
}

void XYEquationCurve2Private::equationVariableCurveAdded(const AbstractAspect* aspect) {
	auto* curve = dynamic_cast<XYCurve*>(const_cast<AbstractAspect*>(aspect));
	if (!curve)
		return;

	const auto& path = aspect->path();
	for (int i = 0; i < equationData().count(); i++) {
		if (equationData().at(i).curveName() == path) {
			// m_equationData[index].setColumn(const_cast<Column*>(column));
			// DEBUG(Q_FUNC_INFO << ", calling updateEquation()")
			setEquationVariableCurve(i, curve);
			recalculate();
			return;
		}
	}
}

/**
 * \brief Sets the equation used to generate column values
 */
void XYEquationCurve2Private::setEquation(const QString& equation, const QVector<XYEquationCurve2::EquationData>& equationData) {
	m_equation = equation;
	m_equationData = equationData; // TODO: disconnecting everything?

	for (auto& connection : m_connectionsUpdateEquation)
		if (static_cast<bool>(connection))
			q->disconnect(connection);

	for (const auto& data : qAsConst(m_equationData)) {
		const auto* curve = data.curve();
		if (curve)
			connectEquationCurve(curve);
	}
	recalculate();
}

void XYEquationCurve2Private::resetResults() {
	m_result = XYEquationCurve2::Result();
}

// ...
// see XYFitCurvePrivate
bool XYEquationCurve2Private::recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) {
	bool valid = true;

	int numberElements = 0;
	if (m_equationData.length() == 0) {
		valid = false;
	} else {
		const auto* curve = m_equationData.first().curve();
		if (!curve->xColumn() || !curve->yColumn() || curve->xColumn()->rowCount() != curve->yColumn()->rowCount())
			valid = false;
		else {
			numberElements = curve->xColumn()->rowCount();
		}
	}

	for (const auto& ed: qAsConst(m_equationData)) {
		const auto* curve = ed.curve();
		if (!curve || !curve->xColumn() || !curve->yColumn() || curve->xColumn()->rowCount() != numberElements || curve->yColumn()->rowCount() != numberElements) {
			valid = false;
			break;
		}
	}

	resultVector->clear();
	if (valid) {
		yVector->resize(m_equationData.first().curve()->xColumn()->rowCount());
		const auto* xColumn = dynamic_cast<const Column*>(m_equationData.first().curve()->xColumn());
		Q_ASSERT(xColumn);
		Q_ASSERT(xColumn->columnMode() == AbstractColumn::ColumnMode::Double);
		xVector = static_cast<QVector<double>*>(xColumn->data());

		QVector<QVector<double>*> xVectors;
		QStringList equationVariableNames;
		for (const auto& equationData : qAsConst(m_equationData)) {
			const auto* curve = equationData.curve();
			const auto& varName = equationData.variableName();
			const auto* column = dynamic_cast<const Column*>(curve->yColumn());
			Q_ASSERT(column);

			equationVariableNames << varName;

			if (column->columnMode() == AbstractColumn::ColumnMode::Integer || column->columnMode() == AbstractColumn::ColumnMode::BigInt) {
				// convert integers to doubles first
				auto* xVector = new QVector<double>(column->rowCount());
				for (int i = 0; i < column->rowCount(); ++i)
					(*xVector)[i] = column->valueAt(i);

				xVectors << xVector;
			} else
				xVectors << static_cast<QVector<double>*>(column->data());
		}

			   // const auto payload = std::make_shared<PayloadColumn>(m_equationData);

			   // evaluate the expression for f(x_1, x_2, ...) and write the calculated values into a new vector.
		auto* parser = ExpressionParser::getInstance();
		// parser->setSpecialFunction1(colfun_size, columnSize, payload);
		// parser->setSpecialFunction1(colfun_min, columnMin, payload);
		// parser->setSpecialFunction1(colfun_max, columnMax, payload);
		// parser->setSpecialFunction1(colfun_mean, columnMean, payload);
		// parser->setSpecialFunction1(colfun_median, columnMedian, payload);
		// parser->setSpecialFunction1(colfun_stdev, columnStdev, payload);
		// parser->setSpecialFunction1(colfun_var, columnVar, payload);
		// parser->setSpecialFunction1(colfun_gm, columnGm, payload);
		// parser->setSpecialFunction1(colfun_hm, columnHm, payload);
		// parser->setSpecialFunction1(colfun_chm, columnChm, payload);
		// parser->setSpecialFunction1(colfun_mode, columnStatisticsMode, payload);
		// parser->setSpecialFunction1(colfun_quartile1, columnQuartile1, payload);
		// parser->setSpecialFunction1(colfun_quartile3, columnQuartile3, payload);
		// parser->setSpecialFunction1(colfun_iqr, columnIqr, payload);
		// parser->setSpecialFunction1(colfun_percentile1, columnPercentile1, payload);
		// parser->setSpecialFunction1(colfun_percentile5, columnPercentile5, payload);
		// parser->setSpecialFunction1(colfun_percentile10, columnPercentile10, payload);
		// parser->setSpecialFunction1(colfun_percentile90, columnPercentile90, payload);
		// parser->setSpecialFunction1(colfun_percentile95, columnPercentile95, payload);
		// parser->setSpecialFunction1(colfun_percentile99, columnPercentile99, payload);
		// parser->setSpecialFunction1(colfun_trimean, columnTrimean, payload);
		// parser->setSpecialFunction1(colfun_meandev, columnMeandev, payload);
		// parser->setSpecialFunction1(colfun_meandevmedian, columnMeandevmedian, payload);
		// parser->setSpecialFunction1(colfun_mediandev, columnMediandev, payload);
		// parser->setSpecialFunction1(colfun_skew, columnSkew, payload);
		// parser->setSpecialFunction1(colfun_kurt, columnKurt, payload);
		// parser->setSpecialFunction1(colfun_entropy, columnEntropy, payload);
		// parser->setSpecialFunction2(colfun_percentile, columnPercentile, payload);
		// parser->setSpecialFunction2(colfun_quantile, columnQuantile, payload);
		parser->evaluateCartesian(m_equation, equationVariableNames, xVectors, resultVector);
	}
	resultColumn->invalidateProperties();
	return valid;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void XYEquationCurve2::save(QXmlStreamWriter* writer) const {
	Q_D(const XYEquationCurve2);

	writer->writeStartElement(QStringLiteral("XYEquationCurve2"));

	// write xy-curve information
	XYCurve::save(writer);

	// write xy-equationCurve specific information
	// save the equation used to generate column values, if available
	if (!equation().isEmpty()) {
		writer->writeStartElement(QStringLiteral("equation"));
		writer->writeTextElement(QStringLiteral("text"), equation());

		QStringList equationVariableNames;
		QStringList equationVariableColumnPaths;
		for (auto& d : equationData()) {
			equationVariableNames << d.variableName();
			equationVariableColumnPaths << d.curveName();
		}

		writer->writeStartElement(QStringLiteral("variableNames"));
		for (const auto& name : equationVariableNames)
			writer->writeTextElement(QStringLiteral("name"), name);
		writer->writeEndElement();

		writer->writeStartElement(QStringLiteral("curvePaths"));
		for (const auto& path : equationVariableColumnPaths)
			writer->writeTextElement(QStringLiteral("path"), path);
		writer->writeEndElement(); // curvePaths

		writer->writeEndElement(); // equation
	}
	writer->writeEndElement(); // XYEquationCurve2
}

//! Load from XML
bool XYEquationCurve2::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYEquationCurve2);

	// QXmlStreamAttributes attribs;
	// QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("XYEquationCurve2"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyCurve")) {
			if (!XYCurve::load(reader, preview))
				return false;
		} else if (reader->name() == QLatin1String("equation")) {
			if (!XmlReadEquation(reader))
				return false;
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	// Recalculate, otherwise xColumn and yColumn are not updated
	// and so autoscale is wrong
	recalculate();

	return true;
}

/**
 * \brief Read XML equation element
 */
bool XYEquationCurve2::XmlReadEquation(XmlStreamReader* reader) {
	Q_D(XYEquationCurve2);
	QString equation;
	QStringList variableNames;
	QStringList curvePaths;

	while (reader->readNext()) {
		if (reader->isEndElement())
			break;

		if (reader->name() == QLatin1String("text"))
			equation = reader->readElementText();
		else if (reader->name() == QLatin1String("variableNames")) {
			while (reader->readNext()) {
				if (reader->name() == QLatin1String("variableNames") && reader->isEndElement())
					break;

				if (reader->isStartElement())
					variableNames << reader->readElementText();
			}
		} else if (reader->name() == QLatin1String("curvePaths")) {
			while (reader->readNext()) {
				if (reader->name() == QLatin1String("curvePaths") && reader->isEndElement())
					break;

				if (reader->isStartElement())
					curvePaths << reader->readElementText();
			}
		}
	}

	d->setEquation(equation, variableNames, curvePaths);

	return true;
}
