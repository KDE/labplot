/*
	File                 : AbstractColumn.h
	Project              : LabPlot
	Description          : Interface definition for data with column logic
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007, 2008 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2013-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2020 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACTCOLUMN_H
#define ABSTRACTCOLUMN_H

#include "backend/core/AbstractAspect.h"
#include <QColor>

class AbstractColumnPrivate;
class AbstractSimpleFilter;
class QString;
class QDateTime;
class QDate;
class QTime;
template<class T>
class Interval;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT AbstractColumn : public AbstractAspect {
#else
class AbstractColumn : public AbstractAspect {
#endif
	Q_OBJECT

public:
	enum class PlotDesignation { NoDesignation, X, Y, Z, XError, XErrorPlus, XErrorMinus, YError, YErrorPlus, YErrorMinus };
	Q_ENUM(PlotDesignation)
	// how to convert numeric <-> datetime
	enum class TimeUnit { Milliseconds, Seconds, Minutes, Hours, Days };
	Q_ENUM(TimeUnit)
	enum class ColumnMode {
		// BASIC FORMATS
		Double = 0, // double
		Text = 1, // QString
		// Time = 2 and Date = 3 are skipped to avoid problems with old obsolete values
		Month = 4, // month of year: numeric or "Jan", etc.
		Day = 5, // day of week: numeric or "Mon", etc.
		DateTime = 6, // any date-time format
		//		Bool = 7,	// bool
		// FLOATING POINT
		// 10 = Half precision
		//		Float = 11,	// float
		// 12 = Long double
		// 13 = Quad precision
		// 14 = decimal 32
		// 15 = decimal 64
		// 16 = decimal 128
		// COMPLEX
		// 17 = complex<float>
		// 18 = complex<double>
		// 19 = complex<long double>
		// INTEGER
		//		Int8 = 20,	// qint8 (char)
		//		UInt8 = 21,	// quint8 (unsigned char)
		//		Int16 = 22,	// qint16 (short)
		//		UInt16 = 23,	// quint16 (unsigned short)
		Integer = 24, // qint32 (int)
		//		UInt32 = 25,	// quint32 (unsigned int)
		BigInt = 26 // qint64 (long)
		//		UInt64 = 27,	// quint64 (unsigned long)
		// MISC
		// QBrush = 30
		// QColor
		// QFont
		// QPoint
		// QQuaternion
		// QVector2D, QVector3D, QVector4D
		// QMatrix
		// etc.
	};
	// TODO: breaks linking
	// Q_ENUM(ColumnMode)
	enum class Properties { // TODO: why bit pattern? Aren't they exclusive?
		No = 0x00, // invalid values or masked values
		Constant = 0x01,
		MonotonicIncreasing = 0x02, // prev_value >= value for all values in column
		MonotonicDecreasing = 0x04, // prev_value <= value for all values in column
		NonMonotonic = 0x8,
		// add new values with next bit set (0x10)
	};
	Q_ENUM(Properties)

	// exposed in function dialog (ColumnPrivate::updateFormula(), ExpressionParser::initFunctions(), functions.h)
	struct ColumnStatistics {
		int size{0};
		int unique{0}; // number of unique values, relevant for text columns only
		double minimum{qQNaN()};
		double maximum{qQNaN()};
		double arithmeticMean{qQNaN()};
		double geometricMean{qQNaN()};
		double harmonicMean{qQNaN()};
		double contraharmonicMean{qQNaN()};
		double mode{qQNaN()};
		double firstQuartile{qQNaN()};
		double median{qQNaN()};
		double thirdQuartile{qQNaN()};
		double iqr{qQNaN()};
		double percentile_1{qQNaN()};
		double percentile_5{qQNaN()};
		double percentile_10{qQNaN()};
		double percentile_90{qQNaN()};
		double percentile_95{qQNaN()};
		double percentile_99{qQNaN()};
		double trimean{qQNaN()};
		double variance{qQNaN()};
		double standardDeviation{qQNaN()};
		double meanDeviation{qQNaN()}; // mean absolute deviation around mean
		double meanDeviationAroundMedian{qQNaN()}; // mean absolute deviation around median
		double medianDeviation{qQNaN()}; // median absolute deviation
		double skewness{qQNaN()};
		double kurtosis{qQNaN()};
		double entropy{qQNaN()};
	};

	AbstractColumn(const QString& name, AspectType type);
	~AbstractColumn() override;

	static QStringList dateFormats(); // supported date formats
	static QStringList timeFormats(); // supported time formats
	static QStringList dateTimeFormats(); // supported datetime formats
	static QString timeUnitString(TimeUnit);
	static QString plotDesignationString(PlotDesignation, bool withBrackets = true);
	static QString columnModeString(ColumnMode);
	static QIcon modeIcon(ColumnMode);

	virtual bool isReadOnly() const {
		return true;
	};
	virtual ColumnMode columnMode() const = 0;
	virtual void setColumnMode(AbstractColumn::ColumnMode);
	virtual PlotDesignation plotDesignation() const = 0;
	virtual void setPlotDesignation(AbstractColumn::PlotDesignation);
	bool isNumeric() const;
	bool isPlottable() const;

	virtual bool copy(const AbstractColumn* source);
	virtual bool copy(const AbstractColumn* source, int source_start, int dest_start, int num_rows);

	virtual int rowCount() const = 0;
	virtual int rowCount(double min, double max) const = 0;
	virtual int availableRowCount(int max = -1) const = 0;
	void insertRows(int before, int count, QUndoCommand* parent = nullptr);
	void removeRows(int first, int count, QUndoCommand* parent = nullptr);
	virtual void clear(QUndoCommand*);

	virtual double maximum(int count = 0) const;
	virtual double maximum(int startIndex, int endIndex) const;
	virtual double minimum(int count = 0) const;
	virtual double minimum(int startIndex, int endIndex) const;
	virtual bool indicesMinMax(double v1, double v2, int& start, int& end) const;
	virtual int indexForValue(double x) const;

	bool isValid(int row) const;

	bool isMasked(int row) const;
	bool isMasked(const Interval<int>& i) const;
	QVector<Interval<int>> maskedIntervals() const;
	void clearMasks();
	void setMasked(const Interval<int>& i, bool mask = true);
	void setMasked(int row, bool mask = true);

	virtual QString formula(int row) const;
	virtual QVector<Interval<int>> formulaIntervals() const;
	virtual void setFormula(const Interval<int>& i, const QString& formula);
	virtual void setFormula(int row, const QString& formula);
	virtual void clearFormulas();

	virtual QString textAt(int row) const;
	virtual void setTextAt(int row, const QString& new_value);
	virtual void replaceTexts(int first, const QVector<QString>& new_values);
	virtual int dictionaryIndex(int row) const;

	virtual QDate dateAt(int row) const;
	virtual void setDateAt(int row, QDate new_value);
	virtual QTime timeAt(int row) const;
	virtual void setTimeAt(int row, QTime new_value);
	virtual QDateTime dateTimeAt(int row) const;
	virtual void setDateTimeAt(int row, const QDateTime& new_value);
	virtual void replaceDateTimes(int first, const QVector<QDateTime>& new_values);

	virtual double doubleAt(int row) const;
	virtual double valueAt(int row) const;
	virtual void setValueAt(int row, double new_value);
	virtual void replaceValues(int first, const QVector<double>& new_values);

	virtual int integerAt(int row) const;
	virtual void setIntegerAt(int row, int new_value);
	virtual void replaceInteger(int first, const QVector<int>& new_values);

	virtual qint64 bigIntAt(int row) const;
	virtual void setBigIntAt(int row, qint64 new_value);
	virtual void replaceBigInt(int first, const QVector<qint64>& new_values);

	virtual Properties properties() const;
	virtual void invalidateProperties(){};

	// conditional formatting
	enum class Formatting { Background, Foreground, Icon };

	struct HeatmapFormat {
		double min = 0.0;
		double max = 1.0;
		QString name;
		Formatting type = Formatting::Background;
		QVector<QColor> colors;
	};

	bool hasHeatmapFormat() const;
	HeatmapFormat& heatmapFormat() const;
	void setHeatmapFormat(const HeatmapFormat&);
	void removeFormat();
	void reset();
	void setChanged();
	void setSuppressDataChangedSignal(const bool);

Q_SIGNALS:
	void plotDesignationAboutToChange(const AbstractColumn* source);
	void plotDesignationChanged(const AbstractColumn* source);
	void modeAboutToChange(const AbstractColumn* source);
	void modeChanged(const AbstractColumn* source);
	void dataAboutToChange(const AbstractColumn* source);
	void dataChanged(const AbstractColumn* source);
	void formatChanged(const AbstractColumn* source);
	/*!
	 * \brief rowsAboutToBeInserted
	 * \param source
	 * \param before Old number of rows
	 * \param count Number of rows added
	 */
	void rowsAboutToBeInserted(const AbstractColumn* source, int before, int count);
	/*!
	 * \brief rowsInserted
	 * \param source
	 * \param before Old number of rows
	 * \param count Number of rows added
	 */
	void rowsInserted(const AbstractColumn* source, int before, int count);
	/*!
	 * \brief rowsAboutToBeRemoved
	 * \param source
	 * \param first Old number of rows
	 * \param count Number of rows removed
	 */
	void rowsAboutToBeRemoved(const AbstractColumn* source, int first, int count);
	/*!
	 * \brief rowsRemoved
	 * \param source
	 * \param first Old number of rows
	 * \param count Number of rows removed
	 */
	void rowsRemoved(const AbstractColumn* source, int first, int count);
	void maskingAboutToChange(const AbstractColumn* source);
	void maskingChanged(const AbstractColumn* source);
	void aboutToBeDestroyed(const AbstractColumn* source);
	void aboutToReset(const AbstractColumn* source); // this signal is emitted when the column is reused for another purpose. The curves must know that and
													 // disconnect all connections

protected:
	bool XmlReadMask(XmlStreamReader*);
	void XmlWriteMask(QXmlStreamWriter*) const;

	virtual void handleRowInsertion(int before, int count, QUndoCommand* parent);
	virtual void handleRowRemoval(int first, int count, QUndoCommand* parent);

private:
	AbstractColumnPrivate* d;

	friend class AbstractColumnRemoveRowsCmd;
	friend class AbstractColumnInsertRowsCmd;
	friend class AbstractColumnClearMasksCmd;
	friend class AbstractColumnSetMaskedCmd;
};

#endif
