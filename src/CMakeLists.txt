find_package(SharedMimeInfo REQUIRED)
set(KDE_FRONTEND true)
set(KDEFRONTEND_DIR kdefrontend)
set(BACKEND_DIR backend)
set(COMMONFRONTEND_DIR commonfrontend)
set(CANTOR_DIR cantor)
set(TOOLS_DIR tools)
set(CMAKE_AUTOMOC ON)
set(SRC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

# do not process this file
set_property(SOURCE gsl_parser.h PROPERTY SKIP_AUTOMOC ON)

set(GUI_SOURCES
    ${KDEFRONTEND_DIR}/DatasetModel.cpp
	${KDEFRONTEND_DIR}/GuiObserver.cpp
	${KDEFRONTEND_DIR}/GuiTools.cpp
	${KDEFRONTEND_DIR}/HistoryDialog.cpp
	${KDEFRONTEND_DIR}/MainWin.cpp
	${KDEFRONTEND_DIR}/SettingsDialog.cpp
	${KDEFRONTEND_DIR}/SettingsGeneralPage.cpp
	${KDEFRONTEND_DIR}/SettingsSpreadsheetPage.cpp
	${KDEFRONTEND_DIR}/SettingsWorksheetPage.cpp
# 	${KDEFRONTEND_DIR}/SettingsWelcomePage.cpp
	${KDEFRONTEND_DIR}/SettingsDatasetsPage.cpp
	${KDEFRONTEND_DIR}/SettingsPage.h
	${KDEFRONTEND_DIR}/TemplateHandler.cpp
	${KDEFRONTEND_DIR}/ThemeHandler.cpp
# 	${KDEFRONTEND_DIR}/welcomescreen/WelcomeScreenHelper.cpp
	${KDEFRONTEND_DIR}/datasources/AsciiOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/BinaryOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/DatabaseManagerDialog.cpp
	${KDEFRONTEND_DIR}/datasources/DatabaseManagerWidget.cpp
	${KDEFRONTEND_DIR}/datasources/DatasetMetadataManagerDialog.cpp
	${KDEFRONTEND_DIR}/datasources/DatasetMetadataManagerWidget.cpp
	${KDEFRONTEND_DIR}/datasources/HDF5OptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/FileInfoDialog.cpp
	${KDEFRONTEND_DIR}/datasources/ImageOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/ImportDatasetWidget.cpp
	${KDEFRONTEND_DIR}/datasources/ImportDatasetDialog.cpp
	${KDEFRONTEND_DIR}/datasources/ImportDialog.cpp
	${KDEFRONTEND_DIR}/datasources/ImportFileWidget.cpp
	${KDEFRONTEND_DIR}/datasources/ImportFileDialog.cpp
	${KDEFRONTEND_DIR}/datasources/ImportProjectDialog.cpp
	${KDEFRONTEND_DIR}/datasources/ImportSQLDatabaseDialog.cpp
	${KDEFRONTEND_DIR}/datasources/ImportSQLDatabaseWidget.cpp
	${KDEFRONTEND_DIR}/datasources/NetCDFOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/ROOTOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/FITSOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/JsonOptionsWidget.cpp
	${KDEFRONTEND_DIR}/dockwidgets/BaseDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/AxisDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/CursorDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/ImageDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/NoteDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/CartesianPlotDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/CartesianPlotLegendDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/CorrelationCoefficientDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/HistogramDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/HypothesisTestDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/CustomPointDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/ColumnDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/LiveDataDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/MatrixDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/PivotTableDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/ProjectDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/ReferenceLineDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/SpreadsheetDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYEquationCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYDataReductionCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYDifferentiationCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYIntegrationCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYInterpolationCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYSmoothCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYFitCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYFourierFilterCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYFourierTransformCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYConvolutionCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYCorrelationCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/WorksheetDock.cpp
        ${KDEFRONTEND_DIR}/generalTest/CorrelationCoefficientView.cpp
	${KDEFRONTEND_DIR}/generalTest/GeneralTestView.cpp
        ${KDEFRONTEND_DIR}/generalTest/HypothesisTestView.cpp
	${KDEFRONTEND_DIR}/matrix/MatrixFunctionDialog.cpp
	${KDEFRONTEND_DIR}/pivot/HierarchicalHeaderView.cpp
	${KDEFRONTEND_DIR}/pivot/PivotTableView.cpp
	${KDEFRONTEND_DIR}/spreadsheet/PlotDataDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/EquidistantValuesDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/ExportSpreadsheetDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/AddSubtractValueDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/DropValuesDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/GoToDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/FunctionValuesDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/RandomValuesDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/RescaleDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/SortDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/StatisticsDialog.cpp
	${KDEFRONTEND_DIR}/worksheet/ExportWorksheetDialog.cpp
	${KDEFRONTEND_DIR}/worksheet/GridDialog.cpp
	${KDEFRONTEND_DIR}/worksheet/DynamicPresenterWidget.cpp
	${KDEFRONTEND_DIR}/worksheet/PresenterWidget.cpp
	${KDEFRONTEND_DIR}/worksheet/SlidingPanel.cpp
	${KDEFRONTEND_DIR}/widgets/ConstantsWidget.cpp
	${KDEFRONTEND_DIR}/widgets/ThemesComboBox.cpp
	${KDEFRONTEND_DIR}/widgets/ThemesWidget.cpp
	${KDEFRONTEND_DIR}/widgets/ExpressionTextEdit.cpp
	${KDEFRONTEND_DIR}/widgets/FitOptionsWidget.cpp
	${KDEFRONTEND_DIR}/widgets/FitParametersWidget.cpp
	${KDEFRONTEND_DIR}/widgets/FunctionsWidget.cpp
	${KDEFRONTEND_DIR}/widgets/LabelWidget.cpp
	${KDEFRONTEND_DIR}/widgets/DatapickerImageWidget.cpp
	${KDEFRONTEND_DIR}/widgets/DatapickerCurveWidget.cpp
	${KDEFRONTEND_DIR}/widgets/FITSHeaderEditWidget.cpp
	${KDEFRONTEND_DIR}/widgets/FITSHeaderEditNewKeywordDialog.cpp
	${KDEFRONTEND_DIR}/widgets/FITSHeaderEditAddUnitDialog.cpp
	${KDEFRONTEND_DIR}/widgets/FITSHeaderEditDialog.cpp
	${KDEFRONTEND_DIR}/widgets/ResizableTextEdit.cpp
)

if (Qt5Mqtt_FOUND)
	list(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/datasources/MQTTConnectionManagerWidget.cpp)
	list(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/datasources/MQTTConnectionManagerDialog.cpp)
	list(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/datasources/MQTTErrorWidget.cpp)
	list(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/datasources/MQTTSubscriptionWidget.cpp)
	list(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/widgets/MQTTWillSettingsWidget.cpp)
endif()

if(APPLE)
  list(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/worksheet/PresenterWidget_mac.mm)
  list(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/worksheet/DynamicPresenterWidget_mac.mm)
endif()


set(UI_SOURCES
	${KDEFRONTEND_DIR}/ui/constantswidget.ui
	${KDEFRONTEND_DIR}/ui/functionswidget.ui
	${KDEFRONTEND_DIR}/ui/fitoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/fitparameterswidget.ui
	${KDEFRONTEND_DIR}/ui/labelwidget.ui
	${KDEFRONTEND_DIR}/ui/settingsgeneralpage.ui
	${KDEFRONTEND_DIR}/ui/settingsspreadsheetpage.ui
	${KDEFRONTEND_DIR}/ui/settingsworksheetpage.ui
# 	${KDEFRONTEND_DIR}/ui/settingswelcomepage.ui
	${KDEFRONTEND_DIR}/ui/settingsdatasetspage.ui
	${KDEFRONTEND_DIR}/ui/settingsprintingpage.ui
	${KDEFRONTEND_DIR}/ui/datasources/asciioptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/binaryoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/databasemanagerwidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/datasetmetadatamanagerwidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/hdf5optionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/imageoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/importdatasetwidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/importfilewidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/importprojectwidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/importsqldatabasewidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/netcdfoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/rootoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/fitsoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/jsonoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/axisdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/cursordock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/cartesianplotdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/cartesianplotlegenddock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/histogramdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/columndock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/custompointdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/correlationcoefficientdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/hypothesistestdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/imagedock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/livedatadock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/notedock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/matrixdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/pivottabledock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/projectdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/referencelinedock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/spreadsheetdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xycurvedock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xycurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xydatareductioncurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xydifferentiationcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyintegrationcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyinterpolationcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xysmoothcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyfitcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyfourierfiltercurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyfouriertransformcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyconvolutioncurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xycorrelationcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyequationcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/worksheetdock.ui
	${KDEFRONTEND_DIR}/ui/matrix/matrixfunctionwidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/plotdatawidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/equidistantvalueswidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/exportspreadsheetwidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/addsubtractvaluewidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/dropvalueswidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/functionvalueswidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/randomvalueswidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/rescalewidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/sortdialogwidget.ui
	${KDEFRONTEND_DIR}/ui/worksheet/exportworksheetwidget.ui
	${KDEFRONTEND_DIR}/ui/datapickerimagewidget.ui
	${KDEFRONTEND_DIR}/ui/datapickercurvewidget.ui
	${KDEFRONTEND_DIR}/ui/fitsheadereditwidget.ui
	${KDEFRONTEND_DIR}/ui/fitsheadereditnewkeywordwidget.ui
	${KDEFRONTEND_DIR}/ui/fitsheadereditaddunitwidget.ui
)

if (Qt5Mqtt_FOUND)
	list(APPEND UI_SOURCES ${KDEFRONTEND_DIR}/ui/datasources/mqttconnectionmanagerwidget.ui)
	list(APPEND UI_SOURCES ${KDEFRONTEND_DIR}/ui/datasources/mqtterrorwidget.ui)
	list(APPEND UI_SOURCES ${KDEFRONTEND_DIR}/ui/datasources/mqttsubscriptionwidget.ui)
	list(APPEND UI_SOURCES ${KDEFRONTEND_DIR}/ui/mqttwillsettingswidget.ui)
endif()

set(BACKEND_SOURCES
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
	${BACKEND_DIR}/datasources/AbstractDataSource.cpp
	${BACKEND_DIR}/datasources/DatasetHandler.cpp
	${BACKEND_DIR}/datasources/LiveDataSource.cpp
	${BACKEND_DIR}/datasources/filters/AbstractFileFilter.cpp
	${BACKEND_DIR}/datasources/filters/AsciiFilter.cpp
	${BACKEND_DIR}/datasources/filters/BinaryFilter.cpp
	${BACKEND_DIR}/datasources/filters/HDF5Filter.cpp
	${BACKEND_DIR}/datasources/filters/ImageFilter.cpp
	${BACKEND_DIR}/datasources/filters/JsonFilter.cpp
	${BACKEND_DIR}/datasources/filters/NetCDFFilter.cpp
	${BACKEND_DIR}/datasources/filters/NgspiceRawAsciiFilter.cpp
	${BACKEND_DIR}/datasources/filters/NgspiceRawBinaryFilter.cpp
	${BACKEND_DIR}/datasources/filters/FITSFilter.cpp
	${BACKEND_DIR}/datasources/filters/QJsonModel.cpp
	${BACKEND_DIR}/datasources/filters/ROOTFilter.cpp
	${BACKEND_DIR}/datasources/projects/ProjectParser.cpp
	${BACKEND_DIR}/datasources/projects/LabPlotProjectParser.cpp
	${BACKEND_DIR}/gsl/ExpressionParser.cpp
	${BACKEND_DIR}/generalTest/CorrelationCoefficient.cpp
	${BACKEND_DIR}/generalTest/GeneralTest.cpp
	${BACKEND_DIR}/generalTest/HypothesisTest.cpp
	${BACKEND_DIR}/generalTest/MyTableModel.cpp
	${BACKEND_DIR}/generalTest/MyTextEdit.cpp
	${BACKEND_DIR}/matrix/Matrix.cpp
	${BACKEND_DIR}/matrix/matrixcommands.cpp
	${BACKEND_DIR}/matrix/MatrixModel.cpp
	${BACKEND_DIR}/pivot/PivotTable.cpp
	${BACKEND_DIR}/spreadsheet/Spreadsheet.cpp
	${BACKEND_DIR}/spreadsheet/SpreadsheetModel.cpp
	${BACKEND_DIR}/lib/XmlStreamReader.cpp
	${BACKEND_DIR}/note/Note.cpp
	${BACKEND_DIR}/worksheet/Image.cpp
	${BACKEND_DIR}/worksheet/WorksheetElement.cpp
	${BACKEND_DIR}/worksheet/TextLabel.cpp
	${BACKEND_DIR}/worksheet/Worksheet.cpp
	${BACKEND_DIR}/worksheet/WorksheetElementContainer.cpp
	${BACKEND_DIR}/worksheet/WorksheetElementGroup.cpp
	${BACKEND_DIR}/worksheet/TreeModel.cpp
	${BACKEND_DIR}/worksheet/plots/AbstractPlot.cpp
	${BACKEND_DIR}/worksheet/plots/AbstractCoordinateSystem.cpp
	${BACKEND_DIR}/worksheet/plots/PlotArea.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/Axis.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianCoordinateSystem.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlot.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlotLegend.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/Curve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/Histogram.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CustomPoint.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/ReferenceLine.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/Symbol.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYAnalysisCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYEquationCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYDataReductionCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYDifferentiationCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYIntegrationCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYInterpolationCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYSmoothCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYFitCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYFourierFilterCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYFourierTransformCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYConvolutionCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYCorrelationCurve.cpp
	${BACKEND_DIR}/lib/SignallingUndoCommand.cpp
	${BACKEND_DIR}/datapicker/DatapickerPoint.cpp
	${BACKEND_DIR}/datapicker/DatapickerImage.cpp
	${BACKEND_DIR}/datapicker/Datapicker.cpp
	${BACKEND_DIR}/datapicker/Transform.cpp
	${BACKEND_DIR}/datapicker/ImageEditor.cpp
	${BACKEND_DIR}/datapicker/Segment.cpp
	${BACKEND_DIR}/datapicker/Segments.cpp
	${BACKEND_DIR}/datapicker/DatapickerCurve.cpp
)

if (Qt5Mqtt_FOUND)
	list(APPEND BACKEND_SOURCES ${KDEFRONTEND_DIR}/widgets/MQTTWillSettingsWidget.cpp)
	list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/datasources/MQTTClient.cpp)
	list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/datasources/MQTTSubscription.cpp)
	list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/datasources/MQTTTopic.cpp)
