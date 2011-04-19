project(networkconnections)

install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.active.messagecollection PATTERN .svn EXCLUDE)

install(FILES plasma-applet-active-messagecollection.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
