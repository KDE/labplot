#ifndef COLORMAPPREVIEW_H
#define COLORMAPPREVIEW_H

#include <QtGui>
#include <kpreviewwidgetbase.h>
#include <kurl.h>

#ifdef HAVE_GL
#include "qwt3d_types.h"
#endif

class ColorMapPreview : public KPreviewWidgetBase{
 	Q_OBJECT

public:
	ColorMapPreview( QWidget *parent );
// 	~ColorMapPreview();

private:
#ifdef HAVE_GL
	Qwt3D::ColorVector cv;
#endif
	QLabel* label;
	QPixmap pixmap;
	bool open(QString);

public slots:
	void showPreview( const KUrl& u );
	void clearPreview();
};

#endif
