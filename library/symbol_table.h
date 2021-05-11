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
        using map_type = std::unordered_map<std::string, T>;
    private:
        map_type map{};
    public:
        using iterator = typename map_type::iterator;
        using const_iterator = typename map_type::const_iterator;

        symbol_table<T>() = default;

        symbol_table<T>(const symbol_table<T>& a) = default;

//        ~symbol_table() = default;
//
//        symbol_table& operator=(const symbol_table&) = default;

        void put(const std::string& key, T value) {
            if (!map.contains(key)) {
                map.insert_or_assign(key, value);
            } else {
                throw SymbolTableException("Key already used");
            }
        }

        T& get(const std::string& key) {
            try {
                return map.at(key);
            } catch (std::out_of_range& e) {
                throw SymbolTableException("Key was not found in symbol table");
            }
        }

        const T& get(const std::string& key) const {
            try {
                return map.at(key);
            } catch (std::out_of_range& e) {
                throw SymbolTableException("Key was not found in symbol table");
            }
        }

        bool contains(const std::string& key) {
            return map.contains(key);
        }

        std::unordered_map<std::string, T> getMap() {
            return map;
        }

        iterator begin() {
            return map.begin();
        }

        iterator end() {
            return map.end();
        }

        const_iterator begin() const {
            return map.begin();
        }

        const_iterator end() const {
            return map.end();
        }

    };
}

#endif //SP_EXAM_PROJECT_SYMBOL_TABLE_H
