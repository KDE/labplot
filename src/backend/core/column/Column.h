/*
    File                 : Column.h
    Project              : LabPlot
    Description          : Aspect that manages a column
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007-2009 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2013-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2017 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef COLUMN_H
#define COLUMN_H

#include "backend/core/column/ColumnPrivate.h"

class AbstractSimpleFilter;
class CartesianPlot;
class ColumnStringIO;
class QAction;
class QActionGroup;
class XmlStreamReader;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT Column : public AbstractColumn {
#else
class Column : public AbstractColumn {
#endif
	Q_OBJECT

public:
	explicit Column(const QString& name, AbstractColumn::ColumnMode = ColumnMode::Double);
	// template constructor for all supported data types (AbstractColumn::ColumnMode) must be defined in header
	template <typename T>
	Column(const QString& name, QVector<T> data, AbstractColumn::ColumnMode mode = ColumnMode::Double)
		: AbstractColumn(name, AspectType::Column), d(new ColumnPrivate(this, mode, new QVector<T>(data))) {
		init();
	}
	void init();
	~Column() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;

	void updateLocale();

	AbstractColumn::ColumnMode columnMode() const override;
	QString columnModeString() const;
	void setColumnMode(AbstractColumn::ColumnMode) override;
	void setColumnModeFast(AbstractColumn::ColumnMode);

	bool isDraggable() const override;
	QVector<AspectType> dropableOn() const override;

	bool copy(const AbstractColumn*) override;
	bool copy(const AbstractColumn* source, int source_start, int dest_start, int num_rows) override;

	AbstractColumn::PlotDesignation plotDesignation() const override;
	QString plotDesignationString(bool withBrackets = true) const;
	void setPlotDesignation(AbstractColumn::PlotDesignation) override;

	bool isReadOnly() const override;
	void resizeTo(int);
	int rowCount() const override;
	int availableRowCount() const override;
	int width() const;
	void setWidth(const int);
	void clear() override;
	AbstractSimpleFilter* outputFilter() const;
	ColumnStringIO* asStringColumn() const;

	void setFormula(const QString& formula, const QStringList& variableNames,
					const QVector<Column*>& columns, bool autoUpdate);
	QString formula() const;
	const QStringList& formulaVariableNames() const;
	const QVector<Column*>& formulaVariableColumns() const;
	const QStringList& formulaVariableColumnPaths() const;
	void setformulVariableColumnsPath(int index, const QString& path);
	void setformulVariableColumn(int index, Column*);
	bool formulaAutoUpdate() const;

	QString formula(int) const  override;
	QVector< Interval<int> > formulaIntervals() const override;
	void setFormula(const Interval<int>&, const QString&) override;
	void setFormula(int, const QString&) override;
	void clearFormulas() override;

	const AbstractColumn::ColumnStatistics& statistics() const;
	void* data() const;
	bool hasValues() const;
	bool hasValueLabels() const;
	void removeValueLabel(const QString&);
	void clearValueLabels();

	Properties properties() const override;
	void invalidateProperties();

	void setFromColumn(int, AbstractColumn*, int);
	QString textAt(int) const override;
	void setTextAt(int, const QString&) override;
	void replaceTexts(int, const QVector<QString>&) override;
	void addValueLabel(const QString&, const QString&);
	const QMap<QString, QString>& textValueLabels();

	QDate dateAt(int) const override;
	void setDateAt(int, QDate) override;
	QTime timeAt(int) const override;
	void setTimeAt(int, QTime) override;
	QDateTime dateTimeAt(int) const override;
	void setDateTimeAt(int, const QDateTime&) override;
	void replaceDateTimes(int, const QVector<QDateTime>&) override;
	void addValueLabel(const QDateTime&, const QString&);
	const QMap<QDateTime, QString>& dateTimeValueLabels();

	double valueAt(int) const override;
	void setValueAt(int, double) override;
	void replaceValues(int, const QVector<double>&) override;
	void addValueLabel(double, const QString&);
	const QMap<double, QString>& valueLabels();

	int integerAt(int) const override;
	void setIntegerAt(int, int) override;
	void replaceInteger(int, const QVector<int>&) override;
	void addValueLabel(int, const QString&);
	const QMap<int, QString>& intValueLabels();

	qint64 bigIntAt(int) const override;
	void setBigIntAt(int, qint64) override;
	void replaceBigInt(int, const QVector<qint64>&) override;
	void addValueLabel(qint64, const QString&);
	const QMap<qint64, QString>& bigIntValueLabels();

	double maximum(int count = 0) const override;
	double maximum(int startIndex, int endIndex) const override;
	double minimum(int count = 0) const override;
	double minimum(int startIndex, int endIndex) const override;
	static int calculateMaxSteps(unsigned int value);
	static int indexForValue(double x, QVector<double>& column, Properties properties = Properties::No);
	static int indexForValue(const double x, const QVector<QPointF> &column, Properties properties = Properties::No);
	static int indexForValue(double x, QVector<QLineF>& lines, Properties properties = Properties::No);
	int indexForValue(double x) const override;
	bool indicesMinMax(double v1, double v2, int& start, int& end) const override;

	void setChanged();
	void setSuppressDataChangedSignal(const bool);

	void addUsedInPlots(QVector<CartesianPlot*>&);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void finalizeLoad();

public Q_SLOTS:
	void pasteData();
	void updateFormula();

private:
	bool XmlReadInputFilter(XmlStreamReader*);
	bool XmlReadOutputFilter(XmlStreamReader*);
	bool XmlReadFormula(XmlStreamReader*);
	bool XmlReadRow(XmlStreamReader*);

	void handleRowInsertion(int before, int count) override;
	void handleRowRemoval(int first, int count) override;

	void calculateStatistics() const;

	bool m_suppressDataChangedSignal{false};
	QAction* m_copyDataAction{nullptr};
	QAction* m_pasteDataAction{nullptr};
	QActionGroup* m_usedInActionGroup{nullptr};

	ColumnPrivate* d;
	ColumnStringIO* m_string_io;

Q_SIGNALS:
	void requestProjectContextMenu(QMenu*);

private Q_SLOTS:
	void navigateTo(QAction*);
	void handleFormatChange();
	void copyData();

	friend class ColumnPrivate;
	friend class ColumnStringIO;
};


#endif
