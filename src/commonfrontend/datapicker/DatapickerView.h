/*
    File                 : DatapickerView.h
    Project              : LabPlot
    Description          : View class for Datapicker
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre (wagadre.ankit@gmail.com)
    SPDX-FileCopyrightText: 2015-2020 Alexander Semke (alexander.semke@web.de)

*/
/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef DATAPICKERVIEW_H
#define DATAPICKERVIEW_H

#include <QWidget>

class AbstractAspect;
class Datapicker;

class QMenu;
class QTabWidget;
class QToolBar;

class DatapickerView : public QWidget {
	Q_OBJECT

public:
	explicit DatapickerView(Datapicker*);
	~DatapickerView() override;

	void createContextMenu(QMenu*) const;
	void fillToolBar(QToolBar*);
	int currentIndex() const;

private:
	QTabWidget* m_tabWidget;
	Datapicker* m_datapicker;
	int lastSelectedIndex{0};
	bool m_initializing;

private  slots:
	void showTabContextMenu(QPoint);
	void itemSelected(int);
	void tabChanged(int);
	void tabMoved(int,int);
	void handleDescriptionChanged(const AbstractAspect*);
	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
};

#endif
