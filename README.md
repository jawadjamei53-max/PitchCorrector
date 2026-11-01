# PitchCorrector (VST3, JUCE) - Fase 1

Corrector de tono propio: deteccion YIN, tonalidad/escala, notas activas, retune speed,
humanize, transpose, referencia A, mix.

## Compilar en Windows
1. Instala Visual Studio 2022 (carga "Desarrollo para escritorio con C++"), CMake y Git.
2. Doble clic en `build_windows.bat` (descarga JUCE solo, hace falta internet la primera vez).
3. Resultado: `build\PitchCorrector_artefacts\Release\VST3\PitchCorrector.vst3`
4. Copia esa carpeta a `C:\Program Files\Common Files\VST3\` (como administrador).
5. FL Studio: Options > Manage plugins > Find plugins. Ponlo en un efecto de la pista de voz.

## Limitaciones de esta fase
- Voz monofonica (una sola voz a la vez).
- Cambio de tono con ventana fija (30 ms): calidad media; el siguiente paso es PSOLA / formantes.
- Latencia ~15 ms (se reporta al host).
- Sin modo Graph ni entrada MIDI todavia.
