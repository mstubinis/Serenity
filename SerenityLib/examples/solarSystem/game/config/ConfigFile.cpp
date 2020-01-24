#include "ConfigFile.h"

#include <iostream>
#include <sstream>

using namespace std;
namespace boost_io = boost::iostreams;

const std::string file_ext           = ".cfg";
const std::string file_name          = "config";
const std::string file_name_with_ext = file_name + file_ext;

ConfigDataStruct ConfigFile::CONFIG_DATA;

ConfigFile::ConfigFile() {

}
ConfigFile::~ConfigFile() {

}
const ConfigDataStruct& ConfigFile::readFromFile() {
    if (boost::filesystem::exists(file_name_with_ext)) {
        //read in file
        {
            //populate the info struct with data from the file
            boost_io::stream<boost_io::mapped_file_source>   stream(file_name_with_ext);
            for (string line; getline(stream, line, '\n');) {
                line.erase(remove(line.begin(), line.end(), '\r'), line.end()); //remove \r from the line
                if (line[0] != '#') {//ignore commented lines
                    string token;
                    istringstream istream(line);
                    while (getline(istream, token, ' ')) {
                        size_t pos = token.find("=");
                        string key = token.substr(0, pos);
                        string value = token.substr(pos + 1, string::npos);

                        if (key == "window_width") {
                            CONFIG_DATA.window_width = stoi(value);
                        }else if (key == "window_height") {
                            CONFIG_DATA.window_height = stoi(value);
                        }else if(key == "window_maximized"){
                            CONFIG_DATA.window_maximized = static_cast<bool>(stoi(value));
                        }else if (key == "window_mode") {
                            CONFIG_DATA.window_mode = (value);
                        }else if (key == "volume_music") {
                            CONFIG_DATA.volume_music = stoi(value);
                        }else if (key == "volume_effects") {
                            CONFIG_DATA.volume_effects = stoi(value);
                        }else if (key == "volume_speech") {
                            CONFIG_DATA.volume_speech = stoi(value);
                        }else if (key == "join_server_port") {
                            CONFIG_DATA.join_server_port = stoi(value);
                        }else if (key == "join_server_ip") {
                            CONFIG_DATA.join_server_ip = (value);
                        }else if (key == "join_server_player_name") {
                            CONFIG_DATA.join_server_player_name = (value);
                        }else if (key == "host_server_port") {
                            CONFIG_DATA.host_server_port = stoi(value);
                        }else if (key == "host_server_player_name") {
                            CONFIG_DATA.host_server_player_name = (value);
                        }
                    }
                }
            }
            stream.close();
        }
    }else{
        writeToFile();
    }
    return CONFIG_DATA;
}
void ConfigFile::updateHostServerName(const string& name) {
    if (CONFIG_DATA.host_server_player_name == name) {
        return;
    }
    CONFIG_DATA.host_server_player_name = name;
    writeToFile();
}
void ConfigFile::updateHostServerPort(const unsigned short& port) {
    if (CONFIG_DATA.host_server_port == port) {
        return;
    }
    CONFIG_DATA.host_server_port = port;
    writeToFile();
}
void ConfigFile::updateWindowMode(const unsigned int& sfStyleEnumBitFlag) {
    string res = CONFIG_DATA.window_mode;
    if (sfStyleEnumBitFlag & sf::Style::Fullscreen && !(sfStyleEnumBitFlag & sf::Style::Default)) {
        res = "fullscreen";
    }else if (sfStyleEnumBitFlag & sf::Style::Default && !(sfStyleEnumBitFlag & sf::Style::None)) {
        res = "windowed";
    }else{
        res = "windowed_fullscreen";
    }
    if (CONFIG_DATA.window_mode == res) {
        return;
    }
    CONFIG_DATA.window_mode = res;
    writeToFile();
}
void ConfigFile::updateWindowMode(const std::string& mode) {
    if (CONFIG_DATA.window_mode == mode) {
        return;
    }
    CONFIG_DATA.window_mode = mode;
    writeToFile();
}
void ConfigFile::updateWindowSize(const unsigned int& width, const unsigned int& height) {
    if (CONFIG_DATA.window_width == width && CONFIG_DATA.window_height == height) {
        return;
    }
    CONFIG_DATA.window_width = width;
    CONFIG_DATA.window_height = height;
    writeToFile();
}
void ConfigFile::updateWindowMaximized(const bool& maximized) {
    if (CONFIG_DATA.window_maximized == maximized) {
        return;
    }
    CONFIG_DATA.window_maximized = maximized;
    writeToFile();
}

void ConfigFile::write_to_file(boost::filesystem::ofstream& stream) {
    stream.clear();
    stream << "# Basic Info" << std::endl;
    stream << "window_width="  + to_string(CONFIG_DATA.window_width) << std::endl;
    stream << "window_height=" + to_string(CONFIG_DATA.window_height) << std::endl;
    stream << "window_mode="   + (CONFIG_DATA.window_mode) << std::endl;
    stream << "window_maximized=" + to_string(static_cast<int>(CONFIG_DATA.window_maximized)) << std::endl;
    stream << std::endl;
    stream << "# Sound Info" << std::endl;
    stream << "volume_music=" + to_string(CONFIG_DATA.volume_music) << std::endl;
    stream << "volume_effects=" + to_string(CONFIG_DATA.volume_effects) << std::endl;
    stream << "volume_speech=" + to_string(CONFIG_DATA.volume_speech) << std::endl;
    stream << std::endl;
    stream << "# Multiplayer Info" << std::endl;
    stream << "join_server_port=" + to_string(CONFIG_DATA.join_server_port) << std::endl;
    stream << "join_server_ip=" + (CONFIG_DATA.join_server_ip) << std::endl;
    stream << "join_server_player_name=" + (CONFIG_DATA.join_server_player_name) << std::endl;
    stream << "host_server_port=" + to_string(CONFIG_DATA.host_server_port) << std::endl;
    stream << "host_server_player_name=" + (CONFIG_DATA.host_server_player_name) << std::endl;
    stream << std::endl;
    stream.flush();
}
void ConfigFile::writeToFile() {
    auto stream = boost::filesystem::ofstream(file_name_with_ext);
    write_to_file(stream);
    stream.close();
}