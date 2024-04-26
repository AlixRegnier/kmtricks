#include <fstream>
#include <iostream>
#include <filesystem>
#include "ConfigurationLiterate.h"

ConfigurationLiterate::ConfigurationLiterate(const std::string& filename, bool load_file) 
{ 
    load(filename, load_file);
}

void ConfigurationLiterate::load(const std::string& filename, bool read_file)
{
    this->filename = filename;
    if(read_file && std::filesystem::exists(filename))
        read();
}

void ConfigurationLiterate::read()
{
    //Parse configuration file
    //Assignments must be specified this way:
    //  PROPERTY1 = VALUE
    //  PROPERTY2 = VALUE
    //  ...

    if(!std::filesystem::exists(filename))
        throw std::runtime_error("File: " + filename + " does not exist !");

    std::string property;
    std::size_t value;
    std::ifstream config(filename);
    char equalSign;

    while(config)
    {
        std::string line;
        std::getline(config, line);
        std::istringstream iss(line);

        if (iss >> property >> equalSign >> value && (equalSign == '=' || equalSign == ':'))
            set_property(to_lower_case(property), value);
        else if(line != "")
            throw std::invalid_argument("Invalid format for property, received: '" + line + "'");
    }

    //Check if parameters values are valid
    assert_valid_config();
}

void ConfigurationLiterate::write() const
{
    assert_valid_config();

    std::ofstream config(filename);
    
    config << "samples = " << nb_samples << "\n";
    config << "linesperblock = " << lines_per_block << "\n";
    config << "preset = " << preset_level << "\n";
}

void ConfigurationLiterate::assert_valid_config() const
{
    if(nb_samples == 0)
        throw std::invalid_argument("Invalid number of input samples.");

    if(preset_level > 9)
        throw std::invalid_argument("Preset compression level shall be in [0;9]");

    if(lines_per_block == 0)
        throw std::invalid_argument("The number of lines per block shall be at least 1.");
}

std::size_t ConfigurationLiterate::get_nb_samples() const { return nb_samples; }
std::size_t ConfigurationLiterate::get_lines_per_block() const { return lines_per_block; }
std::uint8_t ConfigurationLiterate::get_preset_level() const { return preset_level; }

void ConfigurationLiterate::set_nb_samples(std::size_t nb_samples) { assert(this->nb_samples > 0); this->nb_samples = nb_samples; }
void ConfigurationLiterate::set_lines_per_block(std::size_t lines_per_block) { assert(this->lines_per_block > 0); this->lines_per_block = lines_per_block; }
void ConfigurationLiterate::set_preset_level(std::uint8_t preset_level) { assert(this->preset_level < 10); this->preset_level = preset_level; }

//Modify string 's' in lower case
std::string& ConfigurationLiterate::to_lower_case(std::string& s)
{
    for(size_t i = 0; i < s.size(); ++i)
        if(s[i] >= 'A' && s[i] <= 'Z')
            s[i] += (char)32;
    return s;
}

void ConfigurationLiterate::set_property(const std::string& property, std::size_t value)
{
    if(property == "samples")
    {
        set_nb_samples(value);
    }
    else if(property == "linesperblock")
    {
        set_lines_per_block(value);
    }
    else if(property == "preset")
    {
        set_preset_level(value);
    }
    else
    {
        throw std::runtime_error("Unknown property: '" + property + "'");
    }
}