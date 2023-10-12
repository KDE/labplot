/*
	File             : ExpressionParser.cpp
	Project          : LabPlot
	Description      : C++ wrapper for the bison generated parser.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2014-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/parser.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>

#include <QRegularExpression>

#include <gsl/gsl_const_mksa.h>
#include <gsl/gsl_const_num.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_version.h>

ExpressionParser* ExpressionParser::m_instance{nullptr};

ExpressionParser::ExpressionParser() {
	init_table();
	initFunctions();
	initConstants();
}

// initialize function list	(sync with functions.h and FunctionsWidget.cpp!)
void ExpressionParser::initFunctions() {
	for (int i = 0; i < _number_functions; i++) {
		const auto& function = _functions[i];
		m_functionsDescription << function.description;
		m_functions << QLatin1String(function.name);
		m_functionsGroupIndex << function.group;
	}
}

// TODO: decide whether we want to have i18n here in the backend part of the code
void ExpressionParser::initConstants() {
	for (int i = 0; i < _number_constants; i++) {
		const auto& constant = _constants[i];
		m_constantsDescription << constant.description;
		m_constants << QLatin1String(constant.name);
		m_constantsValues << QString::number(constant.value, 'g', 15);
		m_constantsUnits << QLatin1String(constant.unit);
		m_constantsGroupIndex << constant.group;
	}
}

/**********************************************************************************/

ExpressionParser::~ExpressionParser() {
	delete_table();
}

ExpressionParser* ExpressionParser::getInstance() {
	if (!m_instance)
		m_instance = new ExpressionParser();

	return m_instance;
}

const QStringList& ExpressionParser::functions() {
	return m_functions;
}

const QStringList& ExpressionParser::functionsGroups() {
	return m_functionsGroups;
}

const QStringList& ExpressionParser::functionsDescriptions() {
	return m_functionsDescription;
}

const QVector<FunctionGroups>& ExpressionParser::functionsGroupIndices() {
	return m_functionsGroupIndex;
}

/* another idea:
 * https://stackoverflow.com/questions/36797770/get-function-parameters-count
 * but this does not work since all function pointer have zero args in the struct
 */
int ExpressionParser::functionArgumentCount(const QString& functionName) {
	for (int i = 0; i < _number_functions; i++) {
		if (functionName == QLatin1String(_functions[i].name))
			return _functions[i].argc;
	}

	// DEBUG(Q_FUNC_INFO << ", Found function " << STDSTRING(functionName) << " at index " << index);
	// DEBUG(Q_FUNC_INFO << ", function " << STDSTRING(functionName) << " has " << _functions[index].argc << " arguments");
	return 0;
}

QString ExpressionParser::parameters(const QString& functionName) {
	for (int i = 0; i < _number_functions; i++) {
		if (functionName == QLatin1String(_functions[i].name)) {
			int count = _functions[i].argc;
			QString (*parameterFunction)(int) = _functions[i].parameterFunction;

			if (parameterFunction == nullptr)
				return QStringLiteral("");

			if (count == 0)
				return QStringLiteral("()");

			QString parameter = QStringLiteral("(");
			for (int p = 0; p < count - 1; p++)
				parameter += (*parameterFunction)(p) + QStringLiteral("; ");
			parameter += (*parameterFunction)(count - 1);
			parameter += QStringLiteral(")");
			return parameter;
		}
		// DEBUG(Q_FUNC_INFO << ", Found function " << STDSTRING(functionName) << " at index " << index);
		// DEBUG(Q_FUNC_INFO << ", function " << STDSTRING(functionName) << " has " << _functions[index].argc << " arguments");
	}
	return QStringLiteral("");
}

