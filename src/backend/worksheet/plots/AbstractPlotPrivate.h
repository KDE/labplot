class AbstractPlotPrivate:public WorksheetElementContainerPrivate{
public:
	AbstractPlotPrivate(AbstractPlot* owner);
	virtual QString name() const = 0;
	virtual void retransform() = 0;
	
	float horizontalPadding; //horiz. offset between the plot area and the area defining the coodinate system, in scene units
	float verticalPadding; //vert. offset between the plot area and the area defining the coodinate system, in scene units
};