endif()

# add the qml.qrc file
# qt5_add_resources(qml_QRC ${KDEFRONTEND_DIR}/welcomescreen/qml.qrc)
# better: enable AUTORCC

IF (ENABLE_LIBORIGIN)
	list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/datasources/projects/OriginProjectParser.cpp)
ENDIF ()

set(NSL_SOURCES
	${BACKEND_DIR}/nsl/nsl_conv.c
	${BACKEND_DIR}/nsl/nsl_corr.c
	${BACKEND_DIR}/nsl/nsl_dft.c
	${BACKEND_DIR}/nsl/nsl_diff.c
	${BACKEND_DIR}/nsl/nsl_filter.c
	${BACKEND_DIR}/nsl/nsl_fit.c
	${BACKEND_DIR}/nsl/nsl_geom.c
	${BACKEND_DIR}/nsl/nsl_geom_linesim.c
	${BACKEND_DIR}/nsl/nsl_int.c
	${BACKEND_DIR}/nsl/nsl_interp.c
	${BACKEND_DIR}/nsl/nsl_math.c
	${BACKEND_DIR}/nsl/nsl_sf_basic.c
	${BACKEND_DIR}/nsl/nsl_sf_kernel.c
	${BACKEND_DIR}/nsl/nsl_sf_poly.c
	${BACKEND_DIR}/nsl/nsl_sf_stats.c
	${BACKEND_DIR}/nsl/nsl_sf_window.c
	${BACKEND_DIR}/nsl/nsl_smooth.c
	${BACKEND_DIR}/nsl/nsl_sort.c
	${BACKEND_DIR}/nsl/nsl_stats.c
)