QString ExpressionParser::functionArgumentString(const QString& functionName, const XYEquationCurve::EquationType type) {
	QString p = parameters(functionName);
	if (!p.isEmpty())
		return p;

	// Default parameters
	switch (functionArgumentCount(functionName)) {
	case 0:
		return QStringLiteral("()");
	case 1:
		switch (type) {
		case XYEquationCurve::EquationType::Cartesian:
			return QStringLiteral("(x)");
		case XYEquationCurve::EquationType::Polar:
			return QStringLiteral("(phi)");
		case XYEquationCurve::EquationType::Parametric:
			return QStringLiteral("(t)");
		case XYEquationCurve::EquationType::Implicit:
		case XYEquationCurve::EquationType::Neutral:
			return QStringLiteral("(x)");
		}
		break;
	case 2:
		switch (type) {
		case XYEquationCurve::EquationType::Cartesian:
			return QStringLiteral("(x; y)");
		case XYEquationCurve::EquationType::Polar:
			return QStringLiteral("(phi; theta)");
		case XYEquationCurve::EquationType::Parametric:
			return QStringLiteral("(u; v)");
		case XYEquationCurve::EquationType::Implicit:
		case XYEquationCurve::EquationType::Neutral:
			return QStringLiteral("(x; y)");
		}
		break;
	case 3:
		switch (type) {
		case XYEquationCurve::EquationType::Cartesian:
			return QStringLiteral("(x; y; z)");
		case XYEquationCurve::EquationType::Polar:
			return QStringLiteral("(alpha; beta; gamma)");
		case XYEquationCurve::EquationType::Parametric:
			return QStringLiteral("(u; v; w)");
		case XYEquationCurve::EquationType::Implicit:
		case XYEquationCurve::EquationType::Neutral:
			return QStringLiteral("(x; y; z)");
		}
		break;
	case 4:
		switch (type) {
		case XYEquationCurve::EquationType::Cartesian:
			return QStringLiteral("(a; b; c; d)");
		case XYEquationCurve::EquationType::Polar:
			return QStringLiteral("(alpha; beta; gamma; delta)");
		case XYEquationCurve::EquationType::Parametric:
			return QStringLiteral("(a; b; c; d)");
		case XYEquationCurve::EquationType::Implicit:
		case XYEquationCurve::EquationType::Neutral:
			return QStringLiteral("(a; b; c; d)");
		}
		break;
	}

	return QStringLiteral("(...)");
}

QString ExpressionParser::functionDescription(const QString& function) {
	for (int index = 0; index < _number_functions; index++) {
		if (function == QLatin1String(_functions[index].name))
			return m_functionsDescription.at(index);
	}

	return QStringLiteral("");
}
QString ExpressionParser::constantDescription(const QString& constant) {
	for (int index = 0; index < _number_constants; index++) {
		if (constant == QLatin1String(_constants[index].name))
			return m_constantsDescription.at(index) + QStringLiteral(" (") + m_constantsValues.at(index) + QStringLiteral(" ") + m_constantsUnits.at(index)
				+ QStringLiteral(")");
	}
	return QStringLiteral("");
}

const QStringList& ExpressionParser::constants() {
	return m_constants;
}

const QStringList& ExpressionParser::constantsGroups() {
	return m_constantsGroups;
}

const QStringList& ExpressionParser::constantsNames() {
	return m_constantsDescription;
}

const QStringList& ExpressionParser::constantsValues() {
	return m_constantsValues;
}

const QStringList& ExpressionParser::constantsUnits() {
	return m_constantsUnits;
}

const QVector<ConstantGroups>& ExpressionParser::constantsGroupIndices() {
	return m_constantsGroupIndex;
}

bool ExpressionParser::isValid(const QString& expr, const QStringList& vars) {
	QDEBUG(Q_FUNC_INFO << ", expr:" << expr << ", vars:" << vars);
	gsl_set_error_handler_off();

	for (const auto& var : vars)
		assign_symbol(qPrintable(var), 0);

	// cell() supports index i: make i valid
	if (expr.contains(QLatin1String("cell")))
		assign_symbol("i", 0);

	const auto numberLocale = QLocale();
	DEBUG(Q_FUNC_INFO << ", number locale: " << STDSTRING(numberLocale.name()))
	parse(qPrintable(expr), qPrintable(numberLocale.name()));

	// if parsing with number locale fails, try default locale
	if (parse_errors() > 0) {
		DEBUG(Q_FUNC_INFO << ", WARNING: failed parsing expr \"" << STDSTRING(expr) << "\" with locale " << numberLocale.name().toStdString()
						  << ", errors = " << parse_errors())
		parse(qPrintable(expr), "en_US");
		if (parse_errors() > 0)
			DEBUG(Q_FUNC_INFO << ", ERROR: parsing FAILED, errors = " << parse_errors())
	}

	/* remove temporarily defined symbols */
	for (const auto& var : vars)
		remove_symbol(qPrintable(var));
	if (expr.contains(QLatin1String("cell")))
		remove_symbol("i");

	return !(parse_errors() > 0);
}

