//
// Created by Mathias on 10-05-2021.
//

#ifndef SP_EXAM_PROJECT_SYMBOL_TABLE_H
#define SP_EXAM_PROJECT_SYMBOL_TABLE_H

#include <unordered_map>
#include <string>
#include <stdexcept>
#include <utility>
#include <iterator>

namespace StochasticSimulation {

    struct SymbolTableException : public std::exception
    {
        std::string message;
    public:
        explicit SymbolTableException(std::string message): message(std::move(message))
        {}

        [[nodiscard]] const char* what() const override
        {
            return message.c_str();
        }
    };


    template<typename T>

    class symbol_table: public std::iterator<std::output_iterator_tag, T> {
    private:
        std::unordered_map<std::string, T> map{};
    public:
        symbol_table<T>() = default;

        void put(const std::string& key, T value) {
            if (!map.contains(key)) {
                map.insert_or_assign(key, value);
            } else {
                throw SymbolTableException("Key already used");
            }
        }

        T get(const std::string& key) {
            try {
                return map.at(key);
            } catch (std::out_of_range& e) {
                throw SymbolTableException("Key was not found in symbol table");
            }
        }

        std::unordered_map<std::string, T> getMap() {
            return map;
        }

    };



}

#endif //SP_EXAM_PROJECT_SYMBOL_TABLE_H
