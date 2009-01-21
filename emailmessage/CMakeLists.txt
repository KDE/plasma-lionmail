project(plasma-emailmessage)


set(emailmessage_SRCS
    emailmessage.cpp
    emaildialog.cpp
    )

kde4_add_plugin(plasma_applet_emailmessage ${emailmessage_SRCS})

target_link_libraries(
    plasma_applet_emailmessage
    ${KDE4_PLASMA_LIBS}
)

install(TARGETS plasma_applet_emailmessage DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-emailmessage.desktop DESTINATION ${SERVICES_INSTALL_DIR})
