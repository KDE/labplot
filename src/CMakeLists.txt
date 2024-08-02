find_package(SharedMimeInfo REQUIRED)
set(KDE_FRONTEND true)
set(KDEFRONTEND_DIR kdefrontend)
set(BACKEND_DIR backend)
set(COMMONFRONTEND_DIR commonfrontend)
set(CANTOR_DIR cantor)
set(TOOLS_DIR tools)
set(CMAKE_AUTOMOC ON)
set(SRC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

find_package(qt${QT_VERSION_MAJOR}advanceddocking CONFIG)
if(NOT qt${QT_VERSION_MAJOR}advanceddocking_FOUND)
    option(LOCAL_QT_ADVANCED_DOCK_DOWNLOAD "Use local build of qt advanced dock downloaded directly from the git repository" OFF)
    option(LOCAL_QT_ADVANCED_DOCK_COPY "Use local build of qt advanced dock copied into the Qt-Advanced-Docking-System subfolder" ON)
    if(TARGET Qt${QT_VERSION_MAJOR}::GuiPrivate)
        message(STATUS "Building local version of Qt-Advanced-Docking-System.")
    else ()
        message(FATAL_ERROR "Missing Qt${QT_VERSION_MAJOR}::GuiPrivate to build local version of Qt-Advanced-Docking-System.")
    endif()
endif()

set(GUI_SOURCES
    ${KDEFRONTEND_DIR}/DatasetModel.cpp
    ${KDEFRONTEND_DIR}/GuiObserver.cpp
    ${KDEFRONTEND_DIR}/PlotTemplateDialog.cpp
    ${KDEFRONTEND_DIR}/GuiTools.cpp
    ${KDEFRONTEND_DIR}/HistoryDialog.cpp
    ${KDEFRONTEND_DIR}/MainWin.cpp
    ${KDEFRONTEND_DIR}/SettingsDialog.cpp
    ${KDEFRONTEND_DIR}/SettingsGeneralPage.cpp
    ${KDEFRONTEND_DIR}/SettingsNotebookPage.cpp
    ${KDEFRONTEND_DIR}/SettingsSpreadsheetPage.cpp
    ${KDEFRONTEND_DIR}/SettingsWorksheetPage.cpp
#     ${KDEFRONTEND_DIR}/SettingsWelcomePage.cpp
    ${KDEFRONTEND_DIR}/SettingsDatasetsPage.cpp
    ${KDEFRONTEND_DIR}/SettingsPage.h
    ${KDEFRONTEND_DIR}/CASSettingsDialog.cpp
    ${KDEFRONTEND_DIR}/TemplateHandler.cpp
    ${KDEFRONTEND_DIR}/ThemeHandler.cpp
    ${KDEFRONTEND_DIR}/examples/ExamplesDialog.cpp
    ${KDEFRONTEND_DIR}/examples/ExamplesManager.cpp
    ${KDEFRONTEND_DIR}/examples/ExamplesWidget.cpp
#     ${KDEFRONTEND_DIR}/welcomescreen/WelcomeScreenHelper.cpp
    ${KDEFRONTEND_DIR}/colormaps/ColorMapsWidget.cpp
    ${KDEFRONTEND_DIR}/colormaps/ColorMapsDialog.cpp
    ${KDEFRONTEND_DIR}/datasources/AsciiOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/CANOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/BinaryOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/DatabaseManagerDialog.cpp
    ${KDEFRONTEND_DIR}/datasources/DatabaseManagerWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/DatasetMetadataManagerDialog.cpp
    ${KDEFRONTEND_DIR}/datasources/DatasetMetadataManagerWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/OdsOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/XLSXOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/HDF5OptionsWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/ImageOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/ImportDatasetWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/ImportDatasetDialog.cpp
    ${KDEFRONTEND_DIR}/datasources/ImportDialog.cpp
    ${KDEFRONTEND_DIR}/datasources/ImportWarningsDialog.cpp
    ${KDEFRONTEND_DIR}/datasources/ImportFileWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/ImportFileDialog.cpp
    ${KDEFRONTEND_DIR}/datasources/ImportKaggleDatasetWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/ImportKaggleDatasetDialog.cpp
    ${KDEFRONTEND_DIR}/datasources/ImportProjectDialog.cpp
    ${KDEFRONTEND_DIR}/datasources/ImportSQLDatabaseDialog.cpp
    ${KDEFRONTEND_DIR}/datasources/ImportSQLDatabaseWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/MatioOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/NetCDFOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/ROOTOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/FITSOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/JsonOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/datasources/McapOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/BaseDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/AspectDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/AxisDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/CursorDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/ImageDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/InfoElementDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/NoteDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/CartesianPlotDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/CartesianPlotLegendDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/HistogramDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/BarPlotDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/BoxPlotDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/CustomPointDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/ColumnDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/KDEPlotDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/LiveDataDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/LollipopPlotDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/MatrixDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/ProjectDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/ReferenceLineDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/ReferenceRangeDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/SpreadsheetDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/StatisticsSpreadsheetDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/QQPlotDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYAnalysisCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYEquationCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYFunctionCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYDataReductionCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYDifferentiationCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYIntegrationCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYInterpolationCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYSmoothCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYFitCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYFourierFilterCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYFourierTransformCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYHilbertTransformCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYConvolutionCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/XYCorrelationCurveDock.cpp
    ${KDEFRONTEND_DIR}/dockwidgets/WorksheetDock.cpp
    ${KDEFRONTEND_DIR}/matrix/MatrixFunctionDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/PlotDataDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/EquidistantValuesDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/ExportSpreadsheetDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/AddSubtractValueDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/AddValueLabelDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/BatchEditValueLabelsDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/DropValuesDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/FlattenColumnsDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/FormattingHeatmapDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/GoToDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/FunctionValuesDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/RandomValuesDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/RescaleDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/SampleValuesDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/SearchReplaceWidget.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/SortDialog.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/StatisticsColumnWidget.cpp
    ${KDEFRONTEND_DIR}/spreadsheet/StatisticsDialog.cpp
    ${KDEFRONTEND_DIR}/worksheet/ExportWorksheetDialog.cpp
    ${KDEFRONTEND_DIR}/worksheet/GridDialog.cpp
    ${KDEFRONTEND_DIR}/worksheet/PresenterWidget.cpp
    ${KDEFRONTEND_DIR}/worksheet/SlidingPanel.cpp
    ${KDEFRONTEND_DIR}/worksheet/WorksheetPreviewWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/BackgroundWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/ConstantsWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/ThemesComboBox.cpp
    ${KDEFRONTEND_DIR}/widgets/ThemesWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/ExpressionTextEdit.cpp
    ${KDEFRONTEND_DIR}/widgets/FitOptionsWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/FitParametersWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/FunctionsWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/LabelWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/ErrorBarWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/LineWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/SymbolWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/ValueWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/DatapickerImageWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/DatapickerCurveWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/FITSHeaderEditWidget.cpp
    ${KDEFRONTEND_DIR}/widgets/FITSHeaderEditNewKeywordDialog.cpp
    ${KDEFRONTEND_DIR}/widgets/FITSHeaderEditAddUnitDialog.cpp
    ${KDEFRONTEND_DIR}/widgets/FITSHeaderEditDialog.cpp
    ${KDEFRONTEND_DIR}/widgets/ResizableTextEdit.cpp
    ${KDEFRONTEND_DIR}/widgets/TimedLineEdit.cpp
)

if(Qt${QT_VERSION_MAJOR}Mqtt_FOUND)
    list(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/datasources/MQTTConnectionManagerWidget.cpp)
    list(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/datasources/MQTTConnectionManagerDialog.cpp)
    list(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/datasources/MQTTErrorWidget.cpp)
    list(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/datasources/MQTTSubscriptionWidget.cpp)
endif()

if(APPLE)
  list(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/worksheet/PresenterWidget_mac.mm)
endif()

set(GUI_QRC
    ${KDEFRONTEND_DIR}/labplot2.qrc
)

set(UI_SOURCES
    ${KDEFRONTEND_DIR}/ui/colormapswidget.ui
    ${KDEFRONTEND_DIR}/ui/constantswidget.ui
    ${KDEFRONTEND_DIR}/ui/PlotTemplateDialog.ui
    ${KDEFRONTEND_DIR}/ui/exampleswidget.ui
    ${KDEFRONTEND_DIR}/ui/functionswidget.ui
    ${KDEFRONTEND_DIR}/ui/fitoptionswidget.ui
    ${KDEFRONTEND_DIR}/ui/fitparameterswidget.ui
    ${KDEFRONTEND_DIR}/ui/labelwidget.ui
    ${KDEFRONTEND_DIR}/ui/symbolwidget.ui
    ${KDEFRONTEND_DIR}/ui/settingsnotebookpage.ui
    ${KDEFRONTEND_DIR}/ui/settingsgeneralpage.ui
    ${KDEFRONTEND_DIR}/ui/settingsspreadsheetpage.ui
    ${KDEFRONTEND_DIR}/ui/settingsworksheetpage.ui
#     ${KDEFRONTEND_DIR}/ui/settingswelcomepage.ui
    ${KDEFRONTEND_DIR}/ui/settingsdatasetspage.ui
    ${KDEFRONTEND_DIR}/ui/settingsprintingpage.ui
    ${KDEFRONTEND_DIR}/ui/datasources/asciioptionswidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/CANOptionsWidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/binaryoptionswidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/databasemanagerwidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/datasetmetadatamanagerwidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/hdf5optionswidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/imageoptionswidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/importdatasetwidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/importkaggledatasetwidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/importfilewidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/importwarningsdialog.ui
    ${KDEFRONTEND_DIR}/ui/datasources/importprojectwidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/importsqldatabasewidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/matiooptionswidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/netcdfoptionswidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/rootoptionswidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/fitsoptionswidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/jsonoptionswidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/mcapoptionswidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/odsoptionswidget.ui
    ${KDEFRONTEND_DIR}/ui/datasources/xlsxoptionswidget.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/aspectdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/axisdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/backgroundwidget.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/cursordock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/infoelementdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/cartesianplotdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/cartesianplotlegenddock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/histogramdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/barplotdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/boxplotdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/columndock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/custompointdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/errorbarwidget.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/imagedock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/kdeplotdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/linewidget.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/livedatadock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/lollipopplotdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/notedock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/matrixdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/projectdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/referencelinedock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/referencerangedock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/spreadsheetdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/statisticsspreadsheetdock.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/qqplotdock.ui
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
    ${KDEFRONTEND_DIR}/ui/dockwidgets/xyhilberttransformcurvedockgeneraltab.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/xyconvolutioncurvedockgeneraltab.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/xycorrelationcurvedockgeneraltab.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/xyequationcurvedockgeneraltab.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/xyfunctioncurvedockgeneraltab.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/valuewidget.ui
    ${KDEFRONTEND_DIR}/ui/dockwidgets/worksheetdock.ui
    ${KDEFRONTEND_DIR}/ui/matrix/matrixfunctionwidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/plotdatawidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/equidistantvalueswidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/exportspreadsheetwidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/addsubtractvaluewidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/dropvalueswidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/flattencolumnswidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/formattingheatmapwidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/functionvalueswidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/randomvalueswidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/rescalewidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/samplevalueswidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/searchwidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/searchreplacewidget.ui
    ${KDEFRONTEND_DIR}/ui/spreadsheet/sortdialogwidget.ui
    ${KDEFRONTEND_DIR}/ui/worksheet/exportworksheetwidget.ui
    ${KDEFRONTEND_DIR}/ui/worksheet/worksheetpreviewwidget.ui
    ${KDEFRONTEND_DIR}/ui/datapickerimagewidget.ui
    ${KDEFRONTEND_DIR}/ui/datapickercurvewidget.ui
    ${KDEFRONTEND_DIR}/ui/fitsheadereditwidget.ui
    ${KDEFRONTEND_DIR}/ui/fitsheadereditnewkeywordwidget.ui
    ${KDEFRONTEND_DIR}/ui/fitsheadereditaddunitwidget.ui
)

if(Qt${QT_VERSION_MAJOR}Mqtt_FOUND)
    list(APPEND UI_SOURCES ${KDEFRONTEND_DIR}/ui/datasources/mqttconnectionmanagerwidget.ui)
    list(APPEND UI_SOURCES ${KDEFRONTEND_DIR}/ui/datasources/mqtterrorwidget.ui)
    list(APPEND UI_SOURCES ${KDEFRONTEND_DIR}/ui/datasources/mqttsubscriptionwidget.ui)
    list(APPEND UI_SOURCES ${KDEFRONTEND_DIR}/ui/mqttwillsettingswidget.ui)
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
    ${BACKEND_DIR}/worksheet/WorksheetElementGroup.cpp
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
    ${BACKEND_DIR}/worksheet/plots/cartesian/ReferenceLine.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ReferenceRange.cpp
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
    list(APPEND BACKEND_SOURCES ${KDEFRONTEND_DIR}/widgets/MQTTWillSettingsWidget.cpp)
    list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/datasources/MQTTClient.cpp)
    list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/datasources/MQTTSubscription.cpp)
    list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/datasources/MQTTTopic.cpp)
