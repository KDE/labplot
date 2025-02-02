find_package(SharedMimeInfo REQUIRED)
set(FRONTEND_DIR "${CMAKE_CURRENT_LIST_DIR}/frontend")
set(BACKEND_DIR "${CMAKE_CURRENT_LIST_DIR}/backend")
set(CANTOR_DIR "${CMAKE_CURRENT_LIST_DIR}/cantor")
set(TOOLS_DIR "${CMAKE_CURRENT_LIST_DIR}/tools")
set(CMAKE_AUTOMOC ON)
set(SRC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

set(GUI_SOURCES
    ${FRONTEND_DIR}/AboutDialog.cpp
    ${FRONTEND_DIR}/DatasetModel.cpp
    ${FRONTEND_DIR}/GuiObserver.cpp
    ${FRONTEND_DIR}/GuiTools.cpp
    ${FRONTEND_DIR}/HistoryDialog.cpp
    ${FRONTEND_DIR}/MainWin.cpp
    ${FRONTEND_DIR}/PlotTemplateDialog.cpp
    ${FRONTEND_DIR}/ProjectExplorer.cpp
    ${FRONTEND_DIR}/SettingsDialog.cpp
    ${FRONTEND_DIR}/SettingsGeneralPage.cpp
    ${FRONTEND_DIR}/SettingsNotebookPage.cpp
    ${FRONTEND_DIR}/SettingsSpreadsheetPage.cpp
    ${FRONTEND_DIR}/SettingsWorksheetPage.cpp
    ${FRONTEND_DIR}/SettingsDatasetsPage.cpp
    ${FRONTEND_DIR}/SettingsPage.h
    ${FRONTEND_DIR}/TemplateHandler.cpp
    ${FRONTEND_DIR}/ThemeHandler.cpp
    ${FRONTEND_DIR}/examples/ExamplesDialog.cpp
    ${FRONTEND_DIR}/examples/ExamplesManager.cpp
    ${FRONTEND_DIR}/examples/ExamplesWidget.cpp
#     ${FRONTEND_DIR}/welcomescreen/WelcomeScreenHelper.cpp
    ${FRONTEND_DIR}/colormaps/ColorMapsWidget.cpp
    ${FRONTEND_DIR}/colormaps/ColorMapsDialog.cpp
    ${FRONTEND_DIR}/core/ContentDockWidget.cpp
    ${FRONTEND_DIR}/datapicker/DatapickerView.cpp
    ${FRONTEND_DIR}/datapicker/DatapickerImageView.cpp
    ${FRONTEND_DIR}/datasources/AsciiOptionsWidget.cpp
    ${FRONTEND_DIR}/datasources/CANOptionsWidget.cpp
    ${FRONTEND_DIR}/datasources/BinaryOptionsWidget.cpp
    ${FRONTEND_DIR}/datasources/DatabaseManagerDialog.cpp
    ${FRONTEND_DIR}/datasources/DatabaseManagerWidget.cpp
    ${FRONTEND_DIR}/datasources/DatasetMetadataManagerDialog.cpp
    ${FRONTEND_DIR}/datasources/DatasetMetadataManagerWidget.cpp
    ${FRONTEND_DIR}/datasources/OdsOptionsWidget.cpp
    ${FRONTEND_DIR}/datasources/XLSXOptionsWidget.cpp
    ${FRONTEND_DIR}/datasources/HDF5OptionsWidget.cpp
    ${FRONTEND_DIR}/datasources/ImageOptionsWidget.cpp
    ${FRONTEND_DIR}/datasources/ImportDatasetWidget.cpp
    ${FRONTEND_DIR}/datasources/ImportDatasetDialog.cpp
    ${FRONTEND_DIR}/datasources/ImportDialog.cpp
    ${FRONTEND_DIR}/datasources/ImportWarningsDialog.cpp
    ${FRONTEND_DIR}/datasources/ImportFileWidget.cpp
    ${FRONTEND_DIR}/datasources/ImportFileDialog.cpp
    ${FRONTEND_DIR}/datasources/ImportKaggleDatasetWidget.cpp
    ${FRONTEND_DIR}/datasources/ImportKaggleDatasetDialog.cpp
    ${FRONTEND_DIR}/datasources/ImportOriginLayersDialog.cpp
    ${FRONTEND_DIR}/datasources/ImportProjectDialog.cpp
    ${FRONTEND_DIR}/datasources/ImportSQLDatabaseDialog.cpp
    ${FRONTEND_DIR}/datasources/ImportSQLDatabaseWidget.cpp
    ${FRONTEND_DIR}/datasources/MatioOptionsWidget.cpp
    ${FRONTEND_DIR}/datasources/NetCDFOptionsWidget.cpp
    ${FRONTEND_DIR}/datasources/ROOTOptionsWidget.cpp
    ${FRONTEND_DIR}/datasources/FITSOptionsWidget.cpp
    ${FRONTEND_DIR}/datasources/JsonOptionsWidget.cpp
    ${FRONTEND_DIR}/datasources/McapOptionsWidget.cpp
    ${FRONTEND_DIR}/dockwidgets/BaseDock.cpp
    ${FRONTEND_DIR}/dockwidgets/AspectDock.cpp
    ${FRONTEND_DIR}/dockwidgets/AxisDock.cpp
    ${FRONTEND_DIR}/dockwidgets/CursorDock.cpp
    ${FRONTEND_DIR}/dockwidgets/ImageDock.cpp
    ${FRONTEND_DIR}/dockwidgets/InfoElementDock.cpp
    ${FRONTEND_DIR}/dockwidgets/NoteDock.cpp
    ${FRONTEND_DIR}/dockwidgets/CartesianPlotDock.cpp
    ${FRONTEND_DIR}/dockwidgets/CartesianPlotLegendDock.cpp
    ${FRONTEND_DIR}/dockwidgets/HistogramDock.cpp
    ${FRONTEND_DIR}/dockwidgets/BarPlotDock.cpp
    ${FRONTEND_DIR}/dockwidgets/BoxPlotDock.cpp
    ${FRONTEND_DIR}/dockwidgets/CustomPointDock.cpp
    ${FRONTEND_DIR}/dockwidgets/ColumnDock.cpp
    ${FRONTEND_DIR}/dockwidgets/KDEPlotDock.cpp
    ${FRONTEND_DIR}/dockwidgets/LiveDataDock.cpp
    ${FRONTEND_DIR}/dockwidgets/LollipopPlotDock.cpp
    ${FRONTEND_DIR}/dockwidgets/MatrixDock.cpp
    ${FRONTEND_DIR}/dockwidgets/ProcessBehaviorChartDock.cpp
    ${FRONTEND_DIR}/dockwidgets/ProjectDock.cpp
    ${FRONTEND_DIR}/dockwidgets/ReferenceLineDock.cpp
    ${FRONTEND_DIR}/dockwidgets/ReferenceRangeDock.cpp
    ${FRONTEND_DIR}/dockwidgets/RunChartDock.cpp
    ${FRONTEND_DIR}/dockwidgets/SpreadsheetDock.cpp
    ${FRONTEND_DIR}/dockwidgets/StatisticsSpreadsheetDock.cpp
    ${FRONTEND_DIR}/dockwidgets/QQPlotDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYAnalysisCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYEquationCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYFunctionCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYDataReductionCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYDifferentiationCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYIntegrationCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYInterpolationCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYSmoothCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYFitCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYFourierFilterCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYFourierTransformCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYHilbertTransformCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYConvolutionCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/XYCorrelationCurveDock.cpp
    ${FRONTEND_DIR}/dockwidgets/WorksheetDock.cpp
    ${FRONTEND_DIR}/matrix/MatrixFunctionDialog.cpp
    ${FRONTEND_DIR}/matrix/MatrixView.cpp
    ${FRONTEND_DIR}/note/NoteView.cpp
    ${FRONTEND_DIR}/spreadsheet/PlotDataDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/EquidistantValuesDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/ExportSpreadsheetDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/AddSubtractValueDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/AddValueLabelDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/BatchEditValueLabelsDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/DropValuesDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/FlattenColumnsDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/FormattingHeatmapDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/GoToDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/FunctionValuesDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/RandomValuesDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/RescaleDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/SampleValuesDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/SearchReplaceWidget.cpp
    ${FRONTEND_DIR}/spreadsheet/SpreadsheetCommentsHeaderModel.cpp
    ${FRONTEND_DIR}/spreadsheet/SpreadsheetHeaderView.cpp
    ${FRONTEND_DIR}/spreadsheet/SpreadsheetSparkLineHeaderModel.cpp
    ${FRONTEND_DIR}/spreadsheet/SpreadsheetItemDelegate.cpp
    ${FRONTEND_DIR}/spreadsheet/SparklineRunnable.h
    ${FRONTEND_DIR}/spreadsheet/SparklineRunnable.cpp
    ${FRONTEND_DIR}/spreadsheet/SpreadsheetView.cpp
    ${FRONTEND_DIR}/spreadsheet/SortDialog.cpp
    ${FRONTEND_DIR}/spreadsheet/StatisticsColumnWidget.cpp
    ${FRONTEND_DIR}/spreadsheet/StatisticsDialog.cpp
    ${FRONTEND_DIR}/worksheet/ExportWorksheetDialog.cpp
    ${FRONTEND_DIR}/worksheet/GridDialog.cpp
    ${FRONTEND_DIR}/worksheet/PresenterWidget.cpp
    ${FRONTEND_DIR}/worksheet/SlidingPanel.cpp
    ${FRONTEND_DIR}/worksheet/WorksheetPreviewWidget.cpp
    ${FRONTEND_DIR}/worksheet/WorksheetView.cpp
    ${FRONTEND_DIR}/widgets/BackgroundWidget.cpp
    ${FRONTEND_DIR}/widgets/ConstantsWidget.cpp
    ${FRONTEND_DIR}/widgets/ThemesComboBox.cpp
    ${FRONTEND_DIR}/widgets/ThemesWidget.cpp
    ${FRONTEND_DIR}/widgets/ExpressionTextEdit.cpp
    ${FRONTEND_DIR}/widgets/FitOptionsWidget.cpp
    ${FRONTEND_DIR}/widgets/FitParametersWidget.cpp
    ${FRONTEND_DIR}/widgets/FunctionsWidget.cpp
    ${FRONTEND_DIR}/widgets/LabelWidget.cpp
    ${FRONTEND_DIR}/widgets/ErrorBarWidget.cpp
    ${FRONTEND_DIR}/widgets/LineWidget.cpp
    ${FRONTEND_DIR}/widgets/SymbolWidget.cpp
    ${FRONTEND_DIR}/widgets/ValueWidget.cpp
    ${FRONTEND_DIR}/widgets/DatapickerImageWidget.cpp
    ${FRONTEND_DIR}/widgets/DatapickerCurveWidget.cpp
    ${FRONTEND_DIR}/widgets/FITSHeaderEditWidget.cpp
    ${FRONTEND_DIR}/widgets/FITSHeaderEditNewKeywordDialog.cpp
    ${FRONTEND_DIR}/widgets/FITSHeaderEditAddUnitDialog.cpp
    ${FRONTEND_DIR}/widgets/FITSHeaderEditDialog.cpp
    ${FRONTEND_DIR}/widgets/ResizableTextEdit.cpp
    ${FRONTEND_DIR}/widgets/TimedLineEdit.cpp
    ${FRONTEND_DIR}/widgets/TreeViewComboBox.cpp
    ${FRONTEND_DIR}/widgets/qxtspanslider.cpp
    ${FRONTEND_DIR}/widgets/MemoryWidget.cpp
    ${FRONTEND_DIR}/widgets/DateTimeSpinBox.cpp
    ${FRONTEND_DIR}/widgets/NumberSpinBox.cpp
    ${FRONTEND_DIR}/widgets/UTCDateTimeEdit.cpp
    ${FRONTEND_DIR}/workbook/WorkbookView.cpp
)

if(Qt${QT_VERSION_MAJOR}Mqtt_FOUND)
    list(APPEND GUI_SOURCES ${FRONTEND_DIR}/datasources/MQTTConnectionManagerWidget.cpp)
    list(APPEND GUI_SOURCES ${FRONTEND_DIR}/datasources/MQTTConnectionManagerDialog.cpp)
    list(APPEND GUI_SOURCES ${FRONTEND_DIR}/datasources/MQTTErrorWidget.cpp)
    list(APPEND GUI_SOURCES ${FRONTEND_DIR}/datasources/MQTTSubscriptionWidget.cpp)
endif()

if(APPLE)
  list(APPEND GUI_SOURCES ${FRONTEND_DIR}/worksheet/PresenterWidget_mac.mm)
endif()

set(GUI_QRC
    ${FRONTEND_DIR}/labplot.qrc
)

set(UI_SOURCES
    ${FRONTEND_DIR}/ui/colormapswidget.ui
    ${FRONTEND_DIR}/ui/constantswidget.ui
    ${FRONTEND_DIR}/ui/PlotTemplateDialog.ui
    ${FRONTEND_DIR}/ui/exampleswidget.ui
    ${FRONTEND_DIR}/ui/functionswidget.ui
    ${FRONTEND_DIR}/ui/fitoptionswidget.ui
    ${FRONTEND_DIR}/ui/fitparameterswidget.ui
    ${FRONTEND_DIR}/ui/labelwidget.ui
    ${FRONTEND_DIR}/ui/symbolwidget.ui
    ${FRONTEND_DIR}/ui/settingsnotebookpage.ui
    ${FRONTEND_DIR}/ui/settingsgeneralpage.ui
    ${FRONTEND_DIR}/ui/settingsspreadsheetpage.ui
    ${FRONTEND_DIR}/ui/settingsworksheetpage.ui
#     ${FRONTEND_DIR}/ui/settingswelcomepage.ui
    ${FRONTEND_DIR}/ui/settingsdatasetspage.ui
    ${FRONTEND_DIR}/ui/settingsprintingpage.ui
    ${FRONTEND_DIR}/ui/datasources/asciioptionswidget.ui
    ${FRONTEND_DIR}/ui/datasources/CANOptionsWidget.ui
    ${FRONTEND_DIR}/ui/datasources/binaryoptionswidget.ui
    ${FRONTEND_DIR}/ui/datasources/databasemanagerwidget.ui
    ${FRONTEND_DIR}/ui/datasources/datasetmetadatamanagerwidget.ui
    ${FRONTEND_DIR}/ui/datasources/hdf5optionswidget.ui
    ${FRONTEND_DIR}/ui/datasources/imageoptionswidget.ui
    ${FRONTEND_DIR}/ui/datasources/importdatasetwidget.ui
    ${FRONTEND_DIR}/ui/datasources/importkaggledatasetwidget.ui
    ${FRONTEND_DIR}/ui/datasources/importfilewidget.ui
    ${FRONTEND_DIR}/ui/datasources/importoriginlayersdialog.ui
    ${FRONTEND_DIR}/ui/datasources/importwarningsdialog.ui
    ${FRONTEND_DIR}/ui/datasources/importprojectwidget.ui
    ${FRONTEND_DIR}/ui/datasources/importsqldatabasewidget.ui
    ${FRONTEND_DIR}/ui/datasources/matiooptionswidget.ui
    ${FRONTEND_DIR}/ui/datasources/netcdfoptionswidget.ui
    ${FRONTEND_DIR}/ui/datasources/rootoptionswidget.ui
    ${FRONTEND_DIR}/ui/datasources/fitsoptionswidget.ui
    ${FRONTEND_DIR}/ui/datasources/jsonoptionswidget.ui
    ${FRONTEND_DIR}/ui/datasources/mcapoptionswidget.ui
    ${FRONTEND_DIR}/ui/datasources/odsoptionswidget.ui
    ${FRONTEND_DIR}/ui/datasources/xlsxoptionswidget.ui
    ${FRONTEND_DIR}/ui/dockwidgets/aspectdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/axisdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/backgroundwidget.ui
    ${FRONTEND_DIR}/ui/dockwidgets/cursordock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/infoelementdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/cartesianplotdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/cartesianplotlegenddock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/histogramdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/barplotdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/boxplotdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/columndock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/custompointdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/errorbarwidget.ui
    ${FRONTEND_DIR}/ui/dockwidgets/imagedock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/kdeplotdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/linewidget.ui
    ${FRONTEND_DIR}/ui/dockwidgets/livedatadock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/lollipopplotdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/notedock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/matrixdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/processbehaviorchartdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/projectdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/referencelinedock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/referencerangedock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/runchartdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/spreadsheetdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/statisticsspreadsheetdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/qqplotdock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xycurvedock.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xycurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xydatareductioncurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xydifferentiationcurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xyintegrationcurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xyinterpolationcurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xysmoothcurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xyfitcurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xyfourierfiltercurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xyfouriertransformcurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xyhilberttransformcurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xyconvolutioncurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xycorrelationcurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xyequationcurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/xyfunctioncurvedockgeneraltab.ui
    ${FRONTEND_DIR}/ui/dockwidgets/valuewidget.ui
    ${FRONTEND_DIR}/ui/dockwidgets/worksheetdock.ui
    ${FRONTEND_DIR}/ui/matrix/matrixfunctionwidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/plotdatawidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/equidistantvalueswidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/exportspreadsheetwidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/addsubtractvaluewidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/dropvalueswidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/flattencolumnswidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/formattingheatmapwidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/functionvalueswidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/randomvalueswidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/rescalewidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/samplevalueswidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/searchwidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/searchreplacewidget.ui
    ${FRONTEND_DIR}/ui/spreadsheet/sortdialogwidget.ui
    ${FRONTEND_DIR}/ui/worksheet/exportworksheetwidget.ui
    ${FRONTEND_DIR}/ui/worksheet/worksheetpreviewwidget.ui
    ${FRONTEND_DIR}/ui/datapickerimagewidget.ui
    ${FRONTEND_DIR}/ui/datapickercurvewidget.ui
    ${FRONTEND_DIR}/ui/fitsheadereditwidget.ui
    ${FRONTEND_DIR}/ui/fitsheadereditnewkeywordwidget.ui
    ${FRONTEND_DIR}/ui/fitsheadereditaddunitwidget.ui
)

if(Qt${QT_VERSION_MAJOR}Mqtt_FOUND)
    list(APPEND UI_SOURCES ${FRONTEND_DIR}/ui/datasources/mqttconnectionmanagerwidget.ui)
    list(APPEND UI_SOURCES ${FRONTEND_DIR}/ui/datasources/mqtterrorwidget.ui)
    list(APPEND UI_SOURCES ${FRONTEND_DIR}/ui/datasources/mqttsubscriptionwidget.ui)
    list(APPEND UI_SOURCES ${FRONTEND_DIR}/ui/mqttwillsettingswidget.ui)
endif()

set(BACKEND_SOURCES
    ${BACKEND_DIR}/core/Time.cpp
    ${BACKEND_DIR}/core/Folder.cpp
    ${BACKEND_DIR}/core/AbstractAspect.cpp
    ${BACKEND_DIR}/core/AbstractColumn.cpp
    ${BACKEND_DIR}/core/AbstractColumnPrivate.cpp
    ${BACKEND_DIR}/core/abstractcolumncommands.cpp
    ${BACKEND_DIR}/core/AbstractFilter.cpp
    ${BACKEND_DIR}/core/AbstractSimpleFilter.cpp
    ${BACKEND_DIR}/core/column/Column.cpp
    ${BACKEND_DIR}/core/column/ColumnPrivate.cpp
    ${BACKEND_DIR}/core/column/ColumnStringIO.cpp
    ${BACKEND_DIR}/core/column/columncommands.cpp
    ${BACKEND_DIR}/core/Project.cpp
    ${BACKEND_DIR}/core/AbstractPart.cpp
    ${BACKEND_DIR}/core/Workbook.cpp
    ${BACKEND_DIR}/core/AspectTreeModel.cpp
    ${BACKEND_DIR}/core/Settings.cpp
    ${BACKEND_DIR}/core/datatypes/SimpleCopyThroughFilter.h
    ${BACKEND_DIR}/core/datatypes/Double2DateTimeFilter.h
    ${BACKEND_DIR}/core/datatypes/Double2DayOfWeekFilter.h
    ${BACKEND_DIR}/core/datatypes/Double2IntegerFilter.h
    ${BACKEND_DIR}/core/datatypes/Double2BigIntFilter.h
    ${BACKEND_DIR}/core/datatypes/Double2MonthFilter.h
    ${BACKEND_DIR}/core/datatypes/Double2StringFilter.cpp
    ${BACKEND_DIR}/core/datatypes/Integer2DoubleFilter.h
    ${BACKEND_DIR}/core/datatypes/Integer2BigIntFilter.h
    ${BACKEND_DIR}/core/datatypes/Integer2DateTimeFilter.h
    ${BACKEND_DIR}/core/datatypes/Integer2DayOfWeekFilter.h
    ${BACKEND_DIR}/core/datatypes/Integer2MonthFilter.h
    ${BACKEND_DIR}/core/datatypes/Integer2StringFilter.h
    ${BACKEND_DIR}/core/datatypes/BigInt2DoubleFilter.h
    ${BACKEND_DIR}/core/datatypes/BigInt2IntegerFilter.h
    ${BACKEND_DIR}/core/datatypes/BigInt2StringFilter.h
    ${BACKEND_DIR}/core/datatypes/BigInt2DateTimeFilter.h
    ${BACKEND_DIR}/core/datatypes/BigInt2MonthFilter.h
    ${BACKEND_DIR}/core/datatypes/BigInt2DayOfWeekFilter.h
    ${BACKEND_DIR}/core/datatypes/String2DayOfWeekFilter.h
    ${BACKEND_DIR}/core/datatypes/String2DoubleFilter.h
    ${BACKEND_DIR}/core/datatypes/String2IntegerFilter.h
    ${BACKEND_DIR}/core/datatypes/String2BigIntFilter.h
    ${BACKEND_DIR}/core/datatypes/String2MonthFilter.h
    ${BACKEND_DIR}/core/datatypes/String2DateTimeFilter.cpp
    ${BACKEND_DIR}/core/datatypes/DateTime2DoubleFilter.h
    ${BACKEND_DIR}/core/datatypes/DateTime2IntegerFilter.h
    ${BACKEND_DIR}/core/datatypes/DateTime2BigIntFilter.h
    ${BACKEND_DIR}/core/datatypes/DateTime2StringFilter.cpp
    ${BACKEND_DIR}/core/datatypes/Month2DoubleFilter.h
    ${BACKEND_DIR}/core/datatypes/Month2IntegerFilter.h
    ${BACKEND_DIR}/core/datatypes/Month2BigIntFilter.h
    ${BACKEND_DIR}/core/datatypes/DayOfWeek2DoubleFilter.h
    ${BACKEND_DIR}/core/datatypes/DayOfWeek2IntegerFilter.h
    ${BACKEND_DIR}/core/datatypes/DayOfWeek2BigIntFilter.h
    ${BACKEND_DIR}/core/plugin/PluginLoader.cpp
    ${BACKEND_DIR}/core/plugin/PluginManager.cpp
    ${BACKEND_DIR}/datapicker/DatapickerPoint.cpp
    ${BACKEND_DIR}/datapicker/DatapickerImage.cpp
    ${BACKEND_DIR}/datapicker/Datapicker.cpp
    ${BACKEND_DIR}/datapicker/Transform.cpp
    ${BACKEND_DIR}/datapicker/ImageEditor.cpp
    ${BACKEND_DIR}/datapicker/Segment.cpp
    ${BACKEND_DIR}/datapicker/Segments.cpp
    ${BACKEND_DIR}/datapicker/DatapickerCurve.cpp
    ${BACKEND_DIR}/datasources/AbstractDataSource.cpp
    ${BACKEND_DIR}/datasources/DatasetHandler.cpp
    ${BACKEND_DIR}/datasources/LiveDataSource.cpp
    ${BACKEND_DIR}/datasources/filters/AbstractFileFilter.cpp
    ${BACKEND_DIR}/datasources/filters/AsciiFilter.cpp
    ${BACKEND_DIR}/datasources/filters/BinaryFilter.cpp
    ${BACKEND_DIR}/datasources/filters/XLSXFilter.cpp
    ${BACKEND_DIR}/datasources/filters/FITSFilter.cpp
    ${BACKEND_DIR}/datasources/filters/HDF5Filter.cpp
    ${BACKEND_DIR}/datasources/filters/ImageFilter.cpp
    ${BACKEND_DIR}/datasources/filters/JsonFilter.cpp
    ${BACKEND_DIR}/datasources/filters/McapFilter.cpp
    ${BACKEND_DIR}/datasources/filters/MatioFilter.cpp
    ${BACKEND_DIR}/datasources/filters/NetCDFFilter.cpp
    ${BACKEND_DIR}/datasources/filters/OdsFilter.cpp
    ${BACKEND_DIR}/datasources/filters/QJsonModel.cpp
    ${BACKEND_DIR}/datasources/filters/ReadStatFilter.cpp
    ${BACKEND_DIR}/datasources/filters/ROOTFilter.cpp
    ${BACKEND_DIR}/datasources/filters/SpiceReader.cpp
    ${BACKEND_DIR}/datasources/filters/SpiceFilter.cpp
    ${BACKEND_DIR}/datasources/filters/VectorBLFFilter.cpp
    ${BACKEND_DIR}/datasources/filters/CANFilter.cpp
    ${BACKEND_DIR}/datasources/filters/DBCParser.cpp
    ${BACKEND_DIR}/datasources/projects/ProjectParser.cpp
    ${BACKEND_DIR}/datasources/projects/LabPlotProjectParser.cpp
    ${BACKEND_DIR}/gsl/ExpressionParser.cpp
    ${BACKEND_DIR}/gsl/constants.cpp
    ${BACKEND_DIR}/gsl/functions.cpp
    ${BACKEND_DIR}/gsl/Parser.cpp
    ${BACKEND_DIR}/lib/Range.h
    ${BACKEND_DIR}/lib/Debug.cpp
    ${BACKEND_DIR}/lib/XmlStreamReader.cpp
    ${BACKEND_DIR}/lib/SignallingUndoCommand.cpp
    ${BACKEND_DIR}/lib/hostprocess.cpp
    ${BACKEND_DIR}/matrix/Matrix.cpp
    ${BACKEND_DIR}/matrix/matrixcommands.cpp
    ${BACKEND_DIR}/matrix/MatrixModel.cpp
    ${BACKEND_DIR}/note/Note.cpp
    ${BACKEND_DIR}/spreadsheet/Spreadsheet.cpp
    ${BACKEND_DIR}/spreadsheet/SpreadsheetModel.cpp
    ${BACKEND_DIR}/spreadsheet/StatisticsSpreadsheet.cpp
    ${BACKEND_DIR}/worksheet/Background.cpp
    ${BACKEND_DIR}/worksheet/Image.cpp
    ${BACKEND_DIR}/worksheet/Line.cpp
    ${BACKEND_DIR}/worksheet/ResizeItem.cpp
    ${BACKEND_DIR}/worksheet/WorksheetElement.cpp
    ${BACKEND_DIR}/worksheet/TextLabel.cpp
    ${BACKEND_DIR}/worksheet/InfoElement.cpp
    ${BACKEND_DIR}/worksheet/Worksheet.cpp
    ${BACKEND_DIR}/worksheet/WorksheetElementContainer.cpp
    ${BACKEND_DIR}/worksheet/TreeModel.cpp
    ${BACKEND_DIR}/worksheet/plots/AbstractPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/AbstractCoordinateSystem.cpp
    ${BACKEND_DIR}/worksheet/plots/PlotArea.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Axis.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/BarPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/BoxPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianScale.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianCoordinateSystem.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlotLegend.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ErrorBar.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Histogram.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CustomPoint.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/KDEPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/LollipopPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Plot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ProcessBehaviorChart.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ReferenceLine.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ReferenceRange.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/RunChart.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Symbol.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/QQPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYAnalysisCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYEquationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYFunctionCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYDataReductionCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYDifferentiationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYIntegrationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYInterpolationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYSmoothCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYFitCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYFourierFilterCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYFourierTransformCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYHilbertTransformCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYConvolutionCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYCorrelationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Value.cpp
)

if(Qt${QT_VERSION_MAJOR}Mqtt_FOUND)
    list(APPEND BACKEND_SOURCES ${FRONTEND_DIR}/widgets/MQTTWillSettingsWidget.cpp)
    list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/datasources/MQTTClient.cpp)
    list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/datasources/MQTTSubscription.cpp)
    list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/datasources/MQTTTopic.cpp)
