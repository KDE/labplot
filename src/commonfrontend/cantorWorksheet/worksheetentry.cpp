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

#include "worksheetentry.h"
#include "commandentry.h"
#include "textentry.h"
#include "latexentry.h"
#include "imageentry.h"
#include "pagebreakentry.h"
// #include "settings.h"
#include "actionbar.h"
#include "worksheettoolbutton.h"

#include <QDrag>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QMetaMethod>
#include <QMimeData>
#include <QGraphicsProxyWidget>
#include <QBitmap>

#include <QIcon>
#include <KLocale>
#include <QDebug>

struct AnimationData
{
    QAnimationGroup* animation;
    QPropertyAnimation* sizeAnimation;
    QPropertyAnimation* opacAnimation;
    QPropertyAnimation* posAnimation;
    const char* slot;
    QGraphicsObject* item;
};

const qreal WorksheetEntry::VerticalMargin = 4;

WorksheetEntry::WorksheetEntry(Worksheet* worksheet) : QGraphicsObject()
{
    m_next = 0;
    m_prev = 0;
    m_animation = 0;
    m_actionBar = 0;
    m_actionBarAnimation = 0;
    m_aboutToBeRemoved = false;
    setAcceptHoverEvents(true);
    worksheet->addItem(this);
}

WorksheetEntry::~WorksheetEntry()
{
    emit aboutToBeDeleted();
    if (next())
        next()->setPrevious(previous());
    if (previous())
        previous()->setNext(next());
    if (m_animation) {
        m_animation->animation->deleteLater();
        delete m_animation;
    }
}

int WorksheetEntry::type() const
{
    return Type;
}

WorksheetEntry* WorksheetEntry::create(int t, Worksheet* worksheet)
{
    switch(t)
    {
    case TextEntry::Type:
        return new TextEntry(worksheet);
    case CommandEntry::Type:
        return new CommandEntry(worksheet);
    case ImageEntry::Type:
        return new ImageEntry(worksheet);
    case PageBreakEntry::Type:
        return new PageBreakEntry(worksheet);
    case LatexEntry::Type:
        return new LatexEntry(worksheet);
    default:
        return 0;
    }
}

void WorksheetEntry::insertCommandEntry()
{
    worksheet()->insertCommandEntry(this);
}

void WorksheetEntry::insertTextEntry()
{
    worksheet()->insertTextEntry(this);
}

void WorksheetEntry::insertLatexEntry()
{
    worksheet()->insertLatexEntry(this);
}

void WorksheetEntry::insertImageEntry()
{
    worksheet()->insertImageEntry(this);
}

void WorksheetEntry::insertPageBreakEntry()
{
    worksheet()->insertPageBreakEntry(this);
}

void WorksheetEntry::insertCommandEntryBefore()
{
    worksheet()->insertCommandEntryBefore(this);
}

void WorksheetEntry::insertTextEntryBefore()
{
    worksheet()->insertTextEntryBefore(this);
}

void WorksheetEntry::insertLatexEntryBefore()
{
    worksheet()->insertLatexEntryBefore(this);
}

void WorksheetEntry::insertImageEntryBefore()
{
    worksheet()->insertImageEntryBefore(this);
}

void WorksheetEntry::insertPageBreakEntryBefore()
{
    worksheet()->insertPageBreakEntryBefore(this);
}


void WorksheetEntry::showCompletion()
{
}

WorksheetEntry* WorksheetEntry::next() const
{
    return m_next;
}

WorksheetEntry* WorksheetEntry::previous() const
{
    return m_prev;
}

void WorksheetEntry::setNext(WorksheetEntry* n)
{
    m_next = n;
}

void WorksheetEntry::setPrevious(WorksheetEntry* p)
{
    m_prev = p;
}

void WorksheetEntry::startDrag(const QPointF& grabPos)
{
    QDrag* drag = new QDrag(worksheetView());
    qDebug() << size();
    const qreal scale = worksheet()->epsRenderer()->scale();
    QPixmap pixmap((size()*scale).toSize());
    pixmap.fill(QColor(255, 255, 255, 0));
    QPainter painter(&pixmap);
    const QRectF sceneRect = mapRectToScene(boundingRect());
    worksheet()->render(&painter, pixmap.rect(), sceneRect);
    painter.end();
    QBitmap mask = pixmap.createMaskFromColor(QColor(255, 255, 255),
                                              Qt::MaskInColor);
    pixmap.setMask(mask);

    drag->setPixmap(pixmap);
    if (grabPos.isNull()) {
        const QPointF scenePos = worksheetView()->sceneCursorPos();
        drag->setHotSpot((mapFromScene(scenePos) * scale).toPoint());
    } else {
        drag->setHotSpot((grabPos * scale).toPoint());
    }
    drag->setMimeData(new QMimeData());

    worksheet()->startDrag(this, drag);
}


