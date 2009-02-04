project(lionmail)

add_subdirectory(emailmessage)

set(lionmail_SRCS
    lionmail.cpp
    mailextender.cpp
    emailmessage/emailwidget.cpp
    )

kde4_add_ui_files(lionmail_SRCS lionmailConfig.ui )
kde4_add_plugin(plasma_applet_lionmail ${lionmail_SRCS})
target_link_libraries(plasma_applet_lionmail ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${QT_QTWEBKIT_LIBRARY}     ${KDE4_AKONADI_LIBS}
    ${KDE4_KMIME_LIBS}
    ${KDE4_AKONADI_KMIME_LIBS}
)

install(TARGETS plasma_applet_lionmail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-lionmail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES desktoptheme/akonadi.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets)
