/*
	File                 : ColumnTest.h
	Project              : LabPlot
	Description          : Tests for Column
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COLUMNTEST_H
#define COLUMNTEST_H

#include "../../CommonMetaTest.h"

class ColumnTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:
	// ranges
	void doubleMinimum();
	void doubleMaximum();
	void integerMinimum();
	void integerMaximum();
	void bigIntMinimum();
	void bigIntMaximum();

	// statistical properties for different column modes
	void statisticsDouble(); // only positive double values
	void statisticsDoubleNegative(); // contains negative values (> -100)
	void statisticsDoubleBigNegative(); // contains big negative values (<= -100)
	void statisticsDoubleZero(); // contains zero value
	void statisticsInt(); // only positive integer values
	void statisticsIntNegative(); // contains negative values (> -100)
	void statisticsIntBigNegative(); // contains big negative values (<= -100)
	void statisticsIntZero(); // contains zero value
	void statisticsIntOverflow(); // check overflow of integer
	void statisticsBigInt(); // big ints
	void statisticsText();

	void statisticsMaskValues();
	void statisticsClearSpreadsheetMasks();

	// generation of column values via a formula
	void testFormulaAutoUpdateEnabledResize();
	void testFormulaAutoUpdateEnabled();
	void testFormulaAutoUpdateDisabled();
	void testFormulaAutoResizeEnabled();
	void testFormulaAutoResizeDisabled();

	// dictionary related tests for text columns
	void testDictionaryIndex();
	void testTextFrequencies();

	// performance of save and load
	void loadDoubleFromProject();
	void loadIntegerFromProject();
	void loadBigIntegerFromProject();
	void loadTextFromProject();
	void loadDateTimeFromProject();
	void saveLoadDateTime();

	void testIndexForValueStrictlyMonotonouslyRising();
	void testIndexForValueMonotonouslyRising();
	void indexForValueDatetimeStrictlyMonotonouslyRising();
	void indexForValueDatetimeMonotonouslyRising();
	void testIndexForValuePoints();
	void testIndexForValueLines();
	void testIndexForValueDoubleVector();

	void testInsertRow();
	void testRemoveRow();

	void testFormula();
	void testFormula2();
	void testFormulaCell();
	void testFormulaCellDefault();
	void testFormulaCellInvalid();
	void testFormulaCellConstExpression();
	void testFormulaCellMulti();
	void testFormulaCurrentColumnCell();
	void testFormulaCurrentColumnCellDefaultValue();
	void testFormulasmmin();
	void testFormulasmmax();
	void testFormulasma();
	void testFormulapsample();
	void testFormularsample();

	void testFormulasMinColumnInvalid();

	void testFormulasSize();
	void testFormulasMin();
	void testFormulasMax();
	void testFormulasMean();
	void testFormulasMedian();
	void testFormulasStdev();
	void testFormulasVar();
	void testFormulasGm();
	void testFormulasHm();
	void testFormulasChm();
	void testFormulasStatisticsMode();
	void testFormulasQuartile1();
	void testFormulasQuartile3();
	void testFormulasIqr();
	void testFormulasPercentile1();
	void testFormulasPercentile5();
	void testFormulasPercentile10();
	void testFormulasPercentile90();
	void testFormulasPercentile95();
	void testFormulasPercentile99();
	void testFormulasTrimean();
	void testFormulasMeandev();
	void testFormulasMeandevmedian();
	void testFormulasMediandev();
	void testFormulasSkew();
	void testFormulasKurt();
	void testFormulasEntropy();
	void testFormulasQuantile();
	void testFormulasPercentile();

	void clearContentNoFormula();
	void clearContentFormula();

	void testRowCountMonotonIncrease();
	void testRowCountMonotonDecrease();
	void testRowCountNonMonoton();
	void testRowCountDateTime();
	void testRowCountDateTimeMonotonDecrease();

	void testRowCountValueLabels();
	void testRowCountValueLabelsDateTime();

	void testLoadSaveNoData();
	void testLoadSaveWithData();
};

#endif // COLUMNTEST_H
