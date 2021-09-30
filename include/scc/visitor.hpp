//
// Created by Mpho Mbotho on 2020-10-31.
//

#ifndef SCC_VISITOR_HPP
#define SCC_VISITOR_HPP

#include <scc/program.hpp>

#include <functional>

namespace scc {

    template <typename T>
    requires (std::is_base_of_v<Node, T>)
    class Visitor {
    public:
        template<typename TT>
        requires (std::is_base_of_v<Node, TT>)
        using Func = std::function<void(const TT&)>;

        Visitor(const T& node)
            : node{node}
        {}

        template<typename TT>
            requires (std::is_same_v<T, Program> and
                      (std::is_same_v<Node, TT> or std::is_base_of_v<Node, TT>))
        void visit(Func<TT> func)
        {
            if (func == nullptr) return;
            const auto& program = node.as<Program>();

            if constexpr (std::is_same_v<TT, Node>) {
                Visitor<Before>(program.before).visit<Node>(func);
                Visitor<Namespace>(program.space).visit<Node>(func);
                Visitor<After>(program.after).visit<Node>(func);
            }
            else {
                if constexpr (std::is_same_v<TT, Before> or std::is_same_v<Section, TT>) {
                    func(program.before);
                }

                if constexpr (std::is_same_v<TT, Namespace> or std::is_same_v<Section, TT>) {
                    func(program.space);
                }

                if constexpr (std::is_same_v<TT, After> or std::is_same_v<Section, TT>) {
                    func(program.after);
                }
            }
        }

        template<typename TT>
        requires (std::is_same_v<T, Section> and
                  (std::is_same_v<Node, TT> or std::is_base_of_v<Node, TT>))
        void visit(Func<TT> func)
        {
            if (func == nullptr) return;
            const auto& sec = node.as<Section>();
            for (const auto& n: sec.Content) {
                if constexpr (std::is_same_v<TT, Node>) {
                    func(*n);
                }
                else {
                    if (n->is<TT>()) {
                        func(n->as<TT>());
                    }
                }
            }
        }

        template<typename TT>
            requires (std::is_same_v<T, Namespace> and
                      (std::is_same_v<Node, TT> or std::is_base_of_v<Node, TT>))
        void visit(Func<TT> func)
        {
            if (func == nullptr) return;
            const auto& ns = node.as<Namespace>();
            if constexpr (std::is_same_v<Scoped, TT>) {
                func(ns.Name);
            }
            else {
                for (const auto& content: ns.Content) {
                    if constexpr (std::is_same_v<Node, TT>) {
                        func(*content);
                    } else if constexpr (std::is_same_v<Type, TT>) {
                        if (content->is<Class>() or content->is<Struct>()) {
                            func(content->as<Type>());
                        }
                    } else {
                        if (content->is<TT>()) {
                            func(content->as<TT>());
                        }
                    }
                }
            }
        }

        template<typename TT>
        requires (std::is_same_v<T, Before> and
                  (std::is_same_v<Node, TT> or std::is_base_of_v<Node, TT>))
        void visit(Func<TT> func)
        {
            if (func == nullptr) return;
            const auto& before = node.as<Before>();
            for (const auto& content: before.Content) {
                if constexpr (std::is_same_v<Node, TT>) {
                    func(*content);
                } else {
                    if (content->is<TT>()) {
                        func(content->as<TT>());
                    }
                }
            }
        }

        template<typename TT>
        requires (std::is_same_v<T, After> and
                  (std::is_same_v<Node, TT> or std::is_base_of_v<Node, TT>))
        void visit(Func<TT> func)
        {
            if (func == nullptr) return;
            const auto& before = node.as<After>();
            for (const auto& content: before.Content) {
                if constexpr (std::is_same_v<Node, TT>) {
                    func(*content);
                } else {
                    if (content->is<TT>()) {
                        func(content->as<TT>());
                    }
                }
            }
        }

        template<typename TT>
        requires (std::is_same_v<T, Class> and
                  (std::is_same_v<Node, TT> or std::is_base_of_v<Node, TT>))
        void visit(Func<TT> func)
        {
            if (func == nullptr) return;
            const auto& klass = node.as<Class>();
            if constexpr (std::is_same_v<Base, TT>) {
                for (const auto& base: klass.BaseClasses) {
                    func(base);
                }
            }
            else if constexpr (std::is_same_v<TT, Generator>) {
                for (const auto& gen: klass.Generators()) {
                    func(gen);
                }
            }
            else if constexpr (std::is_same_v<TT, Annotation>) {
                for (const auto& ann: klass.Annotations()) {
                    func(ann);
                }
            }
            else if constexpr (std::is_same_v<TT, Ident>) {
                func(klass.Name);
            }
            else {
                for (const auto &member: klass.Members) {
                    if constexpr (std::is_same_v<TT, Node>) {
                        func(*member);
                    }
                    else {
                        if (member->is<TT>()) {
                            func(member->as<TT>());
                        }
                    }
                }
            }
        }

        template<typename TT>
        requires (std::is_same_v<T, Struct> and
                  (std::is_same_v<Node, TT> or std::is_base_of_v<Node, TT>))
        void visit(Func<TT> func)
        {
            if (func == nullptr) return;
            const auto& klass = node.as<Struct>();

            if constexpr (std::is_same_v<TT, Generator>) {
                for (const auto& gen: klass.Generators()) {
                    func(gen);
                }
            }
            else if constexpr (std::is_same_v<TT, Annotation>) {
                for (const auto& ann: klass.Annotations()) {
                    func(ann);
                }
            }
            else if constexpr (std::is_same_v<TT, Ident>) {
                func(klass.Name);
            }
            else {
                for (const auto &member: klass.Members) {
                    if constexpr (std::is_same_v<TT, Node>) {
                        func(*member);
                    }
                    else {
                        if (member->is<TT>()) {
                            func(member->as<TT>());
                        }
                    }
                }
            }
        }

        template<typename TT>
        requires (std::is_same_v<T, Enum> and
                  (std::is_same_v<Node, TT> or std::is_base_of_v<Node, TT>))
        void visit(Func<TT> func)
        {
            if (func == nullptr) return;
            const auto& klass = node.as<Enum>();

            if constexpr (std::is_same_v<TT, Annotation>) {
                for (const auto& ann: klass.Annotations()) {
                    func(ann);
                }
            }
            else if constexpr (std::is_same_v<TT, Generator>) {
                for (const auto& gen: klass.Generators()) {
                    func(gen);
                }
            }
            else if constexpr (std::is_same_v<TT, Ident>) {
                func(klass.Name);
            }
            else {
                for (const auto &member: klass.Members) {
                    if constexpr (std::is_same_v<TT, Node>) {
                        func(*member);
                    }
                    else {
                        if (member->is<TT>()) {
                            func(member->as<TT>());
                        }
                    }
                }
            }
        }

        template<typename TT>
        requires (std::is_same_v<T, Type> and
                  (std::is_same_v<Node, TT> or std::is_base_of_v<Node, TT>))
        inline void visit(Func<TT> func) {
            if (node.is<Class>()) {
                Visitor<Class>(node).visit(func);
            }
            else if (node.is<Struct>()) {
                Visitor<Struct>(node).visit(func);
            }
            else {
                Visitor<Enum>(node)(func);
            }
        }

    private:
        const Node& node;
    };
}

#endif //SCC_VISITOR_HPP
