################################################################################
# install the obligatory source documentation for the exported jars
# unclear if this has to be distributed: commons-cli-1.4-bin.tar.gz

if(HOPS_VEX2XML3)
    install(FILES
        Antlr-LICENSE-ver3.txt
        Apache-LICENSE-ver2.0.txt
        Release-Notes.txt
        #VEX-visualGrammar.pdf 
            DESTINATION ${DOC_INSTALL_RELDIR}/java3)
endif(HOPS_VEX2XML3)
if(HOPS_VEX2XML4)
    install(FILES
        Antlr-LICENSE-ver4.txt
        Apache-cli-1.4-LICENSE.txt
        Apache-cli-1.4-NOTICE.txt
        Apache-cli-1.4-README.md
        Release-Notes.txt
        #VEX-visualGrammar.pdf
    DESTINATION ${DOC_INSTALL_RELDIR}/java4)
endif(HOPS_VEX2XML4)
