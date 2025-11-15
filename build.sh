#!/bin/bash
# Скрипт быстрой сборки проекта для Linux/macOS

set -e  # Выход при ошибке

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Проверка VCPKG_ROOT
if [ -z "$VCPKG_ROOT" ]; then
    echo -e "${RED}Ошибка: VCPKG_ROOT не установлена${NC}"
    echo "Установите переменную окружения VCPKG_ROOT"
    echo "Например: export VCPKG_ROOT=\$HOME/vcpkg"
    exit 1
fi

# Параметры по умолчанию
BUILD_TYPE="Debug"
BUILD_TESTS="ON"
CLEAN_BUILD=false
RUN_TESTS=false
RUN_APP=false

# Обработка аргументов
while [[ $# -gt 0 ]]; do
    case $1 in
        -r|--release)
            BUILD_TYPE="Release"
            BUILD_TESTS="OFF"
            shift
            ;;
        -t|--tests)
            RUN_TESTS=true
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        --run)
            RUN_APP=true
            shift
            ;;
        -h|--help)
            echo "Использование: ./build.sh [опции]"
            echo "Опции:"
            echo "  -r, --release    Сборка в режиме Release"
            echo "  -t, --tests      Запустить тесты после сборки"
            echo "  -c, --clean      Очистить директорию сборки"
            echo "  --run            Запустить приложение после сборки"
            echo "  -h, --help       Показать эту справку"
            exit 0
            ;;
        *)
            echo -e "${RED}Неизвестная опция: $1${NC}"
            echo "Используйте --help для справки"
            exit 1
            ;;
    esac
done

echo -e "${GREEN}=== OPC Game Simulator Build Script ===${NC}"
echo "Build type: $BUILD_TYPE"
echo "Build tests: $BUILD_TESTS"
echo "VCPKG_ROOT: $VCPKG_ROOT"

# Очистка директории сборки
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Очистка директории сборки...${NC}"
    rm -rf build
fi

# Создание директории сборки
mkdir -p build
cd build

# Конфигурация CMake
echo -e "${GREEN}Конфигурация CMake...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DBUILD_TESTS=$BUILD_TESTS \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Сборка
echo -e "${GREEN}Сборка проекта...${NC}"
cmake --build . --config $BUILD_TYPE --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Запуск тестов
if [ "$RUN_TESTS" = true ] && [ "$BUILD_TESTS" = "ON" ]; then
    echo -e "${GREEN}Запуск тестов...${NC}"
    ctest --output-on-failure -C $BUILD_TYPE
fi

# Запуск приложения
if [ "$RUN_APP" = true ]; then
    echo -e "${GREEN}Запуск приложения...${NC}"
    ./bin/OPCGameSimulator
fi

echo -e "${GREEN}Сборка завершена успешно!${NC}"
