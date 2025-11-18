#!/bin/bash
# Скрипт быстрой сборки проекта OPC Game Simulator

set -e  # Выход при ошибке

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
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
ONLY_BUILD=false
VERBOSE=false

# Обработка аргументов
while [[ $# -gt 0 ]]; do
    case $1 in
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
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
        -b|--build-only)
            ONLY_BUILD=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --no-tests)
            BUILD_TESTS="OFF"
            shift
            ;;
        -h|--help)
            echo -e "${GREEN}=== OPC Game Simulator Build Script ===${NC}"
            echo ""
            echo "Использование: ./build.sh [опции]"
            echo ""
            echo "Опции сборки:"
            echo "  -d, --debug      Сборка в режиме Debug (по умолчанию)"
            echo "  -r, --release    Сборка в режиме Release"
            echo "  -c, --clean      Очистить и пересобрать"
            echo "  -b, --build-only Только сборка (без конфигурации CMake)"
            echo "  --no-tests       Не собирать тесты"
            echo "  -v, --verbose    Подробный вывод"
            echo ""
            echo "Опции запуска:"
            echo "  --run            Запустить приложение после сборки"
            echo "  -t, --tests      Запустить тесты после сборки"
            echo ""
            echo "Примеры:"
            echo "  ./build.sh                    # Debug сборка"
            echo "  ./build.sh --run              # Собрать и запустить"
            echo "  ./build.sh -r --run           # Release + запуск"
            echo "  ./build.sh -c -t              # Чистая сборка + тесты"
            echo "  ./build.sh -b                 # Быстрая пересборка"
            exit 0
            ;;
        *)
            echo -e "${RED}Неизвестная опция: $1${NC}"
            echo "Используйте --help для справки"
            exit 1
            ;;
    esac
done

# Получаем директорию скрипта
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo -e "${GREEN}=== OPC Game Simulator Build ===${NC}"
echo -e "Build type: ${BLUE}$BUILD_TYPE${NC}"
echo -e "Build tests: ${BLUE}$BUILD_TESTS${NC}"

# Очистка директории сборки
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Очистка директории сборки...${NC}"
    rm -rf build
    ONLY_BUILD=false  # После очистки нужна конфигурация
fi

# Создание директории сборки
mkdir -p build

# Засекаем время
START_TIME=$(date +%s)

# Конфигурация CMake (если не только сборка или нет CMakeCache)
if [ "$ONLY_BUILD" = false ] || [ ! -f "build/CMakeCache.txt" ]; then
    echo -e "${GREEN}Конфигурация CMake...${NC}"
    cmake -B build \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
        -DBUILD_TESTS=$BUILD_TESTS \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

# Сборка
echo -e "${GREEN}Сборка проекта...${NC}"
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ "$VERBOSE" = true ]; then
    cmake --build build --config $BUILD_TYPE --parallel $JOBS --verbose
else
    cmake --build build --config $BUILD_TYPE --parallel $JOBS
fi

# Время сборки
END_TIME=$(date +%s)
BUILD_TIME=$((END_TIME - START_TIME))
echo -e "${GREEN}Сборка завершена за ${BUILD_TIME}с${NC}"

# Запуск тестов
if [ "$RUN_TESTS" = true ] && [ "$BUILD_TESTS" = "ON" ]; then
    echo -e "${GREEN}Запуск тестов...${NC}"
    ctest --test-dir build --output-on-failure -C $BUILD_TYPE
fi

# Запуск приложения
if [ "$RUN_APP" = true ]; then
    echo ""
    echo -e "${GREEN}Запуск OPC Game Simulator...${NC}"
    echo -e "${YELLOW}----------------------------------------${NC}"
    ./build/bin/OPCGameSimulator
fi
