@echo off
REM Скрипт быстрой сборки проекта для Windows

setlocal enabledelayedexpansion

REM Проверка VCPKG_ROOT
if not defined VCPKG_ROOT (
    echo Ошибка: VCPKG_ROOT не установлена
    echo Установите переменную окружения VCPKG_ROOT
    echo Например: set VCPKG_ROOT=C:\vcpkg
    exit /b 1
)

REM Параметры по умолчанию
set BUILD_TYPE=Debug
set BUILD_TESTS=ON
set CLEAN_BUILD=false
set RUN_TESTS=false
set RUN_APP=false

REM Обработка аргументов
:parse_args
if "%~1"=="" goto end_parse_args
if /i "%~1"=="-r" (
    set BUILD_TYPE=Release
    set BUILD_TESTS=OFF
    shift
    goto parse_args
)
if /i "%~1"=="--release" (
    set BUILD_TYPE=Release
    set BUILD_TESTS=OFF
    shift
    goto parse_args
)
if /i "%~1"=="-t" (
    set RUN_TESTS=true
    shift
    goto parse_args
)
if /i "%~1"=="--tests" (
    set RUN_TESTS=true
    shift
    goto parse_args
)
if /i "%~1"=="-c" (
    set CLEAN_BUILD=true
    shift
    goto parse_args
)
if /i "%~1"=="--clean" (
    set CLEAN_BUILD=true
    shift
    goto parse_args
)
if /i "%~1"=="--run" (
    set RUN_APP=true
    shift
    goto parse_args
)
if /i "%~1"=="-h" goto show_help
if /i "%~1"=="--help" goto show_help

echo Неизвестная опция: %~1
echo Используйте --help для справки
exit /b 1

:show_help
echo Использование: build.bat [опции]
echo Опции:
echo   -r, --release    Сборка в режиме Release
echo   -t, --tests      Запустить тесты после сборки
echo   -c, --clean      Очистить директорию сборки
echo   --run            Запустить приложение после сборки
echo   -h, --help       Показать эту справку
exit /b 0

:end_parse_args

echo === OPC Game Simulator Build Script ===
echo Build type: %BUILD_TYPE%
echo Build tests: %BUILD_TESTS%
echo VCPKG_ROOT: %VCPKG_ROOT%

REM Очистка директории сборки
if "%CLEAN_BUILD%"=="true" (
    echo Очистка директории сборки...
    if exist build rmdir /s /q build
)

REM Создание директории сборки
if not exist build mkdir build
cd build

REM Конфигурация CMake
echo Конфигурация CMake...
cmake .. ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
    -DBUILD_TESTS=%BUILD_TESTS% ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

if errorlevel 1 (
    echo Ошибка конфигурации CMake
    exit /b 1
)

REM Сборка
echo Сборка проекта...
cmake --build . --config %BUILD_TYPE% --parallel

if errorlevel 1 (
    echo Ошибка сборки
    exit /b 1
)

REM Запуск тестов
if "%RUN_TESTS%"=="true" if "%BUILD_TESTS%"=="ON" (
    echo Запуск тестов...
    ctest --output-on-failure -C %BUILD_TYPE%
)

REM Запуск приложения
if "%RUN_APP%"=="true" (
    echo Запуск приложения...
    .\bin\%BUILD_TYPE%\OPCGameSimulator.exe
)

echo Сборка завершена успешно!
cd ..
