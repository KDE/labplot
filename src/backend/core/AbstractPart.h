/*
    File                 : AbstractPart.h
    Project              : LabPlot
    Description          : Base class of Aspects with MDI windows as views.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2012-2015 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACT_PART_H
#define ABSTRACT_PART_H

#include "AbstractAspect.h"

class PartMdiView;
class QMenu;

class AbstractPart : public AbstractAspect {
Q_OBJECT

public:
	AbstractPart(const QString &name, AspectType type);
	~AbstractPart() override;

	virtual QWidget* view() const = 0;
	void deleteView() const;

	PartMdiView* mdiSubWindow() const;
	bool hasMdiSubWindow() const;

	QMenu* createContextMenu() override;
	virtual bool exportView() const = 0;
	virtual bool printView() = 0;
	virtual bool printPreview() const = 0;

	bool isDraggable() const override;
	QVector<AspectType> dropableOn() const override;

	//TODO: move these functions to a new class AbstractPartView
	virtual void registerShortcuts() {};
	virtual void unregisterShortcuts() {};

private:
	mutable PartMdiView* m_mdiWindow{nullptr};

protected:
	mutable QWidget* m_partView{nullptr};

signals:
	void showRequested();
	void importFromFileRequested();
	void importFromSQLDatabaseRequested();
	void exportRequested();
	void printRequested();
	void printPreviewRequested();
};

#endif // ifndef ABSTRACT_PART_H
