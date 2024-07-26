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
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"

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
		m_functionsDescription << function.description();
		m_functions << QLatin1String(function.name);
		m_functionsGroupIndex << function.group;
	}
	for (int i = 0; i < _number_specialfunctions; i++) {
		const auto& function = _special_functions[i];
		m_functionsDescription << function.description();
		m_functions << QLatin1String(function.name);
		m_functionsGroupIndex << function.group;
	}
}

// TODO: decide whether we want to have i18n here in the backend part of the code
void ExpressionParser::initConstants() {
	for (int i = 0; i < _number_constants; i++) {
		const auto& constant = _constants[i];
		m_constantsDescription << constant.description();
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
	for (int i = 0; i < _number_specialfunctions; i++) {
		if (functionName == QLatin1String(_special_functions[i].name))
			return _special_functions[i].argc;
	}

	// DEBUG(Q_FUNC_INFO << ", Found function " << STDSTRING(functionName) << " at index " << index);
	// DEBUG(Q_FUNC_INFO << ", function " << STDSTRING(functionName) << " has " << _functions[index].argc << " arguments");
	return 0;
}

QString ExpressionParser::parameters(const QString& functionName) {
	for (int i = 0; i < _number_functions; i++) {
		if (functionName == QLatin1String(_functions[i].name)) {
			int count = _functions[i].argc;
			const auto& parameterFunction = _functions[i].parameterFunction;

			if (parameterFunction == nullptr)
				return QStringLiteral("");

			if (count == 0)
				return QStringLiteral("()");

			QString parameter = QStringLiteral("(");
			for (int p = 0; p < count - 1; p++)
				parameter += parameterFunction(p) + QStringLiteral("; ");
			parameter += parameterFunction(count - 1);
			parameter += QStringLiteral(")");
			return parameter;
		}
		// DEBUG(Q_FUNC_INFO << ", Found function " << STDSTRING(functionName) << " at index " << index);
		// DEBUG(Q_FUNC_INFO << ", function " << STDSTRING(functionName) << " has " << _functions[index].argc << " arguments");
	}
	for (int i = 0; i < _number_specialfunctions; i++) {
		if (functionName == QLatin1String(_special_functions[i].name)) {
			int count = _special_functions[i].argc;
			const auto& parameterFunction = _special_functions[i].parameterFunction;

			if (parameterFunction == nullptr)
				return QStringLiteral("");

			if (count == 0)
				return QStringLiteral("()");

			QString parameter = QStringLiteral("(");
			for (int p = 0; p < count - 1; p++)
				parameter += parameterFunction(p) + QStringLiteral("; ");
			parameter += parameterFunction(count - 1);
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
			return _functions[index].description();
	}
	for (int index = 0; index < _number_specialfunctions; index++) {
		if (function == QLatin1String(_special_functions[index].name))
			return _special_functions[index].description();
	}
	return QStringLiteral("");
}
QString ExpressionParser::constantDescription(const QString& constant) {
	for (int index = 0; index < _number_constants; index++) {
		if (constant == QLatin1String(_constants[index].name))
			return QStringLiteral("%1 (%2 %3)").arg(_constants[index].description()).arg(_constants[index].value).arg(QLatin1String(_constants[index].unit));
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
	if (expr.isEmpty())
		return true;

	gsl_set_error_handler_off();

	Lock l(skipSpecialFunctionEvaluation);

	for (const auto& var : vars)
		assign_symbol(qPrintable(var), 0);

	// Row index
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
	const QRegularExpression re(QRegularExpression::anchoredPattern(QStringLiteral("[0-9]*")));
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

struct PayloadExpressionParser : public Payload {
	PayloadExpressionParser() {
	}
	PayloadExpressionParser(const QStringList* vars, const QVector<QVector<double>*>* xVectors, bool constant = false)
		: Payload(constant)
		, vars(vars)
		, xVectors(xVectors) {
	}
	const QStringList* vars{nullptr};
	int row{0};
	const QVector<QVector<double>*>* xVectors{nullptr};
};

double cell(double x, const char* variable, const std::weak_ptr<Payload> payload) {
	const auto p = std::dynamic_pointer_cast<PayloadExpressionParser>(payload.lock());
	if (!p) {
		assert(p); // Debug build
		return NAN;
	}

	const int index = (int)x - 1;
	for (int i = 0; i < p->vars->length(); i++) {
		if (p->vars->at(i).compare(QLatin1String(variable)) == 0) {
			if (index >= 0 && index < p->xVectors->at(i)->length())
				return p->xVectors->at(i)->at(index);
			else
				break;
		}
	}

	return NAN;
}

double ma(const char* variable, const std::weak_ptr<Payload> payload) {
	const auto p = std::dynamic_pointer_cast<PayloadExpressionParser>(payload.lock());
	if (!p) {
		assert(p); // Debug build
		return NAN;
	}
	return (cell(p->row, variable, payload) + cell(p->row + 1, variable, payload)) / 2.;
}

double mr(const char* variable, const std::weak_ptr<Payload> payload) {
	const auto p = std::dynamic_pointer_cast<PayloadExpressionParser>(payload.lock());
	if (!p) {
		assert(p); // Debug build
		return NAN;
	}
	return fabs(cell(p->row + 1, variable, payload) - cell(p->row + 1 - 1, variable, payload));
}

double smmin(double x, const char* variable, const std::weak_ptr<Payload> payload) {
	const auto p = std::dynamic_pointer_cast<PayloadExpressionParser>(payload.lock());
	if (!p) {
		assert(p); // Debug build
		return NAN;
	}

	for (int i = 0; i < p->vars->length(); i++) {
		if (p->vars->at(i).compare(QLatin1String(variable)) == 0) {
			const int N = x;
			DEBUG("N = " << N)
			if (N < 1)
				break;
			// calculate min of last n points
			double min = INFINITY;
			const int row = p->row;
			for (int index = std::max(0, row - N + 1); index <= row; index++) {
				const double v = p->xVectors->at(i)->at(index);
				if (v < min)
					min = v;
			}
			return min;
		}
	}
	return NAN;
}

double smmax(double x, const char* variable, const std::weak_ptr<Payload> payload) {
	const auto p = std::dynamic_pointer_cast<PayloadExpressionParser>(payload.lock());
	if (!p) {
		assert(p); // Debug build
		return NAN;
	}

	for (int i = 0; i < p->vars->length(); i++) {
		if (p->vars->at(i).compare(QLatin1String(variable)) == 0) {
			const int N = x;
			DEBUG("N = " << N)
			if (N < 1)
				break;
			// calculate max of last n points
			double max = -INFINITY;
			const int row = p->row;
			for (int index = std::max(0, row - N + 1); index <= row; index++) {
				const double v = p->xVectors->at(i)->at(index);
				if (v > max)
					max = v;
			}
			return max;
		}
	}
	return NAN;
}

double sma(double x, const char* variable, const std::weak_ptr<Payload> payload) {
	const auto p = std::dynamic_pointer_cast<PayloadExpressionParser>(payload.lock());
	if (!p) {
		assert(p); // Debug build
		return NAN;
	}

	for (int i = 0; i < p->vars->length(); i++) {
		if (p->vars->at(i).compare(QLatin1String(variable)) == 0) {
			const int N = x;
			DEBUG("N = " << N)
			if (N < 1)
				break;
			// calculate max of last n points
			double sum = 0.;
			const int row = p->row;
			for (int index = std::max(0, row - N + 1); index <= row; index++)
				sum += p->xVectors->at(i)->at(index);
			return sum / N;
		}
	}
	return NAN;
}

double smr(double x, const char* variable, const std::weak_ptr<Payload> payload) {
	return smmax(x, variable, payload) - smmin(x, variable, payload);
}

double psample(double n, const char* variable, const std::weak_ptr<Payload> payload) {
	const auto p = std::dynamic_pointer_cast<PayloadExpressionParser>(payload.lock());
	if (!p) {
		assert(p); // Debug build
		return NAN;
	}

	// every n-th value, starting @ first
	return cell(n * p->row + 1, variable, payload);
}

void ExpressionParser::setSpecialFunction1(const char* function_name, func_t1Payload funct, std::shared_ptr<Payload> payload) {
	set_specialfunction1(function_name, funct, payload);
}

void ExpressionParser::setSpecialFunction2(const char* function_name, func_t2Payload funct, std::shared_ptr<Payload> payload) {
	set_specialfunction2(function_name, funct, payload);
}

/*!
	evaluates multivariate function y=f(x_1, x_2, ...).
	Variable names (x_1, x_2, ...) are stored in \c vars.
	Data is stored in \c xVectors.
 */
bool ExpressionParser::evaluateCartesian(const QString& expr, const QStringList& vars, const QVector<QVector<double>*>& xVectors, QVector<double>* yVector) {
#if PERFTRACE_EXPRESSION_PARSER
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
#endif
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

	const auto payload = std::make_shared<PayloadExpressionParser>(&vars, &xVectors);
	const auto payloadConst = std::make_shared<PayloadExpressionParser>(&vars, &xVectors, true);

	set_specialfunction2(specialfun_cell, cell, payloadConst);
	set_specialfunction1(specialfun_ma, ma, payload);
	set_specialfunction1(specialfun_mr, mr, payload);
	set_specialfunction2(specialfun_smmin, smmin, payload);
	set_specialfunction2(specialfun_smmax, smmax, payload);
	set_specialfunction2(specialfun_sma, sma, payload);
	set_specialfunction2(specialfun_smr, smr, payload);
	set_specialfunction2(specialfun_psample, psample, payload);

	bool constExpression = false;
	for (int i = 0; i < minSize || (constExpression && i < yVector->size()); i++) {
		QString tmpExpr = expr;

		payload->row = i; // all special functions contain pointer to payload so they get this information
		assign_symbol("i", i + 1);

		for (int n = 0; n < vars.size(); ++n) {
			if (!constExpression)
				assign_symbol(qPrintable(vars.at(n)), xVectors.at(n)->at(i));
		}

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

QString ExpressionParser::errorMessage() const {
	return QLatin1String(lastErrorMessage());
}
