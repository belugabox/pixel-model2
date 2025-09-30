# About ElSemi's Model 2 Emulator

## Overview

ElSemi's Model 2 emulator was one of the pioneering emulators for the SEGA Model 2 arcade system. Developed by ElSemi (Eduardo Cruz), it was among the first to successfully emulate the complex 3D hardware of SEGA's Model 2 arcade board.

## Historical Significance

- **First Release**: Late 1990s / Early 2000s
- **Developer**: ElSemi (Eduardo Cruz)
- **Distribution**: Part of the Nebula emulator suite
- **Impact**: Demonstrated that accurate Model 2 emulation was feasible

## Technical Approach

ElSemi's emulator was notable for:

1. **CPU Emulation**: Accurate Intel i960 instruction set implementation
2. **Graphics Emulation**: Custom TGP (Transforming Geometry Processor) emulation
3. **Game Support**: Successfully ran major titles like Virtua Fighter 2 and Daytona USA
4. **Performance**: Optimized for the hardware available at the time

## Key Features

- Intel i960 CPU core emulation
- TGP geometry processor emulation
- Support for multiple Model 2 games
- Save state functionality
- Input configuration
- Various graphics backends

## Supported Games

ElSemi's emulator successfully ran many Model 2 titles, including:

- Virtua Fighter 2
- Virtua Fighter 2.1
- Virtua Cop
- Daytona USA
- Sega Rally Championship
- The House of the Dead
- Last Bronx
- Dead or Alive
- Desert Tank
- Virtua Striker

## Architecture Insights

ElSemi's work provided valuable insights into Model 2 hardware:

### i960 CPU Implementation

The emulator accurately reproduced:
- Register operations and management
- Memory addressing modes
- Instruction timing
- Interrupt handling
- Cache behavior

### TGP Emulation

Key achievements in graphics emulation:
- Matrix transformation operations
- Perspective correction
- Texture mapping
- Z-buffering
- Polygon rendering

### Memory System

The emulator mapped:
- ROM areas (program and graphics data)
- Work RAM
- Video RAM
- I/O registers
- DMA transfers

## Emulation Challenges

ElSemi documented several technical challenges:

1. **Timing Accuracy**: Synchronizing CPU, GPU, and audio
2. **Undocumented Features**: Reverse-engineering unknown hardware behavior
3. **Performance**: Running on hardware much slower than the original
4. **Game-Specific Quirks**: Each game had unique requirements

## Legacy and Influence

ElSemi's Model 2 emulator:

- Inspired subsequent emulation projects
- Contributed to arcade preservation efforts
- Provided a foundation for MAME's Model 2 driver
- Demonstrated advanced emulation techniques

## Nebula Emulator Suite

The Model 2 emulator was part of Nebula, which included:

- CPS1 emulation (Capcom Play System 1)
- CPS2 emulation (Capcom Play System 2)
- Model 2 emulation
- Neo Geo emulation

## Resources and References

### Historical References

- Nebula website (archived): http://nebula.emulatronia.com/
- ElSemi's technical posts on emulation forums
- MAME's Model 2 driver documentation

### Related Projects

- **MAME**: Integrated Model 2 support based on similar research
- **Model2emu**: Continued development of Model 2 emulation
- **Supermodel**: Modern Model 3 emulator (successor hardware)

## Differences from This Project

Pixel Model 2 differs from ElSemi's approach in several ways:

1. **Modern Libraries**: Uses SDL3 and modern OpenGL
2. **Code Organization**: Modular C++ architecture
3. **Build System**: CMake-based build configuration
4. **Documentation**: Extensive inline and external documentation
5. **CI/CD**: Automated testing and building

## Acknowledgments

This project stands on the shoulders of pioneers like ElSemi, who:

- Reverse-engineered complex arcade hardware
- Shared knowledge with the emulation community
- Preserved arcade gaming history
- Inspired future developers

## Technical Resources

For those interested in ElSemi's techniques:

1. Study MAME's Model 2 driver source code
2. Review archived Nebula documentation
3. Examine reverse-engineering methodologies
4. Understand cycle-accurate emulation principles

## Why Create Another Emulator?

While building on ElSemi's legacy, Pixel Model 2 aims to:

- Modernize the codebase with current tools
- Improve documentation for educational purposes
- Make the code more accessible to new contributors
- Experiment with new emulation techniques
- Support modern platforms and APIs

## Learning from ElSemi

Key lessons from ElSemi's work:

1. **Accuracy Matters**: Precise hardware emulation produces better results
2. **Iterate and Test**: Continuous testing with real games is essential
3. **Document Everything**: Understanding hardware behavior is crucial
4. **Share Knowledge**: The emulation community benefits from open research
5. **Persistence**: Complex emulation takes time and dedication

## Contributing

If you have additional information about ElSemi's Model 2 emulator or related technical details, please contribute to this documentation.

---

**Note**: This document is a tribute to ElSemi's pioneering work. All technical details should be verified against primary sources and proper documentation.