QRectF WorksheetEntry::boundingRect() const
{
    return QRectF(QPointF(0,0), m_size);
}

void WorksheetEntry::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

bool WorksheetEntry::focusEntry(int pos, qreal xCoord)
{
    Q_UNUSED(pos);
    Q_UNUSED(xCoord);

    if (flags() & QGraphicsItem::ItemIsFocusable) {
        setFocus();
        return true;
    }
    return false;
}

void WorksheetEntry::moveToPreviousEntry(int pos, qreal x)
{
    WorksheetEntry* entry = previous();
    while (entry && !(entry->wantFocus() && entry->focusEntry(pos, x)))
        entry = entry->previous();
}

void WorksheetEntry::moveToNextEntry(int pos, qreal x)
{
    WorksheetEntry* entry = next();
    while (entry && !(entry->wantFocus() && entry->focusEntry(pos, x)))
        entry = entry->next();
}

Worksheet* WorksheetEntry::worksheet()
{
    return qobject_cast<Worksheet*>(scene());
}

WorksheetView* WorksheetEntry::worksheetView()
{
    return worksheet()->worksheetView();
}

WorksheetCursor WorksheetEntry::search(QString pattern, unsigned flags,
                                   QTextDocument::FindFlags qt_flags,
                                   const WorksheetCursor& pos)
{
    Q_UNUSED(pattern);
    Q_UNUSED(flags);
    Q_UNUSED(qt_flags);
    Q_UNUSED(pos);

    return WorksheetCursor();
}

void WorksheetEntry::keyPressEvent(QKeyEvent* event)
{
    // This event is used in Entries that set the ItemIsFocusable flag
    switch(event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Up:
        if (event->modifiers() == Qt::NoModifier)
            moveToPreviousEntry(WorksheetTextItem::BottomRight, 0);
        break;
    case Qt::Key_Right:
    case Qt::Key_Down:
        if (event->modifiers() == Qt::NoModifier)
            moveToNextEntry(WorksheetTextItem::TopLeft, 0);
        break;
        /*case Qt::Key_Enter:
    case Qt::Key_Return:
        if (event->modifiers() == Qt::ShiftModifier)
            evaluate();
        else if (event->modifiers() == Qt::ControlModifier)
            worksheet()->insertCommandEntry();
        break;
    case Qt::Key_Delete:
        if (event->modifiers() == Qt::ShiftModifier)
            startRemoving();
        break;
        default:*/
        event->ignore();
    }
}

void WorksheetEntry::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu *menu = worksheet()->createContextMenu();
    populateMenu(menu, event->pos());

    menu->popup(event->screenPos());
}

void WorksheetEntry::populateMenu(QMenu *menu, const QPointF& pos)
{
    if (!worksheet()->isRunning() && wantToEvaluate())
        menu->addAction(i18n("Evaluate Entry"), this, SLOT(evaluate()), 0);

    menu->addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove Entry"), this,
                    SLOT(startRemoving()), 0);
    worksheet()->populateMenu(menu, mapToScene(pos));
}

bool WorksheetEntry::evaluateCurrentItem()
{
    // A default implementation that works well for most entries,
    // because they have only one item.
    return evaluate();
}

void WorksheetEntry::evaluateNext(EvaluationOption opt)
{
    WorksheetEntry* entry = next();

    while (entry && !entry->wantFocus())
        entry = entry->next();

    if (entry) {
        if (opt == EvaluateNext) {
            entry->evaluate(EvaluateNext);
        } else if (opt == FocusNext) {
            worksheet()->setModified();
            entry->focusEntry(WorksheetTextItem::BottomRight);
        } else {
            worksheet()->setModified();
        }
    } else if (opt != DoNothing) {
        if (!isEmpty() || type() != CommandEntry::Type)
            worksheet()->appendCommandEntry();
        else
            focusEntry();
        worksheet()->setModified();
    }
}

qreal WorksheetEntry::setGeometry(qreal x, qreal y, qreal w)
{
    setPos(x, y);
    layOutForWidth(w);
    return size().height();
}

void WorksheetEntry::recalculateSize()
{
    qreal height = size().height();
    layOutForWidth(size().width(), true);
    if (height != size().height())
        worksheet()->updateEntrySize(this);
}

