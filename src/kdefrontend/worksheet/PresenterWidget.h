/***************************************************************************
File                 : PresenterWidget.h
Project              : LabPlot
Description          : Widget for presenting worksheets
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
#ifndef PRESENTERWIDGET_H
#define PRESENTERWIDGET_H

#include <QWidget>
#include <QFrame>

class QLabel;
class QTimeLine;
class QPushButton;
class SlidingPanel;

class PresenterWidget : public QWidget {
    Q_OBJECT

public:
    explicit PresenterWidget(const QPixmap& pixmap,const QString& worksheetName, QWidget *parent = 0);
    ~PresenterWidget();

private:
    QLabel* m_imageLabel;
    QTimeLine* m_timeLine;
    SlidingPanel* m_panel;
    void startTimeline();

protected:
    void keyPressEvent(QKeyEvent* event);
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void slideDown();
    void slideUp();
};

class SlidingPanel : public QFrame {
    Q_OBJECT

public:
    explicit SlidingPanel(QWidget* parent, const QString& worksheetName);
    ~SlidingPanel();

    QLabel* m_worksheetName;
    QPushButton* m_quitPresentingMode;
    virtual QSize sizeHint() const;
    bool shouldHide();

public slots:
    void movePanel(qreal value);
};

#endif // PRESENTERWIDGET_H
