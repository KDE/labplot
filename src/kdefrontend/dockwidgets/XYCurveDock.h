/***************************************************************************
    File                 : XYCurveDock.h
    Project            : LabPlot
    --------------------------------------------------------------------
    Copyright         : (C) 2010-2011 Alexander Semke (alexander.semke*web.de)
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

#ifndef XYCURVEDOCK_H
#define XYCURVEDOCK_H

#include <QList>

#include "core/globals.h"
#include "ui_xycurvedock.h"

class QTextEdit;
class QCheckBox;
class XYCurve;
class TreeViewComboBox;
class CurveSymbolFactory;
class AspectTreeModel;
class Column;

class XYCurveDock: public QWidget{
	Q_OBJECT
	
public:
	XYCurveDock(QWidget *parent);
	void setModel(AspectTreeModel* model);
	void setCurves(QList<XYCurve*>);
	
private:
	Ui::XYCurveDock ui;
	QList<XYCurve*> m_curvesList;
	AspectTreeModel* m_aspectTreeModel;
	bool m_initializing;
	QStringList dateStrings;
	QStringList timeStrings;
	  
	QGridLayout *gridLayout;
    QLabel *lName;
    QLineEdit *leName;
    QLabel *lComment;
    QLineEdit* leComment;
	QCheckBox* chkVisible;
    QSpacerItem *verticalSpacer;
	QLabel* lXColumn;
	QLabel* lYColumn;
	TreeViewComboBox* cbXColumn;
	TreeViewComboBox* cbYColumn;
	TreeViewComboBox* cbValuesColumn;
	
	CurveSymbolFactory *symbolFactory;

	void fillSymbolStyles();
	void updateValuesFormatWidgets(const SciDAVis::ColumnMode);
	void showValuesColumnFormat(const Column*);
	
	void load(const KConfig&);
	void save(const KConfig&);
	
private slots:
	void init();
	void retranslateUi();
  
	void nameChanged();
	void commentChanged();
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);
	void visibilityChanged(int);
	
	//Line-Tab
	void lineTypeChanged(int);
	void lineInterpolationPointsCountChanged(int);
  	void lineStyleChanged(int);
	void lineColorChanged(const QColor&);
	void lineWidthChanged(double);
	void lineOpacityChanged(int);
	
	void dropLineTypeChanged(int);
  	void dropLineStyleChanged(int);
	void dropLineColorChanged(const QColor&);
	void dropLineWidthChanged(double);
	void dropLineOpacityChanged(int);
	
	//Symbol-tab
  	void symbolStyleChanged(int);
	void symbolSizeChanged(double);
	void symbolRotationChanged(int);
	void symbolOpacityChanged(int);
	void symbolFillingStyleChanged(int);
	void symbolFillingColorChanged(const QColor&);
	void symbolBorderStyleChanged(int);
	void symbolBorderColorChanged(const QColor&);
	void symbolBorderWidthChanged(double);
	
	//Values-Tab
	void valuesTypeChanged(int);
	void valuesColumnChanged(int);
	void valuesPositionChanged(int);
	void valuesDistanceChanged(double);
	void valuesRotationChanged(int);
	void valuesOpacityChanged(int);
	void valuesPrefixChanged();
	void valuesSuffixChanged();
	void valuesFontChanged(const QFont&);
	void valuesFontColorChanged(const QColor&);

	//Settings
	void loadSettings();
	void saveSettings();
	void saveDefaults();
};

#endif
