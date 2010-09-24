project(emailnotifier)

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

#add_subdirectory(emailmessage)

set(emailnotifier_SRCS
    emailnotifier.cpp
    dialog.cpp
    emaillist.cpp
    importantemaillist.cpp
    emailwidget.cpp
    )

kde4_add_ui_files(emailnotifier_SRCS emailnotifierConfig.ui )
kde4_add_plugin(plasma_applet_emailnotifier ${emailnotifier_SRCS})

target_link_libraries(plasma_applet_emailnotifier
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
    #ss${QT_QTWEBKIT_LIBRARY}
    ${KDE4_AKONADI_LIBS}
    ${KDE4_KMIME_LIBS}
    ${KDE4_AKONADI_KMIME_LIBS}
    #${KDE4_KABC_LIBS}
#    akonadi-kabccommon
    #${KDE4_KCAL_LIBS}
    ${KDE4_KPIMUTILS_LIBS}


)

install(TARGETS plasma_applet_emailnotifier DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-emailnotifier.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install(FILES desktoptheme/akonadi.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets)
