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

namespace Ui {
class InfoElementDock;
}

class InfoElementDock : public BaseDock {
	Q_OBJECT

public:
	explicit InfoElementDock(QWidget* parent = nullptr);
	~InfoElementDock();
	void setInfoElements(QList<InfoElement*> &list, bool sameParent);
	void initConnections();

public slots:
	void elementCurveRemoved(QString name);

private slots:
	void visibilityChanged(bool state);
	void connectionLineWidthChanged(double);
	void connectionLineColorChanged(const QColor &);
	void xposLineWidthChanged(double);
	void xposLineColorChanged(const QColor &);
	void xposLineVisibilityChanged(bool);
	void connectionLineVisibilityChanged(bool);
	void gluePointChanged(int index);
	void curveChanged();
	void curveSelectionChanged(int state);

	// slots triggered in the InfoElement
	void elementDescriptionChanged(const AbstractAspect*);
	void elementConnectionLineWidthChanged(const double);
	void elementConnectionLineColorChanged(const QColor& );
	void elementXPosLineWidthChanged(const double);
	void elementXposLineColorChanged(const QColor&);
	void elementXPosLineVisibleChanged(const bool);
	void elementConnectionLineVisibleChanged(const bool);
	void elementVisibilityChanged(const bool);
	void elementGluePointIndexChanged(const int);
	void elementConnectionLineCurveChanged(const QString name);
	void elementLabelBorderShapeChanged();

private:
    Ui::InfoElementDock* ui;
	InfoElement* m_element{nullptr};
	QList<InfoElement*> m_elements;
	bool m_sameParent;
};

#endif // INFOELEMENTDOCK_H
