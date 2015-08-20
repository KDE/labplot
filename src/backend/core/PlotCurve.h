#ifndef PLOTCURVE_H
#define PLOTCURVE_H

#include "backend/worksheet/Image.h"
#include "backend/core/AbstractAspect.h"

class CustomItem;
class QAction;
class PlotCurvePrivate;
class Column;
class Spreadsheet;
class AbstractColumn;

class PlotCurve: public AbstractAspect {
    Q_OBJECT

    public:
        explicit PlotCurve(const QString&);
        virtual ~PlotCurve();

        virtual QIcon icon() const;
        virtual QMenu* createContextMenu();
        void setPrinting(bool);

        void addCustomItem(const QPointF&);

        BASIC_D_ACCESSOR_DECL(bool, visible, Visible)
        BASIC_D_ACCESSOR_DECL(Image::Errors, curveErrorTypes, CurveErrorTypes)

        POINTER_D_ACCESSOR_DECL(AbstractColumn, posXColumn, PosXColumn)
        QString& posXColumnPath() const;
        POINTER_D_ACCESSOR_DECL(AbstractColumn, posYColumn, PosYColumn)
        QString& posYColumnPath() const;
        POINTER_D_ACCESSOR_DECL(AbstractColumn, plusDeltaXColumn, PlusDeltaXColumn)
        QString& plusDeltaXColumnPath() const;
        POINTER_D_ACCESSOR_DECL(AbstractColumn, minusDeltaXColumn, MinusDeltaXColumn)
        QString& minusDeltaXColumnPath() const;
        POINTER_D_ACCESSOR_DECL(AbstractColumn, plusDeltaYColumn, PlusDeltaYColumn)
        QString& plusDeltaYColumnPath() const;
        POINTER_D_ACCESSOR_DECL(AbstractColumn, minusDeltaYColumn, MinusDeltaYColumn)
        QString& minusDeltaYColumnPath() const;

        virtual void save(QXmlStreamWriter*) const;
        virtual bool load(XmlStreamReader*);

        typedef PlotCurvePrivate Private;

    protected:
        PlotCurve(const QString& name, PlotCurvePrivate* dd);
        PlotCurvePrivate* const d_ptr;

    private slots:
        void visibilityChanged();
        void updateDatasheet();
        void handleAspectAboutToBeRemoved(const AbstractAspect*);

    public slots:
        void updateData(const CustomItem*);
        void handleAspectAdded(const AbstractAspect*);

    private:
        Q_DECLARE_PRIVATE(PlotCurve)
        void init();
        void initAction();
        Column *appendColumn(const QString&, Spreadsheet*);

        QAction* visibilityAction;
        QAction* updateDatasheetAction;
};
#endif // PLOTCURVE_H
