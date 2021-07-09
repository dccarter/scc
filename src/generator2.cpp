//
// Created by Mpho Mbotho on 2020-10-25.
//

#include <scc/program_generator.hpp>
#include <scc/exception.hpp>

#include <dlfcn.h>
#include <cstring>
#include <sstream>

namespace {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
    static constexpr char* GeneratorLibInit = "LibInitialize";
    static constexpr char* GeneratorLibDeInit = "LibDeInitialize";
#pragma GCC diagnostic pop
}

static Log configureLog{Log::LV1};
Log getConfiguredLog() {
    return configureLog;
}

namespace scc {

    GeneratorLib::~GeneratorLib()
    {
        mCppGenerators.clear();
        mHppGenerators.clear();
        if (mLibHandle != nullptr) {
            if (dlclose(mLibHandle)) {
                // error closing handle
                error() << "closing library handle failed: " << dlerror();
            }
            mLibHandle = nullptr;
        }
    }

    std::unique_ptr<GeneratorLib> GeneratorLib::load(const std::string& lib)
    {
        auto handle = dlopen(lib.c_str(), RTLD_NOW);
        if (handle == nullptr) {
            // failed to open library
            throw Exception("opening '", lib,  "' failed: ", dlerror());
        }

        auto initFuncHandle = dlsym(handle, GeneratorLibInit);
        if (initFuncHandle == nullptr) {
            // could not load initialize function
            std::string ex{dlerror()};
            dlclose(handle);
            throw Exception("could not load '", GeneratorLibInit, "' function from lib '", lib, "': ", ex);
        }

        auto initFunc = reinterpret_cast<GeneratorInit>(initFuncHandle);
        if (initFunc == nullptr) {
            std::string ex{dlerror()};
            dlclose(handle);
            throw Exception("function '", GeneratorLibInit, "' is not a valid LibInitialize function");
        }

        auto generatorLib = std::make_unique<GeneratorLib>();
        auto status = initFunc(generatorLib.get());
        if (status != 0) {
            dlclose(handle);
            throw Exception("Library '", lib, "' initialize function failed: ", strerror(status));
        }
        generatorLib->mLibHandle = handle;
        return std::move(generatorLib);
    }

    void GeneratorLib::setVariables(const Variables& variables, const std::string& ns)
    {
        for (auto& hg: mHppGenerators) {
            hg.second->mVariables = &variables;
            hg.second->mNamespace = ns;
        }

        for (auto& cg: mCppGenerators) {
            cg.second->mVariables = &variables;
            cg.second->mNamespace = ns;
        }
    }

    void GeneratorLib::addGenerator(
                const std::string& name,
                std::shared_ptr<CppGenerator> sg,
                std::shared_ptr<HppGenerator> hg)
    {
        if (hg != nullptr) {
            if (mHppGenerators.find(name) != mHppGenerators.end()) {
                throw Exception("HPP generator with name '", name, "' already registered in current library");
            }
            mHppGenerators.emplace(name, std::move(hg));
        }

        if (sg != nullptr) {
            if (mCppGenerators.find(name) != mCppGenerators.end()) {
                throw Exception("CPP generator with name '", name, "' already registered in current library");
            }
            mHasCppGenerators = true;
            mCppGenerators.emplace(name, std::move(sg));
        }
    }

    std::weak_ptr<HppGenerator> GeneratorLib::hppGenerator(const std::string &name)
    {
        auto it = mHppGenerators.find(name);
        if (it != mHppGenerators.end()) {
            return it->second;
        }
        return std::weak_ptr<HppGenerator>{};
    }

    std::weak_ptr<CppGenerator> GeneratorLib::cppGenerator(const std::string &name)
    {
        auto it = mCppGenerators.find(name);
        if (it != mCppGenerators.end()) {
            return it->second;
        }
        return std::weak_ptr<CppGenerator>{};
    }
}