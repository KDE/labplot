#ifndef BAR3DPLOT_H
#define BAR3DPLOT_H

#include "backend/worksheet/WorksheetElement.h"

class Bar3DPlotPrivate;
class AbstractColumn;
class QBar3DSeries;
class KConfig;

class Bar3DPlot : public WorksheetElement {
	Q_OBJECT
public:
	explicit Bar3DPlot(const QString& name);
	~Bar3DPlot() override;

	BASIC_D_ACCESSOR_DECL(QColor, color, Color)

	QVector<const AbstractColumn*> dataColumns() const;
	void setDataColumns(const QVector<const AbstractColumn*>&);

	QBar3DSeries* series() const;

	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;
	void retransform() override;

	typedef Bar3DPlotPrivate Private;

Q_SIGNALS:
	void colorChanged(QColor);
	void dataColumnsChanged(const QVector<const AbstractColumn*>&);

private:
	Q_DECLARE_PRIVATE(Bar3DPlot)
};

#endif // BAR3DPLOT_H
