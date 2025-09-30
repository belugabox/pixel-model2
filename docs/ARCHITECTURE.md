# SEGA Model 2 Architecture Documentation

## Overview

The SEGA Model 2 is a 3D arcade system board developed by SEGA for use in arcades. It was first released in 1993 and was one of the most advanced arcade systems of its era, introducing true 3D textured polygon graphics.

## Historical Context

The Model 2 was SEGA's successor to the Model 1 arcade board and competed directly with Namco's System 22. It powered many of SEGA's most successful arcade games in the mid-1990s and set the standard for 3D arcade graphics.

### Notable Games

- **Virtua Fighter 2** (1994) - Revolutionary 3D fighting game
- **Daytona USA** (1994) - Groundbreaking 3D racing game
- **Virtua Cop** (1994) - First major 3D light gun game
- **The House of the Dead** (1996) - Horror-themed light gun game
- **Sega Rally Championship** (1995) - Rally racing game
- **Virtua Striker** (1994) - 3D soccer game
- **Last Bronx** (1996) - 3D fighting game
- **Dead or Alive** (1996) - 3D fighting game

## Hardware Specifications

### Main CPU: Intel i960 KB/CA

- **Type**: 32-bit RISC processor
- **Clock Speed**: 25 MHz
- **Architecture**: Superscalar design with dual instruction units
- **Features**:
  - 32 general-purpose registers
  - On-chip instruction and data caches
  - Hardware multiply and divide units
  - Floating-point support (software emulated on KB, hardware on CA variant)

#### i960 Register Set

```
General Registers:
  r0-r15   : Global registers (r0 is always 0 in some variants)
  g0-g15   : Local registers

Special Registers:
  PC       : Program Counter
  AC       : Arithmetic Controls
  IP       : Instruction Pointer
  TC       : Trace Controls
  FP       : Frame Pointer
```

### Graphics Subsystem: TGP (Transforming Geometry Processor)

The TGP is a custom SEGA-designed co-processor specifically built for 3D graphics operations.

#### TGP Components

1. **Geometry Engine**: 
   - Coordinate transformation
   - Perspective projection
   - Clipping and culling operations
   - Matrix operations (4x4 transformation matrices)

2. **Texture Mapping Unit**:
   - Bilinear filtering
   - Perspective-correct texture mapping
   - MIP-mapping support

3. **Polygon Renderer**:
   - Triangle rasterization
   - Gouraud shading
   - Z-buffering for depth sorting
   - Alpha blending for transparency

#### TGP Memory

- **Polygon RAM**: Stores vertex and polygon data
- **Texture RAM**: 2MB-4MB for texture storage
- **Frame Buffer**: 512x384 or 496x384 resolution

### Memory Map

```
0x00000000 - 0x001FFFFF  : Program ROM (2MB)
0x00200000 - 0x003FFFFF  : Extended Program ROM
0x01000000 - 0x0100FFFF  : Work RAM (64KB main)
0x01010000 - 0x0101FFFF  : Backup RAM
0x01800000 - 0x01800003  : I/O Control Registers
0x01810000 - 0x0181FFFF  : Sound RAM
0x01C00000 - 0x01C00FFF  : Palette RAM
0x01C80000 - 0x01CBFFFF  : Video RAM
0x01D00000 - 0x01D00003  : Display Control
0x02000000 - 0x02FFFFFF  : Graphics ROM
```

### Audio System

- **Sound CPU**: MC68000 or similar
- **Sound Chip**: SCSP (Saturn Custom Sound Processor) or MultiPCM
- **Channels**: 32 PCM channels
- **Sample Rate**: 44.1 kHz
- **Features**:
  - Hardware mixing
  - ADPCM compression
  - Effects processing (reverb, etc.)

### Input System

The Model 2 supports various input devices:

1. **Digital Controls**:
   - 8-way joystick
   - Up to 6 action buttons
   - Start/Service buttons
   - Coin inputs

2. **Analog Controls**:
   - Steering wheels (for racing games)
   - Pedals (accelerator, brake)
   - Analog sticks

3. **Special Controllers**:
   - Light guns (Virtua Cop series)
   - Twin sticks
   - Mission sticks (flight games)

## Model 2 Variants

### Model 2 (Original)

- First generation
- Used in Virtua Fighter 2, Daytona USA
- Less powerful TGP

### Model 2A

- Enhanced TGP with improved performance
- Better texture mapping
- Used in Virtua Cop, Virtua Striker

### Model 2B

- Further enhanced graphics
- More RAM for textures
- Better rendering quality
- Used in The House of the Dead, Sega Rally Championship

### Model 2C

- Cost-reduced version
- Similar capabilities to 2B
- More integrated components
- Used in later games

## Technical Details for Emulation

### CPU Emulation Challenges

1. **Timing**: Accurate cycle counting is critical for synchronization
2. **Interrupts**: Model 2 uses complex interrupt handling
3. **Memory Access**: Different speed for ROM vs RAM access
4. **Endianness**: i960 uses little-endian byte ordering

### Graphics Emulation

1. **TGP Command Buffer**: Games write geometry commands to a buffer
2. **Matrix Stack**: Transform matrices are managed in a stack
3. **Display Lists**: Pre-compiled rendering commands
4. **Texture Formats**: Various formats (8-bit, 16-bit RGB)

### Synchronization

The CPU and TGP must be synchronized:
- TGP processes geometry commands
- CPU prepares next frame's data
- Timing is critical for smooth animation

### ROM Loading

Model 2 games typically have:
- Program ROMs (EPR files): CPU code
- Data ROMs (MPR files): Graphics data, textures, sounds
- Sound ROMs: Audio samples

## Emulation Architecture

### Main Loop

```
while (running) {
    // Execute CPU cycles
    cpu_execute(cycles_per_frame);
    
    // Process TGP commands
    tgp_process_frame();
    
    // Render graphics
    render_frame();
    
    // Update audio
    audio_update();
    
    // Handle input
    input_poll();
    
    // Sync timing
    sync_to_60hz();
}
```

### Key Subsystems

1. **CPU Core**: Implements i960 instruction set
2. **Memory Bus**: Routes read/write operations
3. **TGP Emulator**: Processes geometry commands
4. **Video Output**: OpenGL/Direct3D rendering
5. **Audio Output**: SDL or similar audio API
6. **Input Handler**: Maps keyboard/joystick to Model 2 controls

## References

### Technical Documentation

- Intel i960 Processor Family Architecture Manual
- SEGA Model 2 Hardware Service Manual
- System 16 Hardware Database: http://www.system16.com/

### Emulation References

- ElSemi's Model 2 Emulator: http://nebula.emulatronia.com/
- MAME Model 2 Driver: https://github.com/mamedev/mame

### Additional Resources

- SEGA Saturn hardware documentation (SCSP chip shared)
- Arcade PCB collector communities
- Arcade-Museum.com technical specifications

## Implementation Notes

### Current Status

This emulator implements:
- ✅ Basic i960 CPU core
- ✅ Memory bus with ROM loading
- ✅ TGP initialization and basic commands
- ✅ SDL3 window and OpenGL setup
- ✅ Input handling (keyboard and joystick)
- ⚠️  Partial graphics rendering
- ❌ Sound emulation (not yet implemented)
- ❌ Save state support (not yet implemented)

### Next Steps

1. Complete TGP geometry processing
2. Implement full rendering pipeline
3. Add sound emulation
4. Improve CPU accuracy and timing
5. Add more game-specific configurations

## Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines on contributing to the emulator.

---

This documentation is a work in progress and will be updated as the emulator develops.
