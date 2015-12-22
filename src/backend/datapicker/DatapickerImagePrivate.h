#ifndef DATAPICKERIMAGEPRIVATE_H
#define DATAPICKERIMAGEPRIVATE_H

#include <QBrush>

class QGraphicsScene;

class DatapickerImagePrivate {
public:
	explicit DatapickerImagePrivate(DatapickerImage*);
	virtual ~DatapickerImagePrivate();

	DatapickerImage::ReferencePoints axisPoints;
	DatapickerImage::EditorSettings settings;
	DatapickerImage::PointsType plotPointsType;
	DatapickerImage::PlotImageType plotImageType;

	DatapickerImage* const q;
	QRectF pageRect;
	QGraphicsScene* m_scene;
	float rotationAngle;
	QString fileName;
	int pointSeparation;
	int minSegmentLength;

    qreal pointRotationAngle;
    Symbol::Style pointStyle;
    QBrush pointBrush;
    QPen pointPen;
    qreal pointOpacity;
    qreal pointSize;
    bool pointVisibility;

	QString name() const;
    void retransform();
	void updateFileName();
	void discretize();
	void makeSegments();
	bool uploadImage(const QString&);
};

#endif
