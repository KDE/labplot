Include(FetchContent)

option(LOCAL_QT_ADVANCED_DOCKING "Use local build of dbc parser" OFF)
if (${LOCAL_QT_ADVANCED_DOCKING})
  FetchContent_Declare(
    QtAdvancedDockingSystem
    GIT_REPOSITORY https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git
    GIT_TAG        4.0.1
  )
  FetchContent_MakeAvailable(QtAdvancedDockingSystem)
else()
  find_package(qtadvanceddocking CONFIG REQUIRED)
endif()