endif()

# add the qml.qrc file
# qt5_add_resources(qml_QRC ${FRONTEND_DIR}/welcomescreen/qml.qrc)
# better: enable AUTORCC

if(ENABLE_LIBORIGIN)
    list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/datasources/projects/OriginProjectParser.cpp)
endif()

set(NSL_SOURCES
    ${BACKEND_DIR}/nsl/nsl_baseline.cpp
    ${BACKEND_DIR}/nsl/nsl_conv.c
    ${BACKEND_DIR}/nsl/nsl_corr.c
    ${BACKEND_DIR}/nsl/nsl_dft.c
    ${BACKEND_DIR}/nsl/nsl_diff.c
    ${BACKEND_DIR}/nsl/nsl_filter.c
    ${BACKEND_DIR}/nsl/nsl_fit.c
    ${BACKEND_DIR}/nsl/nsl_geom.c
    ${BACKEND_DIR}/nsl/nsl_geom_linesim.c
    ${BACKEND_DIR}/nsl/nsl_hilbert.c
    ${BACKEND_DIR}/nsl/nsl_int.c
    ${BACKEND_DIR}/nsl/nsl_interp.c
    ${BACKEND_DIR}/nsl/nsl_kde.c
    ${BACKEND_DIR}/nsl/nsl_math.c
    ${BACKEND_DIR}/nsl/nsl_pcm.c
    ${BACKEND_DIR}/nsl/nsl_peak.cpp
    ${BACKEND_DIR}/nsl/nsl_randist.c
    ${BACKEND_DIR}/nsl/nsl_sf_basic.c
    ${BACKEND_DIR}/nsl/nsl_sf_kernel.c
    ${BACKEND_DIR}/nsl/nsl_sf_poly.c
    ${BACKEND_DIR}/nsl/nsl_sf_stats.c
    ${BACKEND_DIR}/nsl/nsl_sf_window.c
    ${BACKEND_DIR}/nsl/nsl_smooth.c
    ${BACKEND_DIR}/nsl/nsl_sort.c
    ${BACKEND_DIR}/nsl/nsl_stats.c
)

