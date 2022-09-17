              Building and Using a Custom xtsc-run


Note: For building, use Section I if you are on Linux, use Section II for a
      command line make on MS Windows, and use Section III if you want to use
      the Visual Studio project files on MS Windows.  Use Section IV for 
      using (running) a customized version of xtsc-run on either platform.




           Section I - Command-Line Make  (Linux)


Use the following steps to build a customized version of xtsc-run
on Linux:

1.  Copy this directory and all its contents to a new location.
2.  In the new location, edit the source files as needed for your
    customization.
3.  In the new location, edit Makefile in accordance with the 
    comments in that file.
4.  Build the customized version of xtsc-run.  To do this issue the
    make command in the new location.




           Section II - Command-Line Make  (MS Windows)


Use the following steps to build a customized version of xtsc-run
which is called xtsc-run.exe (Release) and xtsc-rund.exe (Debug):

1.  Copy this directory and all its contents to a new location.
2.  In the new location, edit the source files as needed for your
    customization.
3.  In the new location, edit Makefile in accordance with the 
    comments in that file.
4.  Build the customized version of xtsc-run.  To do this issue the
    xt-make command from a command prompt in the new location and
    with the environment set up for MSVC (e.g. so that cl, the
    Microsoft compiler, is on the execution path).




        Section III - Visual Studio Project (MS Windows only)


Note:  The project file changes discussed below should be applied to both
       the Debug and the Release configurations.

Use the following steps to build a customized version of xtsc-run
(called xtsc-run.exe and xtsc-rund.exe):

1.  Copy this directory and all its contents to a new location.
2.  In the new location, open the xtsc-run.sln file in MSVC 2005 or
    the xtsc-run.vc90.sln file in MSVC 2008.
3.  Change the first directory in the "Additional Include Directories"
    entry to point to the Xtensa Tools include directory:
       <xtensa_tools_root>\include
    You can get to the "Additional Include Directories" entry by selecting
    the xtsc-run project (just below the "Solution 'xtsc-run'" line) in
    the Solution Explorer pane and then using the following drop-down menu
    and dialog sequence:
     Project>Properties>Configuration Properties>C/C++>General>
4.  Change the second directory in the "Additional Include Directories" entry to
    point to the SystemC src directory (<xtensa_tools_root>\Tools\systemc\src).
5.  Change the first directory in the "Additional Library Directories" entry to
    point to the Xtensa Tools ISS library directory:
       <xtensa_tools_root>\lib\iss
    ... or ...
       <xtensa_tools_root>\lib\iss-vc90
    You can get to the "Additional Library Directories" entry by selecting
    the xtsc-run project (just below the "Solution 'xtsc-run'" line) in
    the Solution Explorer pane and then using the following drop-down menu
    and dialog sequence:
     Project>Properties>Configuration Properties>Linker>General>
6.  Change the second directory in the "Additional Library Directories" entry to
    point to the SystemC library directory:
      <xtensa_tools_root>\Tools\systemc\msvc80\SystemC\$(IntDir)
    ... or ...
      <xtensa_tools_root>\Tools\systemc\msvc90\SystemC\$(IntDir)
7.  In the new location, edit the source files as needed for your customizations.
8.  Build your customized version of xtsc-run.





            Section IV -  Using a customized xtsc-run.

To use the customized version of xtsc-run, use the full path to xtsc-run
(Linux) or xtsc-run.exe/xtsc-rund.exe (MS Windows).

