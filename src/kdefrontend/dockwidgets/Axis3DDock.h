/*
	File                 : Axis3DDock.h
	Project              : LabPlot
	Description          : widget for Axis3D properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AXIS3DDOCK_H
#define AXIS3DDOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Axis3D.h"

#include "ui_axis3ddock.h"

class Axis3D;
class AspectTreeModel;
class TreeViewComboBox;

class Axis3DDock : public BaseDock {
	Q_OBJECT

public:
	explicit Axis3DDock(QWidget* parent);
	void setAxes(const QList<Axis3D*>& axes);

private:
	void updateUiVisibility();
	void load();
	void loadConfig(KConfig&);
    void changeParentPlot(Axis3D*);

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in Axis3DDock
    void minRangeChanged(double);
    void maxRangeChanged(double);
	void segmentCountChanged(int);
	void subSegmentCountChanged(int);
	void titleChanged(const QString&);
	void formatChanged(int);

	// SLOTs for changes triggered in Axis3D
    void axisMinRangeChanged(double);
    void axisMaxRangeChanged(double);
	void axisSegmentCountChanged(int);
	void axisSubSegmentCountChanged(int);
	void axisTitleChanged(const QString&);
	void axisFormatChanged(Axis3D::Format);

private:
	Ui::Axis3DDock ui;
	QList<Axis3D*> m_axes;
	Axis3D* m_axis{nullptr};

Q_SIGNALS:
	void info(const QString&);
};

#endif // AXIS3DDOCK_H
