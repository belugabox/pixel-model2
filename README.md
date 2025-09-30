# Pixel Model 2 Emulator

## Overview

Pixel Model 2 is a SEGA Model 2 arcade system emulator, inspired by ElSemi's Model 2 emulator. This project aims to accurately emulate the SEGA Model 2 arcade hardware, which powered popular games like Virtua Fighter 2, Daytona USA, and many others.

## About SEGA Model 2

The SEGA Model 2 is a 3D arcade system board released by SEGA in 1993. It was one of the most powerful arcade systems of its time and introduced true 3D polygon graphics to the arcade scene.

### Hardware Components

- **CPU**: Intel i960 KB/CA RISC processor (25 MHz)
- **Graphics**: Custom SEGA TGP (Transforming Geometry Processor) for 3D calculations
- **Resolution**: Native 496x384 pixels
- **Audio**: Multi-channel PCM audio system

## Features

- **Intel i960 CPU Emulation**: Cycle-accurate emulation of the i960 KB/CA processor
- **TGP GPU Emulation**: 3D geometry transformation and rendering
- **Memory Management**: Accurate memory bus emulation with ROM loading support
- **Input Support**: Keyboard and joystick input for arcade controls
- **ROM Loading**: Support for ZIP-compressed ROM files
- **SDL3 Integration**: Modern graphics and audio using SDL3

## Supported Games

The emulator currently includes configurations for:

- **Virtua Fighter 2** (`vf2`) - ROMs available and compatible
- **Daytona USA** (`daytona`) - ROMs present but need configuration updates

## Building the Project

### Prerequisites

- CMake 3.10 or higher
- C++17 compatible compiler (GCC, Clang, or MSVC)
- SDL3 development libraries
- OpenGL support

### Build Instructions

#### Linux/macOS

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

#### Windows

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Build Targets

The project includes multiple executables:

- **PixelModel2**: Main emulator with full SDL3 window and OpenGL rendering
- **PixelModel2Minimal**: Minimal test executable without graphics
- **PixelModel2Test**: Memory system tests
- **ZipExtractTest**: ZIP extraction functionality tests
- **LoadMemoryTest**: ROM loading tests
- Various test executables for CPU, TGP, and interrupt handling

## Usage

### Running the Emulator

```bash
./PixelModel2 <game_name>
```

### Available Commands

- `--help` or `-h`: Display usage information

### Example

```bash
./PixelModel2 vf2
```

### ROM Files

ROM files should be placed in the `roms/` directory as ZIP archives. The emulator will automatically extract and load the required ROM files from the ZIP archives.

**Note**: You must provide your own legally obtained ROM files. This emulator does not include any copyrighted ROM files.

## Controls

### Keyboard Controls

- **Arrow Keys**: Movement (Up, Down, Left, Right)
- **Z, X, C**: Action buttons (Button 1, 2, 3)
- **Enter**: Start button
- **5**: Insert coin

### Joystick Support

The emulator supports standard USB game controllers with button mapping for:
- A/B/X/Y buttons → Game buttons
- Start/Select → Start/Coin buttons

## Project Structure

```
pixel-model2/
├── src/               # Source files
│   ├── main.cpp       # Main emulator with SDL3
│   ├── i960.cpp       # Intel i960 CPU emulation
│   ├── memory.cpp     # Memory bus and ROM loading
│   ├── tgp.cpp        # TGP GPU emulation
│   └── test_*.cpp     # Various test files
├── include/           # Header files
│   ├── i960.h
│   ├── memory.h
│   └── tgp.h
├── miniz-3.1.0/       # ZIP compression library
├── CMakeLists.txt     # Build configuration
└── README.md          # This file
```

## Technical Details

### CPU Emulation

The Intel i960 KB/CA processor is emulated with:
- Full instruction set implementation
- Register file management
- Interrupt handling
- Memory-mapped I/O

### Graphics Emulation

The TGP (Transforming Geometry Processor) provides:
- 3D coordinate transformation
- Matrix operations
- Perspective calculations
- OpenGL-based rendering

### Memory Map

The Model 2 memory map includes:
- Program ROM (0x000000-0x7FFFFF)
- Work RAM
- Graphics RAM
- I/O registers
- Audio system memory

## Dependencies

### Third-Party Libraries

- **SDL3**: Cross-platform multimedia library for window management and input
- **OpenGL**: 3D graphics rendering
- **miniz**: ZIP compression/decompression (bundled in `miniz-3.1.0/`)

### License

The miniz library is in the public domain. See `miniz-3.1.0/LICENSE` for details.

## Development

### Testing

The project includes several test executables for validating different components:

```bash
# Run memory tests
./PixelModel2Test

# Run minimal emulator test
./PixelModel2Minimal

# Test ZIP extraction
./ZipExtractTest

# Test ROM loading
./LoadMemoryTest
```

### Adding New Games

To add support for a new SEGA Model 2 game:

1. Add ROM configuration in `src/memory.cpp`
2. Define the ROM files and their memory offsets
3. Add the game to the `available_games` array
4. Place the game's ZIP file in the `roms/` directory

## References

- [ElSemi's Model 2 Emulator](http://nebula.emulatronia.com/)
- [SEGA Model 2 Hardware Documentation](http://www.system16.com/hardware.php?id=687)
- [Intel i960 Architecture Manual](https://en.wikipedia.org/wiki/Intel_i960)

## Contributing

Contributions are welcome! Areas that need work:

- Additional game support and ROM configurations
- Improved CPU accuracy and timing
- Enhanced graphics rendering
- Sound emulation improvements
- Performance optimizations

## Disclaimer

This emulator is for educational and preservation purposes only. Users must provide their own legally obtained ROM files. The developers do not condone piracy and are not responsible for any copyright violations.

## Acknowledgments

- ElSemi for the original Model 2 emulator inspiration
- The arcade preservation community
- Rich Geldreich for the miniz library

## Status

This is an active development project. Many features are still being implemented and refined. Expect bugs and incomplete functionality.

## License

[Specify your license here - e.g., MIT, GPL, etc.]

---

**Note**: SEGA and all related trademarks are property of SEGA Corporation. This is an independent project not affiliated with or endorsed by SEGA.
