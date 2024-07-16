#ifndef SCATTER3DPLOTAREA_H
#define SCATTER3DPLOTAREA_H

#include "backend/core/AbstractColumn.h"
#include <Q3DScatter>
#include <backend/worksheet/WorksheetElementContainer.h>

class Scatter3DPlotAreaPrivate;
class WorksheetElementContainer;
class Scatter3DPlotArea : public WorksheetElementContainer {
	Q_OBJECT
public:
	Scatter3DPlotArea(const QString& name);
	~Scatter3DPlotArea() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, zColumn, ZColumn)
	void setRect(const QRectF&) override;
	void setPrevRect(const QRectF&) override;
	CLASS_D_ACCESSOR_DECL(QString, xColumnPath, XColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yColumnPath, YColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, zColumnPath, ZColumnPath)
	typedef Scatter3DPlotAreaPrivate Private;
	Q3DScatter* m_scatter;
	void recalc();
private Q_SLOTS:
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void yColumnAboutToBeRemoved(const AbstractAspect*);
	void zColumnAboutToBeRemoved(const AbstractAspect*);

protected:
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;
	void retransform() override;

private:
	Q_DECLARE_PRIVATE(Scatter3DPlotArea)

Q_SIGNALS:
	// General-Tab
	void xDataChanged();
	void yDataChanged();
	void zDataChanged();
	void selected(double pos);
	void rectChanged(QRectF&);
	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);
	void zColumnChanged(const AbstractColumn*);
};

#endif // SCATTER3DPLOTAREA_H