IF (NOT MSVC_FOUND)
IF (NOT LIBCERF_FOUND)
	list(APPEND NSL_SOURCES
		${BACKEND_DIR}/nsl/Faddeeva.c
	)
ENDIF ()
ENDIF ()

set(COMMONFRONTEND_SOURCES
	${COMMONFRONTEND_DIR}/matrix/MatrixView.cpp
	${COMMONFRONTEND_DIR}/note/NoteView.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetCommentsHeaderModel.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetHeaderView.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetItemDelegate.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetView.cpp
	${COMMONFRONTEND_DIR}/workbook/WorkbookView.cpp
	${COMMONFRONTEND_DIR}/worksheet/WorksheetView.cpp
	${COMMONFRONTEND_DIR}/ProjectExplorer.cpp
	${COMMONFRONTEND_DIR}/core/PartMdiView.cpp
	${COMMONFRONTEND_DIR}/widgets/TreeViewComboBox.cpp
	${COMMONFRONTEND_DIR}/widgets/qxtspanslider.cpp
	${COMMONFRONTEND_DIR}/widgets/MemoryWidget.cpp
	${COMMONFRONTEND_DIR}/widgets/DateTimeSpinBox.cpp
	${COMMONFRONTEND_DIR}/datapicker/DatapickerView.cpp
	${COMMONFRONTEND_DIR}/datapicker/DatapickerImageView.cpp
)

