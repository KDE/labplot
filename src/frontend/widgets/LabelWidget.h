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

#include "backend/worksheet/TextLabel.h"
#include "frontend/dockwidgets/WorksheetElementDock.h"
#include "ui_labelwidget.h"
#include <KConfigGroup>

#ifdef HAVE_KF_SYNTAX_HIGHLIGHTING
#include <KSyntaxHighlighting/repository.h>
namespace KSyntaxHighlighting {
class SyntaxHighlighter;
}
#endif

class Label;
class Axis;
class QMenu;
class KMessageWidget;

class TextLabelTest;

class LabelWidget : public WorksheetElementDock {
	Q_OBJECT

public:
	explicit LabelWidget(QWidget*);

	void setLabels(QList<TextLabel*>);
	void setAxes(QList<Axis*>);
	void updateUnits() override;
	void updateLocale() override;

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
	QMenu* m_dateTimeMenu;
	bool m_initializing{false};
	bool m_teXEnabled{false};
#ifdef HAVE_KF_SYNTAX_HIGHLIGHTING
	KSyntaxHighlighting::SyntaxHighlighter* m_highlighter;
	KSyntaxHighlighting::Repository m_repository;
#endif
	KMessageWidget* m_messageWidget;

	QVector<QMetaObject::Connection> m_connections;

	void initConnections();

Q_SIGNALS:
	void dataChanged(bool);
	/*!
	 * \brief fontColorChangedSignal
	 * Used to send out that font color has changed. So in the case of the axis
	 * the axisdock can update the axis color widget
	 */
	void labelFontColorChangedSignal(const QColor&);

private Q_SLOTS:
	// SLOTs for changes triggered in LabelWidget
	void textChanged();
	void charFormatChanged(const QTextCharFormat&);
	void modeChanged(int);
	void updateMode(TextLabel::Mode mode);
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
	void alignLeft();
	void alignCenter();
	void alignRight();
	void alignJustify();
	void dateTimeMenu();
	void insertDateTime(QAction*);

	void offsetXChanged(double);
	void offsetYChanged(double);

	void borderShapeChanged(int);
	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);
	void borderOpacityChanged(int);

	void showPlaceholderTextChanged(bool checked);

	// SLOTs for changes triggered in TextLabel
	void labelTextWrapperChanged(const TextLabel::TextWrapper&);
	void labelTeXImageUpdated(const TeXRenderer::Result&);
	void labelTeXFontChanged(const QFont&);
	void labelFontColorChanged(const QColor&);
	void labelBackgroundColorChanged(const QColor&);
	void labelOffsetXChanged(qreal);
	void labelOffsetYChanged(qreal);

	void labelBorderShapeChanged(TextLabel::BorderShape);
	void labelBorderPenChanged(const QPen&);
	void labelBorderOpacityChanged(float);

	void labelCartesianPlotParent(bool on);
	void labelModeChanged(TextLabel::Mode);

	friend class AxisDock; // fontColorChanged() and labelFontColorChanged() are private methods of LabelWidget
	friend class TextLabelTest;
	friend class AxisTest;
	friend class AxisTest2;
	friend class WorksheetElementTest;
};

#endif // LABELWIDGET_H
