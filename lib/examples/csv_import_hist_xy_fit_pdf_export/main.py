import sys
import pathlib

from PySide6.QtWidgets import QApplication

from pylabplot import Spreadsheet, AsciiFilter, Worksheet, CartesianPlot, Histogram, XYFitCurve, XYAnalysisCurve, nsl_fit_model_category, nsl_sf_stats_distribution, nsl_fit_algorithm

def main():
	app = QApplication()

	# create a spreadsheet and import the data into it
	spreadsheet = Spreadsheet("data")
	filter = AsciiFilter()
	filter.readDataFromFile(f"{pathlib.Path(__file__).parent.resolve()}/data.txt", spreadsheet)

	# create a worksheet
	worksheet = Worksheet("worksheet")

	# create a plot area and add it to the worksheet
	plotArea = CartesianPlot("plot area")
	plotArea.setType(CartesianPlot.Type.FourAxes)
	plotArea.addLegend()
	worksheet.addChild(plotArea)

	# create a histogram for the imported data and add it to the plot area
	histogram = Histogram("histogram")
	histogram.setNormalization(Histogram.Normalization.ProbabilityDensity)
	histogram.setDataColumn(spreadsheet.column(0))
	plotArea.addChild(histogram)

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
	XYFitCurve.initFitData(fitData)
	fitCurve.setFitData(fitData)

	# perform the actual fit
	fitCurve.recalculate()

	# apply the theme "Dracula"
	worksheet.setTheme("Dracula")

	# export the worksheet to PDF
	worksheet.exportToFile("result.pdf", Worksheet.ExportFormat.PDF)

main()