/*
	File                 : ExpressionParserTest.h
	Project              : LabPlot
	Description          : Tests for ExpressionParser
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2025 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXPRESSIONPARSERTEST_H
#define EXPRESSIONPARSERTEST_H

#include "../../CommonTest.h"

class ExpressionParserTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void testFunctionArguments1();
	void testFunctionArguments2();
	void testUniques();
	void testgreaterThan();
	void testgreaterEqualThan();
	void testlessThan();
	void testlessEqualThan();
	void testequal();
	void testifCondition();
	void testandFunction();
	void testorFunction();
	void testxorFunction();
	void testnotFunction();
	void testbetweenIncluded();
	void testTodayFunction();
	void testTodayExpression();
	void testoutsideIncluded();
	void testbetween();
	void testoutside();
	void testequalEpsilon();
	void testRoundn();
	void testSpecialFunctions();

	void testevaluateCartesian();
	void testevaluateCartesianConstExpr();
	void testevaluateGreaterThan();
	void testevaluateLessThan();
	void testevaluateLessEqualThan();
	void testevaluateGreaterEqualThan();
	void testBenchmark();

	void testEvaluateAnd();
	void testEvaluateOr();
	void testEvaluateNot();
	void testEvaluateLogicalExpression();

	void testIsValid();
	void testIsInValid();
	void testIsInValid2();
	void testIsValidStdev();
	void testLog2();

	void testPolarCircle();
	void testPolarSpiral();
};

#endif // EXPRESSIONPARSERTEST_H
