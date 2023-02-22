#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <optional>
#include <vector>

enum class TokenKind
{
    Unknown,
    Comment,            // https://en.cppreference.com/w/cpp/comment
    Preprocess,         // https://en.cppreference.com/w/cpp/preprocessor
    Keyword,            // https://en.cppreference.com/w/cpp/keyword
    Identifier,         // https://en.cppreference.com/w/cpp/language/identifiers
    Punctuation,        // https://en.cppreference.com/w/cpp/language/punctuators
    IntegerLiteral,     // https://en.cppreference.com/w/cpp/language/integer_literal
    CharacterLiteral,   // https://en.cppreference.com/w/cpp/language/character_literal
    FloatingLiteral,    // https://en.cppreference.com/w/cpp/language/floating_literal
    StringLiteral,      // https://en.cppreference.com/w/cpp/language/string_literal
    UserDefinedLiteral, // https://en.cppreference.com/w/cpp/language/user_literal
};

struct Token
{
    TokenKind kind;    // 类型
    size_t line;       // 所在行
    std::string value; // 内容
};

struct Cursor
{
    const char *buffer;
    size_t length;
    size_t line;

    explicit operator bool() const noexcept
    {
        return buffer == nullptr || *buffer == '\0';
    }

    char at(size_t i) const
    {
        return buffer[i];
    }

    bool start_with(const char *literal)
    {
        if (literal == nullptr)
            return false;

        for (size_t i = 0; i < length; i++)
        {
            auto lhs = buffer[i];
            auto rhs = literal[i];
            if (rhs == '\0')
            {
                return true;
            }
            if (lhs != rhs)
                break;
        }
        return false;
    }

    Cursor advance() const
    {
        Cursor result{};
        if (length >= 1)
        {
            result.length = length - 1;
            result.buffer = buffer + 1;
            result.line = line;
            if (buffer[0] == '\n')
            {
                result.line += 1;
            }
        }
        return result;
    }

    Cursor advance(size_t n) const
    {
        Cursor result{};
        if (length > n)
        {
            result.length = length - n;
            result.buffer = buffer + n;
            result.length = length;
            for (size_t i = 1; i < n; i++)
            {
                if (buffer[i - 1] == '\n')
                {
                    result.length += 1;
                }
            }
        }
        return result;
    }
};

// 解析注释:以/开头
Cursor parse_comment(Cursor cursor)
{
    if (cursor.length < 2 || cursor.at(0) != '/')
        return {};
    auto ch = cursor.at(1);
    if (ch != '/' && ch != '*')
        return {};
    Cursor tmp = cursor.advance(2);
    if (ch == '/')
    { // 单行注释
        while (tmp = tmp.advance())
        {
            if (tmp.at(0) == '\n')
            {
                return tmp.advance();
            }
        }
    }
    else // 多行注释
    {
        while (tmp = tmp.advance())
        {
            if (tmp.start_with("*/"))
            {
                return tmp.advance(2);
            }
        }
    }
    return tmp;
}

// 预处理指令:以#开头
Cursor parse_preprocess(Cursor cursor)
{
    if (cursor.length < 1 || cursor.at(0) != '#')
        return {};
    // 一般到行结束,但是如果预处理指令最后包含“\”, 则需要找到不含“\”的那一行为止
    bool flag = false;
    auto tmp = cursor;
    while (tmp = tmp.advance())
    {
        auto ch = tmp.at(0);
        if (!flag)
        {
            flag = (ch == '\\');
        }
        else
        {
            flag = std::isspace(ch);
        }

        if (ch != '\n')
            continue;

        if (!flag)
        {
            return tmp.advance(1);
        }
    }
    return tmp;
}