IF (RECENT_CANTOR)
	set(CANTOR_SOURCES
		${KDEFRONTEND_DIR}/dockwidgets/CantorWorksheetDock.cpp
		${BACKEND_DIR}/cantorWorksheet/VariableParser.cpp
		${BACKEND_DIR}/cantorWorksheet/CantorWorksheet.cpp
		${COMMONFRONTEND_DIR}/cantorWorksheet/CantorWorksheetView.cpp
	)
	set(CANTOR_UI_SOURCES ${KDEFRONTEND_DIR}/ui/dockwidgets/cantorworksheetdock.ui)
	set(UI_SOURCES ${UI_SOURCES} ${CANTOR_UI_SOURCES})
ELSE ()
	set(CANTOR_SOURCES "")
ENDIF ()

set(TOOLS_SOURCES
	${TOOLS_DIR}/EquationHighlighter.cpp
	${TOOLS_DIR}/ImageTools.cpp
	${TOOLS_DIR}/TeXRenderer.cpp
)

bison_target(GslParser
	${BACKEND_DIR}/gsl/parser.y
	${CMAKE_CURRENT_BINARY_DIR}/gsl_parser.c
)
set(GENERATED_SOURCES
	${BISON_GslParser_OUTPUTS}
)

##############################################################################
INCLUDE_DIRECTORIES( . ${BACKEND_DIR}/gsl ${GSL_INCLUDE_DIR} )
set( LABPLOT_SRCS ${GUI_SOURCES} )
ki18n_wrap_ui( LABPLOT_SRCS ${UI_SOURCES} )
# see also QT_MINIMUM_VERSION
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50500)

