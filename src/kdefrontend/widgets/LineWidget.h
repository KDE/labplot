/*
	File                 : LineWidget.h
	Project              : LabPlot
	Description          : line settings widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LINEWIDGET_H
#define LINEWIDGET_H

#include "backend/worksheet/Line.h"
#include "ui_linewidget.h"

class QShowEvent;
class KConfigGroup;

class LineWidget : public QWidget {
	Q_OBJECT

public:
	explicit LineWidget(QWidget*);

	void setLines(const QList<Line*>&);
	void setEnabled(bool);
	void updateLocale();

	void load();
	void loadConfig(const KConfigGroup&);
	void saveConfig(KConfigGroup&) const;

	void setColor(const QColor&);

private:
	Ui::LineWidget ui;
	Line* m_line{nullptr};
	QList<Line*> m_lines;
	bool m_initializing{false};
	QString m_prefix;

	void showEvent(QShowEvent*) override;
	void adjustLayout();

private Q_SLOTS:
	// SLOTs for changes triggered in LineWidget
	void typeChanged(int);

	void styleChanged(int);
	void colorChangedSlot(const QColor&);
	void widthChanged(double);
	void opacityChanged(int);

	// SLOTs for changes triggered in Line
	void histogramLineTypeChanged(Histogram::LineType);
	void dropLineTypeChanged(XYCurve::DropLineType);

	void lineStyleChanged(Qt::PenStyle);
	void lineColorChanged(const QColor&);
	void lineWidthChanged(double);
	void lineOpacityChanged(double);

Q_SIGNALS:
	void colorChanged(const QColor&);

	friend class AxisTest;
	friend class AxisTest2;
};

#endif // LINEWIDGET_H
