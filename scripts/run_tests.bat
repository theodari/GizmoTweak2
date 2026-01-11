@echo off
echo Current PATH before: %PATH%
set "PATH=C:\Qt\6.6.0\msvc2019_64\bin;%PATH%"
echo Current PATH after: %PATH%
echo.
echo Checking Qt DLLs:
if exist "C:\Qt\6.6.0\msvc2019_64\bin\Qt6Core.dll" (echo Qt6Core.dll found) else (echo Qt6Core.dll NOT FOUND)

echo.
echo ==========================================
echo Running NodeGraph Tests...
echo ==========================================
"D:\QtProjects\GizmoTweak2\build\Desktop_Qt_6_6_0_MSVC2019_64bit-Debug\test\tst_nodegraph.exe"
echo Exit code: %ERRORLEVEL%

echo.
echo ==========================================
echo Running Node Formulas Tests...
echo ==========================================
"D:\QtProjects\GizmoTweak2\build\Desktop_Qt_6_6_0_MSVC2019_64bit-Debug\test\tst_node_formulas.exe"
echo Exit code: %ERRORLEVEL%

echo.
echo ==========================================
echo Running Node Persistence Tests...
echo ==========================================
"D:\QtProjects\GizmoTweak2\build\Desktop_Qt_6_6_0_MSVC2019_64bit-Debug\test\tst_node_persistence.exe"
echo Exit code: %ERRORLEVEL%