// 关键字:字母开头
Cursor parse_keyword(Cursor cursor)
{
    static std::vector<std::string> keywords{
        "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_noexcept", "auto"
                                                                                          "bitand",
        "bitor", "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class",
        "compl", "concept", "const", "consteval", "constexpr", "constinit", "const_cast", "continue",
        "co_await", "co_return", "co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else",
        "enum", "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int", "long",
        "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private",
        "protected", "public", "reflexpr", "register", "reinterpret_cast", "requires", "return", "short", "signed",
        "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "synchronized", "template", "this",
        "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
        "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"};
    for (auto &keyword : keywords)
    {
        // 以关键字开始,后跟空格,才算关键字
        if (cursor.start_with(keyword.c_str()))
        {
            auto tmp = cursor.advance(keyword.length());
            if (std::isspace(tmp.at(0)))
            {
                return tmp;
            }
        }
    }
    return {};
}

// 标识符:字母或_开头
Cursor parse_identifier(Cursor cursor)
{
    auto ch = cursor.at(0);
    // 标识符开头必须是字母或者_,后续跟着字母/数字/_
    if (std::isalpha(ch) || ch == '_')
    {
        auto tmp = cursor;
        while (tmp = tmp.advance())
        {
            auto v = tmp.at(0);
            if (std::isalnum(v) || v == '_')
                continue;
            return tmp;
        }
        return tmp;
    }
    else
    {
        return {};
    }
}

// 标点符号：标点符号开头
Cursor parse_punctuation(Cursor cursor)
{
    static std::vector<std::string> symbols{
        "<=>", "<<=", ">>=", "...", "->*",
        "+=", "-=", "*=", "/=", "%=", "^=", "&=", "|=", "==", "!=",
        "&&", "||", "<<", ">>", "++", "--", "<=", ">=", "##", "::",
        ".*", "->",
        "{", "}", "[", "]", "#", "(", ")", ";", ":", "?", ".", "~", "!", "+", "-", "*", "/", "%", "^", "&",
        "|", "=", "<", ">", ","};
    for (auto &v : symbols)
    {
        // 属于标点符号的范畴,取完标点符号就退出
        if (cursor.start_with(v.c_str()))
        {
            return cursor.advance(v.length());
            // auto tmp = cursor.advance(v.length());
            // auto ch = tmp.at(0);
            // if (std::isspace(ch) ||
            //     std::isalnum(ch) ||
            //     ch == '"' ||
            //     ch == '`' ||
            //     ch == '\''||
            //     ch == '\\'||
            //     ch == '$' ||
            //     ch == '@' ||
            //     ch == '_' ) {
            //     return tmp;
            // }
        }
    }
    return {};
}

// 整数字面量：数字开头
Cursor parse_integer_literal(Cursor cursor)
{
    static const std::vector<std::string> suffixs{
        "ull",
        "llu",
        "Ull",
        "llU",
        "uLL",
        "LLu",
        "ULL",
        "LLU"
        "ll",
        "LL",
        "lu",
        "ul",
        "uL",
        "Lu",
        "Ul",
        "lU",
        "UL",
        "LU",
        "zu",
        "uz",
        "Zu",
        "uZ",
        "Uz",
        "zU",
        "UZ",
        "ZU",
        "l",
        "L",
        "u",
        "U",
        "z",
        "Z",
    };
    auto ch = cursor.at(0);
    if (!std::isdigit(ch))
    {
        return {};
    }
    auto tmp = cursor;

    if (ch != '0')
    {
        // 十进制字面量+整数后缀
        while (tmp = tmp.advance())
        {
            auto v = tmp.at(0);
            if (std::isdigit(v) || v == '\'')
                continue;
        }
    }
    else
    {
        tmp = cursor.advance();
        ch = tmp.at(0);
        if (ch == 'b' || ch == 'B')
        {
            // 0b/0B二进制字面量+整数后缀
            while (tmp = tmp.advance())
            {
                auto v = tmp.at(0);
                if (v == '0' || v == '1')
                    continue;
            }
        }
        else if (ch == 'x' || ch == 'X')
        {
            // 0x/0X十六进制字面量+整数后缀
            while (tmp = tmp.advance())
            {
                auto v = tmp.at(0);
                if (std::isdigit(v) ||
                    v == 'A' || v == 'a' ||
                    v == 'B' || v == 'b' ||
                    v == 'C' || v == 'c' ||
                    v == 'D' || v == 'd' ||
                    v == 'E' || v == 'e' ||
                    v == 'F' || v == 'f')
                    continue;
            }
        }
        else if (ch >= '0' && ch <= '7')
        {
            // 0八进制字面量+整数后缀
            while (tmp = tmp.advance())
            {
                auto v = tmp.at(0);
                if (v >= '0' && v <= '7')
                    continue;
            }
        }
    }
    // 尝试读一下后缀
    for (auto &o : suffixs)
    {
        if (tmp.start_with(o.c_str()))
        {
            return tmp.advance(o.length());
        }
    }
    return tmp;
}