QStringList ExpressionParser::getParameter(const QString& expr, const QStringList& vars) {
	QDEBUG(Q_FUNC_INFO << ", variables:" << vars);
	QStringList parameters;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
	QStringList strings = expr.split(QRegularExpression(QStringLiteral("\\W+")), Qt::SkipEmptyParts);
#else
	QStringList strings = expr.split(QRegularExpression(QStringLiteral("\\W+")), QString::SkipEmptyParts);
#endif
	QDEBUG(Q_FUNC_INFO << ", found strings:" << strings);
	// RE for any number
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
	const QRegularExpression re(QRegularExpression::anchoredPattern(QStringLiteral("[0-9]*")));
#else
	const QRegularExpression re("\\A(?:" + QStringLiteral("[0-9]*") + ")\\z");
#endif
	for (const QString& string : strings) {
		QDEBUG(string << ':' << constants().indexOf(string) << ' ' << functions().indexOf(string) << ' ' << vars.indexOf(string) << ' '
					  << re.match(string).hasMatch());
		// check if token is not a known constant/function/variable or number
		if (constants().indexOf(string) == -1 && functions().indexOf(string) == -1 && vars.indexOf(string) == -1 && re.match(string).hasMatch() == false)
			parameters << string;
	}
	parameters.removeDuplicates();
	QDEBUG(Q_FUNC_INFO << ", parameters found:" << parameters);

	return parameters;
}

/*
 * Evaluate cartesian expression returning true on success and false if parsing fails
 * using given range
 */
