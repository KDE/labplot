#ifndef PLOT3DAREADOCK_H
#define PLOT3DAREADOCK_H
#include "BaseDock.h"
#include "ui_plot3dareadock.h"
#include <backend/worksheet/plots/3d/Plot3DArea.h>
class Plot3DAreaDock : public BaseDock {
	Q_OBJECT
public:
	explicit Plot3DAreaDock(QWidget* parent);
	void setPlots(const QList<Plot3DArea*>& plots);
	void rectChanged(const QRectF&);

private:
	void updateUiVisibility();
	void load();
	void loadConfig(KConfig&);

private Q_SLOTS:
	void retranslateUi();
	void onGeometryChanged(Worksheet::Layout);
	void geometryChanged();

private:
	Ui::Plot3DAreaDock ui;
	QList<Plot3DArea*> m_plots;
	Plot3DArea* m_plot{nullptr};

Q_SIGNALS:
	void info(const QString&);
	void elementVisibilityChanged();
};
#endif // PLOT3DDOCK_H
