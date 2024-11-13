/*
	File                 : XYFunctionCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve that is calculated as a function of other curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class XYFunctionCurve
  \brief A xy-curve that is calculated as a function of other curves

  \ingroup worksheet
*/

#include "XYFunctionCurve.h"
#include "XYFunctionCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QIcon>

XYFunctionCurve::XYFunctionCurve(const QString& name)
	: XYAnalysisCurve(name, new XYFunctionCurvePrivate(this), AspectType::XYFunctionCurve) {
	init();
}

XYFunctionCurve::XYFunctionCurve(const QString& name, XYFunctionCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYFunctionCurve) {
	init();
}

void XYFunctionCurve::init() {
	Q_D(XYFunctionCurve);

	// TODO: read from the saved settings for XYFunctionCurve?
	d->lineType = XYCurve::LineType::Line;
	d->symbol->setStyle(Symbol::Style::NoSymbols);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYFunctionCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-equation-curve"));
}

const XYAnalysisCurve::Result& XYFunctionCurve::result() const {
	Q_D(const XYFunctionCurve);
	return d->m_result;
}

void XYFunctionCurve::handleAspectUpdated(const QString& aspectPath, const AbstractAspect* element) {
	Q_D(XYFunctionCurve);
	d->handleAspectUpdated(aspectPath, element);
}

bool XYFunctionCurve::usingColumn(const AbstractColumn* column, bool indirect) const {
	if (indirect) {
		for (const auto& d : functionData()) {
			const auto* curve = d.curve();
			if (curve && curve->usingColumn(column, indirect))
				return true;
		}
	}
	return false; // Does not directly use the curves
}

