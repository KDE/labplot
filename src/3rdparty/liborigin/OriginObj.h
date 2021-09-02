/*
    File                 : OriginObj.h
    Description          : Origin internal object classes
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2005-2007, 2017 Stefan Gerlach
    SPDX-FileCopyrightText: 2007-2008 Alex Kargovsky Ion Vasilief <kargovsky*yumr.phys.msu.su, ion_vasilief*yahoo.fr (use @ for *)>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ORIGIN_OBJ_H
#define ORIGIN_OBJ_H

#include <cstring>
#include <ctime>
#include <vector>
#include <string>

using namespace std;

#define	_ONAN		(-1.23456789E-300)

namespace Origin
{
	enum ValueType {Numeric = 0, Text = 1, Time = 2, Date = 3,  Month = 4, Day = 5, ColumnHeading = 6, TickIndexedDataset = 7, TextNumeric = 9, Categorical = 10};
	// Numeric Format:
	// 1000 | 1E3 | 1k | 1,000
	enum NumericFormat {Decimal = 0, Scientific = 1, Engineering = 2, DecimalWithMarks = 3};
	// Time Format:
	// hh:mm | hh | hh:mm:ss | hh:mm:ss.zz | hh ap | hh:mm ap | mm:ss
	// mm:ss.zz | hhmm | hhmmss | hh:mm:ss.zzz
	enum TimeFormat {TIME_HH_MM = 0, TIME_HH = 1, TIME_HH_MM_SS = 2, TIME_HH_MM_SS_ZZ = 3, TIME_HH_AP = 4, TIME_HH_MM_AP = 5, TIME_MM_SS = 6,
		TIME_MM_SS_ZZ = 7, TIME_HHMM = 8, TIME_HHMMSS = 9, TIME_HH_MM_SS_ZZZ = 10};
	// Date Format:
	// dd/MM/yyyy | dd/MM/yyyy HH:mm | dd/MM/yyyy HH:mm:ss | dd.MM.yyyy | y. (year abbreviation) | MMM d
	// M/d | d | ddd | First letter of day | yyyy | yy | dd.MM.yyyy hh:mm | dd.MM.yyyy hh:mm:ss
	// yyMMdd | yyMMdd hh:mm | yyMMdd hh:mm:ss | yyMMdd hhmm | yyMMdd hhmmss | MMM
	// First letter of month | Quartal | M-d-yyyy (Custom1) | hh:mm:ss.zzzz (Custom2)
	enum DateFormat {DATE_DD_MM_YYYY = -128, DATE_DD_MM_YYYY_HH_MM = -119, DATE_DD_MM_YYYY_HH_MM_SS = -118, DATE_DDMMYYYY = 0, DATE_Y = 1, DATE_MMM_D = 2,
		DATE_M_D = 3, DATE_D = 4, DATE_DDD = 5, DATE_DAY_LETTER = 6, DATE_YYYY = 7, DATE_YY = 8, DATE_DDMMYYYY_HH_MM = 9, DATE_DDMMYYYY_HH_MM_SS = 10,
		DATE_YYMMDD = 11, DATE_YYMMDD_HH_MM = 12, DATE_YYMMDD_HH_MM_SS = 13, DATE_YYMMDD_HHMM = 14, DATE_YYMMDD_HHMMSS = 15, DATE_MMM = 16,
		DATE_MONTH_LETTER = 17, DATE_Q = 18, DATE_M_D_YYYY = 19, DATE_HH_MM_SS_ZZZZ = 20};
	// Month Format:
	//  MMM | MMMM | First letter of month
	enum MonthFormat {MONTH_MMM = 0, MONTH_MMMM = 1, MONTH_LETTER = 2};
	// ddd | dddd | First letter of day
	enum DayOfWeekFormat {DAY_DDD = 0, DAY_DDDD = 1, DAY_LETTER = 2};

	enum NumericDisplayType {DefaultDecimalDigits = 0, DecimalPlaces = 1, SignificantDigits = 2};
	enum Attach {Frame = 0, Page = 1, Scale = 2, End_};
	enum BorderType {BlackLine = 0, Shadow = 1, DarkMarble = 2, WhiteOut = 3, BlackOut = 4, None = -1};
	enum FillPattern {NoFill = 0, BDiagDense = 1, BDiagMedium = 2, BDiagSparse = 3, FDiagDense = 4, FDiagMedium = 5, FDiagSparse = 6,
		DiagCrossDense = 7, DiagCrossMedium = 8, DiagCrossSparse = 9, HorizontalDense = 10, HorizontalMedium = 11, HorizontalSparse = 12,
		VerticalDense = 13, VerticalMedium = 14, VerticalSparse = 15, CrossDense = 16, CrossMedium = 17, CrossSparse = 18};
	enum ColorGradientDirection {NoGradient = 0, TopLeft = 1, Left = 2, BottomLeft = 3, Top = 4, Center = 5, Bottom = 6, TopRight = 7, Right = 8, BottomRight = 9};

	struct Color
	{
		enum ColorType {None = 0, Automatic = 1, Regular = 2, Custom = 3, Increment = 4, Indexing = 5, RGB = 6, Mapping = 7};
		enum RegularColor {Black = 0, Red = 1, Green = 2, Blue = 3, Cyan = 4, Magenta = 5, Yellow = 6, DarkYellow = 7, Navy = 8,
			Purple = 9, Wine = 10, Olive = 11, DarkCyan = 12, Royal=  13, Orange = 14, Violet = 15, Pink = 16, White = 17,
			LightGray = 18, Gray = 19, LTYellow = 20, LTCyan = 21, LTMagenta = 22, DarkGray = 23, SpecialV7Axis = 0xF7/*, Custom = 255*/};

		ColorType type;
		union
		{
			unsigned char regular;
			unsigned char custom[3];
			unsigned char starting;
			unsigned char column;
		};
	};

	struct Rect
	{
		short left;
		short top;
		short right;
		short bottom;

		Rect(short width = 0, short height = 0)
		:	left(0)
		,	top(0)
		,	right(width)
		,	bottom(height)
		{
		};

		int height() const
		{
			return bottom - top;
		};

		int width() const
		{
			return right - left;
		};

		bool isValid() const
		{
			return height() > 0 && width() > 0;
		}
	};

	struct ColorMapLevel
	{
		Color fillColor;
		unsigned char fillPattern;
		Color fillPatternColor;
		double fillPatternLineWidth;

		bool lineVisible;
		Color lineColor;
		unsigned char lineStyle;
		double lineWidth;

		bool labelVisible;
	};

	typedef vector<pair<double, ColorMapLevel> > ColorMapVector;

	struct ColorMap
	{
		bool fillEnabled;
		ColorMapVector levels;
	};

	struct Window
	{
		enum State {Normal, Minimized, Maximized};
		enum Title {Name, Label, Both};

		string name;
		string label;
		int objectID;
		bool hidden;
		State state;
		Title title;
		Rect frameRect;
		time_t creationDate;
		time_t modificationDate;
		ColorGradientDirection windowBackgroundColorGradient;
		Color windowBackgroundColorBase;
		Color windowBackgroundColorEnd;

		Window(const string& _name = string(), const string& _label = string(), bool _hidden = false)
		:	name(_name)
		,	label(_label)
		,	objectID(-1)
		,	hidden(_hidden)
		,	state(Normal)
		,	title(Both)
		,	creationDate(0)
		,	modificationDate(0)
		,	windowBackgroundColorGradient(NoGradient)
		,	windowBackgroundColorBase({Color::Regular, {Color::White}})
		,	windowBackgroundColorEnd({Color::Regular, {Color::White}})
		{};
	};

	// Variant type with boost-free functions
	// see https://github.com/highperformancecoder/scidavis/commit/7c6e07dfad80dbe190af29ffa8a56c82a8aa9180
	// see https://www.ojdip.net/2013/10/implementing-a-variant-type-in-cpp/
	// https://stackoverflow.com/questions/35648390/tagged-union-c
	// https://books.google.de/books?id=PSUNAAAAQBAJ&pg=PA217&lpg=PA217&dq=c%2B%2B+tagged+union+string&source=bl&ots=DqArIieZ8H&sig=k2a6okxxgUuEkLw48hFJChkIG9o&hl=en&sa=X&ved=0ahUKEwjylreR08DUAhWBVRoKHWPSBqE4ChDoAQhUMAg#v=onepage&q=c%2B%2B%20tagged%20union%20string&f=false
	typedef class Variant {
	public:
		enum vtype {V_DOUBLE, V_STRING};
		vtype type() const {return m_type;}
		double as_double() const {return m_double;}
		const char* as_string() const {return m_string;}

		Variant() {}
		Variant(const double d): m_double(d) {}
		Variant(const string& s): m_type(V_STRING)
		{
			asgString(s.c_str());
		}

		Variant(const Variant& v): m_type(v.m_type) {
			switch (v.m_type) {
			case V_DOUBLE:
				m_double = v.m_double;
				break;
			case V_STRING:
				asgString(v.m_string);
				break;
			}
		}

		Origin::Variant& operator=(const Origin::Variant& v) {
			if (m_type == V_STRING)
				delete [] m_string;

			switch (v.m_type) {
			case V_DOUBLE:
				m_double = v.m_double;
				break;
			case V_STRING:
				asgString(v.m_string);
				break;
			}
			m_type = v.m_type;
			return *this;
		}

		~Variant() {
			//printf("~Variant()\n");
			if (m_type == V_STRING)
				delete [] m_string;
		}
	private:
		vtype m_type=V_DOUBLE;
		union {
			double m_double=0.;
			char* m_string;
		};
		void asgString(const char* x)
		{
			m_string=new char[strlen(x)+1];
			strcpy(m_string,x);
		}
	} variant;

	struct SpreadColumn
	{
		enum ColumnType {X, Y, Z, XErr, YErr, Label, NONE};

		string name;
		string dataset_name;
		ColumnType type;
		ValueType valueType;
		int valueTypeSpecification;
		int significantDigits;
		int decimalPlaces;
		NumericDisplayType numericDisplayType;
		string command;
		string comment;
		int width;
		unsigned int index;
		unsigned int colIndex;
		unsigned int sheet;
		unsigned int numRows;
		unsigned int beginRow;
		unsigned int endRow;
		vector<variant> data;

		SpreadColumn(const string& _name = string(), unsigned int _index = 0)
		:	name(_name)
		,	type(ColumnType::Y)
		,	valueType(Numeric)
		,	valueTypeSpecification(0)
		,	significantDigits(6)
		,	decimalPlaces(6)
		,	numericDisplayType(DefaultDecimalDigits)
		,	width(8)
		,	index(_index)
		,	colIndex(0)
		,	sheet(0)
		,	numRows(0)
		,	beginRow(0)
		,	endRow(0)
		{};
	};

	struct SpreadSheet : public Window
	{
		unsigned int maxRows;
		bool loose;
		unsigned int sheets;
		vector<SpreadColumn> columns;

		SpreadSheet(const string& _name = string())
		:	Window(_name)
		,	maxRows(30)
		,	loose(true)
		,	sheets(1)
		{};
	};

	struct Excel : public Window
	{
		unsigned int maxRows;
		bool loose;
		vector<SpreadSheet> sheets;

		Excel(const string& _name = string(), const string& _label = string(), int _maxRows = 0, bool _hidden = false, bool _loose = true)
		:	Window(_name, _label, _hidden)
		,	maxRows(_maxRows)
		,	loose(_loose)
		{
		};
	};

	struct MatrixSheet
	{
		enum ViewType {DataView, ImageView};

		string name;
		unsigned short rowCount;
		unsigned short columnCount;
		int valueTypeSpecification;
		int significantDigits;
		int decimalPlaces;
		NumericDisplayType numericDisplayType;
		string command;
		unsigned short width;
		unsigned int index;
		ViewType view;
		ColorMap colorMap;
		vector<double> data;
		vector<double> coordinates;

		MatrixSheet(const string& _name = string(), unsigned int _index = 0)
		:	name(_name)
		,	rowCount(8)
		,	columnCount(8)
		,	valueTypeSpecification(0)
		,	significantDigits(6)
		,	decimalPlaces(6)
		,	numericDisplayType(DefaultDecimalDigits)
		,	width(8)
		,	index(_index)
		,	view(DataView)
		,	colorMap()
		{coordinates.push_back(10.0);coordinates.push_back(10.0);coordinates.push_back(1.0);coordinates.push_back(1.0);};
	};

	struct Matrix : public Window
	{
		enum HeaderViewType {ColumnRow, XY};

		unsigned int activeSheet;
		HeaderViewType header;
		vector<MatrixSheet> sheets;

		Matrix(const string& _name = string())
		:	Window(_name)
		,	activeSheet(0)
		,	header(ColumnRow)
		{};
	};

	struct Function
	{
		enum FunctionType {Normal, Polar};

		string name;
		FunctionType type;
		string formula;
		double begin;
		double end;
		int totalPoints;
		unsigned int index;

		Function(const string& _name = string(), unsigned int _index = 0)
		:	name(_name)
		,	type(Normal)
		,	begin(0.0)
		,	end(0.0)
		,	totalPoints(0)
		,	index(_index)
		{};
	};


	struct TextBox
	{
		string text;
		Rect clientRect;
		Color color;
		unsigned short fontSize;
		int rotation;
		int tab;
		BorderType borderType;
		Attach attach;

		TextBox(const string& _text = string())
		:	text(_text)
		,	color({Color::Regular, {Color::Black}})
		,	fontSize(20)
		,	rotation(0)
		,	tab(8)
		,	borderType(BlackLine)
		,	attach(Frame)
		{};

		TextBox(const string& _text, Rect _clientRect, Color _color, unsigned short _fontSize, int _rotation, int _tab, BorderType _borderType, Attach _attach)
		:	text(_text)
		,	clientRect(_clientRect)
		,	color(_color)
		,	fontSize(_fontSize)
		,	rotation(_rotation)
		,	tab(_tab)
		,	borderType(_borderType)
		,	attach(_attach)
		{};
	};

	struct PieProperties
	{
		unsigned char viewAngle;
		unsigned char thickness;
		bool clockwiseRotation;
		short rotation;
		unsigned short radius;
		unsigned short horizontalOffset;
		unsigned long displacedSectionCount; // maximum - 32 sections
		unsigned short displacement;

		//labels
		bool formatAutomatic;
		bool formatValues;
		bool formatPercentages;
		bool formatCategories;
		bool positionAssociate;
		unsigned short distance;

		PieProperties()
		:	viewAngle(33)
		,	thickness(33)
		,	clockwiseRotation(false)
		,	rotation(33)
		,	radius(70)
		,	horizontalOffset(0)
		,	displacedSectionCount(0)
		,	displacement(25)
		,	formatAutomatic(false)
		,	formatValues(false)
		,	formatPercentages(false)
		,	formatCategories(false)
		,	positionAssociate(false)
		,	distance(25)
		{};
	};

	struct VectorProperties
	{
		enum VectorPosition {Tail, Midpoint, Head};

		Color color;
		double width;
		unsigned short arrowLength;
		unsigned char arrowAngle;
		bool arrowClosed;
		string endXColumnName;
		string endYColumnName;

		VectorPosition position;
		string angleColumnName;
		string magnitudeColumnName;
		float multiplier;
		int constAngle;
		int constMagnitude;

		VectorProperties()
		:	color({Color::Regular, {Color::Black}})
		,	width(2.0)
		,	arrowLength(45)
		,	arrowAngle(30)
		,	arrowClosed(false)
		,	position(Tail)
		,	multiplier(1.0)
		,	constAngle(0)
		,	constMagnitude(0)
		{};
	};

	struct TextProperties
	{
		enum Justify {Left, Center, Right};

		Color color;
		bool fontBold;
		bool fontItalic;
		bool fontUnderline;
		bool whiteOut;
		Justify justify;

		short rotation;
		short xOffset;
		short yOffset;
		unsigned short fontSize;
	};

	struct SurfaceProperties
	{
		struct SurfaceColoration
		{
			bool fill;
			bool contour;
			Color lineColor;
			double lineWidth;
		};

		enum Type {ColorMap3D, ColorFill, WireFrame, Bars};
		enum Grids {None, X, Y, XY};

		unsigned char type;
		Grids grids;
		double gridLineWidth;
		Color gridColor;

		bool backColorEnabled;
		Color frontColor;
		Color backColor;

		bool sideWallEnabled;
		Color xSideWallColor;
		Color ySideWallColor;

		SurfaceColoration surface;
		SurfaceColoration topContour;
		SurfaceColoration bottomContour;

		ColorMap colorMap;
	};

	struct PercentileProperties
	{
		unsigned char maxSymbolType;
		unsigned char p99SymbolType;
		unsigned char meanSymbolType;
		unsigned char p1SymbolType;
		unsigned char minSymbolType;
		Color symbolColor;
		Color symbolFillColor;
		unsigned short symbolSize;
		unsigned char boxRange;
		unsigned char whiskersRange;
		double boxCoeff;
		double whiskersCoeff;
		bool diamondBox;
		unsigned char labels;
		PercentileProperties()
		:	maxSymbolType(1)
		,	p99SymbolType(2)
		,	meanSymbolType(3)
		,	p1SymbolType(4)
		,	minSymbolType(5)
		,	symbolColor({Color::Regular, {Color::Black}})
		,	symbolFillColor({Color::Regular, {Color::White}})
		,	symbolSize(5)
		,	boxRange(25)
		,	whiskersRange(5)
		,	boxCoeff(1.0)
		,	whiskersCoeff(1.5)
		,	diamondBox(true)
		,	labels(0)
		{};
	};

	struct GraphCurve
	{
		enum Plot {Scatter3D = 101, Surface3D = 103, Vector3D = 183, ScatterAndErrorBar3D = 184, TernaryContour = 185,
			PolarXrYTheta = 186, SmithChart = 191, Polar = 192, BubbleIndexed = 193, BubbleColorMapped = 194,
			Line = 200, Scatter=201, LineSymbol=202, Column = 203, Area = 204, HiLoClose = 205, Box = 206,
			ColumnFloat = 207, Vector = 208, PlotDot = 209, Wall3D = 210, Ribbon3D = 211, Bar3D = 212, ColumnStack = 213,
			AreaStack = 214, Bar = 215, BarStack = 216, FlowVector = 218, Histogram = 219, MatrixImage = 220, Pie = 225,
			Contour = 226, Unknown = 230, ErrorBar = 231, TextPlot = 232, XErrorBar = 233, SurfaceColorMap = 236,
			SurfaceColorFill = 237, SurfaceWireframe = 238, SurfaceBars = 239, Line3D = 240, Text3D = 241, Mesh3D = 242,
			XYZContour = 243, XYZTriangular = 245, LineSeries = 246, YErrorBar = 254, XYErrorBar = 255};
		enum LineStyle {Solid = 0, Dash = 1, Dot = 2, DashDot = 3, DashDotDot = 4, ShortDash = 5, ShortDot = 6, ShortDashDot = 7};
		enum LineConnect {NoLine = 0, Straight = 1, TwoPointSegment = 2, ThreePointSegment = 3, BSpline = 8, Spline = 9,
			StepHorizontal = 11, StepVertical = 12, StepHCenter = 13, StepVCenter = 14, Bezier = 15};

		bool hidden;
		unsigned char type;
		string dataName;
		string xDataName;
		string xColumnName;
		string yColumnName;
		string zColumnName;
		Color lineColor;
		unsigned char lineTransparency;
		unsigned char lineStyle;
		unsigned char lineConnect;
		unsigned char boxWidth;
		double lineWidth;

		bool fillArea;
		unsigned char fillAreaType;
		unsigned char fillAreaPattern;
		Color fillAreaColor;
		unsigned char fillAreaTransparency;
		bool fillAreaWithLineTransparency;
		Color fillAreaPatternColor;
		double fillAreaPatternWidth;
		unsigned char fillAreaPatternBorderStyle;
		Color fillAreaPatternBorderColor;
		double fillAreaPatternBorderWidth;

		unsigned char symbolInterior;
		unsigned char symbolShape;
		Color symbolColor;
		Color symbolFillColor;
		unsigned char symbolFillTransparency;
		double symbolSize;
		unsigned char symbolThickness;
		unsigned char pointOffset;

		bool connectSymbols;

		//pie
		PieProperties pie;

		//vector
		VectorProperties vector;

		//text
		TextProperties text;

		//surface
		SurfaceProperties surface;

		//contour
		ColorMap colorMap;
	};

	struct GraphAxisBreak
	{
		bool show;

		bool log10;
		double from;
		double to;
		double position;

		double scaleIncrementBefore;
		double scaleIncrementAfter;

		unsigned char minorTicksBefore;
		unsigned char minorTicksAfter;

		GraphAxisBreak()
		:	show(false)
		,	log10(false)
		,	from(4.)
		,	to(6.)
		,	position(50.)
		,	scaleIncrementBefore(5)
		,	scaleIncrementAfter(5)
		,	minorTicksBefore(1)
		,	minorTicksAfter(1)
		{};
	};

	struct GraphGrid
	{
		bool hidden;
		unsigned char color;
		unsigned char style;
		double width;
	};

	struct GraphAxisFormat
	{
		bool hidden;
		unsigned char color;
		double thickness;
		double majorTickLength;
		int majorTicksType;
		int minorTicksType;
		int axisPosition;
		double axisPositionValue;
		TextBox label;
		string prefix;
		string suffix;
		string factor;
	};

	struct GraphAxisTick
	{
		bool showMajorLabels;
		unsigned char color;
		ValueType valueType;
		int valueTypeSpecification;
		int decimalPlaces;
		unsigned short fontSize;
		bool fontBold;
		string dataName;
		string columnName;
		int rotation;
	};

	struct GraphAxis
	{
		enum AxisPosition {Left = 0, Bottom, Right, Top, Front, Back};
		enum Scale {Linear = 0, Log10 = 1, Probability = 2, Probit = 3, Reciprocal = 4, OffsetReciprocal = 5, Logit = 6, Ln = 7, Log2 = 8};

		AxisPosition position;
		bool zeroLine;
		bool oppositeLine;
		double min;
		double max;
		double step;
		unsigned char majorTicks;
		unsigned char minorTicks;
		unsigned char scale;
		GraphGrid majorGrid;
		GraphGrid minorGrid;
		GraphAxisFormat formatAxis[2];
		GraphAxisTick tickAxis[2]; //bottom-top, left-right
	};

	struct Figure
	{
		enum FigureType {Rectangle, Circle};

		FigureType type;
		Rect clientRect;
		Attach attach;
		Color color;
		unsigned char style;
		double width;
		Color fillAreaColor;
		unsigned char fillAreaPattern;
		Color fillAreaPatternColor;
		double fillAreaPatternWidth;
		bool useBorderColor;

		Figure(FigureType _type = Rectangle)
		:	type(_type)
		,	attach(Frame)
		,	color({Color::Regular, {Color::Black}})
		,	style(0)
		,	width(1.0)
		,	fillAreaColor({Color::Regular, {Color::LightGray}})
		,	fillAreaPattern(FillPattern::NoFill)
		,	fillAreaPatternColor({Color::Regular, {Color::Black}})
		,	fillAreaPatternWidth(1)
		,	useBorderColor(false)
		{
		};
	};

	struct LineVertex
	{
		unsigned char shapeType;
		double shapeWidth;
		double shapeLength;
		double x;
		double y;

		LineVertex()
		:	shapeType(0)
		,	shapeWidth(0.0)
		,	shapeLength(0.0)
		,	x(0.0)
		,	y(0.0)
		{};
	};

	struct Line
	{
		Rect clientRect;
		Color color;
		Attach attach;
		double width;
		unsigned char style;
		LineVertex begin;
		LineVertex end;
	};

	struct Bitmap
	{
		Rect clientRect;
		Attach attach;
		unsigned long size;
		string windowName;
		BorderType borderType;
		unsigned char* data;

		Bitmap(const string& _name = string())
		:	attach(Frame)
		,	size(0)
		,	windowName(_name)
		,	borderType(BlackLine)
		,	data(nullptr)
		{
		};

		Bitmap(const Bitmap& bitmap)
		:	clientRect(bitmap.clientRect)
		,	attach(bitmap.attach)
		,	size(bitmap.size)
		,	windowName(bitmap.windowName)
		,	borderType(bitmap.borderType)
		,	data(nullptr)
		{
			if(size > 0)
			{
				data = new unsigned char[size];
				memcpy(data, bitmap.data, size);
			}
		};

		~Bitmap()
		{
			if(size > 0)
				delete[] data;
		};
	};

	struct ColorScale
	{
		bool visible;
		bool reverseOrder;
		unsigned short labelGap;
		unsigned short colorBarThickness;
		Color labelsColor;
		ColorScale()
		:	visible(true)
		,	reverseOrder(false)
		,	labelGap(5)
		,	colorBarThickness(3)
		,	labelsColor({Color::Regular, {Color::Black}})
		{};
	};

	struct GraphLayer
	{
		Rect clientRect;
		TextBox legend;
		Color backgroundColor;
		BorderType borderType;

		GraphAxis xAxis;
		GraphAxis yAxis;
		GraphAxis zAxis;

		GraphAxisBreak xAxisBreak;
		GraphAxisBreak yAxisBreak;
		GraphAxisBreak zAxisBreak;

		double histogramBin;
		double histogramBegin;
		double histogramEnd;

		PercentileProperties percentile;
		ColorScale colorScale;
		ColorMap colorMap;

		vector<TextBox> texts;
		vector<TextBox> pieTexts;
		vector<Line> lines;
		vector<Figure> figures;
		vector<Bitmap> bitmaps;
		vector<GraphCurve> curves;

		float xAngle;
		float yAngle;
		float zAngle;

		float xLength;
		float yLength;
		float zLength;

		int imageProfileTool;
		double vLine;
		double hLine;

		bool isWaterfall;
		int xOffset;
		int yOffset;

		bool gridOnTop;
		bool exchangedAxes;
		bool isXYY3D;
		bool orthographic3D;

		GraphLayer()
		:	backgroundColor({Color::Regular, {Color::White}})
		,	borderType(BlackLine)
		,	xAxis(), yAxis(), zAxis()
		,	histogramBin(0.5)
		,	histogramBegin(0.0)
		,	histogramEnd(10.0)
		,	colorMap()
		,	xAngle(0)
		,	yAngle(0)
		,	zAngle(0)
		,	xLength(10)
		,	yLength(10)
		,	zLength(10)
		,	imageProfileTool(0)
		,	vLine(0.0)
		,	hLine(0.0)
		,	isWaterfall(false)
		,	xOffset(10)
		,	yOffset(10)
		,	gridOnTop(false)
		,	exchangedAxes(false)
		,	isXYY3D(false)
		,	orthographic3D(false)
		{colorScale.visible = false;};

		//bool threeDimensional;
		bool is3D() const
		{
			for (vector<GraphCurve>::const_iterator it = curves.begin(); it != curves.end(); ++it)
			{
				switch (it->type)
				{
					case GraphCurve::Scatter3D:
					case GraphCurve::Surface3D:
					case GraphCurve::Vector3D:
					case GraphCurve::ScatterAndErrorBar3D:
					case GraphCurve::TernaryContour:
					case GraphCurve::Line3D:
					case GraphCurve::Mesh3D:
					case GraphCurve::XYZContour:
					case GraphCurve::XYZTriangular:
						return true;
					default:
						break;
				}
			}
		return false;
		}
	};

	struct GraphLayerRange
	{
		double min;
		double max;
		double step;

		GraphLayerRange(double _min = 0.0, double _max = 0.0, double _step = 0.0)
		:	min(_min)
		,	max(_max)
		,	step(_step)
		{};
	};

	struct Graph : public Window
	{
		vector<GraphLayer> layers;
		unsigned short width;
		unsigned short height;
		bool is3D;
		bool isLayout;
		bool connectMissingData;
		string templateName;

		Graph(const string& _name = string())
		:	Window(_name)
		,	width(400)
		,	height(300)
		,	is3D(false)
		,	isLayout(false)
		,	connectMissingData(false)
		{};
	};

	struct Note : public Window
	{
		string text;
		Note(const string& _name = string())
		:	Window(_name)
		{};
	};

	struct ProjectNode
	{
		enum NodeType {SpreadSheet, Matrix, Excel, Graph, Graph3D, Note, Folder};

		NodeType type;
		string name;
		time_t creationDate;
		time_t modificationDate;
		bool active;

		ProjectNode(const string& _name = string(), NodeType _type = Folder, const time_t _creationDate = time(nullptr), const time_t _modificationDate = time(nullptr), bool _active = false)
		:	type(_type)
		,	name(_name)
		,	creationDate(_creationDate)
		,	modificationDate(_modificationDate)
		,	active(_active)
		{};
	};
}



#endif // ORIGIN_OBJ_H
