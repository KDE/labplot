/***************************************************************************
    File                 : LabelWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2014 Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
    Description          : label settings widget

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
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
	void teXUsedChanged(bool);
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
