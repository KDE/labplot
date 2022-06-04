/*
	File                 : BarPlotDock.h
	Project              : LabPlot
	Description          : Dock widget for the bar plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BARPLOTDOCK_H
#define BARPLOTDOCK_H

#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_barplotdock.h"

class AbstractAspect;
class AspectTreeModel;
class BarPlot;
class TreeViewComboBox;
class KConfig;

class BarPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit BarPlotDock(QWidget*);
	void setBarPlots(QList<BarPlot*>);
	void updateLocale() override;

private:
	Ui::BarPlotDock ui;
	QList<BarPlot*> m_barPlots;
	BarPlot* m_barPlot{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	TreeViewComboBox* cbXColumn{nullptr};

	QGridLayout* m_gridLayout;
	QPushButton* m_buttonNew;
	QVector<TreeViewComboBox*> m_dataComboBoxes;
	QVector<QPushButton*> m_removeButtons;

	void setModel();
	void loadConfig(KConfig&);
	void setDataColumns() const;
	void loadDataColumns();

private Q_SLOTS:
	// SLOTs for changes triggered in BarPlotDock

	//"General"-tab
	void xColumnChanged(const QModelIndex&);
	void removeXColumn();
	void addDataColumn();
	void removeDataColumn();
	void dataColumnChanged(const QModelIndex&) const;
	void typeChanged(int) const;
	void orientationChanged(int) const;
	void visibilityChanged(bool) const;

	//"Box"-tab
	void widthFactorChanged(int) const;

	// box filling
	void fillingEnabledChanged(bool) const;
	void fillingTypeChanged(int) const;
	void fillingColorStyleChanged(int) const;
	void fillingImageStyleChanged(int) const;
	void fillingBrushStyleChanged(int) const;
	void fillingFirstColorChanged(const QColor&);
	void fillingSecondColorChanged(const QColor&) const;
	void selectFile();
	void fileNameChanged() const;
	void fillingOpacityChanged(int) const;

	// box border
	void borderStyleChanged(int) const;
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double) const;
	void borderOpacityChanged(int) const;

	// SLOTs for changes triggered in BarPlot
	// general
	void updatePlotRanges() override;
	void plotXColumnChanged(const AbstractColumn*);
	void plotDataColumnsChanged(const QVector<const AbstractColumn*>&);
	void plotTypeChanged(BarPlot::Type);
	void plotOrientationChanged(BarPlot::Orientation);
	void plotWidthFactorChanged(double);
	void plotVisibilityChanged(bool);

	// box filling
	void plotFillingEnabledChanged(bool);
	void plotFillingTypeChanged(WorksheetElement::BackgroundType);
	void plotFillingColorStyleChanged(WorksheetElement::BackgroundColorStyle);
	void plotFillingImageStyleChanged(WorksheetElement::BackgroundImageStyle);
	void plotFillingBrushStyleChanged(Qt::BrushStyle);
	void plotFillingFirstColorChanged(QColor&);
	void plotFillingSecondColorChanged(QColor&);
	void plotFillingFileNameChanged(QString&);
	void plotFillingOpacityChanged(double);

	// box border
	void plotBorderPenChanged(QPen&);
	void plotBorderOpacityChanged(float);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
