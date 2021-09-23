//
// Created by Mpho Mbotho on 2020-10-26.
//

#include <scc/exception.hpp>
#include <scc/generator.hpp>
#include <scc/parser.hpp>
#include <scc/formatter.hpp>
#include <scc/program_generator.hpp>

#include <scc/clipp.hpp>
#include <iostream>

using scc::Exception;

using namespace clipp;

namespace  {

    void showHelp(const group& cli, const std::string& cmd)
    {
        std::cout << make_man_page(cli, "scc") << '\n';
    }

    struct ReplOptions {
        std::string Grammar;
        std::string Source;
    };

    void cmdRepl(const ReplOptions& opts)
    {
        scc::Parser p;
        if (!p.load(opts.Grammar)) {
            error() << "Loading parser failed...";
            return;
        }

        if (!opts.Source.empty()) {
            auto program = p.parse(std::filesystem::path{opts.Source});
            scc::Formatter fmt;
            program.toString(fmt);
        }
        else {
            p.repl();
        }
    }

    struct BuildOptions {
        std::string OutDir{};
        std::string Input{};
        std::vector<std::string> Inputs{};
    };

    void cmdBuild(const BuildOptions& opts)
    {
        std::filesystem::path outDir{opts.OutDir};
        auto generate = [outDir](scc::Parser& parser, const std::string& input) {
            std::filesystem::path source{input};
            if (!std::filesystem::exists(input)) {
                throw Exception("input file '", input, "' does not exist");
            }

            if (!std::filesystem::is_regular_file(source)) {
                throw Exception("input file '", input, "' is not a readable file");
            }

            auto dir = outDir;
            if (outDir.empty()) {
                if (source.has_parent_path()) {
                    dir = source.parent_path();
                }
                else {
                    dir = std::filesystem::current_path();
                }
            }

            auto name = source.stem().string();
            info() << "compiling source file " << input << "\n";
            auto program = parser.parse(input);
            if (!program) {
                throw Exception("parsing source file '", input, "' failed");
            }

            scc::ProgramGenerator generator;
            generator.generate(program, dir, name);
        };

        try {
            scc::Parser parser;
            if (!parser.load()) {
                error() << "Loading parser failed, !!!contact scc team!!!" << std::endl;
                exit(EXIT_FAILURE);
            }

            for (const auto& other: opts.Inputs) {
                generate(parser, other);
            }
            exit(EXIT_SUCCESS);
        }
        catch (Exception& ex) {
            error() << ex.message() << std::endl;
            exit(EXIT_FAILURE);
        }
        catch (...) {
            auto ex = scc::Exception::fromCurrent();
            error() << ex.message() << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[])
{
    enum class mode {help, repl, build};
    mode selected{mode::help};
    std::string helpCmd{};
    ReplOptions replOptions;
    BuildOptions buildOptions;

    auto helpMode = (
        command("help").set(selected, mode::help),
        opt_value("cmd", helpCmd)
    );
    auto buildMode = (
        command("build").set(selected, mode::build),
        (option("-O", "--outdir") & value("outdir", buildOptions.OutDir)) % "The output directory for the generated files",
        opt_values("inputs", buildOptions.Inputs)
    );

    auto relpMode = (
        command("repl").set(selected, mode::repl),
        (option("-G", "--grammar") & value("grammar", replOptions.Grammar)) % "Optional path to a grammar script to load",
        opt_value("input", replOptions.Source)
    );

    auto cli = (
        (helpMode | buildMode | relpMode));

    if (parse(argc, argv, cli)) {
        switch (selected) {
            case mode::help:
                showHelp(cli, helpCmd);
                break;
            case mode::repl:
                cmdRepl(replOptions);
                break;
            case mode::build:
                cmdBuild(buildOptions);
                break;
            default:
                error() << "unsupported command";
        }
    }
    else {
        std::cerr << usage_lines(cli, "scc") << '\n';
    }
    return 0;
}