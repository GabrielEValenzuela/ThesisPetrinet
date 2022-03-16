#include <iostream>
#include "../include/Engine.hpp"
#include "../lib/args.hxx"

#define VERSION_NUMBER "1.1.9"  ///< Version number of the application


int main(int argc, char **argv) {

    args::ArgumentParser parser(
            "This is a complement of Petrinator.",
            "This program allowed to simulate in a concurrent way, a PN and"
            "calculate the Minimal Coverability Tree");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::Flag simulation(parser, "simulation", "The simulation flag", {'s', "sim"});
    args::Flag algorithm(parser, "algorithm", "The algorithm flag", {'a', "alg"});
    args::Flag logger(parser,"logger","Enable the logger's monitor. This flag works only if you run a simulation",
                      {'l',"log"});
    args::Positional<std::string> filename(parser, "filename", "The PN in JSON or PNML format");
    args::Flag version(parser, "version", "Display the version number of the executable", {'v', "version"});

    try {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Help &error) {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError &error) {
        std::cerr << error.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (const args::ValidationError &error) {
        std::cerr << error.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    if (version) {
        std::cout << VERSION_NUMBER << std::endl;
        return 0;
    }

    if (filename) {
        auto engine = std::make_unique<Engine>();
        if (simulation && logger) {
            std::cout << "Starting simulation with logger...\n";
            engine->run(std::move(args::get(filename)), 2);

        } else if (simulation) {
            std::cout << "Starting simulation without logger...\n";
            engine->run(std::move(args::get(filename)), 0);

        } else if (algorithm){
            std::cout << "Starting minimal coverability algorithm...\n";
            engine->run(std::move(args::get(filename)), 1);
        }
        else {
            std::cout << "Invalid option, plese, run with -h or --help to see the options\n";
            exit(0);
        }
    } else {
        std::cout << "Missing file name, please use a JSON or PNML format\n";
        exit(0);
    }
    std::cout << "*** DONE ***\t Program ends successfully ! :)\n";
}