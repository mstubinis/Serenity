#pragma once
#ifndef GAME_CONFIG_FILE_H
#define GAME_CONFIG_FILE_H

//this class controls the wiritng of config detail to the config.cfg file

class  Core;

#include "ConfigDataStruct.h"

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

class ConfigFile final {
    friend class Core;
    public:
        static ConfigDataStruct CONFIG_DATA;
    private:
        void write_to_file(boost::filesystem::ofstream& stream);
    public:
        ConfigFile();
        ~ConfigFile();

        void updateWindowMode(const unsigned int& sfStyleEnumBitFlag);
        void updateWindowMode(const std::string& mode);
        void updateWindowSize(const unsigned int& width, const unsigned int& height);
        void updateWindowMaximized(const bool& maximized);

        void updateHostServerName(const std::string&);
        void updateHostServerPort(const unsigned short&);

        void writeToFile();
        const ConfigDataStruct& readFromFile();
};

#endif