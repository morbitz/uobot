# Microsoft Developer Studio Project File - Name="uobot" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=uobot - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "uobot.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "uobot.mak" CFG="uobot - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "uobot - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "uobot - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "uobot - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x416 /d "NDEBUG"
# ADD RSC /l 0x416 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wininet.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBC.lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "uobot - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fr /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x416 /d "_DEBUG"
# ADD RSC /l 0x416 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wininet.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /verbose /pdb:none

!ENDIF 

# Begin Target

# Name "uobot - Win32 Release"
# Name "uobot - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\uobot.c
# End Source File
# Begin Source File

SOURCE=.\uobot_commands.c
# End Source File
# Begin Source File

SOURCE=.\uobot_exthand.c
# End Source File
# Begin Source File

SOURCE=.\uobot_gumps.c
# End Source File
# Begin Source File

SOURCE=.\uobot_handles.c
# End Source File
# Begin Source File

SOURCE=.\uobot_hexcp.c
# End Source File
# Begin Source File

SOURCE=.\uobot_jornal.c
# End Source File
# Begin Source File

SOURCE=.\uobot_log.c
# End Source File
# Begin Source File

SOURCE=.\uobot_menu.c
# End Source File
# Begin Source File

SOURCE=.\uobot_net.c
# End Source File
# Begin Source File

SOURCE=.\uobot_obj.c
# End Source File
# Begin Source File

SOURCE=.\uobot_plugins.c
# End Source File
# Begin Source File

SOURCE=.\uobot_spells.c
# End Source File
# Begin Source File

SOURCE=.\uobot_target.c
# End Source File
# Begin Source File

SOURCE=.\uobot_teleto.c
# End Source File
# Begin Source File

SOURCE=.\uobot_threads.c
# End Source File
# Begin Source File

SOURCE=.\uobot_update.c
# End Source File
# Begin Source File

SOURCE=.\uobot_vend.c
# End Source File
# Begin Source File

SOURCE=.\uobot_windows.c
# End Source File
# Begin Source File

SOURCE=.\UOHuffman.c
# End Source File
# End Group
# Begin Group "Header"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\uobot.h
# End Source File
# Begin Source File

SOURCE=.\uobot_commands.h
# End Source File
# Begin Source File

SOURCE=.\uobot_exthand.h
# End Source File
# Begin Source File

SOURCE=.\uobot_gumps.h
# End Source File
# Begin Source File

SOURCE=.\uobot_handles.h
# End Source File
# Begin Source File

SOURCE=.\uobot_hexcp.h
# End Source File
# Begin Source File

SOURCE=.\uobot_jornal.h
# End Source File
# Begin Source File

SOURCE=.\uobot_log.h
# End Source File
# Begin Source File

SOURCE=.\uobot_menu.h
# End Source File
# Begin Source File

SOURCE=.\uobot_net.h
# End Source File
# Begin Source File

SOURCE=.\uobot_obj.h
# End Source File
# Begin Source File

SOURCE=.\uobot_plugins.h
# End Source File
# Begin Source File

SOURCE=.\uobot_spells.h
# End Source File
# Begin Source File

SOURCE=.\uobot_target.h
# End Source File
# Begin Source File

SOURCE=.\uobot_teleto.h
# End Source File
# Begin Source File

SOURCE=.\uobot_threads.h
# End Source File
# Begin Source File

SOURCE=.\uobot_update.h
# End Source File
# Begin Source File

SOURCE=.\uobot_vend.h
# End Source File
# Begin Source File

SOURCE=.\uobot_windows.h
# End Source File
# Begin Source File

SOURCE=.\UOEncryption.h
# End Source File
# Begin Source File

SOURCE=.\UOHuffman.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\uobot.rc
# End Source File
# Begin Source File

SOURCE=.\uoboticon.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\yahoo2.ico
# End Source File
# Begin Source File

SOURCE=.\yahoo3.ico
# End Source File
# Begin Source File

SOURCE=.\yahooicon.ico
# End Source File
# End Target
# End Project
