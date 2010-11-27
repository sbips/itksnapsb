SITE_NAME(SNAP_BUILD_MACHINE)

SET(INSTALL_LIBS lib/snap-${SNAP_VERSION_FULL})

IF(SNAP_BUILD_MACHINE STREQUAL "pauly-laptop" AND NOT WIN32)
  INSTALL(
    FILES /usr/lib/libstdc++.so.6.0.9 
    DESTINATION ${INSTALL_LIBS}
    COMPONENT RUNTIME
    RENAME libstdc++.so.6)
  INSTALL(
    FILES /lib/libgcc_s.so.1
    DESTINATION ${INSTALL_LIBS}
    COMPONENT RUNTIME)
  INSTALL(
    FILES /usr/lib/libGLU.so.1.3.070001
    DESTINATION ${INSTALL_LIBS}
    RENAME libGLU.so.1
    COMPONENT RUNTIME)
ENDIF(SNAP_BUILD_MACHINE STREQUAL "pauly-laptop" AND NOT WIN32)

IF(SNAP_BUILD_MACHINE STREQUAL "mingus.uphs.upenn.edu" AND NOT WIN32)
  INSTALL(
    FILES /usr/lib/libstdc++.so.5.0.3 
    DESTINATION ${INSTALL_LIBS}
    COMPONENT RUNTIME
    RENAME libstdc++.so.5)
  INSTALL(
    FILES /lib/libgcc_s-3.2.2-20030225.so.1
    DESTINATION ${INSTALL_LIBS}
    RENAME libgcc_s.so.1
    COMPONENT RUNTIME)
  INSTALL(
    FILES /usr/X11R6/lib/libGLU.so.1.3
    DESTINATION ${INSTALL_LIBS}
    RENAME libGLU.so.1
    COMPONENT RUNTIME)
ENDIF(SNAP_BUILD_MACHINE STREQUAL "mingus.uphs.upenn.edu" AND NOT WIN32)