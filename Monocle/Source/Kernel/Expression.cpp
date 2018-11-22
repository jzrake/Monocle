#include <cassert>
#include "Expression.hpp"
using namespace mcl;




// ============================================================================
//Expression::Part Expression::symbol (const std::string& name)
//{
//    for (auto c : name)
//        if (! isSymbolCharacter(c))
//            throw std::invalid_argument ("Expression::symbol: invalid symbol name");
//
//    Part part;
//    part.id = part.source.data();
//    part.kw = nullptr;
//    part.kwlen = 0;
//    part.type = 'S';
//    part.idlen = name.size();
//    return part;
//}

//Expression::Part Expression::object (const Object& value, const std::string& keyword)
//{
//    for (auto c : keyword)
//        if (! isSymbolCharacter(c))
//            throw std::invalid_argument ("Expression::symbol: invalid keyword name");
//
//    Part part;
//    part.source = keyword;
//    part.id = nullptr;
//    part.kw = part.source.data();
//    part.kwlen = keyword.size();
//    part.type = 'O';
//    part.idlen = 0;
//    part.value = value;
//    return part;
//}

Expression::Expression (const std::string& expression)
{
    source = std::make_shared<std::string> (expression);
    auto c = source->data();
    root = parsePart (c);

    if (root.er)
    {
        throw std::runtime_error (root.er);
    }
}

//Expression::Expression (Part root) : root (root)
//{
//}

//Expression::Expression (std::initializer_list<Part> parts)
//{
//    root.source = std::string();
//    root.id = nullptr;
//    root.kw = nullptr;
//    root.kwlen = 0;
//    root.type = 'E';
//    root.idlen = 0;
//    root.value = Object::none();
//    root.parts = parts;
//}

Object Expression::evaluate (const Object::Dict& scope) const
{
    return root.evaluate (scope);
}

Object Expression::evaluate (Object::Scope scope) const
{
    return root.evaluate (scope);
}

std::set<std::string> Expression::symbols() const
{
    return root.symbols();
}

std::vector<std::string> Expression::getListParts() const
{
    auto p = std::vector<std::string>();

    for (const auto& part : root.parts)
    {
        p.push_back (part.source());
    }
    return p;
}




// ========================================================================
bool Expression::isSymbolCharacter (char e)
{
    return isalnum (e) || e == '_' || e == '-' || e == ':';
}

bool Expression::isNumber (const char* d)
{
    if (isdigit (*d))
    {
        return true;
    }
    else if (*d == '.')
    {
        return isdigit (d[1]);
    }
    else if (*d == '+' || *d == '-')
    {
        return isdigit (d[1]) || (d[1] == '.' && isdigit (d[2]));
    }
    return false;
}

const char* Expression::getNamedPart (const char*& c)
{
    const char* cc = c;
    const char* start = c;

    while (isSymbolCharacter (*cc++))
    {
        if (*cc == '=')
        {
            c = cc + 1;
            return start;
        }
    }
    return nullptr;
}

Expression::Part Expression::parseNumber (const char*& c)
{
    const char* start = c;
    bool isdec = false;
    bool isexp = false;

    if (*c == '+' || *c == '-')
    {
        ++c;
    }

    while (isdigit (*c) || *c == '.' || *c == 'e' || *c == 'E')
    {
        if (*c == 'e' || *c == 'E')
        {
            if (isexp)
            {
                return Part::error ("Syntax error: bad numeric literal");   
            }
            isexp = true;
        }
        if (*c == '.')
        {
            if (isdec || isexp)
            {
                return Part::error ("Syntax error: bad numeric literal");   
            }
            isdec = true;
        }
        ++c;
    }

    if (! (isspace (*c) || *c == '\0' || *c == ')'))
    {
        return Part::error ("Syntax error: bad numeric literal");
    }

    Part part;
    part.st = start;
    part.en = c;

    if (isdec || isexp)
    {
        part.d = atof (std::string (start, c - start).data());
        part.type = 'd';
    }
    else
    {
        part.i = atoi (std::string (start, c - start).data());
        part.type = 'i';
    }
    return part;
}

Expression::Part Expression::parseSymbol (const char*& c)
{
    const char* start = c;

    while (isSymbolCharacter (*c))
    {
        ++c;
    }

    Part part;
    part.st = start;
    part.en = c;
    part.id = start;
    part.type = 'S';
    part.idlen = c - start;
    return part;
}

