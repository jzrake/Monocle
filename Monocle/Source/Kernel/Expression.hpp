#pragma once
#include <string>
#include <vector>
#include <set>
#include <memory>
#include "Object.hpp"

namespace mcl { class Expression; }




// ============================================================================
class mcl::Expression
{
public:
    Expression() {}

    /** Instantiate an expression from a source string. Throws std::runtime_error
        in case of a syntax error.
     */
    Expression (const std::string& expression);

    /** Evaluate an expression from the given scope. Throws std::runtime_error
        if the evaluation fails for any reason.
     */
    Object evaluate (const Object::Dict& scope) const;

    /** Evaluate an expression from the given scope. The scope function may throw
        std::runtime_error if it cannot find the given symbol.
     */
    Object evaluate (Object::Scope scope) const;

    /** Return a collection of symbols referenced by the expression.
     */
    std::set<std::string> symbols() const;

    static void testParser();

private:
    struct Part
    {
        union {
            bool        b;
            int         i;
            double      d;
            const char* s = nullptr;
        };

        char type = 0;            /**< 0 for None, otherwise one of ['b', 'i', 'd', 's'] or ['S', 'E'] */
        const char* kw = nullptr; /**< keyword name if keyword argument */
        const char* id = nullptr; /**< symbol name if symbol in scope */
        const char* er = nullptr; /**< error string if any occurred */

        size_t slen  = 0;         /**< string length if string, or num args otherwise */
        size_t kwlen = 0;         /**< kw length if keyword */
        size_t idlen = 0;         /**< id length if symbol */

        std::vector<Part> parts;  /**< Non-empty if and only if this is an expression */

        static Part error (const char* message);
        std::string str() const;
        std::string symbol() const;
        std::string keyword() const;
        std::set<std::string> symbols() const;
        Object evaluate (const Object::Dict& scope) const;
        Object evaluate (Object::Scope scope) const;
        Part withKeyword (const char* keyword, size_t len) const;
    };

    static bool isSymbolCharacter (char e);
    static bool isNumber (const char* d);
    static const char* getNamedPart (const char*& c);
    static Part parseNumber (const char*& c);
    static Part parseSymbol (const char*& c);
    static Part parseSingleQuotedString (const char*& c);
    static Part parseExpression (const char*& c);
    static Part parsePart (const char*& c);
    static Part parse (const char* expr);
    static Part error (const char* message);

    Part root;
    std::shared_ptr<std::string> source;
};
