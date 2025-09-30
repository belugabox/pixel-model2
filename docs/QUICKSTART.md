# Quick Start Guide

Welcome to Pixel Model 2! This guide will help you get started quickly.

## Prerequisites

Before you begin, ensure you have:

- A C++17 compatible compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- CMake 3.10 or higher
- SDL3 development libraries
- OpenGL support
- Git (for cloning the repository)

## Installation

### Step 1: Clone the Repository

```bash
git clone https://github.com/belugabox/pixel-model2.git
cd pixel-model2
```

### Step 2: Install SDL3

#### Linux (Ubuntu/Debian)

```bash
# SDL3 is not yet in Ubuntu repos, build from source
git clone --depth 1 https://github.com/libsdl-org/SDL.git SDL3
cd SDL3
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
cd ../..
```

#### macOS

```bash
# Build SDL3 from source
git clone --depth 1 https://github.com/libsdl-org/SDL.git SDL3
cd SDL3
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
sudo make install
cd ../..
```

#### Windows

```powershell
# Clone and build SDL3
git clone --depth 1 https://github.com/libsdl-org/SDL.git SDL3
cd SDL3
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cmake --install . --config Release
cd ..\..
```

### Step 3: Build Pixel Model 2

#### Linux/macOS

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

#### Windows

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Running Your First Game

### Step 1: Obtain ROM Files

You must provide your own legally obtained ROM files. Place them in the `roms/` directory as ZIP files.

```bash
mkdir -p roms
# Copy your game ROM ZIP files here
# Example: roms/vf2.zip
```

### Step 2: Run the Emulator

#### Test Without ROMs (Minimal Test)

```bash
# Linux/macOS
./PixelModel2Minimal

# Windows
.\Release\PixelModel2Minimal.exe
```

#### Run with a Game

```bash
# Linux/macOS
./PixelModel2 vf2

# Windows
.\Release\PixelModel2.exe vf2
```

## Controls

### Keyboard

| Key | Function |
|-----|----------|
| Arrow Keys | Movement (Up/Down/Left/Right) |
| Z | Button 1 |
| X | Button 2 |
| C | Button 3 |
| Enter | Start |
| 5 | Insert Coin |

### Joystick

- Standard USB game controllers are supported
- Buttons A/B/X/Y map to game buttons
- Start/Select map to Start/Coin

## Troubleshooting

### Build Issues

**Problem**: CMake can't find SDL3
```
Solution: Ensure SDL3 is installed and CMAKE_PREFIX_PATH is set
export CMAKE_PREFIX_PATH=/usr/local  # or your SDL3 install path
```

**Problem**: Compiler errors about C++17
```
Solution: Update your compiler or explicitly set the standard:
cmake .. -DCMAKE_CXX_STANDARD=17
```

### Runtime Issues

**Problem**: Missing ROM files
```
Solution: Ensure ROM ZIP files are in the roms/ directory
```

**Problem**: Window doesn't open
```
Solution: Check that SDL3 and OpenGL are properly installed
```

## Running Tests

The project includes several test executables:

```bash
# Test memory system
./PixelModel2Test

# Test ZIP extraction
./ZipExtractTest

# Test ROM loading
./LoadMemoryTest

# Test CPU instructions
./PixelModel2LogicalTest
./PixelModel2InterruptTest

# Test TGP
./PixelModel2TGPTest
./PixelModel2TGP3DTest
```

## Next Steps

- Read the full [README.md](README.md) for detailed information
- Check [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for technical details
- Review [CONTRIBUTING.md](CONTRIBUTING.md) if you want to contribute
- Learn about the inspiration in [docs/ELSEMI.md](docs/ELSEMI.md)

## Getting Help

- Check the [GitHub Issues](https://github.com/belugabox/pixel-model2/issues)
- Read the documentation in the `docs/` directory
- Review existing code and comments

## Quick Command Reference

```bash
# Build project
cmake -B build && cmake --build build

# Run main emulator
./build/PixelModel2 <game_name>

# Run minimal test
./build/PixelModel2Minimal

# Clean build
rm -rf build

# View available targets
cmake --build build --target help
```

## Common Game Names

| Game | Command |
|------|---------|
| Virtua Fighter 2 | `vf2` |
| Daytona USA | `daytona` |

**Note**: Game support depends on having the correct ROM files.

## Additional Resources

- [Intel i960 Documentation](https://en.wikipedia.org/wiki/Intel_i960)
- [SEGA Model 2 Hardware Info](http://www.system16.com/hardware.php?id=687)
- [SDL3 Documentation](https://wiki.libsdl.org/SDL3/)

---

Happy emulating! 🎮