QPropertyAnimation* WorksheetEntry::sizeChangeAnimation(QSizeF s)
{
    QSizeF oldSize;
    QSizeF newSize;
    if (s.isValid()) {
        oldSize = size();
        newSize = s;
    } else {
        oldSize = size();
        layOutForWidth(size().width(), true);
        newSize = size();
    }
    qDebug() << oldSize << newSize;

    QPropertyAnimation* sizeAn = new QPropertyAnimation(this, "size", this);
    sizeAn->setDuration(200);
    sizeAn->setStartValue(oldSize);
    sizeAn->setEndValue(newSize);
    sizeAn->setEasingCurve(QEasingCurve::InOutQuad);
    connect(sizeAn, &QPropertyAnimation::valueChanged, this, &WorksheetEntry::sizeAnimated);
    return sizeAn;
}

void WorksheetEntry::sizeAnimated()
{
    worksheet()->updateEntrySize(this);
}

void WorksheetEntry::animateSizeChange()
{
    if (!worksheet()->animationsEnabled()) {
        recalculateSize();
        return;
    }
    if (m_animation) {
        layOutForWidth(size().width(), true);
        return;
    }
    QPropertyAnimation* sizeAn = sizeChangeAnimation();
    m_animation = new AnimationData;
    m_animation->item = 0;
    m_animation->slot = 0;
    m_animation->opacAnimation = 0;
    m_animation->posAnimation = 0;
    m_animation->sizeAnimation = sizeAn;
    m_animation->sizeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->animation = new QParallelAnimationGroup(this);
    m_animation->animation->addAnimation(m_animation->sizeAnimation);
    connect(m_animation->animation, &QAnimationGroup::finished, this, &WorksheetEntry::endAnimation);
    m_animation->animation->start();
}

void WorksheetEntry::fadeInItem(QGraphicsObject* item, const char* slot)
{
    if (!worksheet()->animationsEnabled()) {
        recalculateSize();
        if (slot)
            invokeSlotOnObject(slot, item);
        return;
    }
    if (m_animation) {
        // this calculates the new size and calls updateSizeAnimation
        layOutForWidth(size().width(), true);
        if (slot)
            invokeSlotOnObject(slot, item);
        return;
    }
    QPropertyAnimation* sizeAn = sizeChangeAnimation();
    m_animation = new AnimationData;
    m_animation->sizeAnimation = sizeAn;
    m_animation->sizeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->opacAnimation = new QPropertyAnimation(item, "opacity", this);
    m_animation->opacAnimation->setDuration(200);
    m_animation->opacAnimation->setStartValue(0);
    m_animation->opacAnimation->setEndValue(1);
    m_animation->opacAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->posAnimation = 0;

    m_animation->animation = new QParallelAnimationGroup(this);
    m_animation->item = item;
    m_animation->slot = slot;

    m_animation->animation->addAnimation(m_animation->sizeAnimation);
    m_animation->animation->addAnimation(m_animation->opacAnimation);

    connect(m_animation->animation, &QAnimationGroup::finished, this, &WorksheetEntry::endAnimation);

    m_animation->animation->start();
}

void WorksheetEntry::fadeOutItem(QGraphicsObject* item, const char* slot)
{
    // Note: The default value for slot is SLOT(deleteLater()), so item
    // will be deleted after the animation.
    if (!worksheet()->animationsEnabled()) {
        recalculateSize();
        if (slot)
            invokeSlotOnObject(slot, item);
        return;
    }
    if (m_animation) {
        // this calculates the new size and calls updateSizeAnimation
        layOutForWidth(size().width(), true);
        if (slot)
            invokeSlotOnObject(slot, item);
        return;
    }
    QPropertyAnimation* sizeAn = sizeChangeAnimation();
    m_animation = new AnimationData;
    m_animation->sizeAnimation = sizeAn;
    m_animation->opacAnimation = new QPropertyAnimation(item, "opacity", this);
    m_animation->opacAnimation->setDuration(200);
    m_animation->opacAnimation->setStartValue(1);
    m_animation->opacAnimation->setEndValue(0);
    m_animation->opacAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->posAnimation = 0;

    m_animation->animation = new QParallelAnimationGroup(this);
    m_animation->item = item;
    m_animation->slot = slot;

    m_animation->animation->addAnimation(m_animation->sizeAnimation);
    m_animation->animation->addAnimation(m_animation->opacAnimation);

    connect(m_animation->animation, &QAnimationGroup::finished, this, &WorksheetEntry::endAnimation);

    m_animation->animation->start();
}

