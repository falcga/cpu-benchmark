# SFBench — Универсальный CPU Бенчмарк

```
   _____ ______ ____                  _     
  / ____|  ____|  _ \                | |    
 | (___ | |__  | |_) | ___ _ __   ___| |__  
  \___ \|  __| |  _ < / _ \ '_ \ / __| '_ \ 
  ____) | |    | |_) |  __/ | | | (__| | | |
 |_____/|_|    |____/ \___|_| |_\___|_| |_|
```

**SFBench v2.1.0** — кроссплатформенный бенчмарк CPU с поддержкой всех архитектур: от старых x86 до RISC-V и микроконтроллеров.

## 🎯 Новые возможности

Начиная с версии 2.1.0, SFBench поддерживает:

### ✅ Архитектуры
- **x86-64** (включая старые процессоры без AVX2/FMA3)
- **x86-32** (Легаси системы)
- **ARM64** (Apple Silicon, modern ARM servers)
- **ARM32** (Raspberry Pi, в том числе ARMv6 без NEON)
- **RISC-V** (RV32/RV64, в том числе с Vector Extension)
- **Микроконтроллеры** (Arduino, ESP32, ESP8266, AVR)

### ✅ Операционные системы
- Windows (включая старые версии с драйверами для старого железа)
- Linux (все дистрибутивы)
- macOS (Intel и Apple Silicon)
- FreeBSD, OpenBSD
- RTOS (через кросскомпиляцию)

### ✅ Уникальные возможности
- **Эмуляция на любом "кирпиче"** — работает даже на микроконтроллерах
- **Тестирование обскурного оборудования** — поддерживает ancient hardware
- **RISC-V с Vector Extension (RVV)** — нативная поддержка
- **Arduino-совместимость** — можно тестировать Arduino Uno, Mega, Nano
- **ESP32/ESP8266** — оптимизации для популярных IoT платформ
- **Raspberry Pi Zero** — поддержка ARMv6 без NEON

---

## 📋 Минимальные требования

### Общие (для всех платформ)
- **CMake** ≥ 3.14
- **C++17 компилятор**

### Рекомендуемые компиляторы
| Платформа | Компилятор |
|----------|------------|
| Windows | MSVC 2019+, Visual Studio 2022 |
| Linux | GCC 9+ или Clang 10+ |
| macOS | Apple Clang (Xcode Command Line Tools) |
| RISC-V | GCC RISC-V или Clang с поддержкой RISC-V |
| Arduino | Arduino IDE 1.8+ или PlatformIO |
| ESP32 | ESP-IDF 4.0+ или PlatformIO |

---

## 🚀 Быстрый старт

### Настольные системы (Linux/macOS/Windows)

```bash
# Клонируем репозиторий
git clone https://github.com/serverflow/sfbenchmark.git
cd sfbenchmark/cpu-benchmark

# Сборка (Linux/macOS)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/cpu_benchmark

# Сборка с поддержкой всех архитектур
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_RISCV=ON \
    -DENABLE_MICROCONTROLLER=ON \
    -DENABLE_UNIVERSAL_FALLBACK=ON
cmake --build build -j$(nproc)
```

### Windows (PowerShell)
```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
.\build\Release\cpu_benchmark.exe
```

### RISC-V
```bash
# Для RISC-V с GCC
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=riscv64-unknown-elf-g++ \
    -DCMAKE_SYSTEM_PROCESSOR=riscv64 \
    -DENABLE_RISCV=ON \
    -DENABLE_NATIVE_ARCH=OFF
cmake --build build

# С RVV (Vector Extension)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-march=rv64gcv -mabi=lp64" \
    -DENABLE_RISCV=ON
cmake --build build
```

### Arduino
```cpp
// Установите библиотеку через Arduino Library Manager
// Или добавьте файлы в ваш скетч

#include "sfbenchmark.h"

void setup() {
    Serial.begin(115200);
    sfbench::run_microcontroller_benchmark();
}

void loop() {
    // Ничего не делаем
}
```

### ESP32 (PlatformIO)
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    https://github.com/serverflow/sfbenchmark.git
build_flags =
    -DENABLE_MICROCONTROLLER=ON
    -DENABLE_ESP32=ON
