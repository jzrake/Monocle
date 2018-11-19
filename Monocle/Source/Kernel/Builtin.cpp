#include <cmath>
#include "Builtin.hpp"
using namespace mcl;




// ============================================================================
Object::Dict Builtin::builtin()
{
    auto m = Object::Dict();
    m["item" ] = Object::Func (item);
    m["attr" ] = Object::Func (attr);
    m["list" ] = Object::Func (list);
    m["dict" ] = Object::Func (dict);
    m["join" ] = Object::Func (join);
    m["merge"] = Object::Func (merge);
    m["range"] = Object::Func (range);
    return m;
}

Object Builtin::item (const Object::List& args, const Object::Dict&)
{
    auto L = Builtin::check<Object::List> (args, 0);
    auto I = Builtin::check<int> (args, 1);
    return L[I];
}

Object Builtin::attr (const Object::List& args, const Object::Dict&)
{
    auto D = Builtin::check<Object::Dict> (args, 0);
    auto I = Builtin::check<std::string> (args, 1);
    return D[I];
}

Object Builtin::list (const Object::List& args, const Object::Dict&)
{
    return args;
}

Object Builtin::dict (const Object::List&, const Object::Dict& kwar)
{
    return kwar;
}

Object Builtin::join (const Object::List& args, const Object::Dict&)
{
    auto res = Object::List();
    int m = 0;

    for (int n = 0; n < args.size(); ++n)
        for (const auto& entry : Builtin::check<Object::List> (args, n))
            res[m++] = entry;

    return res;
}

Object Builtin::merge (const Object::List& args, const Object::Dict&)
{
    auto res = Object::Dict();

    for (int n = 0; n < args.size(); ++n)
        for (const auto& entry : Builtin::check<Object::Dict> (args, n))
            res[entry.first] = entry.second;

    return res;
}

Object Builtin::range (const Object::List& args, const Object::Dict&)
{
    auto l0 = check<int>(args, 0);
    auto l1 = check<int>(args, 1);
    auto x0 = std::min (l0, l1);
    auto x1 = std::max (l0, l1);

    Object::List L;

    for (int x = x0; x < x1; ++x)
        L.push_back (x);

    return L;
}




// ============================================================================
using BinaryOp = std::function<Object (const Object&, const Object&)>;
using GeneralOp = std::function<Object (const Object::List&, const Object::Dict&)>;




// ============================================================================
static GeneralOp generalize (BinaryOp op)
{
    return [op] (const Object::List& ar, const Object::Dict& kw)
    {
        if (ar.size() != 2 || ! kw.empty())
            throw std::runtime_error ("Need exactly two arguments for binary operation");

        return op (ar[0], ar[1]);
    };
}

static BinaryOp broadcast (BinaryOp op)
{
    return [op] (const Object& a, const Object& b) -> Object
    {
        if (a.type() == 'L' && b.type() == 'L')
        {
            auto va = a.get<Object::List>();
            auto vb = b.get<Object::List>();
            auto sa = va.size();
            auto sb = vb.size();

            if (sa != sb)
                throw std::runtime_error ("Cannot broadcast operation over lists with different sizes");

            auto res = Object::List (sa);

            for (size_t n = 0; n < sa; ++n)
                res[n] = op (va[n], vb[n]);

            return res;
        }
        if (a.type() == 'L' && b.type() != 'L')
        {
            auto va = a.get<Object::List>();
            auto sa = va.size();
            auto res = Object::List (sa);

            for (size_t n = 0; n < sa; ++n)
                res[n] = op (va[n], b);

            return res;
        }
        if (a.type() != 'L' && b.type() == 'L')
        {
            auto vb = b.get<Object::List>();
            auto sb = vb.size();
            auto res = Object::List (sb);

            for (size_t n = 0; n < sb; ++n)
                res[n] = op (a, vb[n]);

            return res;
        }
        return op (a, b);
    };
}

static BinaryOp arithmeticize (std::function<int (int, int)> opi, std::function<double (double, double)> opd)
{
    return [opi, opd] (const Object& a, const Object& b) -> Object
    {
        auto ta = a.type();
        auto tb = b.type();

        if (ta == 'i' && tb == 'i') return opi (a.get<int>(), b.get<int>());
        if (ta == 'i' && tb == 'd') return opd (a.get<int>(), b.get<double>());
        if (ta == 'd' && tb == 'i') return opd (a.get<double>(), b.get<int>());
        if (ta == 'd' && tb == 'd') return opd (a.get<double>(), b.get<double>());

        throw std::runtime_error ("Non-numeric values given to binary arithmetic operation");
    };
}




// ============================================================================
Object::Dict Builtin::arithmetic()
{
    using F = Object::Func;
    auto a = Object::Dict();
    a["add"] = F (generalize (broadcast (arithmeticize ([] (auto a, auto b) { return a + b; }, [] (auto a, auto b) { return a + b; }))));
    a["sub"] = F (generalize (broadcast (arithmeticize ([] (auto a, auto b) { return a - b; }, [] (auto a, auto b) { return a - b; }))));
    a["mul"] = F (generalize (broadcast (arithmeticize ([] (auto a, auto b) { return a * b; }, [] (auto a, auto b) { return a * b; }))));
    a["div"] = F (generalize (broadcast (arithmeticize ([] (auto a, auto b) { return a / b; }, [] (auto a, auto b) { return a / b; }))));
    a["pow"] = F (generalize (broadcast (arithmeticize ([] (auto a, auto b) { return std::pow (a, b); }, [] (auto a, auto b) { return std::pow (a, b); }))));
    return a;
}

Object::Dict Builtin::trigonometric()
{
    return Object::Dict();
}

// Object::Dict Builtin::array()
// {
//     using F = Object::Func;
//     auto a = Object::Dict();

//     auto linspace = [] (const Object& A, const Object&)
//     {
//         try {
//             auto a = A.index (0).get<double>();
//             auto b = A.index (1).get<double>();
//             auto N = A.index (2).get<int>();

//             Object::List L (N);

//             for (int n = 0; n < N; ++n)
//                 L[n] = lmap (double (n), 0.0, double (N - 1), a, b);

//             return L;
//         }
//         catch (...)
//         {
//             throw std::runtime_error ("linspace(a:double, b:double, N:int)");
//         }
//     };

//     auto range = [] (const Object& a, const Object& b)
//     {
//         try {
//             auto x0 = a.index (0).get<int>();
//             auto x1 = a.index (1).get<int>();

//             if (x1 < x0)
//                 throw;

//             Object::List L;

//             for (int x = x0; x < x1; ++x)
//                 L.push_back (x);

//             return L;
//         }
//         catch (...)
//         {
//             throw std::runtime_error ("range(a:int, b:int) ; a <= b");
//         }
//     };

//     a["linspace"] = F (linspace);
//     a["range"]    = F (range);
//     return a;
// }
