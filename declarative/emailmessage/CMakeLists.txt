
install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.active.emailmessage PATTERN .svn EXCLUDE)

install(FILES plasma-applet-active-emailmessage.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
