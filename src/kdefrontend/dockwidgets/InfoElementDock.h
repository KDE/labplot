/*
	File                 : InfoElement.cpp
	Project              : LabPlot
	Description          : Dock widget for InfoElemnt
	--------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Martin Marmsoler <martin.marmsoler@gmail.com>
    SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	void setInfoElements(QList<InfoElement*>);

public Q_SLOTS:
	void elementCurveRemoved(const QString&);

private Q_SLOTS:
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
