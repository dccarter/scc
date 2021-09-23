//
// Created by Mpho Mbotho on 2020-10-24.
//

#ifndef SCC_PROGRAM_GENERATOR_HPP
#define SCC_PROGRAM_GENERATOR_HPP

#include <scc/generator.hpp>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <optional>

namespace scc {

    class Program;
    class Namespace;
    class Section;
    class Invoke;
    class Variables;

    class GeneratorLib final {
    public:
        using HppGenerators = std::unordered_map<std::string, std::shared_ptr<HppGenerator>>;
        using CppGenerators = std::unordered_map<std::string, std::shared_ptr<CppGenerator>>;

        void addGenerator(
                    const std::string& name,
                    std::shared_ptr<CppGenerator> sg,
                    std::shared_ptr<HppGenerator> hg);

        inline bool hasSourceGenerators() const { return mHasCppGenerators; }

        std::weak_ptr<HppGenerator> hppGenerator(const std::string& name);
        std::weak_ptr<CppGenerator> cppGenerator(const std::string& name);
        const HppGenerators& getHeaderGenerators() { return mHppGenerators; }
        const CppGenerators& getSourceGenerators() { return mCppGenerators; }

        static std::unique_ptr<GeneratorLib> load(const std::string& lib);

        GeneratorLib() = default;
        ~GeneratorLib();

    private:
        GeneratorLib(const GeneratorLib&) = delete;
        GeneratorLib& operator=(const GeneratorLib&) = delete;
        GeneratorLib(GeneratorLib&&) = delete;
        GeneratorLib& operator=(GeneratorLib&&) = delete;
        friend class ProgramGenerator;
        void setVariables(const Variables& variables, const std::string& ns);
        HppGenerators mHppGenerators;
        CppGenerators mCppGenerators;
        Handle mLibHandle{nullptr};
        bool   mHasCppGenerators{false};
    };

    class ProgramGenerator final {
    public:
        ProgramGenerator() = default;
        void generate(const Program& pg, const std::filesystem::path& ourDir, const std::string& name);

    private:
        using GeneratorLibs = std::unordered_map<std::string, std::unique_ptr<GeneratorLib>>;
        void generateHeader(const Program& pg, const std::filesystem::path& output);
        void generateSource(const Program& pg, const std::filesystem::path& output);
        std::weak_ptr<HppGenerator> findHeaderGenerator(const std::string& lib, const std::string& name);
        std::weak_ptr<CppGenerator> findSourceGenerator(const std::string& lib, const std::string& name);
        void loadLibs(const Program& pg);
        void section(Formatter& fmt, const Section& sc);
        void invoke(Formatter& fmt, const Invoke& cmd, const Variables& vars);
        void hppGenerateType(Formatter& fmt, const Type& type);
        void cppGenerateType(Formatter& fmt, const Type& type);
    private:
        GeneratorLibs  mGenerators;
        bool           mHasSourceGenerators{false};
    };
}
#endif //SCC_WRITER_HPP
