@echo off

REM Script cr�� pour la partie C++ du projet
REM Plateforme utilis�e: Windows 7

REM Effacement du fichier de cache CMake

set CMAKE_CACHE_FILE="CMakeCache.txt"


REM Cache du projet
REM NOTE: demander la question � l'utilisateur reste � impl�menter
if exist %CMAKE_CACHE_FILE% (
  echo Deleting %CMAKE_CACHE_FILE%
  del %CMAKE_CACHE_FILE%
)

REM On lance cmake avec le g�n�rateur MinGW
cmake -G "MinGW Makefiles" . -Wdev
if "%ErrorLevel%" NEQ "0" (
  goto end
)

echo ** cmake . done , you schould be able to compile with mingw32-make

REM mingw32-make
REM if "%ErrorLevel%" NEQ "0" (
REM   goto end
REM )

REM Fin
:end
echo Fin



