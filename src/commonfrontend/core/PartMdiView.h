/*
    File                 : PartMdiView.h
    Project              : LabPlot
    Description          : QMdiSubWindow wrapper for aspect views.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2013-2019 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2007, 2008 Tilman Benkert (thzs@gmx.net)
    SPDX-FileCopyrightText: 2007, 2008 Knut Franke (knut.franke@gmx.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PART_MDI_VIEW_H
#define PART_MDI_VIEW_H

#include <QMdiSubWindow>

class AbstractAspect;
class AbstractPart;

class PartMdiView : public QMdiSubWindow {
	Q_OBJECT

public:
	explicit PartMdiView(AbstractPart*);
	~PartMdiView() override;
	AbstractPart* part() const;

private:
	void closeEvent(QCloseEvent*) override;
	AbstractPart* m_part;
	bool m_closing{false};

private slots:
	void handleAspectDescriptionChanged(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
	void slotWindowStateChanged(Qt::WindowStates oldState, Qt::WindowStates newState);
};

#endif // ifndef PART_MDI_VIEW_H
