# Contributing to Pixel Model 2 Emulator

Thank you for your interest in contributing to the Pixel Model 2 emulator project!

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/pixel-model2.git`
3. Create a feature branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Test your changes thoroughly
6. Commit with clear messages: `git commit -m "Add feature X"`
7. Push to your fork: `git push origin feature/your-feature-name`
8. Create a Pull Request

## Development Setup

### Prerequisites

- CMake 3.10 or higher
- C++17 compatible compiler
- SDL3 development libraries
- OpenGL support

### Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Code Style

- Use consistent indentation (4 spaces or tabs as per existing files)
- Keep functions focused and modular
- Add comments for complex logic
- Follow existing naming conventions:
  - Functions: `snake_case` (e.g., `memory_init`)
  - Structures: `PascalCase` (e.g., `MemoryBus`)
  - Constants: `UPPER_CASE` (e.g., `MEMORY_SIZE`)

## Testing

Before submitting a PR:

1. Build all targets successfully
2. Run existing tests:
   ```bash
   ./PixelModel2Minimal
   ./ZipExtractTest
   ./LoadMemoryTest
   ```
3. Test with actual ROM files if available
4. Verify no memory leaks with valgrind (Linux) or similar tools

## What to Contribute

### High Priority

- Additional SEGA Model 2 game support
- CPU instruction implementation improvements
- TGP accuracy enhancements
- Sound emulation
- Documentation improvements

### Medium Priority

- Performance optimizations
- Better error handling
- UI improvements
- Test coverage expansion

### Low Priority

- Code refactoring
- Additional platform support
- Debug tools

## Commit Guidelines

- Use clear, descriptive commit messages
- Reference issues when applicable: `Fix #123: Description`
- Keep commits focused (one logical change per commit)
- Write commit messages in imperative mood: "Add feature" not "Added feature"

## Pull Request Process

1. Ensure your code builds without warnings
2. Update documentation if you change functionality
3. Add tests for new features
4. Keep PRs focused - one feature or fix per PR
5. Respond to review feedback promptly

## Code of Conduct

- Be respectful and constructive
- Focus on the code, not the person
- Help others learn and grow
- Assume positive intent

## Questions?

- Open an issue for bugs or feature requests
- Use discussions for general questions
- Check existing issues before creating new ones

## License

By contributing, you agree that your contributions will be licensed under the same license as the project.

---

Thank you for helping improve the Pixel Model 2 emulator!
