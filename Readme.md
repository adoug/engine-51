# Engine51

Tooling to work with the Area 51 game from 2005.
Console-based export tool for DFS files.

## Project Structure

- **a51lib**: Core engine library (static library)
- **tools/DFSEXPorter**: Console application for exporting DFS files to GLTF format

## Building

Open `Engine51.sln` in Visual Studio 2022 and build the solution.

## Usage

The DFSEXPorter is a console application that currently prints the contents of DFS archives:

```
DFSEXPorter <dfs_file>
```

**Note**: Currently, the tool only displays the contents of the DFS archive. Export functionality is coming.

## Dependencies

- Visual Studio 2022 with C++20 support
- No external dependencies

# Tools

## DFSEXPorter
Console-based tool for exporting DFS files to GLTF format.
