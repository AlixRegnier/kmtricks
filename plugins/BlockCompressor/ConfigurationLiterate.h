#ifndef CONFIGURATELITERATE_H
#define CONFIGURATELITERATE_H

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

class ConfigurationLiterate
{
    public:
        ConfigurationLiterate(const std::string& = "", bool=true);

        void load(const std::string&, bool=true);
        void read();
        void write() const;

        std::size_t get_nb_samples() const;
        std::size_t get_lines_per_block() const;
        std::uint8_t get_preset_level() const;

        void set_nb_samples(std::size_t);
        void set_lines_per_block(std::size_t);
        void set_preset_level(std::uint8_t);
        
    private:
        //Default values are invalid for ensuring the use of a further proper configuration
        std::size_t nb_samples = 0;
        std::size_t lines_per_block = 0;
        std::uint8_t preset_level = 10;
        std::string filename;

        //Modify string 's' in lower case
        static std::string& to_lower_case(std::string&);

        void assert_valid_config() const;

        void set_property(const std::string&, std::size_t);
};

#endif