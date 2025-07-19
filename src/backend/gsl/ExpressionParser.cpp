/*
	File             : ExpressionParser.cpp
	Project          : LabPlot
	Description      : C++ wrapper for the bison generated parser.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2014-2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/Parser.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "parser_private.h"

#include <KLocalizedString>

#include <QRegularExpression>

#include <random>

#include <gsl/gsl_const_mksa.h>
#include <gsl/gsl_const_num.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_version.h>

using namespace Parsing;

namespace {
class ParserLastErrorMessage {
public:
	ParserLastErrorMessage(const Parser& parser, QString& lastErrorMessage)
		: mParser(parser)
		, mLastErrorMessage(lastErrorMessage) {
	}
	~ParserLastErrorMessage() {
		mLastErrorMessage = QString::fromStdString(mParser.lastErrorMessage());
	}

private:
	const Parser& mParser;
	QString& mLastErrorMessage;
};
}

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

	Parser parser;
	parser.setSkipSpecialFunctionEvaluation(true);

	for (const auto& var : vars)
		parser.assign_symbol(qPrintable(var), 0);

	// Row index
	parser.assign_symbol("i", 0);

	const auto numberLocale = QLocale();
	DEBUG(Q_FUNC_INFO << ", number locale: " << STDSTRING(numberLocale.name()))
	parser.parse(qPrintable(expr), qPrintable(numberLocale.name()));

	// if parsing with number locale fails, try default locale
	if (parser.parseErrors() > 0) {
		DEBUG(Q_FUNC_INFO << ", WARNING: failed parsing expr \"" << STDSTRING(expr) << "\" with locale " << numberLocale.name().toStdString()
						  << ", errors = " << parser.parseErrors())
		parser.parse(qPrintable(expr), "en_US");
		if (parser.parseErrors() > 0)
			DEBUG(Q_FUNC_INFO << ", ERROR: parsing FAILED, errors = " << parser.parseErrors())
	}

	/* remove temporarily defined symbols */
	for (const auto& var : vars)
		parser.remove_symbol(qPrintable(var));

	return parser.parseErrors() == 0;
}

