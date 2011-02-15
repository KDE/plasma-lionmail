project(emailnotifier)

include_directories(
  ${PROJECT_BINARY_DIR}
)

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
    ${KDE4_AKONADI_LIBS}
    ${KDE4_KMIME_LIBS}
    ${KDE4_AKONADI_KMIME_LIBS}
    ${KDE4_KPIMUTILS_LIBS}
)

install(TARGETS plasma_applet_emailnotifier DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-emailnotifier.desktop DESTINATION ${SERVICES_INSTALL_DIR})
