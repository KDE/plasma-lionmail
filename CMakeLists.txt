project(lionmail)

find_package(KDE4 REQUIRED)
find_package(Akonadi REQUIRED)
find_package(KdepimLibs REQUIRED)

include_directories(
  ${AKONADI_INCLUDE_DIR}
  ${KDE4_INCLUDES}
  ${KDEPIMLIBS_INCLUDE_DIRS}
  ${QT_INCLUDES}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory(emailmessage)
add_subdirectory(emailnotifier)
