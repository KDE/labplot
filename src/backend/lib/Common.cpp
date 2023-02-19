#include "Common.h"
#include "ExpressionParser.h"

namespace Common {

ExpressionValue::ExpressionValue() {
}

ExpressionValue::ExpressionValue(qint64 v)
	: m_d{false} {
	m_value.i64 = v;
}

ExpressionValue::ExpressionValue(double v)
	: m_d{true} {
	m_value.d = v;
}

ExpressionValue::ExpressionValue(const QString& e, double v, Worksheet::Unit unit)
	: m_expression(e)
	, m_d{true}
	, m_unit(unit) {
	m_value.d = v;
}

ExpressionValue::ExpressionValue(const QString& e, Worksheet::Unit unit)
	: m_unit(unit) {
	bool ok;
	qint64 v = e.toLongLong(&ok);
	if (ok) {
		m_d = false;
		m_value.i64 = v;
		return;
	}

	m_d = true;
	double vd = e.toDouble(&ok);
	if (ok) {
		m_value.d = vd;
		return;
	}

	ExpressionParser* parser = ExpressionParser::getInstance();
	if (!parser->evaluateCartesian(e, m_value.d))
		m_value.d = std::nan("0");
	else
		m_expression = e;
}

ExpressionValue ExpressionValue::loadFromConfig(const KConfigGroup& group, const QString& prefix, const ExpressionValue& defaultValue) {
	if (!group.hasGroup(prefix))
		return defaultValue;

	auto& g = group.group(prefix);
	const QString expression = g.readEntry(QStringLiteral("Expression"), QStringLiteral("0"));
	const bool type = g.readEntry(QStringLiteral("Type"), true);
	Worksheet::Unit unit = Worksheet::stringToUnit(g.readEntry(QStringLiteral("Unit"), Worksheet::unitToString(Worksheet::Unit::None)));
	return ExpressionValue(expression, unit);
}

ExpressionValue ExpressionValue::loadFromConfig(const KConfigGroup& group, const QString& prefix, double defaultValue) {
	if (!group.hasGroup(prefix))
		return ExpressionValue(defaultValue);

	auto& g = group.group(prefix);
	const QString expression = g.readEntry(QStringLiteral("Expression"), QStringLiteral("0"));
	const bool type = g.readEntry(QStringLiteral("Type"), true);
	Worksheet::Unit unit = Worksheet::stringToUnit(g.readEntry(QStringLiteral("Unit"), Worksheet::unitToString(Worksheet::Unit::None)));
	return ExpressionValue(expression, unit);
}

void ExpressionValue::configWriteEntry(KConfigGroup& group, const QString& prefix, Worksheet::Unit unit) const {
	KConfigGroup g(&group, prefix);
	g.writeEntry(QStringLiteral("Expression"), toString());
	g.writeEntry(QStringLiteral("Type"), QString::number(isDouble()));
	g.writeEntry(QStringLiteral("Unit"), Worksheet::unitToString(unit));
}

} // namespace Common
