#pragma once

#include <string>

class Playsurface;
class LevelTemplate;
class Bitstream;
enum PropertyType;

void exportJSON(Playsurface& playSurface, const std::string& filename);
void exportJSON(LevelTemplate& levelTemplate, const std::string& filename);
std::string getPropStringVal(PropertyType type, Bitstream& bs);