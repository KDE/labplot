/***************************************************************************
File                 : SlidingPanel.h
Project              : LabPlot
Description          : Sliding panel shown in the presenter widget
--------------------------------------------------------------------
Copyright            : (C) 2016 by Fabian Kristof (fkristofszabolcs@gmail.com)
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
#ifndef SLIDINGPANEL_H
#define SLIDINGPANEL_H

#include <QFrame>

class QLabel;
class QPushButton;

class SlidingPanel : public QFrame {
	Q_OBJECT
public:
	explicit SlidingPanel(QWidget* parent, const QString& worksheetName);
	~SlidingPanel() override;
	QPushButton* quitButton() const;
private:

	QLabel* m_worksheetName;
	QPushButton* m_quitPresentingMode;
	QSize sizeHint() const override;

public slots:
	void movePanel(qreal);
};

#endif // SLIDINGPANEL_H