QStringList ExpressionParser::getParameter(const QString& expr, const QStringList& vars) {
	QDEBUG(Q_FUNC_INFO << ", variables:" << vars);
	QStringList parameters;

	QStringList strings = expr.split(QRegularExpression(QStringLiteral("\\W+")), Qt::SkipEmptyParts);
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
bool ExpressionParser::tryEvaluateCartesian(const QString& expr,
											const Range<double> range,
											int count,
											QVector<double>* xVector,
											QVector<double>* yVector,
											const QStringList& paramNames,
											const QVector<double>& paramValues) {
	DEBUG(Q_FUNC_INFO << ", v0: range = " << range.toStdString())
	const double step = range.stepSize(count);
	DEBUG(Q_FUNC_INFO << ", range = " << range.toStdString() << ", step = " << step)

	Parser parser;
	ParserLastErrorMessage lock(parser, m_lastErrorMessage);

	for (int i = 0; i < paramNames.size(); ++i)
		parser.assign_symbol(qPrintable(paramNames.at(i)), paramValues.at(i));

	const auto numberLocale = QLocale();
	gsl_set_error_handler_off();
	for (int i = 0; i < count; i++) {
		const double x{range.start() + step * i};
		parser.assign_symbol("x", x);

		double y = parser.parse(qPrintable(expr), qPrintable(numberLocale.name()));
		if (parser.parseErrors() > 0) // try default locale if failing
			y = parser.parse(qPrintable(expr), "en_US");
		if (parser.parseErrors() > 0)
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
bool ExpressionParser::tryEvaluateCartesian(const QString& expr,
											const QString& min,
											const QString& max,
											int count,
											QVector<double>* xVector,
											QVector<double>* yVector,
											const QStringList& paramNames,
											const QVector<double>& paramValues) {
	DEBUG(Q_FUNC_INFO << ", v1: range = " << STDSTRING(min) << " .. " << STDSTRING(max))

	const Range<double> range{min, max};
	return tryEvaluateCartesian(expr, range, count, xVector, yVector, paramNames, paramValues);
}

bool ExpressionParser::tryEvaluateCartesian(const QString& expr,
											const QString& min,
											const QString& max,
											int count,
											QVector<double>* xVector,
											QVector<double>* yVector) {
	const Range<double> range{min, max};
	return tryEvaluateCartesian(expr, range, count, xVector, yVector, QStringList(), QVector<double>());
}

bool ExpressionParser::tryEvaluateCartesian(const QString& expr,
											const QVector<double>* xVector,
											QVector<double>* yVector,
											const QStringList& paramNames,
											const QVector<double>& paramValues) {
	DEBUG(Q_FUNC_INFO << ", v4")
	gsl_set_error_handler_off();

	Parser parser;
	ParserLastErrorMessage lock(parser, m_lastErrorMessage);

	for (int i = 0; i < paramNames.size(); ++i)
		parser.assign_symbol(qPrintable(paramNames.at(i)), paramValues.at(i));

	const auto numberLocale = QLocale();
	for (int i = 0; i < xVector->count(); i++) {
		parser.assign_symbol("x", xVector->at(i));

		double y = parser.parse(qPrintable(expr), qPrintable(numberLocale.name()));
		if (parser.parseErrors() > 0) // try default locale if failing
			y = parser.parse(qPrintable(expr), "en_US");
		if (parser.parseErrors() > 0)
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

double cell(double x, const std::string_view& variable, const std::weak_ptr<Payload> payload) {
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

double cell_default_value(double x, double defaultValue, const std::string_view& variable, const std::weak_ptr<Payload> payload) {
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

	return defaultValue;
}

double ma(const std::string_view& variable, const std::weak_ptr<Payload> payload) {
	const auto p = std::dynamic_pointer_cast<PayloadExpressionParser>(payload.lock());
	if (!p) {
		assert(p); // Debug build
		return NAN;
	}
	return (cell(p->row, variable, payload) + cell(p->row + 1, variable, payload)) / 2.;
}

double mr(const std::string_view& variable, const std::weak_ptr<Payload> payload) {
	const auto p = std::dynamic_pointer_cast<PayloadExpressionParser>(payload.lock());
	if (!p) {
		assert(p); // Debug build
		return NAN;
	}
	return fabs(cell(p->row + 1, variable, payload) - cell(p->row + 1 - 1, variable, payload));
}

double smmin(double x, const std::string_view& variable, const std::weak_ptr<Payload> payload) {
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

double smmax(double x, const std::string_view& variable, const std::weak_ptr<Payload> payload) {
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

double sma(double x, const std::string_view& variable, const std::weak_ptr<Payload> payload) {
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

double smr(double x, const std::string_view& variable, const std::weak_ptr<Payload> payload) {
	return smmax(x, variable, payload) - smmin(x, variable, payload);
}

double psample(double n, const std::string_view& variable, const std::weak_ptr<Payload> payload) {
	const auto p = std::dynamic_pointer_cast<PayloadExpressionParser>(payload.lock());
	if (!p) {
		assert(p); // Debug build
		return NAN;
	}

	// every n-th value, starting @ first
	return cell((int)n * p->row + 1, variable, payload);
}

double rsample(const std::string_view& variable, const std::weak_ptr<Payload> payload) {
	const auto p = std::dynamic_pointer_cast<PayloadExpressionParser>(payload.lock());
	if (!p) {
		assert(p); // Debug build
		return NAN;
	}

	// random value of all rows
	auto size = p->xVectors->at(0)->size();

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, size - 1);
	double value;
	do { // loop until valid index generated
		int randomIndex = dist(gen);
		value = cell(randomIndex + 1, variable, payload);
	} while (std::isnan(value));

	return value;
}

void ExpressionParser::setSpecialFunctionValuePayload(const char* function_name, func_tValuePayload funct, std::shared_ptr<Payload> payload) {
	Parser::set_specialfunctionValuePayload(function_name, funct, payload);
}

void ExpressionParser::setSpecialFunction2ValuePayload(const char* function_name, func_t2ValuePayload funct, std::shared_ptr<Payload> payload) {
	Parser::set_specialfunction2ValuePayload(function_name, funct, payload);
}

void ExpressionParser::setSpecialFunctionVariablePayload(const char* function_name, func_tVariablePayload funct, std::shared_ptr<Payload> payload) {
	Parser::set_specialfunctionVariablePayload(function_name, funct, payload);
}

void ExpressionParser::setSpecialFunctionValueVariablePayload(const char* function_name, func_tValueVariablePayload funct, std::shared_ptr<Payload> payload) {
	Parser::set_specialfunctionValueVariablePayload(function_name, funct, payload);
}

/*!
	evaluates multivariate function y=f(x_1, x_2, ...).
	Variable names (x_1, x_2, ...) are stored in \c vars.
	Data is stored in \c xVectors.
 */
bool ExpressionParser::tryEvaluateCartesian(const QString& expr,
											const QStringList& vars,
											const QVector<QVector<double>*>& xVectors,
											QVector<double>* yVector,
											bool performanceOptimization) {
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

	Parser parser(performanceOptimization);

	parser.set_specialfunctionValueVariablePayload(specialfun_cell, cell, payloadConst);
	parser.set_specialfunction2ValueVariablePayload(specialfun_cell_default_value, cell_default_value, payloadConst);
	parser.set_specialfunctionVariablePayload(specialfun_ma, ma, payload);
	parser.set_specialfunctionVariablePayload(specialfun_mr, mr, payload);
	parser.set_specialfunctionValueVariablePayload(specialfun_smmin, smmin, payload);
	parser.set_specialfunctionValueVariablePayload(specialfun_smmax, smmax, payload);
	parser.set_specialfunctionValueVariablePayload(specialfun_sma, sma, payload);
	parser.set_specialfunctionValueVariablePayload(specialfun_smr, smr, payload);
	parser.set_specialfunctionValueVariablePayload(specialfun_psample, psample, payload);
	parser.set_specialfunctionVariablePayload(specialfun_rsample, rsample, payload);

	bool constExpression = false;
	for (int i = 0; i < minSize || (constExpression && i < yVector->size()); i++) {
		QString tmpExpr = expr;

		payload->row = i; // all special functions contain pointer to payload so they get this information
		parser.assign_symbol("i", i + 1);

		for (int n = 0; n < vars.size(); ++n) {
			if (!constExpression)
				parser.assign_symbol(qPrintable(vars.at(n)), xVectors.at(n)->at(i));
		}

		double y = parser.parse(qPrintable(tmpExpr), qPrintable(numberLocale.name()));
		if (parser.parseErrors() > 0) { // try default locale if failing
			// DEBUG("Parsing with locale failed. Using en_US.")
			y = parser.parse(qPrintable(tmpExpr), "en_US");
		}

		if (parser.parseErrors() == 0)
			constExpression = parser.variablesCounter() == 0;
		// continue with next value
		// if (parser.parseErrors() > 0)
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

bool ExpressionParser::tryEvaluatePolar(const QString& expr,
										const QString& min,
										const QString& max,
										int count,
										QVector<double>* xVector,
										QVector<double>* yVector) {
	gsl_set_error_handler_off();

	const Range<double> range{min, max};
	const double step = range.stepSize(count);

	Parser parser;
	ParserLastErrorMessage lock(parser, m_lastErrorMessage);

	const auto numberLocale = QLocale();
	for (int i = 0; i < count; i++) {
		const double phi = range.start() + step * i;
		parser.assign_symbol("phi", phi);

		double r = parser.parse(qPrintable(expr), qPrintable(numberLocale.name()));
		if (parser.parseErrors() > 0) // try default locale if failing
			r = parser.parse(qPrintable(expr), "en_US");
		if (parser.parseErrors() > 0)
			return false;

		if (std::isnan(r))
			WARN(Q_FUNC_INFO << ", WARNING: expression " << STDSTRING(expr) << " evaluated @ " << phi << " is NAN")

		(*xVector)[i] = r * cos(phi);
		(*yVector)[i] = r * sin(phi);
	}

	return true;
}

bool ExpressionParser::tryEvaluateParametric(const QString& xexpr,
											 const QString& yexpr,
											 const QString& min,
											 const QString& max,
											 int count,
											 QVector<double>* xVector,
											 QVector<double>* yVector) {
	gsl_set_error_handler_off();

	const Range<double> range{min, max};
	const double step = range.stepSize(count);

	Parser parser;
	ParserLastErrorMessage lock(parser, m_lastErrorMessage);

	const auto numberLocale = QLocale();
	for (int i = 0; i < count; i++) {
		parser.assign_symbol("t", range.start() + step * i);

		double x = parser.parse(qPrintable(xexpr), qPrintable(numberLocale.name()));
		if (parser.parseErrors() > 0) // try default locale if failing
			x = parser.parse(qPrintable(xexpr), "en_US");
		if (parser.parseErrors() > 0)
			return false;

		double y = parser.parse(qPrintable(yexpr), qPrintable(numberLocale.name()));
		if (parser.parseErrors() > 0) // try default locale if failing
			y = parser.parse(qPrintable(yexpr), "en_US");
		if (parser.parseErrors() > 0)
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
	return m_lastErrorMessage;
}
