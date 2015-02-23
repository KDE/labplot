/***************************************************************************
    File                 : LabelWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2014 Alexander Semke (alexander.semke@web.de)
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
#include <KConfigGroup>

class Label;
class Axis;
class QMenu;

class LabelWidget: public QWidget{
	Q_OBJECT

public:
	explicit LabelWidget(QWidget *);

	void setLabels(QList<TextLabel*>);
	void setAxes(QList<Axis*>);

	void load();
	void loadConfig(KConfigGroup&);
	void saveConfig(KConfigGroup&);

	void setNoGeometryMode(const bool);
	void setFixedLabelMode(const bool);

private:
	Ui::LabelWidget ui;
	TextLabel *m_label;
	QList<TextLabel*> m_labelsList;
	QList<Axis*> m_axesList;
	bool m_initializing;
    QMenu* m_dateTimeMenu;

	void initConnections();

signals:
	void dataChanged(bool);

private slots:
	//SLOTs for changes triggered in LabelWidget
	void textChanged();
	void charFormatChanged(const QTextCharFormat& format);
	void teXUsedChanged(bool);
	void fontColorChanged(const QColor&);
	void fontBoldChanged(bool);
	void fontItalicChanged(bool);
	void fontUnderlineChanged(bool);
	void fontStrikeOutChanged(bool);
	void fontSuperScriptChanged(bool);
	void fontSubScriptChanged(bool);
	void charMenu();
	void insertChar(QChar);
	void fontChanged(const QFont&);
	void fontSizeChanged(int);
	void dateTimeMenu();
	void insertDateTime(QAction*);

	void positionXChanged(int);
	void positionYChanged(int);
	void customPositionXChanged(double);
	void customPositionYChanged(double);
	void horizontalAlignmentChanged(int);
	void verticalAlignmentChanged(int);
	void rotationChanged(int);
	void offsetChanged(double);

	void visibilityChanged(bool);

	//SLOTs for changes triggered in TextLabel
	void labelTextWrapperChanged(const TextLabel::TextWrapper&);
	void labelTeXFontSizeChanged(const int);
	void labelTeXFontColorChanged(const QColor);
	void labelPositionChanged(const TextLabel::PositionWrapper&);
	void labelHorizontalAlignmentChanged(TextLabel::HorizontalAlignment);
	void labelVerticalAlignmentChanged(TextLabel::VerticalAlignment);
	void labelOffsetChanged(float);
	void labelRotationAngleChanged(float);
	void labelVisibleChanged(bool);
};

#endif //LABELWIDGET_H