QVector<const Plot*> XYFunctionCurve::dependingPlots() const {
	QVector<const Plot*> plots;
	for (const auto& d : functionData()) {
		const auto* curve = d.curve();
		if (curve)
			plots.append(curve);
	}
	return plots;
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

class CurveSetGlobalFunctionCmd : public QUndoCommand {
public:
	explicit CurveSetGlobalFunctionCmd(XYFunctionCurvePrivate* curve,
									   QString function,
									   QStringList variableNames,
									   QVector<const XYCurve*> variableCurves,
									   QUndoCommand* parent = nullptr)
		: QUndoCommand(parent)
		, m_curve(curve)
		, m_newFunction(std::move(function))
		, m_newVariableNames(std::move(variableNames))
		, m_newVariableCurves(std::move(variableCurves)) {
		setText(i18n("%1: set function", curve->name()));
	}

	void redo() override {
		if (!m_copied) {
			m_function = m_curve->function();
			for (auto& d : m_curve->functionData()) {
				m_variableNames << d.variableName();
				m_variableCurves << d.m_curve;
			}
			m_copied = true;
		}

		QVector<XYFunctionCurve::FunctionData> functionData;
		for (int i = 0; i < m_newVariableNames.count(); i++)
			if (i < m_newVariableCurves.size()) // names may be defined but without column
				functionData << XYFunctionCurve::FunctionData(m_newVariableNames.at(i), m_newVariableCurves.at(i));

		m_curve->setFunction(m_newFunction, functionData);
	}

	void undo() override {
		QVector<XYFunctionCurve::FunctionData> functionData;
		for (int i = 0; i < m_variableNames.count(); i++)
			functionData << XYFunctionCurve::FunctionData(m_variableNames.at(i), m_variableCurves.at(i));
		m_curve->setFunction(m_function, functionData);
	}

private:
	XYFunctionCurvePrivate* m_curve;
	QString m_function;
	QStringList m_variableNames;
	QVector<const XYCurve*> m_variableCurves;
	QString m_newFunction;
	QStringList m_newVariableNames;
	QVector<const XYCurve*> m_newVariableCurves;
	bool m_copied{false};
};

void XYFunctionCurve::setFunction(const QString& function, const QStringList& variableNames, const QVector<const XYCurve*>& curves) {
	Q_D(XYFunctionCurve);
	exec(new CurveSetGlobalFunctionCmd(d, function, variableNames, curves));
}

/**
 * \brief Clears the function used to generate column values
 */
void XYFunctionCurve::clearFunction() {
	setFunction(QString(), QStringList(), QVector<const XYCurve*>());
}

QString XYFunctionCurve::function() const {
	Q_D(const XYFunctionCurve);
	return d->function();
}

const QVector<XYFunctionCurve::FunctionData>& XYFunctionCurve::functionData() const {
	Q_D(const XYFunctionCurve);
	return d->functionData();
}

void XYFunctionCurve::setFunctionVariableCurve(const XYCurve* c) {
	Q_D(XYFunctionCurve);
	d->setFunctionVariableCurve(c);
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################

void XYFunctionCurve::functionVariableCurveRemoved(const AbstractAspect* aspect) {
	Q_D(XYFunctionCurve);
	d->functionVariableCurveRemoved(aspect);
}

void XYFunctionCurve::functionVariableCurveAdded(const AbstractAspect* aspect) {
	Q_D(XYFunctionCurve);
	d->functionVariableCurveAdded(aspect);
}

void XYFunctionCurve::recalculate() {
	Q_D(XYFunctionCurve);
	d->recalculate();
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYFunctionCurvePrivate::XYFunctionCurvePrivate(XYFunctionCurve* owner)
	: XYAnalysisCurvePrivate(owner)
	, q(owner) {
	dataSourceType = XYAnalysisCurve::DataSourceType::Curve;
}

QString XYFunctionCurvePrivate::function() const {
	return m_function;
}

const QVector<XYFunctionCurve::FunctionData>& XYFunctionCurvePrivate::functionData() const {
	return m_functionData;
}

void XYFunctionCurvePrivate::setFunction(const QString& function, const QStringList& variableNames, const QStringList& variableCurvePaths) {
	m_function = function;
	m_functionData.clear();
	for (int i = 0; i < variableNames.count(); i++)
		m_functionData.append(XYFunctionCurve::FunctionData(variableNames.at(i), variableCurvePaths.at(i)));
}

void XYFunctionCurvePrivate::setFunctionVariableCurvesPath(int index, const QString& path) {
	if (!m_functionData[index].setCurvePath(path))
		DEBUG(Q_FUNC_INFO << ": For some reason, there was already a curve assigned");
}

void XYFunctionCurvePrivate::setFunctionVariableCurve(int index, const XYCurve* curve) {
	if (m_functionData.at(index).curve()) // if there exists already a valid curve, disconnect it first
		q->disconnect(m_functionData.at(index).m_curve, nullptr, q, nullptr);
	m_functionData[index].setCurve(curve);
	connectCurve(curve);
}

void XYFunctionCurvePrivate::setFunctionVariableCurve(const XYCurve* c) {
	for (auto& d : m_functionData) {
		if (d.curvePath() == c->path()) {
			d.setCurve(c);
			break;
		}
	}
}

void XYFunctionCurvePrivate::connectCurve(const XYCurve* curve) {
	if (!curve)
		return;

	// avoid circular dependencies - the current curve cannot be part of the variable curve.
	// this should't actually happen because of the checks done when the function is defined,
	// but in case we have bugs somewhere or somebody manipulated the project xml file we add
	// a sanity check to avoid recursive calls here and crash because of the stack overflow.
	if (curve == q)
		return;

	DEBUG(Q_FUNC_INFO)
	m_connections << q->connect(curve, &XYCurve::dataChanged, q, &XYFunctionCurve::recalculate);
	m_connections << q->connect(curve, &XYCurve::xDataChanged, q, &XYFunctionCurve::recalculate);
	m_connections << q->connect(curve, &XYCurve::yDataChanged, q, &XYFunctionCurve::recalculate);
	m_connections << q->connect(curve, &AbstractAspect::aspectAboutToBeRemoved, q, &XYFunctionCurve::functionVariableCurveRemoved);
	// m_connectionsUpdateFunction << q->connect(curve->parentAspect(), &AbstractAspect::childAspectAdded, q, &XYFunctionCurve::functionVariableCurveAdded);
}

void XYFunctionCurvePrivate::functionVariableCurveRemoved(const AbstractAspect* aspect) {
	const XYCurve* curve = dynamic_cast<const XYCurve*>(aspect);
	if (!curve)
		return;
	q->disconnect(curve, nullptr, q, nullptr);
	int index = -1;
	for (int i = 0; i < functionData().count(); i++) {
		auto& d = functionData().at(i);
		if (d.curve() == curve) {
			index = i;
			break;
		}
	}
	if (index != -1) {
		m_functionData[index].setCurve(nullptr);
		q->recalculate();
	}
}

void XYFunctionCurvePrivate::functionVariableCurveAdded(const AbstractAspect* aspect) {
	auto* curve = dynamic_cast<XYCurve*>(const_cast<AbstractAspect*>(aspect));
	if (!curve)
		return;

	const auto& path = aspect->path();
	for (int i = 0; i < functionData().count(); i++) {
		if (functionData().at(i).curvePath() == path) {
			// m_functionData[index].setColumn(const_cast<Column*>(column));
			// DEBUG(Q_FUNC_INFO << ", calling updateFunction()")
			setFunctionVariableCurve(i, curve);
			recalculate();
			return;
		}
	}
}

/**
 * \brief Sets the function used to generate column values
 */
void XYFunctionCurvePrivate::setFunction(const QString& function, const QVector<XYFunctionCurve::FunctionData>& functionData) {
	m_function = function;
	m_functionData = functionData; // TODO: disconnecting everything?

	for (auto& connection : m_connections)
		if (static_cast<bool>(connection))
			q->disconnect(connection);

	for (const auto& data : std::as_const(m_functionData)) {
		const auto* curve = data.curve();
		if (curve)
			connectCurve(curve);
	}
	q->recalculate();
}

void XYFunctionCurvePrivate::resetResults() {
	m_result = XYFunctionCurve::Result();
}

// ...
// see XYFitCurvePrivate
bool XYFunctionCurvePrivate::recalculateSpecific(const AbstractColumn*, const AbstractColumn*) {
	QElapsedTimer timer;
	timer.start();

	bool valid = true;
	QString status = QStringLiteral("Valid");

	int numberElements = 0;
	if (m_functionData.length() == 0) {
		valid = false;
	} else {
		const auto* curve = m_functionData.first().curve();
		if (!curve || !curve->xColumn() || !curve->yColumn() || curve->xColumn()->rowCount() != curve->yColumn()->rowCount()) {
			if (!curve)
				status = i18n("First curve not valid");
			else if (!curve->xColumn())
				status = i18n("xColumn of first curve not valid");
			else if (!curve->yColumn())
				status = i18n("yColumn of first curve not valid");
			else if (curve->xColumn()->rowCount() != curve->yColumn()->rowCount())
				status = i18n("Number of x and y values do not match for the first curve");
			valid = false;
		} else
			numberElements = curve->xColumn()->rowCount();
	}

	if (valid) {
		for (const auto& ed : std::as_const(m_functionData)) {
			const auto* curve = ed.curve();
			if (!curve || !curve->xColumn() || !curve->yColumn() || curve->xColumn()->rowCount() != numberElements
				|| curve->yColumn()->rowCount() != numberElements) {
				valid = false;
				if (!curve)
					status = i18n("Curve '%1' not valid").arg(ed.curvePath());
				else if (!curve->xColumn())
					status = i18n("xColumn of curve '%1' not valid").arg(ed.curvePath());
				else if (!curve->yColumn())
					status = i18n("yColumn of curve '%1' not valid").arg(ed.curvePath());
				else if (curve->xColumn()->rowCount() != curve->yColumn()->rowCount())
					status = i18n("Number of x and y values do not match for curve '%1'").arg(ed.curvePath());
				break;
			}
		}
	}

	if (valid) {
		const auto rowCount = m_functionData.first().curve()->xColumn()->rowCount();
		yVector->resize(rowCount);
		const auto* xColumn = dynamic_cast<const Column*>(m_functionData.first().curve()->xColumn());
		Q_ASSERT(xColumn);
		xVector->resize(rowCount);
		// convert integers to doubles first
		for (int i = 0; i < xColumn->rowCount(); ++i)
			(*xVector)[i] = xColumn->valueAt(i);

		QVector<QVector<double>*> xVectors;
		QStringList functionVariableNames;
		for (const auto& functionData : std::as_const(m_functionData)) {
			const auto* curve = functionData.curve();
			const auto& varName = functionData.variableName();
			const auto* column = dynamic_cast<const Column*>(curve->yColumn());
			Q_ASSERT(column);

			functionVariableNames << varName;

			if (column->columnMode() == AbstractColumn::ColumnMode::Double)
				xVectors << static_cast<QVector<double>*>(column->data());
			else {
				// convert integers to doubles first
				auto* xVector = new QVector<double>(column->rowCount());
				for (int i = 0; i < column->rowCount(); ++i)
					(*xVector)[i] = column->valueAt(i);

				xVectors << xVector;
			}
		}

		// evaluate the expression for f(x_1, x_2, ...) and write the calculated values into a new vector.
		auto* parser = ExpressionParser::getInstance();
		bool valid = parser->tryEvaluateCartesian(m_function, functionVariableNames, xVectors, yVector);
		if (!valid)
			DEBUG(Q_FUNC_INFO << ", WARNING: failed parsing function!")
	}
	m_result.available = true;
	m_result.valid = valid;
	m_result.status = status;
	m_result.elapsedTime = timer.elapsed();
	return valid;
}

bool XYFunctionCurvePrivate::preparationValid(const AbstractColumn*, const AbstractColumn*) {
	return true;
}

void XYFunctionCurvePrivate::prepareTmpDataColumn(const AbstractColumn**, const AbstractColumn**) const {
	// Nothing to do, because we have more than two columns
}

void XYFunctionCurvePrivate::handleAspectUpdated(const QString& aspectPath, const AbstractAspect* element) {
	const auto curve = dynamic_cast<const XYCurve*>(element);
	if (!curve)
		return;
	for (auto& data : m_functionData) {
		if (data.curvePath() == aspectPath) {
			data.m_curve = curve;
			// No break, because it could be used multiple times
		}
	}
	q->recalculate();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void XYFunctionCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYFunctionCurve);

	writer->writeStartElement(XYFunctionCurve::saveName);

	// write xy-curve information
	XYAnalysisCurve::save(writer);

	// write xy-functionCurve specific information
	// save the function used to generate column values, if available
	if (!function().isEmpty()) {
		writer->writeStartElement(QStringLiteral("function"));
		writer->writeTextElement(QStringLiteral("text"), function());

		QStringList functionVariableNames;
		QStringList functionVariableColumnPaths;
		for (auto& d : functionData()) {
			functionVariableNames << d.variableName();
			functionVariableColumnPaths << d.curvePath();
		}

		writer->writeStartElement(QStringLiteral("variableNames"));
		for (const auto& name : functionVariableNames)
			writer->writeTextElement(QStringLiteral("name"), name);
		writer->writeEndElement();

		writer->writeStartElement(QStringLiteral("curvePaths"));
		for (const auto& path : functionVariableColumnPaths)
			writer->writeTextElement(QStringLiteral("path"), path);
		writer->writeEndElement(); // curvePaths

		// save calculated columns if available
		if (saveCalculations() && d->xColumn) {
			d->xColumn->save(writer);
			d->yColumn->save(writer);
		}

		writer->writeEndElement(); // function
	}
	writer->writeEndElement(); // XYFunctionCurve
}

//! Load from XML
bool XYFunctionCurve::load(XmlStreamReader* reader, bool preview) {
	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == saveName)
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyAnalysisCurve")) {
			if (!XYAnalysisCurve::load(reader, preview))
				return false;
		} else if (reader->name() == QLatin1String("function")) {
			if (!XmlReadFunction(reader, preview))
				return false;
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}

/**
 * \brief Read XML function element
 */
bool XYFunctionCurve::XmlReadFunction(XmlStreamReader* reader, bool preview) {
	Q_D(XYFunctionCurve);
	QString function;
	QStringList variableNames;
	QStringList curvePaths;

	while (reader->readNext()) {
		if (reader->isEndElement())
			break;

		if (reader->name() == QLatin1String("text"))
			function = reader->readElementText();
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
		} else if (reader->name() == QLatin1String("column")) {
			Column* column = new Column(QString(), AbstractColumn::ColumnMode::Double);
			if (!column->load(reader, preview)) {
				delete column;
				return false;
			}
			if (column->name() == QLatin1String("x"))
				d->xColumn = column;
			else if (column->name() == QLatin1String("y"))
				d->yColumn = column;
			else {
				delete column;
				return false;
			}
		}
	}

	d->setFunction(function, variableNames, curvePaths);

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
