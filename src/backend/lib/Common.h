#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QtGlobal>

#include <cmath>

// TODO: get rid of this include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
#include "backend/worksheet/Worksheet.h"

class QXmlStreamWriter;
class XmlStreamReader;

namespace Common {

struct ExpressionValue {
	ExpressionValue();
	explicit ExpressionValue(qint64 v);
	explicit ExpressionValue(double v);
	ExpressionValue(const QString& expression, double v, Worksheet::Unit);
	ExpressionValue(const QString& expression, Worksheet::Unit);

	bool expressionEmpty() const {
		return m_expression.isEmpty();
	}

	const QString& expression() const {
		return m_expression;
	}

	bool isDouble() const {
		return m_d;
	}

	template<typename T>
	void setValue(T v) {
		m_expression.clear();
		if (m_d)
			m_value.d = v;
		else
			m_value.i64 = v;
	}

	template<typename T>
	T value() const {
		if (m_d)
			return m_value.d;
		return m_value.i64;
	}

	double valueDouble() const {
		if (m_d)
			return m_value.d;
		return std::nan("0");
	}

	qint64 valueInt64() const {
		if (m_d)
			return 0;
		return m_value.i64;
	}

	/*!
	 * \brief toString
	 * Creating a string of the value
	 * \return
	 */
	QString toString() const {
		if (!m_expression.isEmpty())
			return m_expression;
		if (m_d)
			return QString::number(m_value.d);
		else
			return QString::number(m_value.i64);
	}

	// bool operator ==(const ExpressionValue& rhs) {
	//     if (m_expression.compare(rhs.m_expression) == 0 && m_d == rhs.m_d) {
	//         if (m_d)
	//             return m_value.d == rhs.m_value.d;
	//         return m_value.i64 == rhs.m_value.i64;
	//     }
	//     return false;
	// }

	bool operator!=(const ExpressionValue& rhs) const {
		if (m_expression.isEmpty()) {
			if (!rhs.m_expression.isEmpty())
				return true;
		} else {
			if (m_expression.compare(rhs.m_expression) != 0)
				return true;
		}

		if (m_d != rhs.m_d)
			return true;

		if (m_d)
			return m_value.d != rhs.m_value.d;
		return m_value.i64 != rhs.m_value.i64;
	}

	double operator*(const ExpressionValue& other) const {
		return value<double>() * other.value<double>();
	}

	template<typename T>
	double operator*(T other) const {
		return value<double>() * other;
	}

	static ExpressionValue loadFromConfig(const KConfigGroup& group, const QString& prefix, const ExpressionValue& defaultValue);
	void configWriteEntry(KConfigGroup& group, const QString& prefix, Worksheet::Unit unit) const;

	void save(QXmlStreamWriter*, const QString& prefix) const;
	bool load(const XmlStreamReader*, const QString& prefix);

private:
	QString m_expression;
	bool m_d{true}; // if true double, otherwise qint64
	union Type {
		double d;
		qint64 i64;
	};
	Type m_value{std::nan("0")};
	Worksheet::Unit m_unit{Worksheet::Unit::None};
}; // class ExpressionValue

} // namespace Common

#endif // COMMON_H
