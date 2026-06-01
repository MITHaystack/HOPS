################################################################################
# install the two pairs of jars according to options
# HOPS_VEX2XML3 uses these (and is what ALMA has)
#  antlr-3.5.2-complete.jar commons-cli-1.2.jar
# HOPS_VEX2XML4 uses these (and it possibly works)
#  antlr-4.8-complete.jar commons-cli-1.4.jar

if(HOPS_VEX2XML3)
    install(FILES java3/commons-cli-1.2.jar java3/antlr-3.5.2-complete.jar DESTINATION ${JAVA_INSTALL_RELDIR}/java3)
endif(HOPS_VEX2XML3)
if(HOPS_VEX2XML4)
    install(FILES java4/commons-cli-1.4.jar java4/antlr-4.8-complete.jar DESTINATION ${JAVA_INSTALL_RELDIR}/java4)
endif(HOPS_VEX2XML4)

