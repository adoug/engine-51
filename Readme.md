# Engine51

Tooling to work with the Area 51 game from 2005.
Console-based export tool for DFS files.

## Project Structure

- **a51lib**: Core engine library (static library)
- **tools/DFSViewer**: Console application for exporting DFS files to GLTF format

## Building

Open `Engine51.sln` in Visual Studio 2022 and build the solution.

## Usage

The DFSViewer is a console application that exports DFS files to GLTF format:

```
DFSViewer <input_file> <output_file>
```

## Dependencies

- Visual Studio 2022 with C++20 support
- No external dependencies (removed Qt, SDL3, Vulkan)

# Tools

## DFSViewer
Console-based tool for exporting DFS files to GLTF format. No GUI - command line only.
