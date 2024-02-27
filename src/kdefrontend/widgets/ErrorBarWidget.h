/*
	File                 : ErrorBarWidget.h
	Project              : LabPlot
	Description          : error bar widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ERRORBARWIDGET_H
#define ERRORBARWIDGET_H

#include "backend/worksheet/plots/cartesian/ErrorBar.h"
#include "ui_errorbarwidget.h"

class AspectTreeModel;
class LineWidget;
class TreeViewComboBox;
class QShowEvent;
class KConfigGroup;

class ErrorBarWidget : public QWidget {
	Q_OBJECT

public:
	explicit ErrorBarWidget(QWidget*, bool poissonAvailable = false);

	void setErrorBars(const QList<ErrorBar*>&);
	void setModel(AspectTreeModel*);
	void updateLocale();

	void load();
	void loadConfig(const KConfigGroup&);
	void saveConfig(KConfigGroup&) const;

private:
	Ui::ErrorBarWidget ui;
	LineWidget* lineWidget{nullptr};
	ErrorBar* m_errorBar{nullptr};
	QList<ErrorBar*> m_errorBars;
	bool m_initializing{false};
	TreeViewComboBox* cbXPlusColumn{nullptr};
	TreeViewComboBox* cbXMinusColumn{nullptr};
	TreeViewComboBox* cbYPlusColumn{nullptr};
	TreeViewComboBox* cbYMinusColumn{nullptr};

	void showEvent(QShowEvent*) override;
	void adjustLayout();
	void updateStylingWidgets();

private Q_SLOTS:
	// SLOTs for changes triggered in ErrorBarWidget
	void xErrorTypeChanged(int);
	void xPlusColumnChanged(const QModelIndex&);
	void xMinusColumnChanged(const QModelIndex&);

	void yErrorTypeChanged(int);
	void yPlusColumnChanged(const QModelIndex&);
	void yMinusColumnChanged(const QModelIndex&);

	void typeChanged(int);
	void capSizeChanged(double);

	// SLOTs for changes triggered in ErrorBarStyle
	void errorBarXErrorTypeChanged(ErrorBar::ErrorType);
	void errorBarXPlusColumnChanged(const AbstractColumn*);
	void errorBarXMinusColumnChanged(const AbstractColumn*);

	void errorBarYErrorTypeChanged(ErrorBar::ErrorType);
	void errorBarYPlusColumnChanged(const AbstractColumn*);
	void errorBarYMinusColumnChanged(const AbstractColumn*);

	void errorBarTypeChanged(ErrorBar::Type);
	void errorBarCapSizeChanged(double);
};

#endif // ERRORBARWIDGET_H
