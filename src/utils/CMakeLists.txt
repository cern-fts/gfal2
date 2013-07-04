cmake_minimum_required (VERSION 2.6)

file (GLOB src_exceptions   "exceptions/*.c*")
file (GLOB src_gsimplecache "gsimplecache/*.c*")
file (GLOB src_uri          "uri/*.c*")

add_definitions (-fPIC)

add_library (gfal2_utils STATIC ${src_exceptions}
                                ${src_gsimplecache}
                                ${src_uri})


add_subdirectory (mds)

