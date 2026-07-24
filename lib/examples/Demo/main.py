from PySide6.QtWidgets import QApplication
from pylabplot import *

app = QApplication()

# create a spreadsheet and import the data into it
spreadsheet = Spreadsheet("data")
filter = AsciiFilter()
p = filter.properties()
p.headerEnabled = False
filter.setProperties(p)
filter.readDataFromFile("Demo/data.txt", spreadsheet)

# create a worksheet
worksheet = Worksheet("worksheet")

# create a plot area and add it to the worksheet
plotArea = CartesianPlot("plot area")
plotArea.setType(CartesianPlot.Type.FourAxes)
plotArea.addLegend()
worksheet.addChild(plotArea)

# create a histogram for the imported data and add it to the plot area
histogram = Histogram("histogram")
plotArea.addChild(histogram)
histogram.setNormalization(Histogram.Normalization.ProbabilityDensity)
histogram.setDataColumn(spreadsheet.column(0))

# perform a fit to the raw data and show it
fitCurve = XYFitCurve("fit")
fitCurve.setDataSourceType(XYAnalysisCurve.DataSourceType.Histogram)
fitCurve.setDataSourceHistogram(histogram)
plotArea.addChild(fitCurve)

# initialize the fit
fitData = fitCurve.fitData()
fitData.modelCategory = nsl_fit_model_category.nsl_fit_model_distribution
fitData.modelType = nsl_sf_stats_distribution.nsl_sf_stats_gaussian
fitData.algorithm = nsl_fit_algorithm.nsl_fit_algorithm_ml # ML distribution fit
fitData = XYFitCurve.initFitData(fitData)
fitData = fitCurve.initStartValues(fitData)
fitCurve.setFitData(fitData)

# perform the actual fit
fitCurve.recalculate()

# apply the theme "Dracula"
worksheet.setTheme("Dracula")

# export the worksheet to PDF
worksheet.exportToFile("result.pdf", Worksheet.ExportFormat.PDF)
