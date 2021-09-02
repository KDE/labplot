/*
    File                 : LabelWidget.h
    Project              : LabPlot
    Description          : label settings widget
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2012-2014 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LABELWIDGET_H
#define LABELWIDGET_H

#include "ui_labelwidget.h"
#include "backend/worksheet/TextLabel.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include <KConfigGroup>

#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
#include <repository.h>
namespace KSyntaxHighlighting {
	class SyntaxHighlighter;
}
#endif

class Label;
class Axis;
class QMenu;

class LabelWidget : public QWidget {
	Q_OBJECT

public:
	explicit LabelWidget(QWidget*);

	void setLabels(QList<TextLabel*>);
	void setAxes(QList<Axis*>);
	void updateUnits();
	void updateLocale();

	void load();
	void loadConfig(KConfigGroup&);
	void saveConfig(KConfigGroup&);

	void setGeometryAvailable(bool);
	void setFixedLabelMode(bool);
	void setBorderAvailable(bool);

private:
	Ui::LabelWidget ui;
	TextLabel* m_label{nullptr};
	QList<TextLabel*> m_labelsList;
	QList<Axis*> m_axesList;
	bool m_initializing{false};
	QMenu* m_dateTimeMenu;
	bool m_teXEnabled{false};
	BaseDock::Units m_units{BaseDock::Units::Metric};
	Worksheet::Unit m_worksheetUnit{Worksheet::Unit::Centimeter};
#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
	KSyntaxHighlighting::SyntaxHighlighter* m_highlighter;
	KSyntaxHighlighting::Repository m_repository;
#endif

	void initConnections() const;

signals:
	void dataChanged(bool);

private slots:
	//SLOTs for changes triggered in LabelWidget
	void textChanged();
	void charFormatChanged(const QTextCharFormat&);
	void modeChanged(int);
	void fontColorChanged(const QColor&);
	void updateBackground() const;
	void backgroundColorChanged(const QColor&);
	void fontBoldChanged(bool);
	void fontItalicChanged(bool);
	void fontUnderlineChanged(bool);
	void fontStrikeOutChanged(bool);
	void fontSuperScriptChanged(bool);
	void fontSubScriptChanged(bool);
	void charMenu();
	void insertChar(QChar);
	void fontChanged(const QFont&);
	void teXFontChanged(const QFont&);
	void fontSizeChanged(int);
	void dateTimeMenu();
	void insertDateTime(QAction*);

	void positionXChanged(int);
	void positionYChanged(int);
	void customPositionXChanged(double);
	void customPositionYChanged(double);
	void horizontalAlignmentChanged(int);
	void verticalAlignmentChanged(int);

	void positionXLogicalChanged(const QString&);
	void positionXLogicalDateTimeChanged(const QDateTime&);
	void positionYLogicalChanged(const QString&);

	void rotationChanged(int);
	void offsetXChanged(double);
	void offsetYChanged(double);

	void borderShapeChanged(int);
	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);
	void borderOpacityChanged(int);

	void visibilityChanged(bool);
	void bindingChanged(bool checked);
	void showPlaceholderTextChanged(bool checked);

	//SLOTs for changes triggered in TextLabel
	void labelTextWrapperChanged(const TextLabel::TextWrapper&);
	void labelTeXImageUpdated(bool);
	void labelTeXFontChanged(const QFont&);
	void labelFontColorChanged(const QColor);
	void labelBackgroundColorChanged(const QColor);
	void labelPositionChanged(const TextLabel::PositionWrapper&);
	void labelHorizontalAlignmentChanged(const TextLabel::HorizontalAlignment);
	void labelVerticalAlignmentChanged(const TextLabel::VerticalAlignment);
	void labelPositionLogicalChanged(QPointF);
	void labelOffsetxChanged(qreal);
	void labelOffsetyChanged(qreal);
	void labelRotationAngleChanged(qreal);

	void labelBorderShapeChanged(TextLabel::BorderShape);
	void labelBorderPenChanged(const QPen&);
	void labelBorderOpacityChanged(float);

	void labelVisibleChanged(bool);
	void labelCartesianPlotParent(bool on);
};

#endif //LABELWIDGET_H