# static library
add_library( labplot2lib STATIC ${LABPLOT_SRCS} ${BACKEND_SOURCES} ${NSL_SOURCES} ${CANTOR_SOURCES} ${DATASOURCES_SOURCES} ${COMMONFRONTEND_SOURCES} ${TOOLS_SOURCES} ${GENERATED_SOURCES} ${QTMOC_HDRS} )
# set_property(TARGET ${objlib} PROPERTY POSITION_INDEPENDENT_CODE 1)

target_link_libraries( labplot2lib
	KF5::Archive
	KF5::Completion
	KF5::ConfigCore
	KF5::Crash
	KF5::I18n
	KF5::IconThemes
	KF5::KIOCore
	KF5::KIOFileWidgets
	KF5::KIOWidgets
	KF5::TextWidgets
	KF5::XmlGui
	KF5::NewStuff
	KF5::NewStuffCore
	Qt5::Svg
	Qt5::Core
	Qt5::Network
	Qt5::PrintSupport
	Qt5::Sql
# 	Qt5::Qml
# 	Qt5::Quick
# 	Qt5::QuickWidgets
	${GSL_LIBRARIES}
	${GSL_CBLAS_LIBRARIES} )

IF (Qt5SerialPort_FOUND)
	target_link_libraries( labplot2lib Qt5::SerialPort )
ENDIF ()
IF (Qt5Mqtt_FOUND)
	target_link_libraries( labplot2lib Qt5::Mqtt )
ENDIF ()
IF (KF5SyntaxHighlighting_FOUND)
	target_link_libraries( labplot2lib KF5::SyntaxHighlighting )
ENDIF ()
if (KUserFeedback_FOUND)
    target_link_libraries(labplot2lib KUserFeedbackWidgets)
