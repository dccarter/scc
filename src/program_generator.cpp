//
// Created by Mpho Mbotho on 2020-10-26.
//

#include <scc/program_generator.hpp>
#include <scc/program.hpp>
#include <scc/meta.hpp>
#include <scc/exception.hpp>
#include <scc/visitor.hpp>
#include <scc/includes.hpp>

#include <filesystem>
#include <fstream>
#include <set>

namespace fs = std::filesystem;

namespace {
    std::optional<std::string> resolveLibPath(const scc::Library& lib) {
        auto path = fs::path{lib.Path};
        if (auto env = std::getenv("LD_LIBRARY_PATH")) {
            debug(Log::LV3) << "LD_LIBRARY_PATH=" << env << "\n";
        }
        if (!path.empty()) {
            if (!fs::exists(path)) {
                return {};
            }
            return path.string();
        }
        return "lib" + lib.Name.Content + ".so";
    }

    std::string constructHeaderGuard(const fs::path& output) {
        std::stringstream  ss;
        auto parent = output.parent_path();
        auto transform = [](char c) -> char {
            if (isalnum(c) || c == '_') {
                return std::toupper(c);
            }
            else {
                return c;
            }
        };
        if (!parent.empty()) {
            for (auto &c: parent.string()) {
                ss << transform(c);
            }
        }

        for (auto& c: output.filename().string()) {
            ss << transform(c);
        }
        return ss.str();
    }
}

namespace scc {

    void ProgramGenerator::loadLibs(const Program &pg)
    {
        Visitor<Before>(pg.before).visit<Library>([&](const Library& lib) {
            // load all requested libraries
            debug(Log::LV3) << "loading library {name=" << lib.Name.Content
                            << ", path=" << lib.Path << "}";
            if (lib.Name.Content == "meta") {
                // meta is an internal library
                return;
            }

            auto path = resolveLibPath(lib);
            if (!path) {
                throw Exception("library {name: ", lib.Name.Content, ", path: ", lib.Path, "} not found");
            }
            auto loaded = GeneratorLib::load(*path);
            if (!loaded) {
                throw Exception("library {name: ", lib.Name.Content, ", path: ", lib.Path, "} not found");
            }

            debug(Log::LV2) << " library '" << lib.Name.Content << "' loaded";
            mHasSourceGenerators = mHasSourceGenerators || loaded->hasSourceGenerators();
            loaded->setVariables(pg.before.mVars, pg.space.Name.toString());
            mGenerators.emplace(lib.Name.Content, std::move(loaded));
        });

        auto metaLib = std::make_unique<GeneratorLib>();
        metaLib->mHppGenerators.emplace("meta", std::make_shared<MetaHeader>());
        metaLib->setVariables(pg.before.mVars, pg.space.Name.toString());
        mGenerators.emplace("meta", std::move(metaLib));
    }

    void ProgramGenerator::generate(
            const Program& pg,
            const fs::path& outDir,
            const std::string& name)
    {
        std::stringstream  ss;
        if (!fs::exists(outDir)) {
            // create output directory
            debug(ss,Log::LV3) << "Creating output directory '" << outDir << "'\n";
            std::error_code ec;
            if (!fs::create_directories(outDir, ec)) {
                // failed to create directory
               throw Exception("creating output dir '", outDir, "' failed: ", ec);
            }
        }

        loadLibs(pg);

        generateHeader(pg, outDir / (name + ".scc.hpp"));

        generateSource(pg, outDir / (name + ".scc.cpp"));
    }

