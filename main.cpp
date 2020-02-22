#include <iostream>
#include <boost/program_options.hpp>
#include "chip8.h"

int main(int argc, char *argv[])
{
    std::pair<int, int> logical_dimensions;
    int scale_factor;
    bool fullscreen;
    std::string rom_file;

    namespace po = boost::program_options;
    po::variables_map vm;

    try
    {
        po::options_description visibleDesc("Usage: yachip8 rom_file [options]\nOptions");
        visibleDesc.add_options()("help", "Display this information and exit.")
                                 ("fullscreen", po::bool_switch(&fullscreen)->default_value(false), "Full screen mode")
                                 ("lwidth", po::value<int>(&logical_dimensions.first)->default_value(64), "Logical screen width.")
                                 ("lheight", po::value<int>(&logical_dimensions.second)->default_value(32), "Logical screen height.")
                                 ("scale", po::value<int>(&scale_factor)->default_value(16), "Scale factor to be used to determine the initial screen size.");

        po::options_description fileLocatioDesc("ROM file option");
        fileLocatioDesc.add_options()("rom-file,f", po::value<std::string>(&rom_file)->required(), "CHIP-8 ROM file location.");

        po::store(po::command_line_parser(argc, argv)
                      .options(po::options_description().add(visibleDesc).add(fileLocatioDesc))
                      .positional(po::positional_options_description().add("rom-file", 1))
                      .run(), vm);

        if (vm.count("help"))
        {
            std::cout << visibleDesc;
            return 0;
        }

        po::notify(vm);

        if ((logical_dimensions.first <= 0) ||
            (logical_dimensions.second <= 0) ||
            (scale_factor <= 0))
            {
                throw po::error("Invalid screen size arguments");
            }

        YAChip8::Chip8 chip8(logical_dimensions,
                             scale_factor,
                             rom_file);
        chip8.run();
    }
    catch (const po::error &e)
    {
        if (vm.count("rom-file") == 0)
        {
            std::cerr << "Path to a CHIP-8 ROM file is required but missing" << std::endl;
        }
        else
        {
            std::cerr << e.what() << std::endl;
        }

        return 1;
    }
    catch (const std::ios_base::failure &e)
    {
        std::cerr << "Unable to read file " << rom_file << std::endl;
        return 2;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return 3;
    }

    return 0;
}
