   Instructions for Rebuilding the XTSC_SD Models

1)  Copy this directory and all sub-directories recursively to a new location.
2)  Ensure the MAXSIM_HOME environment variable is set for the correct version of SoC
    Designer and that the MAXSIM_VERSION_NUMBER environment variable is set and matches (for
    example, 7.7).
3)  MS Windows Only:  Ensure the XTSC_SD_RELEASE_WITH_DEBUG environment variable is set if
    you are using the Release_with_Debug version.  
4)  Edit Makefile.include in the new location (follow the instructions in that file).
5)  (Optional) Edit the source files in the new location if required for your customization.
6)  Run make (Linux) or xt-make (MS Windows) on Makefile in the new location.
7)  Point SoC Designer to xtsclib.conf or xtsclib.without.AMBA2.conf in the new location.

MS Windows Only:  Also, be sure to add the Release or Release_with_Debug sub-directory of
the location in which you rebuilt the XTSC component library to the front of your PATH so
the customized version of xtsc_comp.dll will be used.
