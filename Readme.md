# Engine51

Tooling to work with the Area 51 game from 2005.
Console-based export tool for DFS files.

## Project Structure

- **a51lib**: Core engine library (static library)
- **tools/DFSEXPorter**: Console application for exporting DFS files to GLTF format

## Building

Open `Engine51.sln` in Visual Studio 2022 and build the solution.

## Usage

The DFSEXPorter is a console application for exporting DFS archives to various formats (GLTF, JSON, PNG, raw files):

```
DFSEXPorter [options] <dfs_file>
```

### Command Line Options

- `-l` - List all files in the DFS archive
- `-x [dir]` - Export all files to directory (current dir if not specified)
- `-e <filename>` - Export specific file by name
- `-t <type> [dir]` - Export all files of specific type (e.g., RIGIDGEOM, SKINGEOM, XBMP)
- `--external-tex` - Use external PNG files for textures instead of embedding
- (no flags) - Show DFS header and file list

### Export Examples

#### 1. List Files in Archive
```bash
# Show DFS header and file list
DFSEXPorter RESOURCE.DFS

# List files only (no header info)
DFSEXPorter -l RESOURCE.DFS
```

#### 2. Export All Files
```bash
# Export all files to current directory
DFSEXPorter -x RESOURCE.DFS

# Export all files to specific directory
DFSEXPorter -x exported/ RESOURCE.DFS

# Export all files with external textures (not embedded)
DFSEXPorter -x --external-tex RESOURCE.DFS
```

#### 3. Export Single File
```bash
# Export specific file by name (case-insensitive)
DFSEXPorter -e "NPC_BRIDGES_01_LEVELBLUE_SL0-1" RESOURCE.DFS

# Export single file (no directory option for single file export)
DFSEXPorter -e "texture_name" RESOURCE.DFS

# Export with external textures (not embedded)
DFSEXPorter --external-tex -e "NPC_BRIDGES_01_LEVELBLUE_SL0-1" RESOURCE.DFS
```

**Note**: The `-e` option searches for files by name (either base name or full name with extension). If multiple files match, it exports the first one found.

#### 4. Export by File Type
```bash
# Export all rigid geometry files (.RIGIDGEOM)
DFSEXPorter -t RIGIDGEOM RESOURCE.DFS

# Export all skinned geometry files (.SKINGEOM)
DFSEXPorter -t SKINGEOM RESOURCE.DFS

# Export all textures (.XBMP)
DFSEXPorter -t XBMP RESOURCE.DFS

# Export to specific directory
DFSEXPorter -t RIGIDGEOM exported/geometries/ RESOURCE.DFS
```

### Supported File Types

The exporter supports the following file types with automatic format conversion:

- **.RIGIDGEOM** → GLTF format (3D rigid geometry)
- **.SKINGEOM** → GLTF format (3D skinned/animated geometry)
- **.XBMP** → PNG format (textures)

## Known Issues

Some files may not export correctly due to various limitations and dependencies:

### File Format Issues
- **.XBMP textures**: Currently exports as raw RGBA data instead of proper PNG files. The exported files have `.png` extension but contain raw pixel data.
- **Complex geometry**: Some complex 3D models may not export correctly due to unsupported geometry features or malformed data.

### Export Limitations
- **Texture references**: GLTF exports may reference textures that don't exist in the DFS archive, causing texture export failures.

## Dependencies

- Visual Studio 2022 with C++20 support
- No external dependencies

# Tools

## DFSEXPorter
Console-based tool for exporting DFS files to GLTF format.
