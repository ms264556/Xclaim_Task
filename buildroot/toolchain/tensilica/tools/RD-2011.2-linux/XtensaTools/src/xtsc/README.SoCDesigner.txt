    Building and Using a Custom XTSC Component Library with SoC Designer


Note: These instructions only apply to users of SoC Designer (a product of
      Carbon Design Systems).  If you are not using SoC Designer please consult
      file README.txt instead of this file.

Note: Use Section I if you are on Linux or if you want to use the command
      line make on MS Windows.  Use Section II if you want to use the
      Visual Studio project files on MS Windows.

Note: These instructions explain how to build a customized XTSC component library
      compatible with SoC Designer.  Before using the customized XTSC component
      library, you must first rebuild the XTSC_SD wrappers and then point SoC
      Designer to them.   Follow the instuctions in 
        ../xtsc_sd/README.txt (<xtensa_tools_root>/src/xtsc_sd/README.txt)




           Section I - Command Line Make  (Linux or MS Windows)



Part A)  Building a customized XTSC component library.

Use the following steps to build a customized version of the XTSC component library
which is called libxtsc_comp.so on Linux and xtsc_comp.dll (Release) and xtsc_compd.dll
(Release_with_Debug) on MS Windows.

1.  Copy this directory and all its contents to a new location.
2.  Ensure the MAXSIM_HOME environment variable is set for the correct version 
    of SoC Designer and that the MAXSIM_VERSION_NUMBER environment variable is
    set and matches (for example, 7.7).
3.  Ensure the correct version of the compiler is on your path.  On Linux this is
    gcc-3.4.3 and on MS Windows it is MSVC 2005 (cl Version 14.00).
4.  In the new location, edit the source files as needed for your customization.
5.  In the new location, edit Makefile.SoCDesigner in accordance with the instructions
    in that file.
6.  Build the customized version of the XTSC component library.  To do this issue the
    make command in the new location.
    On Linux:
      make -f Makefile.SoCDesigner
    On MS Windows:
      xt-make -f Makefile.SoCDesigner



Part B)  Using a customized XTSC component library.

To use a customized XTSC component library with SoC Designer, you must first
rebuild the XTSC_SD wrappers and then point SoC Designer to them.   Follow the
instuctions in ../xtsc_sd/README.txt (<xtensa_tools_root>/src/xtsc_sd/README.txt).

MS Windows Only:  Also, be sure to add the Release or Release_with_Debug sub-directory
to the front of your PATH so the customized version of xtsc_comp.dll will be used.




        Section II - Visual Studio Project (MS Windows only)



Part A)  Building a customized XTSC component library.

Use the following steps to build a customized version of the XTSC component
library which is called xtsc_comp.dll (Release) and xtsc_compd.dll
(Release_with_Debug)

1.  Copy this directory and all its contents to a new location.
2.  Ensure the MAXSIM_HOME environment variable is set for the correct version 
    of SoC Designer and that the MAXSIM_VERSION_NUMBER environment variable is
    set and matches (for example, 7.7).
3.  Set the environment variable XTTOOLS to point to <xtensa_tools_root>,
    your installation of the Xtensa tools (this is the XtensaTools directory
    two levels up from the original directory holding this file).
4.  In the new location, open xtsc_comp.SoCDesigner.sln with Visual C++ 2005.
5.  Edit the source files in the project as needed for your customizations.
6.  Build the Release and/or Release_with_Debug variants of your customized
    version of the XTSC component library (in Visual Studio terminology these
    variants are called targets).  



Part B)  Using a customized XTSC component library.

To use a customized XTSC component library with SoC Designer, you must first
rebuild the XTSC_SD wrappers and then point SoC Designer to them.   Follow the
instuctions in ../xtsc_sd/README.txt (<xtensa_tools_root>/src/xtsc_sd/README.txt).

Also, be sure to add the Release or Release_with_Debug sub-directory to the front
of your PATH so the customized version of xtsc_comp.dll will be used.

