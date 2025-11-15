# Настройка окружения разработки

Это руководство поможет вам настроить окружение для разработки OPC Game Simulator.

## Требования

### Минимальные требования

- **ОС:** Windows 10+, Ubuntu 20.04+, macOS 12+
- **Компилятор:**
  - GCC 11+ (Linux)
  - Clang 14+ (macOS)
  - MSVC 2022+ (Windows)
- **CMake:** 3.20+
- **Git:** 2.30+
- **vcpkg:** последняя версия
- **RAM:** 4GB+ (рекомендуется 8GB+)
- **Дисковое пространство:** 5GB+

### Рекомендуемое ПО

- **IDE:** CLion, Visual Studio 2022, VS Code
- **Tiled Map Editor:** для создания карт
- **Aseprite:** для создания спрайтов (опционально)

## Установка на Windows

### 1. Установка Visual Studio 2022

Скачайте и установите Visual Studio 2022 Community:
- Выберите "Desktop development with C++"
- Включите CMake tools

### 2. Установка vcpkg

```powershell
# Клонирование vcpkg
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg

# Установка
.\bootstrap-vcpkg.bat

# Добавление в PATH (опционально)
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
```

### 3. Клонирование проекта

```powershell
git clone https://github.com/yourusername/opc-game-sim.git
cd opc-game-sim
```

### 4. Сборка

```powershell
# Используя CMake Presets
cmake --preset windows-debug
cmake --build --preset debug

# Или вручную
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Debug
```

## Установка на Linux (Ubuntu/Debian)

### 1. Установка зависимостей системы

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    libx11-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libgl1-mesa-dev \
    pkg-config
```

### 2. Установка vcpkg

```bash
# Клонирование vcpkg
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg

# Установка
./bootstrap-vcpkg.sh

# Добавление в .bashrc или .zshrc
echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.bashrc
source ~/.bashrc
```

### 3. Клонирование и сборка

```bash
git clone https://github.com/yourusername/opc-game-sim.git
cd opc-game-sim

# Используя CMake Presets
cmake --preset linux-debug
cmake --build --preset debug

# Запуск
./build/linux-debug/bin/OPCGameSimulator
```

## Установка на macOS

### 1. Установка Xcode и Homebrew

```bash
# Установка Xcode Command Line Tools
xcode-select --install

# Установка Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Установка зависимостей
brew install cmake ninja git
```

### 2. Установка vcpkg

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg
./bootstrap-vcpkg.sh

# Добавление в .zshrc
echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.zshrc
source ~/.zshrc
```

### 3. Сборка проекта

```bash
git clone https://github.com/yourusername/opc-game-sim.git
cd opc-game-sim

cmake --preset debug
cmake --build --preset debug
```

## Настройка IDE

### Visual Studio Code

1. Установите расширения:
   - C/C++ (Microsoft)
   - CMake Tools
   - clangd (опционально)

2. Откройте папку проекта

3. CMake Tools автоматически обнаружит CMakePresets.json

### CLion

1. Откройте папку проекта
2. CLion автоматически обнаружит CMakeLists.txt
3. Выберите нужный preset в настройках

### Visual Studio 2022

1. Откройте папку проекта (File > Open > Folder)
2. Visual Studio обнаружит CMakeLists.txt
3. Выберите конфигурацию из CMakePresets.json

## Проверка установки

```bash
# Сборка
cmake --build build --config Debug

# Запуск тестов
cd build
ctest --output-on-failure

# Запуск приложения
./bin/OPCGameSimulator
```

## Решение проблем

### vcpkg не находит пакеты

```bash
# Обновление vcpkg
cd $VCPKG_ROOT
git pull
./bootstrap-vcpkg.sh  # Linux/macOS
.\bootstrap-vcpkg.bat  # Windows
```

### Ошибки компиляции

1. Проверьте версию компилятора (должен поддерживать C++20)
2. Убедитесь, что все зависимости установлены
3. Очистите кеш CMake:
   ```bash
   rm -rf build
   rm -rf CMakeCache.txt
   ```

### Проблемы с SFML на Linux

```bash
# Установите дополнительные зависимости
sudo apt-get install -y \
    libfreetype6-dev \
    libopenal-dev \
    libflac-dev \
    libvorbis-dev
```

## Дополнительная информация

- [CMake Documentation](https://cmake.org/documentation/)
- [vcpkg Documentation](https://vcpkg.io/en/getting-started.html)
- [SFML Tutorials](https://www.sfml-dev.org/tutorials/)
