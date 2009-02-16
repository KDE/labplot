-> see cmake-2.4.6/Source/CTest/Curl/CmakeLists.txt

	* check for used libs
	gsl	: VERSION, HAVE_GSL14, etc.
		MAJOR : gsl-config --version | sed -e 's/^\([0-9]*\).*/\1/'
		MINOR : gsl-config --version | sed 's/^\([0-9]*\)\.\{0,1\}\([0-9]*\).*/\2/'
		EXECUTE_PROCESS(...)
		STRING(REGEX REPLACE ".([bd])." "[\\1]" RESULT "a(b)c(d)e")
			-> REPLACE ALL "."?
		? -> STRING(REGEX MATCH "(?)\." "[\\1]" RESULT GSL_VERSION)
			./cmake-2.4.6/Tests/ComplexRelativePaths/CMakeLists.txt
			or STRING(COMPARE) ?
		IF("${GSL_MAJOR_VERSION}.${GSL_MINOR_VERSION}" GREATER 1.5)	
	qwtplot3d
	ImageMagick
	textvc
	pstoedit
	netcdf
	cdf
	hdf5
	qhull
	audiofile
	...
	* add options "-disable-qwtplot3d", etc.
		OPTION(OPTION_VAR "help string describing option"
                       [initial value])
	* include all subdirs
	* install
	* uninstall	-> see cmake FAQ