bool ExpressionParser::evaluateCartesian(const QString& expr,
										 const Range<double> range,
										 int count,
										 QVector<double>* xVector,
										 QVector<double>* yVector,
										 const QStringList& paramNames,
										 const QVector<double>& paramValues) {
	DEBUG(Q_FUNC_INFO << ", v0: range = " << range.toStdString())
	const double step = range.stepSize(count);
	DEBUG(Q_FUNC_INFO << ", range = " << range.toStdString() << ", step = " << step)

	for (int i = 0; i < paramNames.size(); ++i)
		assign_symbol(qPrintable(paramNames.at(i)), paramValues.at(i));

	const auto numberLocale = QLocale();
	gsl_set_error_handler_off();
	for (int i = 0; i < count; i++) {
		const double x{range.start() + step * i};
		assign_symbol("x", x);

		double y = parse(qPrintable(expr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0) // try default locale if failing
			y = parse(qPrintable(expr), "en_US");
		if (parse_errors() > 0)
			return false;

		if (std::isnan(y))
			WARN(Q_FUNC_INFO << ", WARNING: expression " << STDSTRING(expr) << " evaluated @ " << x << " is NAN")

		(*xVector)[i] = x;
		(*yVector)[i] = y;
	}

	return true;
}
/*
 * Evaluate cartesian expression returning true on success and false if parsing fails
 * min and max are localized strings which are parsed to support expressions like "pi + 1.5"
 */
bool ExpressionParser::evaluateCartesian(const QString& expr,
										 const QString& min,
										 const QString& max,
										 int count,
										 QVector<double>* xVector,
										 QVector<double>* yVector,
										 const QStringList& paramNames,
										 const QVector<double>& paramValues) {
	DEBUG(Q_FUNC_INFO << ", v1: range = " << STDSTRING(min) << " .. " << STDSTRING(max))

	const Range<double> range{min, max};
	return evaluateCartesian(expr, range, count, xVector, yVector, paramNames, paramValues);
}

bool ExpressionParser::evaluateCartesian(const QString& expr,
										 const QString& min,
										 const QString& max,
										 int count,
										 QVector<double>* xVector,
										 QVector<double>* yVector) {
	DEBUG(Q_FUNC_INFO << ", v2")
	gsl_set_error_handler_off();

	const Range<double> range{min, max};
	const double step = range.stepSize(count);

	const auto numberLocale = QLocale();
	for (int i = 0; i < count; i++) {
		const double x{range.start() + step * i};
		assign_symbol("x", x);

		double y = parse(qPrintable(expr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0) { // try default locale if failing
			y = parse(qPrintable(expr), "en_US");
			// DEBUG(Q_FUNC_INFO << ", WARNING: PARSER failed, trying default locale: y = " << y)
		}
		if (parse_errors() > 0)
			return false;

		if (std::isnan(y))
			WARN(Q_FUNC_INFO << ", WARNING: expression " << STDSTRING(expr) << " evaluated @ " << x << " is NAN")

		(*xVector)[i] = x;
		(*yVector)[i] = y;
	}

	return true;
}

bool ExpressionParser::evaluateCartesian(const QString& expr, QVector<double>* xVector, QVector<double>* yVector) {
	DEBUG(Q_FUNC_INFO << ", v3")
	gsl_set_error_handler_off();

	const auto numberLocale = QLocale();
	for (int i = 0; i < xVector->count(); i++) {
		assign_symbol("x", xVector->at(i));
		double y = parse(qPrintable(expr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0) // try default locale if failing
			y = parse(qPrintable(expr), "en_US");
		if (parse_errors() > 0)
			return false;

		if (std::isnan(y))
			WARN(Q_FUNC_INFO << ", WARNING: expression " << STDSTRING(expr) << " evaluated @ " << xVector->at(i) << " is NAN")

		(*yVector)[i] = y;
	}

	return true;
}

bool ExpressionParser::evaluateCartesian(const QString& expr,
										 const QVector<double>* xVector,
										 QVector<double>* yVector,
										 const QStringList& paramNames,
										 const QVector<double>& paramValues) {
	DEBUG(Q_FUNC_INFO << ", v4")
	gsl_set_error_handler_off();

	for (int i = 0; i < paramNames.size(); ++i)
		assign_symbol(qPrintable(paramNames.at(i)), paramValues.at(i));

	const auto numberLocale = QLocale();
	for (int i = 0; i < xVector->count(); i++) {
		assign_symbol("x", xVector->at(i));

		double y = parse(qPrintable(expr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0) // try default locale if failing
			y = parse(qPrintable(expr), "en_US");
		if (parse_errors() > 0)
			return false;

		if (std::isnan(y))
			WARN(Q_FUNC_INFO << ", WARNING: expression " << STDSTRING(expr) << " evaluated @ " << xVector->at(i) << " is NAN")

		(*yVector)[i] = y;
	}

	return true;
}

/*!
	evaluates multivariate function y=f(x_1, x_2, ...).
	Variable names (x_1, x_2, ...) are stored in \c vars.
	Data is stored in \c xVectors.
 */
bool ExpressionParser::evaluateCartesian(const QString& expr, const QStringList& vars, const QVector<QVector<double>*>& xVectors, QVector<double>* yVector) {
	DEBUG(Q_FUNC_INFO << ", v5")
	Q_ASSERT(vars.size() == xVectors.size());
	gsl_set_error_handler_off();

	// determine the minimal size of involved vectors
	int minSize{std::numeric_limits<int>::max()};
	for (auto* xVector : xVectors) {
		if (xVector->size() < minSize)
			minSize = xVector->size();
	}
	if (yVector->size() < minSize)
		minSize = yVector->size();

	// calculate values
	const auto numberLocale = QLocale();
	DEBUG("Parsing with locale " << qPrintable(numberLocale.name()))

	bool constExpression = false;
	for (int i = 0; i < minSize || (constExpression && i < yVector->size()); i++) {
		QString tmpExpr = expr;

		// assign vars with value from xVectors
		for (int n = 0; n < vars.size(); ++n) {
			if (!constExpression)
				assign_symbol(qPrintable(vars.at(n)), xVectors.at(n)->at(i));

			// if expr contains cell(f(i), g(x,..)): replace this with xVectors.at(n)->at(f(i))
			// inverted greedy: only match one method call at a time
			QRegularExpression rxcell(QStringLiteral("cell\\((.*),(.*)\\)"), QRegularExpression::InvertedGreedinessOption);

			int pos = 0;
			auto match = rxcell.match(tmpExpr, 0);
			while (match.hasMatch()) {
				const QString f = match.captured(1);
				QString g = match.captured(2);
				// QDEBUG("f(i) =" << f)
				// QDEBUG("g(x,..) =" << g)
				assign_symbol("i", i + 1); // row number i = 1 .. minSize
				const int index = parse(qPrintable(f), qPrintable(numberLocale.name()));
				// DEBUG("INDEX = " << index)
				pos = match.capturedStart(1);

				if (index > 0 && index <= xVectors.at(n)->size()) {
					const QString newg = g.replace(vars.at(n), numberLocale.toString(xVectors.at(n)->at(index - 1)));
					// QDEBUG("new g(x,..) =" << newg)
					const QString replace = QStringLiteral("cell(") + f + QStringLiteral(",") + newg + QStringLiteral(")");
					// QDEBUG("MATCH =" << match.captured(0))
					// QDEBUG("replacement =" << replace)
					tmpExpr.replace(match.captured(0), replace);
					pos++; // avoid endless loop
				} else
					tmpExpr.replace(match.captured(0), numberLocale.toString(NAN));

				match = rxcell.match(tmpExpr, pos);
			}

			// if expr contains smmin(N, x)
			QRegularExpression rxmin(QLatin1String("smmin\\((.*),.*%1\\)").arg(vars.at(n)), QRegularExpression::InvertedGreedinessOption);
			pos = 0;
			match = rxmin.match(tmpExpr, 0);
			while (match.hasMatch()) {
				const QString arg = match.captured(1);
				// QDEBUG("ARG = " << arg)
				//  number of points to consider
				const int N = numberLocale.toDouble(match.captured(1));
				// DEBUG("N = " << N)
				if (N < 1)
					continue;
				// calculate min of last n points
				double min = INFINITY;
				for (int index = std::max(0, i - N + 1); index <= i; index++) {
					const double v = xVectors.at(n)->at(index);
					if (v < min)
						min = v;
				}

				tmpExpr.replace(match.captured(0), numberLocale.toString(min));

				pos = match.capturedStart(1);
				match = rxmin.match(tmpExpr, pos);
			}
			// if expr contains smmax(N, x)
			QRegularExpression rxmax(QLatin1String("smmax\\((.*),.*%1\\)").arg(vars.at(n)), QRegularExpression::InvertedGreedinessOption);
			pos = 0;
			match = rxmax.match(tmpExpr, 0);
			while (match.hasMatch()) {
				const QString arg = match.captured(1);
				// QDEBUG("ARG = " << arg)
				//  number of points to consider
				const int N = numberLocale.toDouble(match.captured(1));
				// DEBUG("N = " << N)
				if (N < 1)
					continue;
				// calculate max of last n points
				double max = -INFINITY;
				for (int index = std::max(0, i - N + 1); index <= i; index++) {
					const double v = xVectors.at(n)->at(index);
					if (v > max)
						max = v;
				}

				tmpExpr.replace(match.captured(0), numberLocale.toString(max));

				pos = match.capturedStart(1);
				match = rxmax.match(tmpExpr, pos);
			}
			// if expr contains sma(N, x)
			QRegularExpression rxsma(QLatin1String("sma\\((.*),.*%1\\)").arg(vars.at(n)), QRegularExpression::InvertedGreedinessOption);
			pos = 0;
			match = rxsma.match(tmpExpr, 0);
			while (match.hasMatch()) {
				const QString arg = match.captured(1);
				// QDEBUG("ARG = " << arg)
				// number of points to consider
				const int N = numberLocale.toDouble(match.captured(1));
				// DEBUG("N = " << N)
				if (N < 1)
					continue;
				// calculate avg of last n points
				double sum = 0.;
				for (int index = std::max(0, i - N + 1); index <= i; index++)
					sum += xVectors.at(n)->at(index);

				tmpExpr.replace(match.captured(0), numberLocale.toString(sum / N));

				pos = match.capturedStart(1);
				match = rxsma.match(tmpExpr, pos);
			}
		}

		// QDEBUG("PRE expression to parse = " << tmpExpr)

		// finally replace all cell() calls with second argument (g)
		QRegularExpression rxcellfinal(QLatin1String("cell\\(.*,(.*)\\)"), QRegularExpression::InvertedGreedinessOption);
		int pos = 0;
		auto match = rxcellfinal.match(tmpExpr, 0);
		while (match.hasMatch()) {
			tmpExpr.replace(match.captured(0), match.captured(1));

			pos = match.capturedStart(0);
			match = rxcellfinal.match(tmpExpr, pos);
		}

		// QDEBUG("FINAL expression to parse = " << tmpExpr)

		double y = parse(qPrintable(tmpExpr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0) { // try default locale if failing
			// DEBUG("Parsing with locale failed. Using en_US.")
			y = parse(qPrintable(tmpExpr), "en_US");
		}

		if (parse_errors() == 0)
			constExpression = variablesCounter() == 0;
		// continue with next value
		// if (parse_errors() > 0)
		//	return false;

		if (std::isnan(y))
			WARN(Q_FUNC_INFO << ", WARNING: expression " << STDSTRING(tmpExpr) << " evaluated to NAN")

		(*yVector)[i] = y;
	}

	// if the y-vector is longer than the x-vector(s), set all exceeding elements to NaN
	if (!constExpression) {
		for (int i = minSize; i < yVector->size(); ++i)
			(*yVector)[i] = NAN;
	}

	return true;
}

bool ExpressionParser::evaluatePolar(const QString& expr,
									 const QString& min,
									 const QString& max,
									 int count,
									 QVector<double>* xVector,
									 QVector<double>* yVector) {
	gsl_set_error_handler_off();

	const Range<double> range{min, max};
	const double step = range.stepSize(count);

	const auto numberLocale = QLocale();
	for (int i = 0; i < count; i++) {
		const double phi = range.start() + step * i;
		assign_symbol("phi", phi);

		double r = parse(qPrintable(expr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0) // try default locale if failing
			r = parse(qPrintable(expr), "en_US");
		if (parse_errors() > 0)
			return false;

		if (std::isnan(r))
			WARN(Q_FUNC_INFO << ", WARNING: expression " << STDSTRING(expr) << " evaluated @ " << phi << " is NAN")

		(*xVector)[i] = r * cos(phi);
		(*yVector)[i] = r * sin(phi);
	}

	return true;
}

bool ExpressionParser::evaluateParametric(const QString& xexpr,
										  const QString& yexpr,
										  const QString& min,
										  const QString& max,
										  int count,
										  QVector<double>* xVector,
										  QVector<double>* yVector) {
	gsl_set_error_handler_off();

	const Range<double> range{min, max};
	const double step = range.stepSize(count);

	const auto numberLocale = QLocale();
	for (int i = 0; i < count; i++) {
		assign_symbol("t", range.start() + step * i);

		double x = parse(qPrintable(xexpr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0) // try default locale if failing
			x = parse(qPrintable(xexpr), "en_US");
		if (parse_errors() > 0)
			return false;

		double y = parse(qPrintable(yexpr), qPrintable(numberLocale.name()));
		if (parse_errors() > 0) // try default locale if failing
			y = parse(qPrintable(yexpr), "en_US");
		if (parse_errors() > 0)
			return false;

		if (std::isnan(x))
			WARN(Q_FUNC_INFO << ", WARNING: X expression " << STDSTRING(xexpr) << " evaluated @ " << range.start() + step * i << " is NAN")
		if (std::isnan(y))
			WARN(Q_FUNC_INFO << ", WARNING: Y expression " << STDSTRING(yexpr) << " evaluated @ " << range.start() + step * i << " is NAN")

		(*xVector)[i] = x;
		(*yVector)[i] = y;
	}

	return true;
}
