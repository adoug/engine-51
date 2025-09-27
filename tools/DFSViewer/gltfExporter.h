#pragma once

#include <string>

class RigidGeom;
class SkinGeom;
class DFSFile;

void exportGLTF(RigidGeom& rigidGeom, const std::string& fileName, DFSFile* dfsFile = nullptr, bool embedTextures = true);
void exportGLTF(SkinGeom& geom, const std::string& fileName, DFSFile* dfsFile = nullptr, bool embedTextures = true);