```

---

## 🎛️ Опции CMake

### Основные опции
| Опция | По умолчанию | Описание |
|-------|-------------|----------|
| `BUILD_TESTS` | ON | Собирать тесты |
| `ENABLE_OPENMP` | OFF | Включить OpenMP |
| `ENABLE_NATIVE_ARCH` | ON | Использовать `-march=native` |
| `BUILD_PORTABLE` | OFF | Портативный бинарник со статической линковкой |

### Новые опции для универсальной совместимости
| Опция | По умолчанию | Описание |
|-------|-------------|----------|
| `ENABLE_RISCV` | ON | Включить поддержку RISC-V |
| `ENABLE_MICROCONTROLLER` | ON | Включить поддержку микроконтроллеров |
| `ENABLE_LEGACY_X86` | ON | Включить поддержку старых x86 |
| `ENABLE_UNIVERSAL_FALLBACK` | ON | Включить универсальные scalar ядра |
| `FORCE_SCALAR_ONLY` | OFF | Принудительно использовать только scalar ядра |

### Примеры конфигураций

**Минимальная сборка (только scalar):**
```bash
cmake -S . -B build -DFORCE_SCALAR_ONLY=ON -DENABLE_TESTS=OFF
```

**Полная кроссплатформенная сборка:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_RISCV=ON \
    -DENABLE_MICROCONTROLLER=ON \
    -DENABLE_UNIVERSAL_FALLBACK=ON \
    -DBUILD_PORTABLE=ON
```

**Сборка для микроконтроллеров:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_MICROCONTROLLER=ON \
    -DENABLE_RISCV=OFF \
    -DFORCE_SCALAR_ONLY=ON
```

---

## 🔧 Режимы работы

### Основные тесты (попадают на сайт bench.serverflow.ru)

#### `--mode=compute` — кросспик CPU
- **Что измеряет:** чистую вычислительную мощность
- **Основа:** scalar FP64 FMA (без SIMD)
- **Время:** 10 секунд на ST и MT
- **Работает на:** ВСЕХ архитектурах, включая микроконтроллеры

#### `--precision=all` — сравнение типов данных
- **Что измеряет:** относительную производительность разных форматов
- **Типы:** FP64, FP32, FP16 (native/emulated), INT8, FP4
- **Работает на:** ВСЕХ архитектурах

### Дополнительные режимы

| Режим | Описание | Поддержка микроконтроллеров |
|-------|----------|---------------------------|
| `--mode=mem` | Пропускная способность памяти | ✅ Да |
| `--mode=stencil` | 7-точечный stencil | ✅ Да |
| `--mode=matmul3d` | Батч умножение матриц | ❌ Нет |
| `--mode=cache` | Уровни кэша | ❌ Нет |

---

## 📊 Примеры запуска

### Обычный запуск
```bash
# Полный тест (с подтверждением)
./cpu_benchmark

# Только compute-тест
./cpu_benchmark --mode=compute

# Сравнение всех типов данных
./cpu_benchmark --precision=all
```

### Для микроконтроллеров
```bash
# Минимальный тест с маленькими данными
./cpu_benchmark --mode=compute --size=16x16x16 --time=1

# Тест памяти с int8
./cpu_benchmark --mode=mem --precision=int8 --size=32x32x32

# Принудительно scalar только
./cpu_benchmark --force-scalar
```

### Для старых систем
```bash
# Отключить AVX2/FMA требования
./cpu_benchmark --force-scalar --no-warmup

# Уменьшить размер тестов
./cpu_benchmark --size=64x64x64 --time=1
```

---

## 🎯 Teстирование обскурного оборудования

SFBench специально оптимизирован для тестирования редкого и старого оборудования:

### Поддерживаемое "обскурное" железо
- **Старые Intel:** Pentium III, Pentium 4, Core 2 Duo
- **Старые AMD:** Athlon XP, Athlon 64, Phenom
- **ARMv6:** Raspberry Pi Zero, Banana Pi
- **MIPS:** Loongson, Cavium Octeon
- **PowerPC:** IBM Power, older Macs
- **RISC-V:** SiFive, Andes, T-Head
- **Микроконтроллеры:** ATmega, ATtiny, ESP8266, ESP32

### Решение проблем совместимости

**Проблема:** `Illegal instruction` на старом x86
```bash
# Используйте scalar-only режим
./cpu_benchmark --force-scalar

