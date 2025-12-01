# Сборка для Windows

Этот документ описывает различные способы сборки проекта для Windows.

## Рекомендуемый способ: Нативная Windows сборка (MSVC)

Самый надежный способ собрать проект для Windows - использовать нативную Windows систему с Visual Studio.

### Предварительные требования (Windows):

1. **Visual Studio 2022** или **Visual Studio 2019**
   - Включить "Desktop development with C++"

2. **vcpkg**:
```cmd
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
setx VCPKG_ROOT "C:\vcpkg"
```

3. **CMake 3.20+**:
```cmd
winget install Kitware.CMake
```

### Сборка (Windows):

```cmd
# Используя скрипт
build.bat

# Или CMake напрямую
cmake -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

---

## Альтернатива: Кросс-компиляция MinGW в WSL2

⚠️ **Ограничения**:
- Требуется PowerShell Core для некоторых зависимостей (brotli, freetype)
- open62541 (OPC UA) имеет проблемы совместимости с MinGW
- Некоторые библиотеки могут работать нестабильно

### Предварительные требования (WSL2):

1. **MinGW-w64 компилятор**:
```bash
sudo apt update
sudo apt install mingw-w64
```

2. **PowerShell Core** (обязательно!):
```bash
# Ubuntu/Debian
wget https://github.com/PowerShell/PowerShell/releases/download/v7.4.0/powershell_7.4.0-1.deb_amd64.deb
sudo dpkg -i powershell_7.4.0-1.deb_amd64.deb
```

3. **vcpkg** с установленной переменной `VCPKG_ROOT`:
```bash
export VCPKG_ROOT=/home/$(whoami)/vcpkg
echo 'export VCPKG_ROOT=/home/$(whoami)/vcpkg' >> ~/.bashrc
```

4. **CMake 3.20+** и build tools:
```bash
sudo apt install cmake ninja-build
```

## Установка зависимостей vcpkg

Все зависимости должны быть установлены для triplet `x64-mingw-static`:

```bash
cd /path/to/project
$VCPKG_ROOT/vcpkg install --triplet=x64-mingw-static
```

Это установит ~69 пакетов (займет 10-30 минут):
- SFML 3.0.2
- EnTT 3.15.0
- Box2D 3.1.1
- open62541 1.4.14
- Lua 5.4.8
- ImGui 1.91.9
- spdlog 1.16.0
- И другие...

## Сборка проекта

### Вариант 1: Использование скрипта build-windows.sh (рекомендуется)

```bash
# Debug сборка
./build-windows.sh

# Release сборка
./build-windows.sh Release

# Чистая пересборка
./build-windows.sh Debug --clean
```

Результат: `build-windows-debug/bin/OPCGameSimulator.exe`

### Вариант 2: Ручная сборка через CMake

```bash
# Создать директорию
mkdir build-windows
cd build-windows

# Конфигурация
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-mingw-static \
  -DVCPKG_HOST_TRIPLET=x64-linux \
  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=../cmake/mingw-w64-x86_64.cmake \
  -DBUILD_TESTS=OFF

# Сборка
cmake --build . --parallel
```

## Запуск на Windows

После успешной сборки скопируйте исполняемый файл на Windows машину:

```bash
# В WSL2
cd build-windows-release/bin
cp OPCGameSimulator.exe /mnt/c/Users/YourName/Desktop/
```

Затем на Windows просто запустите `OPCGameSimulator.exe`

### Запуск через Wine (опционально, для тестирования в Linux)

```bash
# Установка Wine
sudo apt install wine64

# Запуск
wine build-windows-release/bin/OPCGameSimulator.exe
```

**Примечание**: Wine может не поддерживать все функции SFML 3.0, рекомендуется тестировать на нативной Windows системе.

## Структура файлов

```
cmake/
└── mingw-w64-x86_64.cmake      # MinGW toolchain для CMake

build-windows.sh                # Скрипт автоматической сборки
build-windows-debug/            # Debug сборка для Windows
└── bin/
    └── OPCGameSimulator.exe    # Исполняемый файл
