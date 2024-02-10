/*
	File                 : InfoElement.cpp
	Project              : LabPlot
	Description          : Dock widget for InfoElemnt
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2020-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INFOELEMENTDOCK_H
#define INFOELEMENTDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"

class InfoElement;
class LineWidget;
class LabelWidget;

namespace Ui {
class InfoElementDock;
}

class InfoElementDock : public BaseDock {
	Q_OBJECT

public:
	explicit InfoElementDock(QWidget* parent = nullptr);
	~InfoElementDock();
	void setInfoElements(QList<InfoElement*>);

public Q_SLOTS:
	void elementCurveRemoved(const QString&);

private Q_SLOTS:
	void gluePointChanged(int index);
	void curveChanged();
	void positionChanged(double);
	void positionDateTimeChanged(qint64);
	void curveSelectionChanged(bool);

	// slots triggered in the InfoElement
	void elementPositionChanged(double pos);
	void elementGluePointIndexChanged(const int);
	void elementConnectionLineCurveChanged(const QString&);
	void elementLabelBorderShapeChanged();

private:
	Ui::InfoElementDock* ui;
	InfoElement* m_element{nullptr};
	QList<InfoElement*> m_elements;
	bool m_sameParent{false};
	LabelWidget* m_labelWidget{nullptr};
	LineWidget* m_verticalLineWidget{nullptr};
	LineWidget* m_connectionLineWidget{nullptr};
};

#endif // INFOELEMENTDOCK_H
