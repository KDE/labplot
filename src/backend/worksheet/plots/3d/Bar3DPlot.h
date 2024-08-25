#ifndef BAR3DPLOTAREA_H
#define BAR3DPLOTAREA_H

#include "Base3DArea.h"
#include "backend/core/AbstractColumn.h"
#include <Q3DBars>
#include <backend/worksheet/WorksheetElementContainer.h>

class Bar3DPlotPrivate;
class WorsheetElementContainer;
class Plot3DArea;
class Bar3DPlot : public Base3DArea {
	Q_OBJECT
public:
	explicit Bar3DPlot(const QString& name);
	~Bar3DPlot() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void recalc();
	BASIC_D_ACCESSOR_DECL(QVector<AbstractColumn*>, dataColumns, DataColumns)
	CLASS_D_ACCESSOR_DECL(QVector<QString>, columnPaths, ColumnPaths)
	BASIC_D_ACCESSOR_DECL(QColor, color, Color)
	void setRect(const QRectF&) override;
	void setPrevRect(const QRectF&) override;
	typedef Bar3DPlotPrivate Private;

private:
	Q_DECLARE_PRIVATE(Bar3DPlot)
protected:
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;
	void retransform() override;
private Q_SLOTS:
	void columnAboutToBeRemoved(const AbstractAspect*);
Q_SIGNALS:
	void dataChanged();
	void dataColumnsChanged(QVector<AbstractColumn*>);
	void colorChanged(QColor);
	void rectChanged(QRectF&);
};
#endif // BAR3DPLOTAREA_H
