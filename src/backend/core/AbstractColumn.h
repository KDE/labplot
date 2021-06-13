/***************************************************************************
    File                 : AbstractColumn.h
    Project              : LabPlot
    Description          : Interface definition for data with column logic
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2013 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017-2020 Stefan Gerlach (stefan.gerlach@uni.kn)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef ABSTRACTCOLUMN_H
#define ABSTRACTCOLUMN_H

#include "backend/core/AbstractAspect.h"
#include <cmath>	// NAN
#include <QColor>

class AbstractColumnPrivate;
class AbstractSimpleFilter;
class QStringList;
class QString;
class QDateTime;
class QDate;
class QTime;
template<class T> class QVector;
template<class T> class Interval;

class AbstractColumn : public AbstractAspect {
	Q_OBJECT
	Q_ENUMS(PlotDesignation)
	Q_ENUMS(ColumnMode)

public:
	enum class PlotDesignation {NoDesignation, X, Y, Z, XError, XErrorPlus, XErrorMinus, YError, YErrorMinus, YErrorPlus};
	enum class ColumnMode {
		// BASIC FORMATS
		Numeric = 0,	// double
		Text = 1,	// QString
		// Time = 2 and Date = 3 are skipped to avoid problems with old obsolete values
		Month = 4,	// month of year: numeric or "Jan", etc.
		Day = 5,	// day of week: numeric or "Mon", etc.
		DateTime = 6,	// any date-time format
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
		Integer = 24,	// qint32 (int)
//		UInt32 = 25,	// quint32 (unsigned int)
		BigInt = 26,	// qint64 (long)
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
	enum class Properties {	// TODO: why bit pattern? Aren't they exclusive?
		No = 0x00,
		Constant = 0x01,
		MonotonicIncreasing = 0x02, // prev_value >= value for all values in column
		MonotonicDecreasing = 0x04 // prev_value <= value for all values in column
		// add new values with next bit set (0x08)
	};

	struct ColumnStatistics {
		ColumnStatistics() {
			size = 0;
			minimum = NAN;
			maximum = NAN;
			arithmeticMean = NAN;
			geometricMean = NAN;
			harmonicMean = NAN;
			contraharmonicMean = NAN;
			mode = NAN;
			firstQuartile = NAN;
			median = NAN;
			thirdQuartile = NAN;
			iqr = NAN;
			trimean = NAN;
			variance = NAN;
			standardDeviation = NAN;
			meanDeviation = NAN;
			meanDeviationAroundMedian = NAN;
			medianDeviation = NAN;
			skewness = NAN;
			kurtosis = NAN;
			entropy = NAN;
		}
		int size;
		double minimum;
		double maximum;
		double arithmeticMean;
		double geometricMean;
		double harmonicMean;
		double contraharmonicMean;
		double mode;
		double firstQuartile;
		double median;
		double thirdQuartile;
		double iqr;
		double percentile_1;
		double percentile_5;
		double percentile_10;
		double percentile_90;
		double percentile_95;
		double percentile_99;
		double trimean;
		double variance;
		double standardDeviation;
		double meanDeviation; // mean absolute deviation around mean
		double meanDeviationAroundMedian; // mean absolute deviation around median
		double medianDeviation; // median absolute deviation
		double skewness;
		double kurtosis;
		double entropy;
	};

	AbstractColumn(const QString& name, AspectType type);
	~AbstractColumn() override;

	static QStringList dateFormats();	// supported date formats
	static QStringList timeFormats();	// supported time formats
	static QStringList dateTimeFormats();	// supported datetime formats
	static QString modeName(ColumnMode mode);
	static QIcon modeIcon(ColumnMode mode);

	virtual bool isReadOnly() const {
		return true;
	};
	virtual ColumnMode columnMode() const = 0;
	virtual void setColumnMode(AbstractColumn::ColumnMode);
	virtual PlotDesignation plotDesignation() const = 0;
	virtual QString plotDesignationString() const = 0;
	virtual void setPlotDesignation(AbstractColumn::PlotDesignation);
	bool isNumeric() const;
	bool isPlottable() const;

	virtual bool copy(const AbstractColumn *source);
	virtual bool copy(const AbstractColumn *source, int source_start, int dest_start, int num_rows);

	virtual int rowCount() const = 0;
	virtual int availableRowCount() const = 0;
	void insertRows(int before, int count);
	void removeRows(int first, int count);
	virtual void clear();

	virtual double maximum(int count = 0) const;
	virtual double maximum(int startIndex, int endIndex) const;
	virtual double minimum(int count = 0) const;
	virtual double minimum(int startIndex, int endIndex) const;
	virtual bool indicesMinMax(double v1, double v2, int& start, int& end) const;
	virtual int indexForValue(double x) const;

	bool isValid(int row) const;

	bool isMasked(int row) const;
	bool isMasked(const Interval<int>& i) const;
	QVector< Interval<int> > maskedIntervals() const;
	void clearMasks();
	void setMasked(const Interval<int>& i, bool mask = true);
	void setMasked(int row, bool mask = true);

	virtual QString formula(int row) const;
	virtual QVector< Interval<int> > formulaIntervals() const;
	virtual void setFormula(const Interval<int>& i, const QString& formula);
	virtual void setFormula(int row, const QString& formula);
	virtual void clearFormulas();

	virtual QString textAt(int row) const;
	virtual void setTextAt(int row, const QString& new_value);
	virtual void replaceTexts(int first, const QVector<QString>& new_values);
	virtual QDate dateAt(int row) const;
	virtual void setDateAt(int row, QDate new_value);
	virtual QTime timeAt(int row) const;
	virtual void setTimeAt(int row, QTime new_value);
	virtual QDateTime dateTimeAt(int row) const;
	virtual void setDateTimeAt(int row, const QDateTime& new_value);
	virtual void replaceDateTimes(int first, const QVector<QDateTime>& new_values);
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

	//conditional formatting
	enum class Formatting {Background, Foreground, Icon};

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

signals:
	void plotDesignationAboutToChange(const AbstractColumn* source);
	void plotDesignationChanged(const AbstractColumn* source);
	void modeAboutToChange(const AbstractColumn* source);
	void modeChanged(const AbstractColumn* source);
	void dataAboutToChange(const AbstractColumn* source);
	void dataChanged(const AbstractColumn* source);
	void formatChanged(const AbstractColumn* source);
	void rowsAboutToBeInserted(const AbstractColumn* source, int before, int count);
	void rowsInserted(const AbstractColumn* source, int before, int count);
	void rowsAboutToBeRemoved(const AbstractColumn* source, int first, int count);
	void rowsRemoved(const AbstractColumn* source, int first, int count);
	void maskingAboutToChange(const AbstractColumn* source);
	void maskingChanged(const AbstractColumn* source);
	void aboutToBeDestroyed(const AbstractColumn* source);
	void reset(const AbstractColumn* source); // this signal is emitted when the column is reused for another purpose. The curves must know that and disconnect all connections

protected:
	bool XmlReadMask(XmlStreamReader*);
	void XmlWriteMask(QXmlStreamWriter*) const;

	virtual void handleRowInsertion(int before, int count);
	virtual void handleRowRemoval(int first, int count);

private:
	AbstractColumnPrivate* d;

	friend class AbstractColumnRemoveRowsCmd;
	friend class AbstractColumnInsertRowsCmd;
	friend class AbstractColumnClearMasksCmd;
	friend class AbstractColumnSetMaskedCmd;
};

#endif
