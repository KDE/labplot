/***************************************************************************
File                 : LiveDataDock.h
Project              : LabPlot
Description          : Dock widget for live data properties
--------------------------------------------------------------------
Copyright            : (C) 2017 by Fabian Kristof (fkristofszabolcs@gmail.com)
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
#ifndef LIVEDATADOCK_H
#define LIVEDATADOCK_H

#include <QWidget>

#include "ui_livedatadock.h"
#include "backend/datasources/LiveDataSource.h"
#include <QList>

class LiveDataDock : public QWidget {
	Q_OBJECT

public:
	explicit LiveDataDock(QWidget *parent = 0);
	void setLiveDataSources(const QList<LiveDataSource*>& sources);
	~LiveDataDock();

private:
	Ui::LiveDataDock ui;
	QList<LiveDataSource*> m_liveDataSources;

	bool m_paused;

	void pauseReading();
	void continueReading();
private slots:

	void updateTypeChanged(int);
	void readingTypeChanged(int);
	void sampleRateChanged(int);
	void updateIntervalChanged(int);
	void keepNvaluesChanged(const QString&);

	void updateNow();
	void pauseContinueReading();
public slots:

signals:


};

#endif // LIVEDATADOCK_H