// 浮点数字面量：数字或.开头
Cursor parse_floating_literal(Cursor cursor)
{
    static const std::vector<std::string> suffixs{
        "BF16", "bf16", "F128", "f128", "F64", "f64", "F32", "f32", "F16", "f16",
        "f", "F", "L", "l"};
    // 开头是数字或.
    auto ch = cursor.at(0);
    if (!std::isdigit(ch) && ch != '.')
    {
        return {};
    }

    // TODO FIXME 暂不处理0X这种情况
    auto tmp = cursor;
    // 先读数字序列
    while (tmp = tmp.advance())
    {
        auto v = tmp.at(0);
        if (std::isdigit(v))
            continue;
    }
    if (!tmp)
        return tmp;

    ch = tmp.at(0);
    if (ch == '.')
    {
        //.,.e,.e-
        size_t n = 1;
        if (tmp.start_with(".e-"))
        {
            n = 3;
        }
        else if (tmp.start_with(".e"))
        {
            n = 2;
        }
        tmp = tmp.advance(n);
    }
    else if (ch == 'e')
    {
        // e,e-
        tmp = tmp.advance();
        if (tmp.start_with("-"))
        {
            tmp = tmp.advance();
        }
    }
    // 再读一下数字
    while (tmp)
    {
        auto v = tmp.at(0);
        if (!std::isdigit(v))
            break;
        tmp = tmp.advance();
    }
    // 尝试读一下后缀
    for (auto &o : suffixs)
    {
        if (tmp.start_with(o.c_str()))
        {
            return tmp.advance(o.length());
        }
    }
    return tmp;
}

// 字符串字面量：字母或"开头
Cursor parse_string_literal(Cursor cursor)
{
    static const std::vector<std::string> prefix{
        "L\"", "u8\"", "u\"", "U\"", "R\"", "\""};
    size_t n = 0;
    for (auto &v : prefix)
    {
        if (cursor.start_with(v.c_str()))
        {
            n = v.length();
            break;
        }
    }
    if (n == 0)
        return {};

    auto tmp = cursor.advance(n);
    if (cursor.start_with("R\""))
    {
        // 找到";为止,过程中忽略断行
        while (tmp)
        {
            if (tmp.start_with("\";"))
                break;
            tmp = tmp.advance();
        }
    }
    else
    {
        char last = '"';
        while (tmp)
        {
            auto ch = tmp.at(0);
            if (ch == '"' && last != '\\')
                break;
            last = ch;
            tmp = tmp.advance();
        }
    }
    return tmp;
}

// 用户自定义字面量:暂不支持

int main()
{
    SetConsoleOutputCP(65001); // 避免输出中文乱码
    constexpr auto file = "D:/Repos/VisualStudio/lexer/lexer.cpp";
    std::ifstream ifs(file);
    std::string buffer;
    std::getline(ifs, buffer);
    // 文件编码格式为utf-8-bom,移除掉bom
    buffer = buffer.substr(3);
    std::cout << buffer << "\n";
    while (std::getline(ifs, buffer))
    {
        std::cout << buffer << "\n";
    }
    return 0;
}
