/*
	File                 : BoxPlotDock.h
	Project              : LabPlot
	Description          : Dock widget for the box plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BOXPLOTDOCK_H
#define BOXPLOTDOCK_H

#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_boxplotdock.h"

class AbstractAspect;
class AspectTreeModel;
class BoxPlot;
class SymbolWidget;
class TreeViewComboBox;
class KConfig;

class BoxPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit BoxPlotDock(QWidget*);
	void setBoxPlots(QList<BoxPlot*>);
	void updateLocale() override;

private:
	Ui::BoxPlotDock ui;
	QList<BoxPlot*> m_boxPlots;
	BoxPlot* m_boxPlot{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	SymbolWidget* symbolWidget{nullptr};

	QGridLayout* m_gridLayout;
	QPushButton* m_buttonNew;
	QVector<TreeViewComboBox*> m_dataComboBoxes;
	QVector<QPushButton*> m_removeButtons;

	void setModel();
	void loadConfig(KConfig&);
	void setDataColumns() const;
	void loadDataColumns();
	void updateSymbolWidgets();

private Q_SLOTS:
	// SLOTs for changes triggered in BoxPlotDock

	//"General"-tab
	void addDataColumn();
	void removeDataColumn();
	void dataColumnChanged(const QModelIndex&) const;
	void orderingChanged(int) const;
	void orientationChanged(int) const;
	void variableWidthChanged(bool) const;
	void notchesEnabledChanged(bool) const;
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

	// median line
	void medianLineStyleChanged(int) const;
	void medianLineColorChanged(const QColor&);
	void medianLineWidthChanged(double) const;
	void medianLineOpacityChanged(int) const;

	// symbols
	void symbolCategoryChanged();
	void jitteringEnabledChanged(bool) const;

	// whiskers
	void whiskersTypeChanged(int) const;
	void whiskersRangeParameterChanged(const QString&) const;
	void whiskersStyleChanged(int) const;
	void whiskersColorChanged(const QColor&);
	void whiskersWidthChanged(double) const;
	void whiskersOpacityChanged(int) const;
	void whiskersCapSizeChanged(double) const;
	void whiskersCapStyleChanged(int) const;
	void whiskersCapColorChanged(const QColor&);
	void whiskersCapWidthChanged(double) const;
	void whiskersCapOpacityChanged(int) const;

	// SLOTs for changes triggered in BoxPlot
	// general
	void plotDescriptionChanged(const AbstractAspect*);
	void plotDataColumnsChanged(const QVector<const AbstractColumn*>&);
	void plotOrderingChanged(BoxPlot::Ordering);
	void plotOrientationChanged(BoxPlot::Orientation);
	void plotVariableWidthChanged(bool);
	void plotWidthFactorChanged(double);
	void plotNotchesEnabledChanged(bool);
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

	// median line
	void plotMedianLinePenChanged(QPen&);
	void plotMedianLineOpacityChanged(float);

	// symbols
	void plotJitteringEnabledChanged(bool);

	// whiskers
	void plotWhiskersTypeChanged(BoxPlot::WhiskersType);
	void plotWhiskersRangeParameterChanged(double);
	void plotWhiskersPenChanged(QPen&);
	void plotWhiskersOpacityChanged(float);
	void plotWhiskersCapSizeChanged(double);
	void plotWhiskersCapPenChanged(QPen&);
	void plotWhiskersCapOpacityChanged(float);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
