/*
	File                 : AbstractFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for AbstractFilter and AbstractSimpleFilter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 LabPlot contributors
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AbstractFilterTest.h"
#include "backend/core/AbstractFilter.h"
#include "backend/core/AbstractSimpleFilter.h"
#include "backend/core/CopyThroughFilter.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/SimpleCopyThroughFilter.h"

#include <memory>

namespace {
class TwoPortFilter : public AbstractFilter {
public:
	explicit TwoPortFilter(const QString& name = QStringLiteral("test filter"))
		: AbstractFilter(name) {
	}

	int inputCount() const override {
		return 2;
	}
	int outputCount() const override {
		return 2;
	}
	void save(QXmlStreamWriter*) const override {
	}
	bool load(XmlStreamReader*, bool) override {
		return true;
	}
	AbstractColumn* output(int port) override {
		return const_cast<AbstractColumn*>(m_inputs.value(port));
	}
	const AbstractColumn* output(int port) const override {
		return m_inputs.value(port);
	}

	int rejectedPort{-1};
	int disconnectCount{0};

protected:
	bool inputAcceptable(int port, const AbstractColumn*) override {
		return port != rejectedPort;
	}
	void inputAboutToBeDisconnected(const AbstractColumn*) override {
		++disconnectCount;
	}
};
}

void AbstractFilterTest::testInputPorts() {
	TwoPortFilter filter;
	Column first(QStringLiteral("first"), QVector<double>{1., 2.});
	Column second(QStringLiteral("second"), QVector<double>{3., 4.});

	QCOMPARE(filter.highestConnectedInput(), -1);
	QVERIFY(!filter.input(-1, &first));
	QVERIFY(!filter.input(2, &first));
	QVERIFY(filter.input(1, &first));
	QCOMPARE(filter.highestConnectedInput(), 1);
	QCOMPARE(static_cast<const AbstractFilter&>(filter).input(0), static_cast<const AbstractColumn*>(nullptr));
	QCOMPARE(filter.portIndexOf(&first), 1);
	QCOMPARE(filter.inputLabel(0), QStringLiteral("In1"));
	QVERIFY(filter.input(1, &first)); // Reconnecting the same source is a no-op.
	QCOMPARE(filter.disconnectCount, 0);

	filter.rejectedPort = 0;
	QVERIFY(!filter.input(0, &second));
	QCOMPARE(static_cast<const AbstractFilter&>(filter).input(0), static_cast<const AbstractColumn*>(nullptr));
	filter.rejectedPort = -1;
	QVERIFY(filter.input(0, &second));
	QCOMPARE(filter.highestConnectedInput(), 1);
	QVERIFY(filter.input(1, nullptr));
	QCOMPARE(filter.disconnectCount, 1);
	QCOMPARE(filter.highestConnectedInput(), 0);
	QVERIFY(filter.input(0, nullptr));
	QCOMPARE(filter.disconnectCount, 2);
	QCOMPARE(filter.highestConnectedInput(), -1);
	QCOMPARE(filter.portIndexOf(&first), -1);
}

void AbstractFilterTest::testFilterInputForwarding() {
	TwoPortFilter source(QStringLiteral("source"));
	TwoPortFilter target(QStringLiteral("target"));
	Column first(QStringLiteral("first"));
	Column second(QStringLiteral("second"));

	QVERIFY(source.input(0, &first));
	QVERIFY(source.input(1, &second));
	QVERIFY(target.input(&source));
	QCOMPARE(static_cast<const AbstractFilter&>(target).input(0), &first);
	QCOMPARE(static_cast<const AbstractFilter&>(target).input(1), &second);
	QVERIFY(!target.input(nullptr));
}

void AbstractFilterTest::testInputDestroyed() {
	TwoPortFilter filter;
	{
		auto source = std::make_unique<Column>(QStringLiteral("temporary"));
		QVERIFY(filter.input(0, source.get()));
		QCOMPARE(filter.highestConnectedInput(), 0);
	}

	QCOMPARE(static_cast<const AbstractFilter&>(filter).input(0), static_cast<const AbstractColumn*>(nullptr));
	QCOMPARE(filter.highestConnectedInput(), -1);
}

void AbstractFilterTest::testSimpleFilterOutputColumn() {
	SimpleCopyThroughFilter filter;
	const auto* output = filter.output(0);
	QVERIFY(output);
	QCOMPARE(filter.inputCount(), 1);
	QCOMPARE(filter.outputCount(), 1);
	QCOMPARE(output->rowCount(), 0);
	QCOMPARE(output->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(filter.output(1), nullptr);

	Column input(QStringLiteral("values"), QVector<double>{1.5, 2.5, 4.});
	input.setPlotDesignation(AbstractColumn::PlotDesignation::Y);
	QVERIFY(filter.input(0, &input));
	QCOMPARE(filter.inputColumn(), &input);
	QCOMPARE(output->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(output->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(output->rowCount(), 3);
	QCOMPARE(output->availableRowCount(2), 2);
	QCOMPARE(output->valueAt(1), 2.5);

	QVERIFY(filter.input(0, nullptr));
	QCOMPARE(filter.inputColumn(), nullptr);
	QCOMPARE(output->rowCount(), 0);
	QCOMPARE(output->columnMode(), AbstractColumn::ColumnMode::Text);
}

void AbstractFilterTest::testCopyThroughFilter() {
	CopyThroughFilter filter(QStringLiteral("pass through"));
	Column first(QStringLiteral("first"));
	Column second(QStringLiteral("second"));

	QCOMPARE(filter.inputCount(), -1);
	QCOMPARE(filter.outputCount(), 0);
	QVERIFY(filter.input(0, &first));
	QVERIFY(filter.input(1, &second));
	QCOMPARE(filter.outputCount(), 2);
	QCOMPARE(filter.output(0), &first);
	QCOMPARE(filter.output(1), &second);
	QCOMPARE(filter.output(2), nullptr);
}

QTEST_MAIN(AbstractFilterTest)
