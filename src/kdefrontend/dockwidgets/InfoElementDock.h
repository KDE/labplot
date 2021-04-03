/***************************************************************************
	File                 : InfoElement.cpp
	Project              : LabPlot
	Description          : Dock widget for InfoElemnt
	--------------------------------------------------------------------
	Copyright            : (C) 2020 Martin Marmsoler (martin.marmsoler@gmail.com)
	Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)
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

#ifndef INFOELEMENTDOCK_H
#define INFOELEMENTDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"

class InfoElement;
class LabelWidget;

namespace Ui {
class InfoElementDock;
}

class InfoElementDock : public BaseDock {
	Q_OBJECT

public:
	explicit InfoElementDock(QWidget* parent = nullptr);
	~InfoElementDock();
	void setInfoElements(QList<InfoElement*>, bool sameParent);

public slots:
	void elementCurveRemoved(const QString&);

private slots:
	void gluePointChanged(int index);
	void curveChanged();
	void positionChanged(const QString&);
	void positionDateTimeChanged(const QDateTime&);
	void curveSelectionChanged(int);
	void visibilityChanged(bool);
	void verticalLineStyleChanged(int);
	void verticalLineWidthChanged(double);
	void verticalLineColorChanged(const QColor&);
	void verticalLineOpacityChanged(int);
	void connectionLineStyleChanged(int);
	void connectionLineWidthChanged(double);
	void connectionLineColorChanged(const QColor&);
	void connectionLineOpacityChanged(int);

	// slots triggered in the InfoElement
	void elementDescriptionChanged(const AbstractAspect*);
	void elementPositionChanged(double pos);
	void elementGluePointIndexChanged(const int);
	void elementConnectionLineCurveChanged(const QString&);
	void elementLabelBorderShapeChanged();
	void elementVisibilityChanged(const bool);
	void elementVerticalLinePenChanged(const QPen&);
	void elementVerticalLineOpacityChanged(qreal);
	void elementConnectionLinePenChanged(const QPen&);
	void elementConnectionLineOpacityChanged(qreal);
	void updatePlotRanges() const override;

private:
	Ui::InfoElementDock* ui;
	InfoElement* m_element{nullptr};
	QList<InfoElement*> m_elements;
	bool m_sameParent{false};
	LabelWidget* m_labelWidget{nullptr};
};

#endif // INFOELEMENTDOCK_H
