/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include "commandentry.h"
#include "animationresultitem.h"
#include "cantor/result.h"
#include "cantor/animationresult.h"

#include <KFileDialog>
#include <QDebug>
#include <KLocale>

AnimationResultItem::AnimationResultItem(QGraphicsObject* parent)
    : WorksheetImageItem(parent), ResultItem(), m_height(0), m_movie(0)
{
    connect(this, SIGNAL(removeResult()), parentEntry(),
            SLOT(removeResult()));
}

AnimationResultItem::~AnimationResultItem()
{
}

double AnimationResultItem::setGeometry(double x, double y, double w)
{
    Q_UNUSED(w);
    setPos(x,y);

    return m_height;
}

void AnimationResultItem::populateMenu(QMenu* menu, const QPointF& pos)
{
    addCommonActions(this, menu);

    menu->addSeparator();
    if (m_movie) {
        if (m_movie->state() == QMovie::Running)
            menu->addAction(QIcon::fromTheme(QLatin1String("media-playback-pause")), i18n("Pause"),
                            this, SLOT(pauseMovie()));
        else
            menu->addAction(QIcon::fromTheme(QLatin1String("media-playback-start")), i18n("Start"),
                            m_movie, SLOT(start()));
        if (m_movie->state() == QMovie::Running ||
            m_movie->state() == QMovie::Paused)
            menu->addAction(QIcon::fromTheme(QLatin1String("media-playback-stop")), i18n("Stop"),
                            this, SLOT(stopMovie()));
    }
    qDebug() << "populate Menu";
    emit menuCreated(menu, mapToParent(pos));
}

ResultItem* AnimationResultItem::updateFromResult(Cantor::Result* result)
{
    QMovie* mov;
    switch(result->type()) {
    case Cantor::AnimationResult::Type:
        mov = static_cast<QMovie*>(result->data().value<QObject*>());
        setMovie(mov);
        return this;
    default:
        deleteLater();
        return create(parentEntry(), result);
    }
}

QRectF AnimationResultItem::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}

double AnimationResultItem::width() const
{
    return WorksheetImageItem::width();
}

double AnimationResultItem::height() const
{
    return WorksheetImageItem::height();
}


void AnimationResultItem::setMovie(QMovie* movie)
{
    if (m_movie) {
        m_movie->disconnect(this, SLOT(updateFrame()));
        m_movie->disconnect(this, SLOT(updateSize()));
    }
    m_movie = movie;
    m_height = 0;
    if (m_movie) {
        connect(m_movie, &QMovie::frameChanged, this, &AnimationResultItem::updateFrame);
        connect(m_movie, &QMovie::resized, this, &AnimationResultItem::updateSize);
        m_movie->start();
    }
}

void AnimationResultItem::updateFrame()
{
    setImage(m_movie->currentImage());
    worksheet()->update(mapRectToScene(boundingRect()));
}

void AnimationResultItem::updateSize(const QSize& size)
{
    if (m_height != size.height()) {
        m_height = size.height();
        emit sizeChanged();
    }
}

void AnimationResultItem::saveResult()
{
    Cantor::Result* res = result();
    const QString& filename=KFileDialog::getSaveFileName(QUrl(), res->mimeType(), worksheet()->worksheetView());
    qDebug()<<"saving result to "<<filename;
    res->save(filename);
}

void AnimationResultItem::stopMovie()
{
    if (m_movie) {
        m_movie->stop();
        m_movie->jumpToFrame(0);
        worksheet()->update(mapRectToScene(boundingRect()));
    }
}

void AnimationResultItem::pauseMovie()
{
    if (m_movie)
        m_movie->setPaused(true);
}

void AnimationResultItem::deleteLater()
{
    WorksheetImageItem::deleteLater();
}

CommandEntry* AnimationResultItem::parentEntry()
{
    return qobject_cast<CommandEntry*>(parentObject());
}

Cantor::Result* AnimationResultItem::result()
{
    return parentEntry()->expression()->result();
}