build-windows-release/          # Release сборка для Windows
└── bin/
    └── OPCGameSimulator.exe    # Исполняемый файл
```

## Технические детали

### Triplet: x64-mingw-static

Используется статическая линковка для минимизации зависимостей:
- Все библиотеки встроены в .exe
- Не требуется поставка DLL файлов
- Больший размер исполняемого файла (~10-50 MB)
- Полностью standalone приложение

### Toolchain настройки

Файл `cmake/mingw-w64-x86_64.cmake` настраивает:
- Компилятор: `x86_64-w64-mingw32-g++`
- Статическую линковку: `-static -static-libgcc -static-libstdc++`
- Правильные пути поиска для Windows библиотек

### Поддерживаемые версии Windows

- Windows 10 (64-bit) - рекомендуется
- Windows 11 (64-bit)
- Windows Server 2019/2022

Минимальная версия: Windows 10 1809 (October 2018 Update)

## Известные проблемы MinGW сборки

### 1. PowerShell Core требуется для brotli/freetype

**Проблема:** vcpkg требует PowerShell Core для сборки некоторых пакетов (brotli, freetype)

**Решение:** Установите PowerShell Core:
```bash
wget https://github.com/PowerShell/PowerShell/releases/download/v7.4.0/powershell_7.4.0-1.deb_amd64.deb
sudo dpkg -i powershell_7.4.0-1.deb_amd64.deb
```

### 2. open62541 не совместим с MinGW

**Проблема:** open62541 (OPC UA библиотека) имеет конфликты POSIX/Windows API при сборке с MinGW

**Решение:** В vcpkg.json и CMakeLists.txt open62541 временно отключен для MinGW сборки. Для полной функциональности используйте нативную Windows сборку с MSVC.

### 3. "MinGW компилятор не найден"

```bash
# Проверить установку
which x86_64-w64-mingw32-g++

# Переустановить если нужно
sudo apt install --reinstall mingw-w64
```

### 4. "vcpkg triplet не поддерживается"

Убедитесь что используется community triplet `x64-mingw-static` или `x64-mingw-dynamic`:

```bash
ls $VCPKG_ROOT/triplets/community/x64-mingw-*.cmake
```

### 5. "SFML окно не открывается на Windows"

Проверьте наличие графических драйверов на Windows машине. SFML 3.0 требует поддержки OpenGL 3.3+.

## Производительность

Сборка MinGW обычно работает с производительностью аналогичной нативной Windows сборке:
- 60 FPS стабильный
- Минимальная задержка ввода
- Полная поддержка всех возможностей SFML

## Отличия от нативной Windows сборки

### Преимущества MinGW сборки:
- ✅ Сборка из Linux окружения
- ✅ Автоматизация через CI/CD
- ✅ Консистентная среда разработки
- ✅ Статическая линковка (нет DLL проблем)

### Недостатки:
- ❌ Больший размер exe файла
- ❌ Немного медленнее компиляция (кросс-компиляция)
- ❌ Нет поддержки MSVC специфичных оптимизаций

## Рекомендации по выбору метода сборки

| Критерий | Windows (MSVC) | WSL2 (MinGW) |
|----------|----------------|--------------|
| **Простота** | ✅ Очень просто | ⚠️ Требует настройки |
| **Надежность** | ✅ Полная совместимость | ⚠️ Ограниченная |
| **OPC UA** | ✅ Работает | ❌ Не поддерживается |
| **Производительность** | ✅ Оптимальная | ✅ Хорошая |
| **Размер .exe** | ⚠️ Требует DLL | ✅ Standalone |
| **Отладка** | ✅ Visual Studio | ⚠️ GDB через Wine |

**Вывод:** Для разработки и production используйте **нативную Windows сборку с MSVC**. MinGW подходит только для экспериментов и CI/CD автоматизации (при наличии PowerShell Core).

## Дополнительная информация

- **Нативная Windows сборка:** См. `build.bat` и `CMakePresets.json` (preset: `windows-release`)
- **CI/CD:** См. `.github/workflows/build.yml` для примеров автоматической сборки

---

**Автор:** Claude Code
**Дата:** 2025-11-16
**Версия документа:** 1.1
