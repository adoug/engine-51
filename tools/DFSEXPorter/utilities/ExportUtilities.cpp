#include "ExportUtilities.h"
#include "../../a51lib/Playsurface.h"
#include "../../a51lib/LevelTemplate.h"
#include "../../a51lib/dataUtil/Bitstream.h"
#include "../../a51lib/PropertyDefs.h"
#include "../../a51lib/gltf/json.hpp"
#include <filesystem>
#include <fstream>
#include <vector>

using json = nlohmann::json;

void exportJSON(Playsurface& playSurface, const std::string& filename)
{
    json zones;
    for (auto& zi : playSurface.zones) {
        if (zi.numSurfaces > 0) {
            json zone;
            for (auto& surface : zi.surfaces) {
                json surf;
                surf["geomName"] = playSurface.geoms.at(surface.GeomNameIndex);

                json localToWorld;
                for (int r = 0; r < 4; ++r) {
                    for (int c = 0; c < 4; ++c) {
                        localToWorld.push_back(surface.L2W.cells[r][c]);
                    }
                }
                surf["localToWorld"] = localToWorld;
                zone["surfaces"].push_back(surf);
            }
            zones.push_back(zone);
        }
    }
    json output;
    output["zones"] = zones;
    auto          outputString = output.dump(2);
    std::ofstream outputFile(filename);
    if (outputFile.is_open()) {
        outputFile << outputString << std::endl;
    }
}

void exportJSON(LevelTemplate& levelTemplate, const std::string& filename)
{
    Bitstream bs;
    bs.init(levelTemplate.bitstreamData, levelTemplate.bitstreamLen);

    json jsTemplates;
    for (auto& entry : levelTemplate.templates)
    {
        json jsTemplate;
        jsTemplate["name"] = levelTemplate.dictionary[entry.nameIndex];
        std::vector<float> apvec;
        apvec.push_back(entry.anchorPos.x);
        apvec.push_back(entry.anchorPos.y);
        apvec.push_back(entry.anchorPos.z);
        jsTemplate["anchorpos"] = apvec;

        bs.setCursor(entry.iStartBitStream);
        json objects;
        for (int io = entry.iObject; io < entry.iObject + entry.nObjects; ++io) {
            auto& object = levelTemplate.objects[io];
            json jsonObject;
            jsonObject["type"] = levelTemplate.dictionary[object.typeIndex];
            json properties;
            for (int ip = object.iProperty; ip < object.iProperty + object.nProperty; ++ip) {
                auto& property = levelTemplate.properties[ip];
                properties[levelTemplate.dictionary[property.nameIndex]] = getPropStringVal(property.type, bs);
            }
            jsonObject["properties"] = properties;
            objects.push_back(jsonObject);
        }
        jsTemplate["objects"] = objects;
        jsTemplates.push_back(jsTemplate);
    }
    json output;
    output["templates"] = jsTemplates;
    auto          outputString = output.dump(2);
    std::ofstream outputFile(filename);
    if (outputFile.is_open()) {
        outputFile << outputString << std::endl;
    }
}

std::string getPropStringVal(PropertyType type, Bitstream& bs)
{
    // TODO: Implement property string value parsing
    // This would depend on the PropertyType enum and Bitstream implementation
    return "";
}

namespace ExportUtilities {

    std::string getDirectory(const std::string& filePath)
    {
        std::filesystem::path path(filePath);
        return path.parent_path().string();
    }

    std::string getFilenameWithoutExtension(const std::string& filePath)
    {
        std::filesystem::path path(filePath);
        return path.stem().string();
    }

    std::string getFileExtension(const std::string& filePath)
    {
        std::filesystem::path path(filePath);
        return path.extension().string();
    }

}