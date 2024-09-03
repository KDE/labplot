#ifndef PLOT3DAREA_H
#define PLOT3DAREA_H

#include <backend/worksheet/WorksheetElementContainer.h>

#include <backend/worksheet/plots/AbstractPlot.h>

class Plot3DAreaPrivate;
class Plot3DArea : public AbstractPlot {
	Q_OBJECT
public:
	explicit Plot3DArea(const QString& name);
	void init(bool transform = true);
	void retransform() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void setPrevRect(const QRectF&) override;
	void setRect(const QRectF&) override;

	QMenu* createContextMenu() override;

	typedef Plot3DArea BaseClass;
	typedef Plot3DAreaPrivate Private;

private:
	void initActions();
	void initMenus();
	void configureAspect(AbstractAspect* aspect);

	QAction* addSurfaceAction;
	QAction* addBarAction;
	QAction* addScatterAction;

	QMenu* addNewMenu;

private:
	Q_DECLARE_PRIVATE(Plot3DArea)

private Q_SLOTS:
	void addSurface();
	void addScatter();
	void addBar();
Q_SIGNALS:
	void rectChanged(const QRectF&);
};
#endif // PLOT3DAREA_H
