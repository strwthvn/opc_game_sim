#!/bin/bash
# Скрипт для кросс-компиляции проекта для Windows в WSL2

set -e  # Exit on error

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== OPC Game Simulator - Windows Build (MinGW) ===${NC}"

# Проверка переменной VCPKG_ROOT
if [ -z "$VCPKG_ROOT" ]; then
    echo -e "${RED}Ошибка: VCPKG_ROOT не установлена${NC}"
    echo "Установите: export VCPKG_ROOT=/home/daniil/vcpkg"
    exit 1
fi

# Параметры
BUILD_TYPE="${1:-Debug}"
BUILD_DIR="build-windows-${BUILD_TYPE,,}"

echo -e "${YELLOW}Build type: ${BUILD_TYPE}${NC}"
echo -e "${YELLOW}Build directory: ${BUILD_DIR}${NC}"
echo -e "${YELLOW}VCPKG_ROOT: ${VCPKG_ROOT}${NC}"

# Создание директории сборки
if [ "$2" == "--clean" ]; then
    echo -e "${YELLOW}Очистка директории сборки...${NC}"
    rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Конфигурация CMake
echo -e "${GREEN}Конфигурация CMake для Windows (MinGW)...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET=x64-mingw-static \
    -DVCPKG_HOST_TRIPLET=x64-linux \
    -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="../cmake/mingw-w64-x86_64.cmake" \
    -DBUILD_TESTS=OFF \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -G "Unix Makefiles"

# Сборка
echo -e "${GREEN}Сборка проекта...${NC}"
cmake --build . --parallel $(nproc)

echo -e "${GREEN}Сборка завершена успешно!${NC}"
echo -e "${YELLOW}Исполняемый файл: ${BUILD_DIR}/bin/OPCGameSimulator.exe${NC}"
echo -e "${YELLOW}Запустите его на Windows машине или через Wine:${NC}"
echo -e "  wine ${BUILD_DIR}/bin/OPCGameSimulator.exe"

cd ..
