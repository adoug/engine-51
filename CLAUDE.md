# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Engine51 is a C++ project for working with the Area 51 game from 2005. It provides tools to extract and analyze game assets, with the main focus being the DFSViewer tool for viewing DFS (game archive) files.

## Build System

This project uses CMake with Qt 6 as the main build system.

### Windows Build
```bash
# Manual configuration
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.8.0\msvc2022_64"

# Or use the provided batch file
ConfigureWindows.bat
```

### General Build Commands
```bash
# Configure build
cmake -B build

# Build project
cmake --build build

# Run tests
cd build && ctest
```

## Project Structure

### Core Library (`a51lib/`)
The main library containing game asset parsing and manipulation code:

- **File Formats**: `DFSFile` (archive files), `InevFile`, `BinLevel`, `Geom`, `SkinGeom`, `RigidGeom`, `Bitmap`
- **Resource Management**: `ResourceManager`, `ResourceLoaders` for loading game assets
- **Level System**: `Level`, `LevelLoader`, `LevelTemplate` for game level data
- **UI System**: Complete UI framework in `ui/` with dialogs, controls, fonts, and managers
- **Game State**: `StateMachine`, `PlayerProfile`, `GameConfig`, `Inventory` for game state management
- **Animation**: Animation data structures and decompression in `animation/`
- **Navigation**: Navigation mesh support in `levels/navigation/`
- **Utilities**: Math (`VectorMath`), data parsing (`DataReader`, `Tokenizer`, `TextIn`), and I/O (`FileSystem`)

### Tools (`tools/`)
Multiple viewer and game applications:

- **DFSViewer**: Qt-based GUI for viewing DFS archive contents and 3D models
- **qtgame**: Qt-based game renderer with OpenGL support
- **sdlgame**: SDL-based game renderer
- **rhiwindow**: Qt RHI (Rendering Hardware Interface) demo

### Tests (`a51lib_test/`)
GoogleTest-based unit tests for the core library.

## Key Components

### DFS File System
The core file format is DFS (archive files containing game assets). The `DFSFile` class handles:
- Reading DFS archive headers and file listings
- Extracting individual files from archives
- Lazy loading of file data for memory efficiency

### Geometry System
Multiple geometry types are supported:
- `RigidGeom`: Static 3D geometry
- `SkinGeom`: Skinned/animated geometry with bone weights
- Both support export to glTF format via `gltfExporter`

### Resource Management
The `ResourceManager` provides centralized loading and caching of game assets with priority-based file system searching across multiple DFS archives.

## Testing

Tests use GoogleTest framework:
```bash
# Build and run all tests
cmake --build build
cd build && ctest

# Run specific test executable
./build/a51lib_test
```

## Development Notes

### File Format Support
The project handles various Area 51 game file formats:
- `.dfs` - Archive files containing multiple game assets
- `.inev` - Level/scene files
- `.skn` - Skinned geometry files
- `.rigid` - Static geometry files
- `.bmp` - Custom bitmap format

### Qt Integration
DFSViewer and other Qt tools require Qt 6.x with OpenGL support. The build system expects Qt to be available via CMAKE_PREFIX_PATH.

### Cross-Platform Support
The project builds on Windows, macOS, and Linux with GitHub Actions CI ensuring compatibility across all platforms.