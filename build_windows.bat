@echo off
REM Requiere: Visual Studio 2022 (carga "Desarrollo para escritorio con C++"), CMake y Git
cmake -B build -G "Visual Studio 17 2022" -A x64 || goto :err
cmake --build build --config Release || goto :err
echo.
echo Listo: build\PitchCorrector_artefacts\Release\VST3\PitchCorrector.vst3
echo Copia esa carpeta a C:\Program Files\Common Files\VST3\ (como administrador)
echo y en FL Studio: Options > Manage plugins > Find plugins.
pause
exit /b 0
:err
echo ERROR en la compilacion
pause
exit /b 1
