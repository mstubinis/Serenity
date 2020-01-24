#include "MapDatabase.h"
#include "../ResourceManifest.h"
#include "../Helper.h"

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <core/engine/resources/Engine_Resources.h>

using namespace std;
using namespace Engine;
namespace boost_io = boost::iostreams;

unordered_map<string, MapEntryData> MapDatabase::DATABASE;

void MapDatabase::init() {
    string path = (ResourceManifest::BasePath + "data/Systems/");
    if (!path.empty()) {
        boost::filesystem::path base_path(path);
        boost::filesystem::directory_iterator end;
        for (boost::filesystem::directory_iterator map_folder_itr(base_path); map_folder_itr != end; ++map_folder_itr) {
            const auto map_folder_path = (*map_folder_itr).path();
            boost::filesystem::path map_folder(map_folder_path);
            boost::filesystem::directory_iterator end1;

            MapEntryData data;
            data.map_folder_path = map_folder_path.string();
            for (boost::filesystem::directory_iterator map_file_itr(map_folder); map_file_itr != end1; ++map_file_itr) {
                boost::filesystem::path cp = (*map_file_itr);
                const string file = (cp.filename().string());
                const string ext = boost::filesystem::extension(file);
                string file_without_ext = file.substr(0, file.size() - ext.size());
                const auto map_file_path = (*map_file_itr).path();
                if (ext == ".txt") {
                    //map .txt file
                    {
                        string res;
                        unsigned int count = 0;
                        bool done = false;

                        boost_io::stream<boost_io::mapped_file_source> str(map_file_path);
                        data.map_file_path = map_file_path.string();
                        for (res; std::getline(str, res, '\n');) {
                            res.erase(std::remove(res.begin(), res.end(), '\r'), res.end()); //remove \r from the line
                            if (count == 1) { // map name
                                data.map_name = res;
                            }else if (count == 3) {
                                data.map_skybox = ResourceManifest::BasePath + res;
                            }else if (count == 5) {
                                res = res.substr(1, res.size() - 1);
                                data.map_desc = res;
                            }else if (count == 6) {//this line has the allowed gameplay types
                                const auto list = Helper::SeparateStringByCharacter(res, ',');
                                for (auto& str : list) {
                                    data.map_valid_game_modes.push_back(static_cast<unsigned int>(stoi(str)));
                                }
                                break;
                            }
                            ++count;
                        }
                    }
                }else if (file_without_ext == "screen" && (ext == ".jpg" || ext == ".png" || ext == ".tga" || ext == ".dds")) {
                    //map screenshot
                    {
                        data.map_screenshot_handle = Resources::loadTexture(map_file_path.string());
                    }
                }
            }
            DATABASE.emplace(data.map_name, data);
        }
    }
}