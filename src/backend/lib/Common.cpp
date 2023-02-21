#include "Common.h"
#include "ExpressionParser.h"
#include "backend/core/Project.h"

#include "XmlStreamReader.h"
#include <QXmlStreamWriter>

namespace Common {

ExpressionValue::ExpressionValue() {
}

ExpressionValue::ExpressionValue(qint64 v)
	: m_d{false}
	, m_unit(Worksheet::Unit::None) {
	m_value.i64 = v;
}

ExpressionValue::ExpressionValue(double v)
	: m_d{true}
	, m_unit(Worksheet::Unit::None) {
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

void ExpressionValue::save(QXmlStreamWriter* writer, const QString& prefix) const {
	writer->writeAttribute(prefix + QStringLiteral("_Expression"), m_expression);
	writer->writeAttribute(prefix + QStringLiteral("_Type"), QString::number(isDouble()));
	writer->writeAttribute(prefix + QStringLiteral("_Unit"), Worksheet::unitToString(m_unit));
}

bool ExpressionValue::load(const XmlStreamReader* reader, const QString& prefix) {
	const auto& attribs = reader->attributes();
	if (Project::xmlVersion() < 8) {
		m_d = true;
		const QString str = attribs.value(prefix).toString();
		if (str.isEmpty())
			return false;
		else
			m_value.d = str.toDouble();
	} else {
		if (!attribs.hasAttribute(prefix + QStringLiteral("_Expression")))
			return false;
		m_expression = attribs.value(prefix + QStringLiteral("_Expression")).toString();

		if (!attribs.hasAttribute(prefix + QStringLiteral("_Type")))
			return false;
		m_d = attribs.value(prefix + QStringLiteral("_Type")).toString().toInt();

		if (!attribs.hasAttribute(prefix + QStringLiteral("_Unit")))
			return false;

		m_unit = Worksheet::stringToUnit(attribs.value(prefix + QStringLiteral("_Unit")).toString());

		bool ok;
		if (m_d) {
			m_value.d = m_expression.toDouble(&ok);
			if (ok) {
				m_value.d = Worksheet::convertUnits(m_value.d, m_unit, Worksheet::Unit::Scene);
				m_expression.clear();
			}
		} else {
			m_value.i64 = m_expression.toLongLong(&ok);
			if (ok) {
				m_value.i64 = Worksheet::convertUnits(m_value.i64, m_unit, Worksheet::Unit::Scene);
				m_expression.clear();
			}
		}

		if (!m_expression.isEmpty()) {
			// Calculate value

			ExpressionParser* parser = ExpressionParser::getInstance();
			if (!parser->evaluateCartesian(m_expression, m_value.d))
				return false;

			m_value.d = Worksheet::convertUnits(m_value.d, m_unit, Worksheet::Unit::Scene);
		}
	}
	return true;
}

} // namespace Common
