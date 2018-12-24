########### install files ###############
#
#
kdoctools_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR ${PROJECT_NAME})
kdoctools_create_manpage(man-labplot2.1.docbook 1 INSTALL_DESTINATION ${KDE_INSTALL_MANDIR})