endif()
#TODO: KF5::NewStuff

IF (RECENT_CANTOR)
	# KF5::Service, KF5::Parts is optional but probably required by Cantor
	target_link_libraries( labplot2lib Cantor::cantorlibs KF5::Service KF5::Parts Poppler::Qt5)
ENDIF ()
IF (HDF5_FOUND)
	target_link_libraries( labplot2lib ${HDF5_LIBRARIES} )
ENDIF ()
IF (FFTW3_FOUND)
	target_link_libraries( labplot2lib ${FFTW3_LIBRARIES} )
ENDIF ()
IF (netCDF_FOUND)
	target_link_libraries( labplot2lib ${netCDF_LIBRARIES} )
ENDIF ()
IF (CFITSIO_FOUND)
	target_link_libraries( labplot2lib ${CFITSIO_LIBRARIES} )
ENDIF ()
IF (LIBCERF_FOUND)
	target_link_libraries( labplot2lib ${LIBCERF_LIBRARY} )
ENDIF ()
IF (ZLIB_FOUND AND LZ4_FOUND)
	target_link_libraries( labplot2lib ${ZLIB_LIBRARY} ${LZ4_LIBRARY} )
ENDIF ()
IF (ENABLE_LIBORIGIN)
	target_link_libraries( labplot2lib liborigin-static )
ENDIF ()
IF (WIN32)
	target_link_libraries( labplot2lib ${PSAPI} )
ENDIF ()

# icons for the executable and project files on Windows and Mac OS X

set(LABPLOT_ICONS
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/16-apps-labplot2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/32-apps-labplot2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/48-apps-labplot2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/64-apps-labplot2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/128-apps-labplot2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/256-apps-labplot2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/512-apps-labplot2.png
)
set(LML_ICONS
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/16-application-x-labplot2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/32-application-x-labplot2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/48-application-x-labplot2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/64-application-x-labplot2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/128-application-x-labplot2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/256-application-x-labplot2.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../icons/512-application-x-labplot2.png
)

add_subdirectory(3rdparty)

# main executable
set(LABPLOT_SOURCE ${KDEFRONTEND_DIR}/LabPlot.cpp)

# create icon files on WIN/MAC and add icons to the executable
IF (${ECM_VERSION} VERSION_GREATER "5.48.0")
	# creates LABPLOT_ICONS.ico/LABPLOT_ICONS.icns
	ecm_add_app_icon(LABPLOT_SOURCE ICONS ${LABPLOT_ICONS} OUTFILE_BASENAME LABPLOT_ICONS)
ELSE ()
	# creates LABPLOT_SOURCE.ico/LABPLOT_SOURCE.icns
	ecm_add_app_icon(LABPLOT_SOURCE ICONS ${LABPLOT_ICONS})
ENDIF ()

# create LML_ICONS.icns on MACOSX
IF (APPLE AND ${ECM_VERSION} VERSION_GREATER "5.48.0")
	ecm_add_app_icon(LABPLOT_SOURCE ICONS ${LML_ICONS} OUTFILE_BASENAME LML_ICONS)
ENDIF ()

add_executable(labplot2 ${LABPLOT_SOURCE} ${qml_QRC})
target_link_libraries(labplot2 labplot2lib)
IF (APPLE)
	target_link_libraries(labplot2 KDMacTouchBar)
ENDIF()

############## installation ################################
install( TARGETS labplot2 DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES ${KDEFRONTEND_DIR}/labplot2ui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/${PROJECT_NAME} )
install( FILES ${KDEFRONTEND_DIR}/splash.png ${KDEFRONTEND_DIR}/labplot2.ico ${CMAKE_CURRENT_SOURCE_DIR}/../icons/application-x-labplot2.ico DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME} )
install( PROGRAMS org.kde.labplot2.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES labplot2.xml DESTINATION ${XDG_MIME_INSTALL_DIR} )
# if (${ECM_VERSION} STRGREATER "5.58.0")
# 	install( FILES labplot2_themes.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR} )
# 	install( FILES labplot2_datasets.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR} )
# else ()
# 	install( FILES labplot2_themes.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )
# 	install( FILES labplot2_datasets.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )
# endif ()
update_xdg_mimetypes( ${XDG_MIME_INSTALL_DIR} )
