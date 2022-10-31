/*
	File                 : LineWidget.h
	Project              : LabPlot
	Description          : line settings widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LINEWIDGET_H
#define LINEWIDGET_H

#include "backend/worksheet/Line.h"
#include "ui_linewidget.h"

class KConfigGroup;

class LineWidget : public QWidget {
	Q_OBJECT

public:
	explicit LineWidget(QWidget*);

	void setLines(const QList<Line*>&);
	void setPrefix(const QString&);
	void adjustLayout();
	void setEnabled(bool);
	void updateLocale();

	void load();
	void loadConfig(const KConfigGroup&);
	void saveConfig(KConfigGroup&) const;

private:
	Ui::LineWidget ui;
	Line* m_line{nullptr};
	QList<Line*> m_lines;
	bool m_initializing{false};
	QString m_prefix{QLatin1String("Line")};

private Q_SLOTS:
	// SLOTs for changes triggered in LineWidget
	void typeChanged(int);
	void capSizeChanged(double) const;

	void styleChanged(int) const;
	void colorChanged(const QColor&);
	void widthChanged(double);
	void opacityChanged(int) const;

	// SLOTs for changes triggered in Line
	void histogramLineTypeChanged(Histogram::LineType);
	void errorBarsTypeChanged(XYCurve::ErrorBarsType);
	void errorBarsCapSizeChanged(double);
	void dropLineTypeChanged(XYCurve::DropLineType);

	void linePenChanged(QPen&);
	void lineOpacityChanged(double);
};

#endif // LINEWIDGET_H