void WorksheetEntry::endAnimation()
{
    if (!m_animation)
        return;
    QAnimationGroup* anim = m_animation->animation;
    if (anim->state() == QAbstractAnimation::Running) {
        anim->stop();
        if (m_animation->sizeAnimation)
            setSize(m_animation->sizeAnimation->endValue().value<QSizeF>());
        if (m_animation->opacAnimation) {
            qreal opac = m_animation->opacAnimation->endValue().value<qreal>();
            m_animation->item->setOpacity(opac);
        }
        if (m_animation->posAnimation) {
            const QPointF& pos = m_animation->posAnimation->endValue().value<QPointF>();
            m_animation->item->setPos(pos);
        }

        // If the animation was connected to a slot, call it
        if (m_animation->slot)
            invokeSlotOnObject(m_animation->slot, m_animation->item);
    }
    m_animation->animation->deleteLater();
    delete m_animation;
    m_animation = 0;
}

bool WorksheetEntry::animationActive()
{
    return m_animation;
}

void WorksheetEntry::updateSizeAnimation(const QSizeF& size)
{
    // Update the current animation, so that the new ending will be size

    if (!m_animation)
        return;

    if (m_aboutToBeRemoved)
        // do not modify the remove-animation
        return;
    if (m_animation->sizeAnimation) {
        QPropertyAnimation* sizeAn = m_animation->sizeAnimation;
        qreal progress = static_cast<qreal>(sizeAn->currentTime()) /
            sizeAn->totalDuration();
        QEasingCurve curve = sizeAn->easingCurve();
        qreal value = curve.valueForProgress(progress);
        sizeAn->setEndValue(size);
        QSizeF newStart = 1/(1-value)*(sizeAn->currentValue().value<QSizeF>() -
                                       value * size);
        sizeAn->setStartValue(newStart);
    } else {
        m_animation->sizeAnimation = sizeChangeAnimation(size);
        int d = m_animation->animation->duration() -
            m_animation->animation->currentTime();
        m_animation->sizeAnimation->setDuration(d);
        m_animation->animation->addAnimation(m_animation->sizeAnimation);
    }
}

void WorksheetEntry::invokeSlotOnObject(const char* slot, QObject* obj)
{
    const QMetaObject* metaObj = obj->metaObject();
    const QByteArray normSlot = QMetaObject::normalizedSignature(slot);
    const int slotIndex = metaObj->indexOfSlot(normSlot.constData());
    if (slotIndex == -1)
        qDebug() << "Warning: Tried to invoke an invalid slot:" << slot;
    const QMetaMethod method = metaObj->method(slotIndex);
    method.invoke(obj, Qt::DirectConnection);
}

bool WorksheetEntry::aboutToBeRemoved()
{
    return m_aboutToBeRemoved;
}

void WorksheetEntry::startRemoving()
{
    if (!worksheet()->animationsEnabled()) {
        m_aboutToBeRemoved = true;
        remove();
        return;
    }
    if (m_aboutToBeRemoved)
        return;

    if (focusItem()) {
        if (!next()) {
            if (previous() && previous()->isEmpty() &&
                !previous()->aboutToBeRemoved()) {
                previous()->focusEntry();
            } else {
                WorksheetEntry* next = worksheet()->appendCommandEntry();
                setNext(next);
                next->focusEntry();
            }
        } else {
            next()->focusEntry();
        }
    }

    if (m_animation) {
        endAnimation();
    }

    m_aboutToBeRemoved = true;
    m_animation = new AnimationData;
    m_animation->sizeAnimation = new QPropertyAnimation(this, "size", this);
    m_animation->sizeAnimation->setDuration(300);
    m_animation->sizeAnimation->setEndValue(QSizeF(size().width(), 0));
    m_animation->sizeAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    connect(m_animation->sizeAnimation, &QPropertyAnimation::valueChanged, this, &WorksheetEntry::sizeAnimated);
    connect(m_animation->sizeAnimation, &QPropertyAnimation::finished, this, &WorksheetEntry::remove);

    m_animation->opacAnimation = new QPropertyAnimation(this, "opacity", this);
    m_animation->opacAnimation->setDuration(300);
    m_animation->opacAnimation->setEndValue(0);
    m_animation->opacAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->posAnimation = 0;

    m_animation->animation = new QParallelAnimationGroup(this);
    m_animation->animation->addAnimation(m_animation->sizeAnimation);
    m_animation->animation->addAnimation(m_animation->opacAnimation);

    m_animation->animation->start();
}

