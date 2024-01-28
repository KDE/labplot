/*
	File                 : ErrorBarStyleWidget.h
	Project              : LabPlot
	Description          : error bar style widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ERRORBARSTYLEWIDGET_H
#define ERRORBARSTYLEWIDGET_H

#include "backend/worksheet/plots/cartesian/ErrorBarStyle.h"
#include "ui_errorbarstylewidget.h"

class LineWidget;
class QShowEvent;
class KConfigGroup;

class ErrorBarStyleWidget : public QWidget {
	Q_OBJECT

public:
	explicit ErrorBarStyleWidget(QWidget*);

	void setErrorBarStyles(const QList<ErrorBarStyle*>&);
	void setEnabled(bool);
	void updateLocale();

	void load();
	void loadConfig(const KConfigGroup&);
	void saveConfig(KConfigGroup&) const;

private:
	Ui::ErrorBarStyleWidget ui;
	LineWidget* lineWidget{nullptr};
	ErrorBarStyle* m_style{nullptr};
	QList<ErrorBarStyle*> m_styles;
	bool m_initializing{false};
	QString m_prefix;

	void showEvent(QShowEvent*) override;
	void adjustLayout();

private Q_SLOTS:
	// SLOTs for changes triggered in ErrorBarStyleWidget
	void typeChanged(int);
	void capSizeChanged(double);

	// SLOTs for changes triggered in ErrorBarStyle
	void errorBarStyleTypeChanged(ErrorBarStyle::Type);
	void errorBarStyleCapSizeChanged(double);

Q_SIGNALS:
	void colorChanged(const QColor&);
};

#endif // ERRORBARSTYLE_H