if(NOT MSVC_FOUND)
if(NOT LIBCERF_FOUND)
    list(APPEND NSL_SOURCES
        ${BACKEND_DIR}/nsl/Faddeeva.c
    )
endif()
endif()

if(Cantor_FOUND)
    set(CANTOR_SOURCES
        ${FRONTEND_DIR}/dockwidgets/NotebookDock.cpp
        ${BACKEND_DIR}/notebook/VariableParser.cpp
        ${BACKEND_DIR}/notebook/Notebook.cpp
        ${FRONTEND_DIR}/notebook/NotebookView.cpp
    )
    set(CANTOR_UI_SOURCES ${FRONTEND_DIR}/ui/dockwidgets/notebookdock.ui)
    set(UI_SOURCES ${UI_SOURCES} ${CANTOR_UI_SOURCES})
endif()

set(BACKEND_TOOLS_SOURCES
    ${TOOLS_DIR}/ImageTools.cpp
)
set(TOOLS_SOURCES
    ${TOOLS_DIR}/ColorMapsManager.cpp
    ${TOOLS_DIR}/EquationHighlighter.cpp
    ${TOOLS_DIR}/TeXRenderer.cpp
)

bison_target(GslParser
    ${BACKEND_DIR}/gsl/parser.ypp
    ${CMAKE_CURRENT_BINARY_DIR}/gsl_parser.cpp
)
set(GENERATED_SOURCES
    ${BISON_GslParser_OUTPUTS}
)

