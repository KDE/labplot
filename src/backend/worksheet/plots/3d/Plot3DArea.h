#ifndef PLOT3DAREA_H
#define PLOT3DAREA_H

#include <backend/worksheet/WorksheetElementContainer.h>

class Plot3DAreaPrivate;
class Plot3DArea : public WorksheetElementContainer {
	Q_OBJECT
public:
	explicit Plot3DArea(QString& name);
	~Plot3DArea() override;
	void init(bool transform = true);
	void retransform() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void setRect(const QRectF&) override;

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
};
#endif // PLOT3DAREA_H