# Или соберите с отключенными SIMD
cmake -DFORCE_SCALAR_ONLY=ON
```

**Проблема:** Не хватает памяти на микроконтроллере
```bash
# Уменьшите размер тестов
./cpu_benchmark --size=8x8x8 --time=0.5
```

**Проблема:** Нет FPU на устройстве
```bash
# Используйте integer-only режим
./cpu_benchmark --precision=int8 --force-scalar
```

---

## 🔄 Эмуляция на любом устройстве

SFBench можно запускать в эмулируемых средах:

### QEMU
```bash
# x86-64 эмуляция
qemu-x86_64 -cpu max ./cpu_benchmark

# ARM64 эмуляция
qemu-aarch64 -cpu max ./cpu_benchmark

# RISC-V эмуляция
qemu-riscv64 -cpu rv64 ./cpu_benchmark
```

### Docker
```bash
# Сборка в Docker для кроссплатформенности
docker run --rm -v $(pwd):/src -w /src \
    gcc:latest \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc)
```

---

## 📦 Сборка для конкретных платформ

### Raspberry Pi (ARMv6/ARMv7/ARMv8)
```bash
# ARMv6 (Pi Zero)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-march=armv6zk -mfpu=vfp -O3" \
    -DENABLE_MICROCONTROLLER=ON
cmake --build build

# ARMv7 (Pi 2/3)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-march=armv7-a -mfpu=vfpv3 -mfloat-abi=hard -O3" \
    -DENABLE_MICROCONTROLLER=ON
cmake --build build

# ARMv8 (Pi 4/5)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-march=armv8-a -O3" \
    -DENABLE_MICROCONTROLLER=ON
cmake --build build
```

### ESP32
```bash
# Используя ESP-IDF
export IDF_TARGET=esp32
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$IDF_PATH/tools/cmake/toolchain-esp32.cmake \
    -DENABLE_MICROCONTROLLER=ON \
    -DENABLE_ESP32=ON \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Arduino
```bash
# библиотека доступна через Arduino Library Manager
# Или в PlatformIO
[env:uno]
platform = atmelavr
board = uno
framework = arduino
lib_deps =
    https://github.com/serverflow/sfbenchmark.git
```

---

## 🎓 Примеры использования для тестирования оборудования

### 1. Тестирование старого сервера
```bash
# Старый Intel Xeon (без AVX2)
./cpu_benchmark --mode=compute --force-scalar --threads=4

# Проверка возможностей
./cpu_benchmark --version
```

### 2. Тестирование Raspberry Pi Zero
```bash
# ARMv6 без NEON
./cpu_benchmark --mode=compute --size=64x64x64 --time=5

# Тест памяти
./cpu_benchmark --mode=mem --precision=float --size=128x128x128
```

### 3. Тестирование ESP32
```cpp
// В PlatformIO или Arduino IDE
#include <SFBench.h>

void setup() {
    Serial.begin(115200);
    sfbench::Config config;
    config.mode = sfbench::Mode::Compute;
    config.size = {32, 32, 32};
    config.time = 1.0f;
    
    auto results = sfbench::run(config);
    Serial.println("Score: " + String(results.score));
}
```

### 4. Тестирование RISC-V
```bash
# На SiFive HiFive Unmatched
./cpu_benchmark --mode=compute --precision=all

# С RVV
./cpu_benchmark --mode=mem --size=256x256x256
```

### 5. Тестирование Arduino Uno
```cpp
#include <SFBench.h>

void setup() {
    Serial.begin(9600);
    
    // Минимальная конфигурация для Arduino
    sfbench::MicroConfig config;
    config.array_size = 16;  // 16x16x16
    config.iterations = 100;
    
    float score = sfbench::run_micro(config);
    Serial.print("Arduino Score: ");
    Serial.println(score);
}

void loop() {
    delay(10000);
}
```

