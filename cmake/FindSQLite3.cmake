message(INFO " looking for dependency: SQLite3")
find_path(SQLITE3_INCLUDE NAMES sqlite3.h)
mark_as_advanced(SQLITE3_INCLUDE)
if(SQLITE3_INCLUDE)
	include_directories(${SQLITE3_INCLUDE})
endif()

find_library(SQLITE3_LIBRARY NAMES sqlite3)
mark_as_advanced(SQLITE3_LIBRARY)
if(SQLITE3_LIBRARY)
	add_library(sqlite3 SHARED IMPORTED)
	set_property(TARGET sqlite3 PROPERTY IMPORTED_LOCATION "${SQLITE3_LIBRARY}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SQLite3 DEFAULT_MSG SQLITE3_LIBRARY SQLITE3_INCLUDE)
