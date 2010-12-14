/***************************************************************************
    File                 : LineSymbolCurveDock.h
    Project            : LabPlot
    --------------------------------------------------------------------
    Copyright         : (C) 2010 Alexander Semke (alexander.semke*web.de)
							(replace * with @ in the email addresses)
    Description      : widget for curve properties
                           
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

#ifndef LINESYMBOLCURVEDOCK_H
#define LINESYMBOLCURVEDOCK_H

#include <QList>
#include "ui_linesymbolcurvedock.h"

class QTextEdit;
class QCheckBox;
class LineSymbolCurve;
class TreeViewComboBox;
class CurveSymbolFactory;
class AspectTreeModel;

class LineSymbolCurveDock: public QWidget{
	Q_OBJECT
	
public:
	LineSymbolCurveDock(QWidget *parent);
	void setModel(AspectTreeModel* model);
	void setCurves(QList<LineSymbolCurve*>);
	
private:
	Ui::LineSymbolCurveDock ui;
	QList<LineSymbolCurve*> m_curvesList;
	AspectTreeModel* m_aspectTreeModel;
	bool m_initializing;
	
	QGridLayout *gridLayout;
    QLabel *lName;
    QLineEdit *leName;
    QLabel *lComment;
    QTextEdit *teComment;
	QCheckBox* chkVisible;
    QSpacerItem *verticalSpacer;
	QLabel* lXColumn;
	QLabel* lYColumn;
	TreeViewComboBox* cbXColumn;
	TreeViewComboBox* cbYColumn;
	TreeViewComboBox* cbValuesColumn;
	
	CurveSymbolFactory *symbolFactory;
	
	void fillSymbolStyles();
	void updateBrushStyles(QComboBox*, const QColor&);
	void updatePenStyles(QComboBox*, const QColor&);
	
private slots:
	void init();
	void retranslateUi();
  
	void nameChanged();
	void commentChanged();
	void xColumnChanged(int);
	void yColumnChanged(int);
	void visibilityChanged(int);
	
	//Line-Tab
	void lineTypeChanged(int);
	void lineInterpolationPointsCountChanged(int);
  	void lineStyleChanged(int);
	void lineColorChanged(const QColor&);
	void lineWidthChanged(int);
	void lineOpacityChanged(int);
	
	void dropLineTypeChanged(int);
  	void dropLineStyleChanged(int);
	void dropLineColorChanged(const QColor&);
	void dropLineWidthChanged(int);
	void dropLineOpacityChanged(int);
	
	//Symbol-tab
  	void symbolStyleChanged(int);
	void symbolSizeChanged(int);
	void symbolRotationChanged(int);
	void symbolOpacityChanged(int);
	void symbolFillingStyleChanged(int);
	void symbolFillingColorChanged(const QColor&);
	void symbolBorderStyleChanged(int);
	void symbolBorderColorChanged(const QColor&);
	void symbolBorderWidthChanged(int);
	
	//Values-Tab
	void valuesTypeChanged(int);
	void valuesPositionChanged(int);
	void valuesDistanceChanged(int);
	void valuesRotationChanged(int);
	void valuesOpacityChanged(int);
	void valuesPrefixChanged();
	void valuesSuffixChanged();
	void valuesFontChanged(const QFont&);
	void valuesFontColorChanged(const QColor&);
};

#endif