bool WorksheetEntry::stopRemoving()
{
    if (!m_aboutToBeRemoved)
        return true;

    if (m_animation->animation->state() == QAbstractAnimation::Stopped)
        // we are too late to stop the deletion
        return false;

    m_aboutToBeRemoved = false;
    m_animation->animation->stop();
    m_animation->animation->deleteLater();
    delete m_animation;
    m_animation = 0;
    return true;
}

void WorksheetEntry::remove()
{
    if (!m_aboutToBeRemoved)
        return;

    if (previous() && previous()->next() == this)
        previous()->setNext(next());
    else
        worksheet()->setFirstEntry(next());
    if (next() && next()->previous() == this)
        next()->setPrevious(previous());
    else
        worksheet()->setLastEntry(previous());

    // make the entry invisible to QGraphicsScene's itemAt() function
    hide();
    worksheet()->updateLayout();
    deleteLater();
}

void WorksheetEntry::setSize(QSizeF size)
{
    prepareGeometryChange();
    if (m_actionBar && size != m_size)
        m_actionBar->updatePosition();
    m_size = size;
}

QSizeF WorksheetEntry::size()
{
    return m_size;
}

bool WorksheetEntry::hasActionBar()
{
    return m_actionBar;
}

void WorksheetEntry::showActionBar()
{
    if (m_actionBar && !m_actionBarAnimation)
        return;

    if (m_actionBarAnimation) {
        if (m_actionBarAnimation->endValue().toReal() == 1)
            return;
        m_actionBarAnimation->stop();
        delete m_actionBarAnimation;
        m_actionBarAnimation = 0;
    }

    if (!m_actionBar) {
        m_actionBar = new ActionBar(this);

        m_actionBar->addButton(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove Entry"),
                               this, SLOT(startRemoving()));

        WorksheetToolButton* dragButton;
        dragButton = m_actionBar->addButton(QIcon::fromTheme(QLatin1String("transform-move")),
                                            i18n("Drag Entry"));
        connect(dragButton, SIGNAL(pressed()), this, SLOT(startDrag()));

        if (wantToEvaluate()) {
            QString toolTip = i18n("Evaluate Entry");
            m_actionBar->addButton(QIcon::fromTheme(QLatin1String("view-refresh")), toolTip,
                                   this, SLOT(evaluate()));
        }

        m_actionBar->addSpace();

        addActionsToBar(m_actionBar);
    }

    if (worksheet()->animationsEnabled()) {
        m_actionBarAnimation = new QPropertyAnimation(m_actionBar, "opacity",
                                                      this);
        m_actionBarAnimation->setStartValue(0);
        m_actionBarAnimation->setKeyValueAt(0.666, 0);
        m_actionBarAnimation->setEndValue(1);
        m_actionBarAnimation->setDuration(600);
        connect(m_actionBarAnimation, &QPropertyAnimation::finished, this, &WorksheetEntry::deleteActionBarAnimation);

        m_actionBarAnimation->start();
    }
}

void WorksheetEntry::hideActionBar()
{
    if (!m_actionBar)
        return;

    if (m_actionBarAnimation) {
        if (m_actionBarAnimation->endValue().toReal() == 0)
            return;
        m_actionBarAnimation->stop();
        delete m_actionBarAnimation;
        m_actionBarAnimation = 0;
    }

    if (worksheet()->animationsEnabled()) {
        m_actionBarAnimation = new QPropertyAnimation(m_actionBar, "opacity",
                                                      this);
        m_actionBarAnimation->setEndValue(0);
        m_actionBarAnimation->setEasingCurve(QEasingCurve::Linear);
        m_actionBarAnimation->setDuration(200);
        connect(m_actionBarAnimation, &QPropertyAnimation::finished, this, &WorksheetEntry::deleteActionBar);

        m_actionBarAnimation->start();
    } else {
        deleteActionBar();
    }
}

void WorksheetEntry::deleteActionBarAnimation()
{
    if (m_actionBarAnimation) {
        delete m_actionBarAnimation;
        m_actionBarAnimation = 0;
    }
}

void WorksheetEntry::deleteActionBar()
{
    if (m_actionBar) {
        delete m_actionBar;
        m_actionBar = 0;
    }

    deleteActionBarAnimation();
}

void WorksheetEntry::addActionsToBar(ActionBar*)
{
}

void WorksheetEntry::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    showActionBar();
}

void WorksheetEntry::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    hideActionBar();
}

WorksheetTextItem* WorksheetEntry::highlightItem()
{
    return 0;
}

bool WorksheetEntry::wantFocus()
{
    return true;
}


