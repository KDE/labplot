#ifndef ABSTRACTPLOTPRIVATE_H
#define ABSTRACTPLOTPRIVATE_H

class AbstractPlotPrivate:public WorksheetElementContainerPrivate{
public:
	AbstractPlotPrivate(AbstractPlot* owner);
	virtual ~AbstractPlotPrivate(){}
	virtual QString name() const;
	virtual void retransform(){}
	
	float horizontalPadding; //horiz. offset between the plot area and the area defining the coodinate system, in scene units
	float verticalPadding; //vert. offset between the plot area and the area defining the coodinate system, in scene units
};

#endif
