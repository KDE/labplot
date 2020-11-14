#ifndef INFOELEMENTDOCK_H
#define INFOELEMENTDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"

class InfoElement;

namespace Ui {
class InfoElementDock;
}

class InfoElementDock : public BaseDock {
	Q_OBJECT

public:
    explicit InfoElementDock(QWidget* parent = nullptr);
	~InfoElementDock();
	void setInfoElements(QList<InfoElement*> &list, bool sameParent);
	void initConnections();
public slots:
	void elementCurveRemoved(QString name);

private slots:
	void visibilityChanged(bool state);
	void addCurve();
	void removeCurve();
    void connectionLineWidthChanged(double);
    void connectionLineColorChanged(const QColor &);
    void xposLineWidthChanged(double);
    void xposLineColorChanged(const QColor &);
    void xposLineVisibilityChanged(bool);
    void connectionLineVisibilityChanged(bool);
	void gluePointChanged(int index);
    void curveChanged();

	// slots triggered in the InfoElement
    void elementConnectionLineWidthChanged(const double);
    void elementConnectionLineColorChanged(const QColor& );
    void elementXPosLineWidthChanged(const double);
    void elementXposLineColorChanged(const QColor&);
    void elementXPosLineVisibleChanged(const bool);
    void elementConnectionLineVisibleChanged(const bool);
    void elementVisibilityChanged(const bool);
    void elementGluePointIndexChanged(const int);
	void elementConnectionLineCurveChanged(const QString name);
    void elementLabelBorderShapeChanged();

private:
    Ui::InfoElementDock* ui;
	InfoElement* m_element;
	QList<InfoElement*> m_elements;
	bool m_sameParent;
};

#endif // INFOELEMENTDOCK_H