    void ProgramGenerator::generateHeader(const Program &pg, const std::filesystem::path &output)
    {
        debug(Log::LV2) << "scc: generating header '" << output << "'\n";
        std::ofstream ofs{output.string(), std::ofstream::out|std::ofstream::ate};
        if (!ofs) {
            throw Exception("creating file '", output, "' failed");
        }

        Formatter fmt(ofs);
        IncludeBag incs;
        Line(fmt) << "#pragma once";
        Line(fmt) << "//";
        Line(fmt) << "// !!!Generated by scc DO NOT MODIFY!!!";
        Line(fmt) << "// file: " << output;
        Line(fmt) << "// date: " << __DATE__ << " " << __TIME__;
        Line(fmt) << "//";
        Line(fmt);
        incs.write(fmt, "iod/symbols.hh");
        for (auto& [_, lib]: mGenerators) {
            for (auto& [_, gen]: lib->mHppGenerators) {
                gen->includes(fmt, incs);
            }
        }
        Line(fmt);

        std::set<std::string> generatedSymbols{};
        auto generateSymbol = [&generatedSymbols](Formatter& fmt, const Symbol& sym) {
            if (!generatedSymbols.contains(sym.Name)) {
                sym.toString(fmt);
                generatedSymbols.emplace(sym.Name);
            }
        };

        Visitor<Before>(pg.before).visit<Node>([&](const Node& node) {
            if (auto symbol = node.cast<Symbol>()) {
                generateSymbol(fmt, *symbol);
            }
            else if (node.is<Include>()) {
                incs.write(fmt, node.as<Include>());
            }
            else if (node.is<Native>() and !node.as<Native>().ForCpp) {
                // generate header only native code
                node.toString(fmt);
            }
            else if (node.is<Comment>() or node.is<Include>()) {
                // generate comments, includes, symbols and native code
                node.toString(fmt);
            }
            else if (node.is<Invoke>() and !node.as<Invoke>().ForCpp) {
                // invoke requested command using provided variables
                invoke(fmt, node.as<Invoke>(), pg.before.mVars);
            }
            else if (node.is<Library>()) {
                debug(Log::LV2) << "ignoring library code: " << node;
            }
        });

        if (pg.space) {
            Visitor<Namespace>(pg.space).visit<Struct>([&](const Struct& st) {
                // generate symbols for all fields in a struct
                Visitor<Struct>(st).visit<Field>([&](const Field& fd) {
                    Symbol sym;
                    sym.Name = fd.Name.Content;
                    generateSymbol(fmt, sym);
                });
            });

            Line(fmt);
            Line(fmt) << "#pragma GCC diagnostic push";
            Line(fmt) << R"(#pragma GCC diagnostic ignored "-Wattributes")";
            Line(fmt);

            auto nsName = pg.space.Name.toString();
            Visitor<Namespace>(pg.space).visit<Node>([&](const Node& node) {
                if (node.is<Comment>()) {
                    node.toString(fmt);
                }
                else if (node.is<Native>() and !node.as<Native>().ForCpp) {
                    UsingNamespace(nsName, fmt);
                    node.toString(fmt);
                }
                else if (node.is<Invoke>() and !node.as<Invoke>().ForCpp) {
                    // invoke requested command using provided variables
                    invoke(fmt, node.as<Invoke>(), pg.space.mVars);
                }
                else if (node.is<Class>() || node.is<Struct>() || node.is<Enum>()) {
                    // generate classes and structs in files
                    Line(fmt);
                    hppGenerateType(fmt, node.as<Type>());
                }
            });
            Line(fmt);
            Line(fmt) << "#pragma GCC diagnostic pop";
            Line(fmt);
        }

        Visitor<After>(pg.after).visit<Node>([&](const Node& node) {
            if (node.is<Native>() and !node.as<Native>().ForCpp) {
                // generate header only native content for CPP
                node.toString(fmt);
            }
            else if (node.is<Invoke>() and !node.as<Invoke>().ForCpp) {
                // invoke requested command using provided variables
                invoke(fmt, node.as<Invoke>(), pg.after.mVars);
            }
            else {
                node.toString(fmt);
            }
        });

        Line(fmt);
        debug(Log::LV3) << "header file '" << output << "' successfully generated\n";
    }

    void ProgramGenerator::generateSource(const Program &pg, const std::filesystem::path &output)
    {
        debug() << "scc: generating source file '" << output << "'\n";
        std::ofstream ofs{output.string(), std::ofstream::out|std::ofstream::ate};
        if (!ofs) {
            throw Exception("creating source file '", output, "' failed");
        }

        Formatter fmt{ofs};
        Line(fmt) << "//";
        Line(fmt) << "// !!!Generated by scc DO NOT MODIFY!!!";
        Line(fmt) << "// file: " << output.string();
        Line(fmt) << "// date: " << __DATE__ << " " << __TIME__;
        Line(fmt) << "//";
        if (!mHasSourceGenerators) {
            // Only generate source file there are source generators
            Line(fmt) << "// !!! No source generators loaded !!!";
            Line(fmt);
        }

        Line(fmt);
        Line(fmt) << R"(#include ")" << output.stem().string() << R"(.hpp")";
        Line(fmt);
        section(fmt, pg.before);
        Line(fmt);
        const auto& ns = pg.space;
        Line(fmt);

        auto nsName = pg.space.Name.toString();
        Visitor<Namespace>(pg.space).visit<Node>([&](const Node& node) {
            if (node.is<Native>() and node.as<Native>().ForCpp) {
                // native content meant for source file
                UsingNamespace(nsName, fmt);
                node.toString(fmt);
            }
            else if (node.is<Invoke>() and node.as<Invoke>().ForCpp) {
                // invoke cpp generator
                invoke(fmt, node.as<Invoke>(), pg.space.mVars);
            }
            else if (node.is<Class>() || node.is<Struct>() || node.is<Enum>()) {
                // generate classes and structs in files
                cppGenerateType(fmt, node.as<Type>());
            }
        });
        Line(fmt);
        section(fmt, pg.after);
        debug(Log::LV3) << "source file '" << output << "' successfully generated";
    }