Expression::Part Expression::parseSingleQuotedString (const char*& c)
{
    const char* start = c;

    assert (*c == '\'');
    ++c;

    while (*c != '\'')
    {
        if (*c == '\0')
        {
            return Part::error ("Syntax error: unterminated string");
        }
        ++c;
    }

    ++c;

    if (! (isspace (*c) || *c == '\0' || *c == ')'))
    {
        return Part::error ("Syntax error: non-whitespace character following single-quoted string");
    }
    Part part;
    part.st = start;
    part.en = c;
    part.s = start + 1;
    part.type = 's';
    part.slen = c - start - 2;
    return part;
}

Expression::Part Expression::parseExpression (const char*& c)
{
    Part part;
    part.st = c;

    assert (*c == '(');
    ++c;

    while (*c != ')')
    {
        if (*c == '\0')
        {
            return Part::error ("Syntax error: unterminated expression");
        }
        else if (isspace (*c))
        {
            ++c;
        }
        else
        {
            Part p = parsePart (c);

            if (p.er)
            {
                return p;
            }
            part.parts.push_back (p);
        }
    }

    ++c;

    part.type = 'E';
    part.en = c;
    return part;
}

Expression::Part Expression::parsePart (const char*& c)
{
    const char* kw = nullptr;
    size_t kwlen = 0;

    while (*c != '\0')
    {
        if (isspace (*c))
        {
            ++c;
        }
        else if (const char* kwstart = getNamedPart (c))
        {
            kwlen = c - (kw = kwstart) - 1;
        }
        else if (isNumber (c))
        {
            return parseNumber (c).withKeyword (kw, kwlen);
        }
        else if (isalpha (*c))
        {
            return parseSymbol (c).withKeyword (kw, kwlen);
        }
        else if (*c == '\'')
        {
            return parseSingleQuotedString (c).withKeyword (kw, kwlen);
        }
        else if (*c == '(')
        {
            return parseExpression (c).withKeyword (kw, kwlen);
        }
        else
        {
            return Part::error ("Syntax error: unkown character");
        }
    }
    return Part();
}

Expression::Part Expression::parse (const char* expr)
{
    return parsePart (expr);
}




// ============================================================================
Expression::Part Expression::Part::error (const char* message)
{
    Part part;
    part.er = message;
    return part;
}

std::string Expression::Part::source() const
{
    return std::string (st, en);
}

std::string Expression::Part::str() const
{
    return std::string (s, s + slen);
}

std::string Expression::Part::symbol() const
{
    return std::string (id, id + idlen);
}

std::string Expression::Part::keyword() const
{
    return std::string (kw, kw + kwlen);
}

std::set<std::string> Expression::Part::symbols() const
{
    if (id != nullptr)
    {
        return { symbol() };
    }

    std::set<std::string> s;

    for (const auto& part : parts)
    {
        auto ps = part.symbols();
        s.insert (ps.begin(), ps.end());
    }
    return s;
}

Object Expression::Part::evaluate (const Object::Dict& scope) const
{
    return evaluate ([&scope] (const auto& key) { return scope.at (key); });
}

Object Expression::Part::evaluate (Object::Scope scope) const
{
    switch (type)
    {
        case 'b': return b;
        case 'i': return i;
        case 'd': return d;
        case 's': return str();
        case 'S': return scope (symbol());
        case 'E':
        {
            if (parts.size() == 0)
            {
                return Object::None();
            }
            else if (scope (parts.at (0).symbol()).type() != 'F')
            {
                throw std::runtime_error ("Expression head is not a function");
            }

            auto func = scope (parts.at (0).symbol()).get<Object::Func>().f;
            auto args = Object::List();
            auto kwar = Object::Dict();
            auto first = true;

            for (const auto& part : parts)
            {
                if (! first)
                {
                    if (part.kw)
                    {
                        kwar.emplace (part.keyword(), part.evaluate (scope));
                    }
                    else
                    {
                        args.push_back (part.evaluate (scope));
                    }
                }
                else
                {
                    first = false;
                }
            }
            return func (args, kwar);
        }
        default: assert (type == 0); return Object();
    }
}

Expression::Part Expression::Part::withKeyword (const char* keyword, size_t len) const
{
    auto p = *this;
    p.kw = keyword;
    p.kwlen = len;
    return p;
}




