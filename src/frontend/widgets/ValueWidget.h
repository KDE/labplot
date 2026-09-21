/*
	File                 : ValueWidget.h
	Project              : LabPlot
	Description          : value settings widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VALUEWIDGET_H
#define VALUEWIDGET_H

#include "backend/worksheet/plots/cartesian/Value.h"
#include "ui_valuewidget.h"

#include <KConfigGroup>

class AspectTreeModel;
class TreeViewComboBox;

class ValueWidget : public QWidget {
	Q_OBJECT

public:
	explicit ValueWidget(QWidget*, bool xy = false);
	~ValueWidget();

	void setValues(const QList<Value*>&);
	void setXColumn(const AbstractColumn*);
	void setYColumn(const AbstractColumn*);
	void updateLocale();

	void load();
	void loadConfig(const KConfigGroup&);
	void saveConfig(KConfigGroup&) const;

private:
	Ui::ValueWidget ui;
	Value* m_value{nullptr};
	QList<Value*> m_values;
	bool m_initializing{false};
	TreeViewComboBox* cbColumn{nullptr};
	AspectTreeModel* m_aspectModel{nullptr};
	const AbstractColumn* m_xColumn{nullptr};
	const AbstractColumn* m_yColumn{nullptr};

	void updateWidgets();

Q_SIGNALS:
	void dataChanged(bool);

private Q_SLOTS:
	// SLOTs for changes triggered in ValueWidget
	void typeChanged(int);
	void columnChanged(const QModelIndex&);
	void positionChanged(int);
	void distanceChanged(double);
	void rotationChanged(int);
	void opacityChanged(int);
	void numericFormatChanged(int);
	void precisionChanged(int);
	void dateTimeFormatChanged(const QString&);
	void prefixChanged();
	void suffixChanged();
	void fontChanged(const QFont&);
	void colorChanged(const QColor&);

	// SLOTs for changes triggered in Value
	void valueTypeChanged(Value::Type);
	void valueColumnChanged(const AbstractColumn*);
	void valuePositionChanged(Value::Position);
	void valueDistanceChanged(qreal);
	void valueOpacityChanged(qreal);
	void valueRotationAngleChanged(qreal);
	void valueNumericFormatChanged(char);
	void valuePrecisionChanged(int);
	void valueDateTimeFormatChanged(const QString&);
	void valuePrefixChanged(const QString&);
	void valueSuffixChanged(const QString&);
	void valueFontChanged(QFont);
	void valueColorChanged(QColor);
};

#endif // VALUEWIDGET_H