    void ProgramGenerator::section(Formatter &fmt, const Section& sc)
    {
        Visitor<Section>(sc).visit<Node>([&](const Node& node) {
            if (node.is<Native>() and node.as<Native>().ForCpp) {
                node.toString(fmt);
            }
            else if (node.is<Invoke>() and node.as<Invoke>().ForCpp) {
                invoke(fmt, node.as<Invoke>(), sc.mVars);
            }
        });
    }

    void ProgramGenerator::invoke(Formatter& fmt, const Invoke& cmd, const Variables& vars)
    {
        auto doInvoke = [&](auto& gen) {
            // generator found
            if (cmd.ParamVar.Content.empty()) {
                gen->invoke(cmd.Function.Content, !fmt, cmd.Params);
            }
            else if (const auto& param = vars[cmd.ParamVar.Content]) {
                gen->invoke(cmd.Function.Content, !fmt, param);
            }
            else {
                throw Exception("cannot invoke '", cmd.Lib.Content, "::",
                                cmd.Generator.Content, ".", cmd.Function.Content,
                                " - variable '", cmd.ParamVar.Content,
                                "' does not exist");
            }
        };

        if (cmd.ForCpp) {
            if (auto cg = findSourceGenerator(cmd.Lib.Content, cmd.Generator.Content).lock()) {
                doInvoke(cg);
            }
            else {
                throw Exception("cannot invoke '", cmd.Lib.Content, "::",
                                cmd.Generator.Content, ".", cmd.Function.Content,
                                " in source file - source generator was not found");
            }
        }
        else {
            if (auto hg = findHeaderGenerator(cmd.Lib.Content, cmd.Generator.Content).lock()) {
                doInvoke(hg);
            }
            else {
                throw Exception("cannot invoke '", cmd.Lib.Content, "::",
                                cmd.Generator.Content, ".", cmd.Function.Content,
                                " in header file - header generator was not found");
            }
        }
    }

    void ProgramGenerator::hppGenerateType(Formatter &fmt, const Type &tp)
    {
        for (const auto& gen: tp.Generators()) {
            const auto& lib  = gen.Name[0].Content;
            const auto& name = gen.Name[1].Content;
            if (auto hg = findHeaderGenerator(lib, name).lock()) {
                Line(fmt) << "// generated by " << lib << "/" << name;
                hg->generate(!fmt, tp);
            }
            else {
                throw Exception(gen.src(), "generator '", lib, "/", name,
                                "' not found needed by type: ", tp.Name.Content);
                }
        }
    }

    void ProgramGenerator::cppGenerateType(Formatter &fmt, const Type& tp)
    {
        for (const auto& gen: tp.Generators()) {
            const auto& lib  = gen.Name[0].Content;
            const auto& name = gen.Name[1].Content;
            if (auto hg = findSourceGenerator(lib, name).lock()) {
                Line(fmt) << "#pragma region " << lib << "/" << name;
                hg->generate(!fmt, tp);
                Line(fmt) << "#pragma endregion " << lib << "/" << name;
                Line(fmt);
            }
        }
    }

    std::weak_ptr<HppGenerator> ProgramGenerator::findHeaderGenerator(
            const std::string &lib,
            const std::string &name)
    {
        static auto metaGenerator = std::make_shared<MetaHeader>();
        auto it = mGenerators.find(lib);
        if (it == mGenerators.end()) {
            return {};
        }
        if (lib == "meta") {
            return it->second->hppGenerator("meta");
        }
        return it->second->hppGenerator(name);
    }

    std::weak_ptr<CppGenerator> ProgramGenerator::findSourceGenerator(
            const std::string &lib,
            const std::string &name)
    {
        auto it = mGenerators.find(lib);
        if (it == mGenerators.end()) {
            return std::weak_ptr<CppGenerator>{};
        }
        return it->second->cppGenerator(name);
    }
}