---

## 🔍 Детекшн архитектуры

SFBench автоматически определяет:
- Архитектуру CPU (x86, ARM, RISC-V, etc.)
- Доступные SIMD инструкции (SSE2, AVX2, NEON, RVV)
- Количество ядер
- Размеры кэша
- Частоту CPU
- Операционную систему

```bash
# Показать информацию о системе
./cpu_benchmark --version

# Показать возможности CPU
./cpu_benchmark --cpu-info
```

---

## 🛠️ Решение проблем

### "Illegal instruction" на старом CPU
```bash
# Используйте scalar-only сборку
cmake -DFORCE_SCALAR_ONLY=ON
./cpu_benchmark

# Или используйте флаг времени выполнения
./cpu_benchmark --force-scalar
```

### Не хватает памяти на устройстве
```bash
# Уменьшите размер тестов
./cpu_benchmark --size=32x32x32 --time=1

# Или используйте минимальный режим
./cpu_benchmark --mode=compute --size=16x16x16
```

### Нет поддержки floating point
```bash
# Используйте integer precision
./cpu_benchmark --precision=int8 --force-scalar
```

### Проблемы сборки на RISC-V
```bash
# Убедитесь что компилятор поддерживает RISC-V
riscv64-unknown-elf-g++ --version

# Соберите с правильными флагами
cmake -DCMAKE_CXX_COMPILER=riscv64-unknown-elf-g++ \
    -DCMAKE_SYSTEM_PROCESSOR=riscv64 \
    -DENABLE_RISCV=ON
```

---

## 📈 Результаты и сравнение

Результаты можно публиковать на [bench.serverflow.ru](https://bench.serverflow.ru)

### Что публикуется
- `--mode=compute` результаты
- `--precision=all` результаты
- Информация о CPU и системе

### Пример результата
```
SFBench v2.1.0 - Universal
CPU: Raspberry Pi Zero (ARMv6-compatible)
Architecture: armv6l
Cores: 1 / 1
SIMD: Scalar (no NEON)

Compute Score: 125.5 GFLOPS
  ST: 125.5 GFLOPS
  MT: 125.5 GFLOPS

Precision All:
  FP64: 120.0 GFLOPS
  FP32: 125.0 GFLOPS  
  FP16: 45.0 GFLOPS (emulated)
  INT8: 450.0 GOPS
```

---

## 🎨 Системные требования для сборки

### Минимальные требования
| Платформа | RAM | Диск | Время сборки |
|----------|-----|------|--------------|
| Desktop | 512MB | 1GB | 1-5 мин |
| Raspberry Pi 4 | 1GB | 500MB | 5-10 мин |
| Raspberry Pi Zero | 256MB | 200MB | 15-30 мин |
| ESP32 | 1MB | 100MB | Cross-compile |
| Arduino | 2KB | 50MB | Cross-compile |

---

## 📚 Документация

- [Основная документация](README.md) — на английском
- [API справочник](docs/API.md) — программный интерфейс
- [Протоколы тестирования](docs/METHODOLOGY.md) — как проводятся тесты

---

## 🤝 Вклад в проект

Мы приветствуем вклад в проект!

### Как помочь
- Тестируйте на разных платформах
- Добавьте поддержку новых архитектур
- Улучшайте документацию
- Сообщайте о багах

### Добавление новой архитектуры
1. Создайте новый kernel файл: `src/kernels/kernel_<arch>.cpp`
2. Добавьте детекшн в `cpu_capabilities.hpp`
3. Обновите `kernel_dispatcher.hpp`
4. Добавьте флаги компиляции в `CMakeLists.txt`

---

## 📜 Лицензия

MIT License — свободное использование для любых целей.

---

## 🙏 Благодарности

Спасибо всем, кто тестирует SFBench на разных платформах и помогает делать его универсальнее!

Особая благодарность:
- Тестерам на RISC-V платформах
- Разработчикам Arduino и ESP32 сообщества
- Всем, кто использует SFBench для тестирования обскурного оборудования

---

*SFBench — универсальный бенчмарк CPU для всех платформ*