endif()

# add the qml.qrc file
# qt5_add_resources(qml_QRC ${KDEFRONTEND_DIR}/welcomescreen/qml.qrc)
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

set(COMMONFRONTEND_SOURCES
    ${COMMONFRONTEND_DIR}/matrix/MatrixView.cpp
    ${COMMONFRONTEND_DIR}/note/NoteView.cpp
    ${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetCommentsHeaderModel.cpp
    ${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetHeaderView.cpp
    ${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetSparkLineHeaderModel.cpp
    ${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetItemDelegate.cpp
    ${COMMONFRONTEND_DIR}/spreadsheet/SparklineRunnable.h
    ${COMMONFRONTEND_DIR}/spreadsheet/SparklineRunnable.cpp
    ${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetView.cpp
    ${COMMONFRONTEND_DIR}/workbook/WorkbookView.cpp
    ${COMMONFRONTEND_DIR}/worksheet/WorksheetView.cpp
    ${COMMONFRONTEND_DIR}/ProjectExplorer.cpp
    ${COMMONFRONTEND_DIR}/core/ContentDockWidget.cpp
    ${COMMONFRONTEND_DIR}/widgets/TreeViewComboBox.cpp
    ${COMMONFRONTEND_DIR}/widgets/qxtspanslider.cpp
    ${COMMONFRONTEND_DIR}/widgets/MemoryWidget.cpp
    ${COMMONFRONTEND_DIR}/widgets/DateTimeSpinBox.cpp
    ${COMMONFRONTEND_DIR}/widgets/NumberSpinBox.cpp
    ${COMMONFRONTEND_DIR}/widgets/UTCDateTimeEdit.cpp
    ${COMMONFRONTEND_DIR}/datapicker/DatapickerView.cpp
    ${COMMONFRONTEND_DIR}/datapicker/DatapickerImageView.cpp
)

if(RECENT_CANTOR)
    set(CANTOR_SOURCES
        ${KDEFRONTEND_DIR}/dockwidgets/CantorWorksheetDock.cpp
        ${BACKEND_DIR}/cantorWorksheet/VariableParser.cpp
        ${BACKEND_DIR}/cantorWorksheet/CantorWorksheet.cpp
        ${COMMONFRONTEND_DIR}/cantorWorksheet/CantorWorksheetView.cpp
    )
    set(CANTOR_UI_SOURCES ${KDEFRONTEND_DIR}/ui/dockwidgets/cantorworksheetdock.ui)
    set(UI_SOURCES ${UI_SOURCES} ${CANTOR_UI_SOURCES})
else()
    set(CANTOR_SOURCES "")
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
INCLUDE_DIRECTORIES(. ${BACKEND_DIR}/gsl ${GSL_INCLUDE_DIR} )
set(LABPLOT_SRCS ${GUI_SOURCES} )
ki18n_wrap_ui(LABPLOT_SRCS ${UI_SOURCES} )
# see also QT_MIN_VERSION in CMakeLists.txt
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50700)
#if(QT_MAJOR_VERSION MATCHES 5 AND QT_MINOR_VERSION MATCHES 15)
#    add_definitions(-DQT_NO_DEPRECATED_WARNINGS)
#endif()
# check for Qt6 compatability
#add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50F00)

# static library
add_library(labplotnsllib STATIC ${NSL_SOURCES} )
add_library(labplotbackendlib STATIC ${BACKEND_SOURCES} ${UI_SOURCES} ${BACKEND_TOOLS_SOURCES} ${GENERATED_SOURCES} )
add_library(labplotlib STATIC ${LABPLOT_SRCS} ${CANTOR_SOURCES} ${DATASOURCES_SOURCES} ${COMMONFRONTEND_SOURCES} ${TOOLS_SOURCES} ${QTMOC_HDRS} )
# set_property(TARGET ${objlib} PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(labplotbackendlib PUBLIC ${COMMONFRONTEND_DIR}/widgets) # so that the MOC finds numberSpinbox.h
target_include_directories(labplotlib PUBLIC ${COMMONFRONTEND_DIR}/widgets) # so that the MOC finds numberSpinbox.h

# add system QADS include path
if(qt${QT_VERSION_MAJOR}advanceddocking_FOUND)
    get_target_property(QADS_INCLUDES ads::qt${QT_VERSION_MAJOR}advanceddocking INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(QADS_LIBRARIES ads::qt${QT_VERSION_MAJOR}advanceddocking INTERFACE_LINK_LIBRARIES)
    message(STATUS "QADS Found LINK_LIBRARIES: ${QADS_LIBRARIES} INCLUDES: ${QADS_INCLUDES}")
    target_include_directories(labplotbackendlib PRIVATE ${QADS_INCLUDES} )
    target_include_directories(labplotlib PRIVATE ${QADS_INCLUDES} )
endif()
target_compile_definitions(labplotbackendlib PRIVATE ${LABPLOT_COMPILE_DEFINITIONS})
target_compile_definitions(labplotlib PRIVATE ${LABPLOT_COMPILE_DEFINITIONS})

target_link_libraries(labplotnsllib
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Widgets    # QApplication
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
    ${GSL_LIBRARIES}
    ${GSL_CBLAS_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${FFTW3_LIBRARIES}
    ${netCDF_LIBRARIES}
    ${CFITSIO_LIBRARIES}
    ${LIBCERF_LIBRARIES}
    ${MATIO_LIBRARIES}
    ads::qt${QT_VERSION_MAJOR}advanceddocking
    )
target_link_libraries(labplotlib
    Qt${QT_MAJOR_VERSION}::Svg
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
    target_link_libraries(labplotbackendlib Qt${QT_MAJOR_VERSION}::SerialPort )
    target_link_libraries(labplotlib Qt${QT_MAJOR_VERSION}::SerialPort )
endif()
if(Qt${QT_MAJOR_VERSION}Mqtt_FOUND)
    target_link_libraries(labplotbackendlib Qt${QT_MAJOR_VERSION}::Mqtt )
    target_link_libraries(labplotlib Qt${QT_MAJOR_VERSION}::Mqtt )
endif()
if(KF${KF_MAJOR_VERSION}SyntaxHighlighting_FOUND)
    target_link_libraries(labplotlib KF${KF_MAJOR_VERSION}::SyntaxHighlighting )
endif()
if(KUserFeedback_FOUND)
    target_link_libraries(labplotlib KUserFeedbackWidgets )
endif()
if(KUserFeedbackQt6_FOUND)
    target_link_libraries(labplotlib KUserFeedbackCoreQt6 KUserFeedbackWidgetsQt6 )
endif()
if(Discount_FOUND)
    target_link_libraries(labplotlib Discount::Lib )
endif()
if(Orcus_FOUND)
    target_link_libraries(labplotlib ${Orcus_LIBRARIES} ${Ixion_LIBRARY} )
endif()
# TODO: KF6::NewStuff not found even though KF6NewStuff_FOUND
if(KF5NewStuff_FOUND)
    target_link_libraries(labplotlib KF5::NewStuff )
endif()
if(KF${KF_MAJOR_VERSION}Purpose_FOUND)
    target_link_libraries(labplotlib KF${KF_MAJOR_VERSION}::PurposeWidgets )
endif()
if(ZSTD_FOUND)
        target_link_libraries(labplotbackendlib ${ZSTD_LIBRARIES})
endif()

if(RECENT_CANTOR)
    # KFX::Service, KFX::Parts is optional but probably required by Cantor
    target_link_libraries(labplotbackendlib Cantor::cantorlibs KF${KF_MAJOR_VERSION}::Service KF${KF_MAJOR_VERSION}::Parts )
    target_link_libraries(labplotlib Cantor::cantorlibs KF${KF_MAJOR_VERSION}::Service KF${KF_MAJOR_VERSION}::Parts )
endif()

if(ENABLE_READSTAT)
    # build before using
    if(BUILD_READSTAT)
        add_dependencies(labplotbackendlib 3rdparty_readstat )
    endif()
    # link system readstat with iconv
    if(READSTAT_FOUND)
        find_library(ICONV_LIBRARIES NAMES iconv libiconv libconv-2)
        if(ICONV_LIBRARIES)
            target_link_libraries (labplotbackendlib ${ICONV_LIBRARIES})
        endif()
    endif()
        target_link_libraries(labplotbackendlib ${READSTAT_LIBRARIES} ${ZLIB_LIBRARIES})
endif()

if(ENABLE_XLSX)
    # build before use
    if(BUILD_QXLSX)
        target_include_directories(labplotbackendlib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QXlsx/header")
        add_dependencies(labplotbackendlib QXlsx)
        target_include_directories(labplotlib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QXlsx/header")
        add_dependencies(labplotlib QXlsx)
    endif()

    if(QXLSX_FOUND)
        target_include_directories(labplotbackendlib PRIVATE ${QXLSX_INCLUDE_DIR})
        target_include_directories(labplotlib PRIVATE ${QXLSX_INCLUDE_DIR})
    endif()

    if(QXLSX_FOUND OR BUILD_QXLSX)
    target_link_libraries(labplotbackendlib QXlsx)
    endif()
endif()

if(ENABLE_LIBORIGIN)
    if(LIBORIGIN_FOUND)
        # ifsystem headers are missing: use the provided
        target_include_directories(labplotbackendlib PRIVATE ${LIBORIGIN_INCLUDE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liborigin")
        target_link_libraries(labplotbackendlib ${LIBORIGIN_LIBRARIES})
        target_include_directories(labplotlib PRIVATE ${LIBORIGIN_INCLUDE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liborigin")
    else() # own version
        target_include_directories(labplotbackendlib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liborigin")
        target_link_libraries(labplotbackendlib liborigin-static)
        target_include_directories(labplotlib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liborigin")
    endif()
endif()

IF(LZ4_FOUND)
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
set(LABPLOT_SOURCE ${KDEFRONTEND_DIR}/LabPlot.cpp)

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
target_link_libraries(labplot labplotlib labplotbackendlib labplotnsllib )

#if(KUserFeedbackQt6_FOUND)
#    target_link_libraries(labplot KUserFeedbackCoreQt6 KUserFeedbackWidgetsQt6 )
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
install(FILES ${KDEFRONTEND_DIR}/splash.png ${KDEFRONTEND_DIR}/labplot.ico ${CMAKE_CURRENT_SOURCE_DIR}/../icons/application-x-labplot.ico DESTINATION ${KDE_INSTALL_DATADIR}/${PROJECT_NAME} )
install(FILES org.kde.labplot2.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
install(FILES labplot2.xml DESTINATION ${KDE_INSTALL_MIMEDIR} )
# if(${ECM_VERSION} VERSION_GREATER "5.58.0")
#     install(FILES labplot2_themes.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR} )
#     install(FILES labplot2_datasets.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR} )
# else ()
#     install(FILES labplot2_themes.knsrc DESTINATION ${KDE_INSTALL_CONFDIR} )
#     install(FILES labplot2_datasets.knsrc DESTINATION ${KDE_INSTALL_CONFDIR} )
# endif()
update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR} )