##############################################################################
INCLUDE_DIRECTORIES(. ${BACKEND_DIR}/gsl ${GSL_INCLUDE_DIR})
set(LABPLOT_SRCS ${GUI_SOURCES})
ki18n_wrap_ui(LABPLOT_SRCS ${UI_SOURCES})

# static library
add_library(labplotnsllib STATIC ${NSL_SOURCES})
add_library(labplotbackendlib STATIC ${BACKEND_SOURCES} ${UI_SOURCES} ${BACKEND_TOOLS_SOURCES} ${GENERATED_SOURCES})
add_library(labplotlib STATIC ${LABPLOT_SRCS} ${CANTOR_SOURCES} ${DATASOURCES_SOURCES} ${FRONTEND_SOURCES} ${TOOLS_SOURCES} ${QTMOC_HDRS})
# set_property(TARGET ${objlib} PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(labplotbackendlib PUBLIC ${FRONTEND_DIR}/widgets) # so that the MOC finds numberSpinbox.h
target_include_directories(labplotlib PUBLIC ${FRONTEND_DIR}/widgets) # so that the MOC finds numberSpinbox.h

# add system QADS include path
if(qt${QT_VERSION_MAJOR}advanceddocking_FOUND)
    get_target_property(QADS_INCLUDES ads::qt${QT_VERSION_MAJOR}advanceddocking INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(QADS_LIBRARIES ads::qt${QT_VERSION_MAJOR}advanceddocking INTERFACE_LINK_LIBRARIES)
    message(STATUS "QADS Found LINK_LIBRARIES: ${QADS_LIBRARIES} INCLUDES: ${QADS_INCLUDES}")
    target_include_directories(labplotbackendlib PUBLIC ${QADS_INCLUDES})
    target_include_directories(labplotlib PUBLIC ${QADS_INCLUDES})
endif()
target_compile_definitions(labplotbackendlib PRIVATE ${LABPLOT_COMPILE_DEFINITIONS})
target_compile_definitions(labplotlib PRIVATE ${LABPLOT_COMPILE_DEFINITIONS})

if(READSTAT_FOUND)   # system readstat needs libiconv
    find_library(ICONV_LIBRARIES NAMES iconv libiconv libconv-2)
    if(ICONV_LIBRARIES)
        message(status "FOUND iconv: ${ICONV_LIBRARIES}. Adding with --no-as-needed")
    endif()
endif()
if(NOT ICONV_LIBRARIES)
    set(ICONV_LIBRARIES "")
endif()
if(!APPLE)
    set(TURN_OFF_AS_NEEDED "-Wl,--no-as-needed")
    set(TURN_ON_AS_NEEDED "-Wl,--as-needed")
endif()
target_link_libraries(labplotnsllib
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Widgets    # QApplication
    ${TURN_OFF_AS_NEEDED}
    ${GSL_LIBRARIES}
    ${ICONV_LIBRARIES}
    ${TURN_ON_AS_NEEDED}
    ${LIBCERF_LIBRARIES}
    ${FFTW3_LIBRARIES}
)
target_link_libraries(labplotbackendlib
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui    # QColor
    Qt${QT_MAJOR_VERSION}::Widgets    # QApplication
    Qt${QT_MAJOR_VERSION}::Network    # QLocalSocket
    Qt${QT_MAJOR_VERSION}::Xml    # QDomElement (Cantor)
    Qt${QT_MAJOR_VERSION}::PrintSupport    # QPrintDialog
    KF${KF_MAJOR_VERSION}::ConfigCore    # KConfigGroup
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::ConfigWidgets    # KStandardAction
    KF${KF_MAJOR_VERSION}::Archive    # KFilterDev
    KF${KF_MAJOR_VERSION}::TextWidgets    # KTextEdit
    KF${KF_MAJOR_VERSION}::CoreAddons    # kcoreaddons_version.h
    ${ZLIB_LIBRARIES}
    ${LZ4_LIBRARIES}
    ${TURN_OFF_AS_NEEDED}
    ${GSL_LIBRARIES}
    ${TURN_ON_AS_NEEDED}
    ${HDF5_LIBRARIES}
    ${FFTW3_LIBRARIES}
    ${netCDF_LIBRARIES}
    ${CFITSIO_LIBRARIES}
    ${LIBCERF_LIBRARIES}
    ${MATIO_LIBRARIES}
    ads::qt${QT_VERSION_MAJOR}advanceddocking
)
target_link_libraries(labplotlib
    Qt${QT_MAJOR_VERSION}::Sql
    Qt${QT_MAJOR_VERSION}::PrintSupport    # QPrinter
#     Qt${QT_MAJOR_VERSION}::Qml
#     Qt${QT_MAJOR_VERSION}::Quick
#     Qt${QT_MAJOR_VERSION}::QuickWidgets
    KF${KF_MAJOR_VERSION}::Completion
    KF${KF_MAJOR_VERSION}::Crash
    KF${KF_MAJOR_VERSION}::IconThemes
    KF${KF_MAJOR_VERSION}::KIOCore
    KF${KF_MAJOR_VERSION}::KIOFileWidgets
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KF${KF_MAJOR_VERSION}::XmlGui
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Archive    # KFilterDev
    KF${KF_MAJOR_VERSION}::TextWidgets    # KTextEdit
    ${Poppler_LIBRARIES}
    ads::qt${QT_VERSION_MAJOR}advanceddocking
)

if(Qt${QT_MAJOR_VERSION}SerialPort_FOUND)
    target_link_libraries(labplotbackendlib Qt${QT_MAJOR_VERSION}::SerialPort)
    target_link_libraries(labplotlib Qt${QT_MAJOR_VERSION}::SerialPort)
endif()
if(Qt${QT_MAJOR_VERSION}Svg_FOUND)
    target_link_libraries(labplotlib Qt${QT_MAJOR_VERSION}::Svg)
endif()
if(Qt${QT_MAJOR_VERSION}Mqtt_FOUND)
    target_link_libraries(labplotbackendlib Qt${QT_MAJOR_VERSION}::Mqtt)
    target_link_libraries(labplotlib Qt${QT_MAJOR_VERSION}::Mqtt)
endif()
if(KF${KF_MAJOR_VERSION}SyntaxHighlighting_FOUND)
    target_link_libraries(labplotlib KF${KF_MAJOR_VERSION}::SyntaxHighlighting)
endif()
if(KF${KF_MAJOR_VERSION}UserFeedback_FOUND)
    target_link_libraries(labplotlib KF${KF_MAJOR_VERSION}::UserFeedbackCore KF${KF_MAJOR_VERSION}::UserFeedbackWidgets)
endif()
if(Discount_FOUND)
    target_link_libraries(labplotlib Discount::Lib)
endif()
if(Orcus_FOUND)
    target_link_libraries(labplotlib ${Orcus_LIBRARIES} ${Ixion_LIBRARY})
endif()
if(KF${KF_MAJOR_VERSION}NewStuff_FOUND)
	target_link_libraries(labplotlib KF${KF_MAJOR_VERSION}::NewStuffWidgets)
endif()
if(KF${KF_MAJOR_VERSION}Purpose_FOUND)
    target_link_libraries(labplotlib KF${KF_MAJOR_VERSION}::PurposeWidgets)
endif()
if(ZSTD_FOUND)
    target_link_libraries(labplotbackendlib ${ZSTD_LIBRARIES})
endif()

if(Cantor_FOUND)
    # KFX::Service, KFX::Parts is optional but probably required by Cantor
    target_link_libraries(labplotbackendlib Cantor::cantorlibs KF${KF_MAJOR_VERSION}::Service KF${KF_MAJOR_VERSION}::Parts)
    target_link_libraries(labplotlib Cantor::cantorlibs KF${KF_MAJOR_VERSION}::Service KF${KF_MAJOR_VERSION}::Parts)
endif()

if(ENABLE_READSTAT)
    # build before using
    if(BUILD_READSTAT)
        add_dependencies(labplotbackendlib 3rdparty_readstat)
    endif()
    if(READSTAT_FOUND)
        target_link_libraries(labplotbackendlib ${READSTAT_LIBRARIES} ${ZLIB_LIBRARIES})
    endif()
endif()

if(ENABLE_XLSX)
    # build before use
    if(BUILD_QXLSX)
        target_include_directories(labplotbackendlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QXlsx/header")
        add_dependencies(labplotbackendlib QXlsx)
        target_include_directories(labplotlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QXlsx/header")
        add_dependencies(labplotlib QXlsx)
    endif()

    if(QXLSX_FOUND)
        target_include_directories(labplotbackendlib PUBLIC ${QXLSX_INCLUDE_DIR})
        target_include_directories(labplotlib PUBLIC ${QXLSX_INCLUDE_DIR})
    endif()

    if(QXLSX_FOUND OR BUILD_QXLSX)
    target_link_libraries(labplotbackendlib QXlsx)
    endif()
endif()

if(ENABLE_LIBORIGIN)
    if(LIBORIGIN_FOUND)
        # ifsystem headers are missing: use the provided
        target_include_directories(labplotbackendlib PUBLIC ${LIBORIGIN_INCLUDE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liborigin")
        target_link_libraries(labplotbackendlib ${LIBORIGIN_LIBRARIES})
        target_include_directories(labplotlib PUBLIC ${LIBORIGIN_INCLUDE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liborigin")
    else() # own version
        target_include_directories(labplotbackendlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liborigin")
        target_link_libraries(labplotbackendlib liborigin-static)
        target_include_directories(labplotlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liborigin")
    endif()
endif()

if(LZ4_FOUND)
    target_include_directories(labplotbackendlib PUBLIC ${LZ4_INCLUDE_DIR})
endif()

if(PSAPI)
    target_link_libraries(labplotlib ${PSAPI})
endif()

if(APPLE AND ENABLE_TOUCHBAR)
    target_link_libraries(labplotlib KDMacTouchBar)
endif()


# icons for the executable and project files on Windows and Mac OS X

set(LABPLOT_ICONS
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/16-apps-labplot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/32-apps-labplot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/48-apps-labplot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/64-apps-labplot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/128-apps-labplot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/256-apps-labplot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/512-apps-labplot.png
)
set(LML_ICONS
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/16-application-x-labplot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/32-application-x-labplot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/48-application-x-labplot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/64-application-x-labplot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/128-application-x-labplot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/256-application-x-labplot.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/512-application-x-labplot.png
)

add_subdirectory(3rdparty)

# main executable
set(LABPLOT_SOURCE ${FRONTEND_DIR}/LabPlot.cpp)

# create icon files on WIN/MAC and add icons to the executable
if(${ECM_VERSION} VERSION_GREATER "5.48.0")
    # creates LABPLOT_ICONS.ico/LABPLOT_ICONS.icns
    ecm_add_app_icon(LABPLOT_SOURCE ICONS ${LABPLOT_ICONS} OUTFILE_BASENAME LABPLOT_ICONS)
else()
    # creates LABPLOT_SOURCE.ico/LABPLOT_SOURCE.icns
    ecm_add_app_icon(LABPLOT_SOURCE ICONS ${LABPLOT_ICONS})
endif()

# create LML_ICONS.icns on MACOSX
if(APPLE AND ${ECM_VERSION} VERSION_GREATER "5.48.0")
    ecm_add_app_icon(LABPLOT_SOURCE ICONS ${LML_ICONS} OUTFILE_BASENAME LML_ICONS)
endif()

add_executable(labplot ${LABPLOT_SOURCE} ${GUI_QRC} ${qml_QRC})
target_link_libraries(labplot labplotlib labplotbackendlib labplotnsllib)

#if(KUserFeedbackQt6_FOUND)
#    target_link_libraries(labplot KUserFeedbackCoreQt6 KUserFeedbackWidgetsQt6)
#endif()
target_compile_definitions(labplot PRIVATE ${LABPLOT_COMPILE_DEFINITIONS})

if(APPLE) # Apple app package
    set_target_properties(labplot PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
    set_target_properties(labplot PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "LabPlot"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${labplot_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${labplot_VERSION}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.labplot")
endif()

############## installation ################################
install(TARGETS labplot ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${FRONTEND_DIR}/splash.png ${FRONTEND_DIR}/labplot.ico ${CMAKE_CURRENT_SOURCE_DIR}/../icons/application-x-labplot.ico DESTINATION ${KDE_INSTALL_DATADIR}/${PROJECT_NAME})
install(FILES org.kde.labplot.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES labplot.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
# if(${ECM_VERSION} VERSION_GREATER "5.58.0")
#     install(FILES labplot2_themes.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
#     install(FILES labplot2_datasets.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
# else ()
#     install(FILES labplot2_themes.knsrc DESTINATION ${KDE_INSTALL_CONFDIR})
#     install(FILES labplot2_datasets.knsrc DESTINATION ${KDE_INSTALL_CONFDIR})
# endif()
update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})