// ============================================================================
void Expression::testParser()
{
    assert (parse ("12").i == 12);
    assert (parse ("13").i == 13);
    assert (parse ("13a").er != nullptr);
    assert (parse ("13.5").type == 'd');
    assert (parse ("13.5").d == 13.5);
    assert (parse ("+12").i == 12);
    assert (parse ("-12").i ==-12);
    assert (parse ("+13.5").d == 13.5);
    assert (parse ("-13.5").d ==-13.5);
    assert (parse ("+13.5e2").d == 13.5e2);
    assert (parse ("-13.5e2").d ==-13.5e2);
    assert (parse ("+13e2").d == 13e2);
    assert (parse ("-13e2").d ==-13e2);
    assert (parse ("-.5").d == -.5);
    assert (parse ("+.5").d == +.5);
    assert (parse (".5").d == +.5);
    assert (parse ("-").er != nullptr);
    assert (parse ("1e2e2").er != nullptr);
    assert (parse ("1.2.2").er != nullptr);
    assert (parse ("1e2.2").er != nullptr);

    assert (parse ("Dog").type == 'S');
    assert (parse ("Dog").idlen == 3);
    assert (parse ("Dog").symbol() == "Dog");
    assert (parse ("'Hello'").type == 's');
    assert (parse ("'Hello'").slen == 5);
    assert (parse ("'Hello'").str() == "Hello");
    assert (parse ("'Hello, cat'").str() == "Hello, cat");

    assert (parse ("(Dog 5)").type == 'E');
    assert (parse ("(Dog 5)").parts.size() == 2);
    assert (parse ("(Dog 5)").parts[0].type == 'S');
    assert (parse ("(Dog 5)").parts[0].symbol() == "Dog");
    assert (parse ("(Dog 5)").parts[1].type == 'i');
    assert (parse ("(Dog 5)").parts[1].i == 5);
    assert (parse ("(Dog 5 )").parts[1].i == 5);

    assert (parse ("(Dog 9").er != nullptr);
    assert (parse ("'Cat 9").er != nullptr);

    assert (parse ("(Cat 'chicken' 'goose' 12)").er == nullptr);
    assert (parse ("(Cat 'chicken' 'goose' 12)").type == 'E');
    assert (parse ("(add (mul 1 2) (div 2 3))").type == 'E');

    assert (parse ("a=1").type == 'i');
    assert (parse ("a=1").keyword() == "a");
    assert (parse ("cow='moo'").type == 's');
    assert (parse ("cow='moo'").keyword() == "cow");
    assert (parse ("deer=(0 1 2 3)").type == 'E');
    assert (parse ("deer=(0 1 2 3)").keyword() == "deer");
    assert (parse ("deer=(0 1 2 3)").parts.size() == 4);
    assert (parse ("deer=(0 1 2 3)").parts[0].i == 0);
    assert (parse ("deer=(0 1 2 3)").parts[3].i == 3);

    assert (parse ("(a b c)").symbols().size() == 3);
    assert (parse ("(a b (A B C))").symbols().size() == 5);
    assert (parse ("(a b (a b c))").symbols().size() == 3);

    auto s = Object::Dict();
    s.emplace ("a", 1.0);
    s.emplace ("b", 2.0);
    s.emplace ("add", Object::Func ([] (const Object::List& ar, const Object::Dict& kw)
    {
        return ar.at (0).get<double>() + ar.at (1).get<double>();
    }));

    assert (Expression ("a").evaluate (s).get<double>() == 1.0);
    assert (Expression ("b").evaluate (s).get<double>() == 2.0);
    assert (Expression ("(add a b)").evaluate (s).get<double>() == 3.0);
    assert (Expression ("(add a b)").root.parts[0].source() == "add");
    assert (Expression ("(add a b)").root.parts[1].source() == "a");
    assert (Expression ("(add a b)").root.parts[2].source() == "b");
}

void Expression::testProgrammaticConstruction()
{
//    Object::Dict scope;
//    scope["a"] = 12;
//    scope["f"] = Object::Func ([] (auto, auto) { return 10; });
//    assert (Expression::symbol ("a").parts.empty());
//    assert (Expression (Expression::symbol ("a")).evaluate (scope) == 12);
//    assert (Expression ({Expression::symbol ("f")}).evaluate (scope) == 10);
}
