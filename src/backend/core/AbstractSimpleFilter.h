/*
    File                 : AbstractSimpleFilter.h
    Project              : AbstractColumn
    Description          : Simplified filter interface for filters with
    only one output port.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke@gmx.de>
    SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2017-2020 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef ABSTRACTSIMPLEFILTER_H
#define ABSTRACTSIMPLEFILTER_H

#include "AbstractFilter.h"
#include "AbstractColumn.h"
#include "backend/lib/IntervalAttribute.h"

#include <QLocale>

class SimpleFilterColumn;

class AbstractSimpleFilter : public AbstractFilter {
	Q_OBJECT

public:
	AbstractSimpleFilter();
	int inputCount() const override;
	int outputCount() const override;
	AbstractColumn* output(int port) override;
	const AbstractColumn * output(int port) const override;
	virtual AbstractColumn::PlotDesignation plotDesignation() const;
	virtual AbstractColumn::ColumnMode columnMode() const;
	virtual QString textAt(int row) const;
	virtual QDate dateAt(int row) const;
	virtual QTime timeAt(int row) const;
	virtual QDateTime dateTimeAt(int row) const;
	virtual double valueAt(int row) const;
	virtual int integerAt(int row) const;
	virtual qint64 bigIntAt(int row) const;

	void setNumberLocale(const QLocale& locale) { m_numberLocale = locale; m_useDefaultLocale = false; }
	void setNumberLocaleToDefault() { m_useDefaultLocale = true; }

	virtual int rowCount() const;
	virtual int availableRowCount() const;
	virtual QList<Interval<int>> dependentRows(const Interval<int>& inputRange) const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	virtual void writeExtraAttributes(QXmlStreamWriter*) const;

Q_SIGNALS:
	void formatChanged();
	void digitsChanged();

protected:
	void inputPlotDesignationAboutToChange(const AbstractColumn*) override;
	void inputPlotDesignationChanged(const AbstractColumn*) override;
	void inputModeAboutToChange(const AbstractColumn*) override;
	void inputModeChanged(const AbstractColumn*) override;
	void inputDataAboutToChange(const AbstractColumn*) override;
	void inputDataChanged(const AbstractColumn*) override;

	void inputRowsAboutToBeInserted(const AbstractColumn * source, int before, int count) override;
	void inputRowsInserted(const AbstractColumn * source, int before, int count) override;
	void inputRowsAboutToBeRemoved(const AbstractColumn * source, int first, int count) override;
	void inputRowsRemoved(const AbstractColumn * source, int first, int count) override;

	SimpleFilterColumn* m_output_column;
	QLocale m_numberLocale;
	bool m_useDefaultLocale{true};
};

class SimpleFilterColumn : public AbstractColumn {
	Q_OBJECT

public:
	explicit SimpleFilterColumn(AbstractSimpleFilter* owner) : AbstractColumn(owner->name(), AspectType::SimpleFilterColumn), m_owner(owner) {}

	AbstractColumn::ColumnMode columnMode() const override;
	int rowCount() const override { return m_owner->rowCount(); }
	int availableRowCount() const override { return m_owner->availableRowCount(); }
	AbstractColumn::PlotDesignation plotDesignation() const override { return m_owner->plotDesignation(); }
	QString textAt(int row) const override;
	QDate dateAt(int row) const override;
	QTime timeAt(int row) const override;
	QDateTime dateTimeAt(int row) const override;
	double valueAt(int row) const override;
	int integerAt(int row) const override;
	qint64 bigIntAt(int row) const override;
	void save(QXmlStreamWriter*) const override {};
	bool load(XmlStreamReader*, bool preview) override {Q_UNUSED(preview); return true;};
private:
	AbstractSimpleFilter* m_owner;

	friend class AbstractSimpleFilter;
};

#endif // ifndef ABSTRACTSIMPLEFILTER_H

