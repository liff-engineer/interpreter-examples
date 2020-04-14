#include <exception>
#include <string>
#include <sstream>
#include <iostream>

enum TokenType
{
    Error,
    Plus,
    Minus,
    Mul,
    Div,
    EndOfText,
    OpenParenthesis,
    CloseParenthesis,
    Number
};

struct Token
{
    TokenType type = TokenType::Error;
    double value = 0.0;
    char symbol = 0;
};

class ParserException : public std::exception
{
    int m_Pos;

public:
    ParserException(const std::string &message, int pos)
        : std::exception(message.c_str()),
          m_Pos{pos} {};
};

class Parser
{
    Token m_crtToken;
    const char *m_Text;
    size_t m_Index;

private:
    void Expression()
    {
        Term();
        Expression1();
    }

    void Expression1()
    {
        switch (m_crtToken.type)
        {
        case Plus:
            GetNextToken();
            Term();
            Expression1();
            break;
        case Minus:
            GetNextToken();
            Term();
            Expression1();
            break;
        }
    }

    void Term()
    {
        Factor();
        Term1();
    }

    void Term1()
    {
        switch (m_crtToken.type)
        {
        case Mul:
            GetNextToken();
            Factor();
            Term1();
            break;
        case Div:
            GetNextToken();
            Factor();
            Term1();
            break;
        }
    }

    void Factor()
    {
        switch (m_crtToken.type)
        {
        case OpenParenthesis:
            GetNextToken();
            Expression();
            Match(')');
            break;
        case Minus:
            GetNextToken();
            Factor();
            break;
        case Number:
            GetNextToken();
            break;
        default:
        {
            std::stringstream sstr;
            sstr << "Unexpected token '" << m_crtToken.symbol << "' at position " << m_Index;
            throw ParserException(sstr.str(), m_Index);
        }
        }
    }

    void Match(char expected)
    {
        if (m_Text[m_Index - 1] == expected)
        {
            GetNextToken();
        }
        else
        {
            std::stringstream sstr;
            sstr << "Expected token '" << expected << "' at position " << m_Index;
            throw ParserException(sstr.str(), m_Index);
        }
    }

    void SkipWhitespaces()
    {
        while (std::isspace(m_Text[m_Index]))
            m_Index++;
    }

    void GetNextToken()
    {
        SkipWhitespaces();

        m_crtToken.value = 0;
        m_crtToken.symbol = 0;

        //check is eof
        if (m_Text[m_Index] == 0)
        {
            m_crtToken.type = EndOfText;
            return;
        }

        if (std::isdigit(m_Text[m_Index]))
        {
            m_crtToken.type = Number;
            m_crtToken.value = GetNumber();
            return;
        }

        m_crtToken.type = Error;
        switch (m_Text[m_Index])
        {
        case '+':
            m_crtToken.type = Plus;
            break;
        case '-':
            m_crtToken.type = Minus;
            break;
        case '*':
            m_crtToken.type = Mul;
            break;
        case '/':
            m_crtToken.type = Div;
            break;
        case '(':
            m_crtToken.type = OpenParenthesis;
            break;
        case ')':
            m_crtToken.type = CloseParenthesis;
            break;
        }

        if (m_crtToken.type != Error)
        {
            m_crtToken.symbol = m_Text[m_Index];
            m_Index++;
        }
        else
        {
            std::stringstream sstr;
            sstr << "Unexpected token '" << m_Text[m_Index] << "' at position " << m_Index;
            throw ParserException(sstr.str(), m_Index);
        }
    }

    double GetNumber()
    {
        SkipWhitespaces();
        int index = m_Index;
        while (std::isdigit(m_Text[m_Index]))
            m_Index++;
        if (m_Text[m_Index] == '.')
            m_Index++;
        while (std::isdigit(m_Text[m_Index]))
            m_Index++;
        if (m_Index - index == 0)
            throw ParserException("Number expected but not found!", m_Index);

        char buffer[32] = {0};
        std::memcpy(buffer, &m_Text[index], m_Index - index);
        return std::atof(buffer);
    }

public:
    void Parse(const char *text)
    {
        m_Text = text;
        m_Index = 0;
        GetNextToken();
        Expression();
    }
};

void Test(const char *text)
{
    Parser parser;
    try
    {
        parser.Parse(text);
        std::cout << "\"" << text << "\"\t OK\n";
    }
    catch (ParserException &ex)
    {
        std::cout << "\"" << text << "\"\t " << ex.what() << "\n";
    }
}
int main()
{
    Test("1+2+3+4");
    Test("1*2*3*4");
    Test("1-2-3-4");
    Test("1/2/3/4");
    Test("1*2+3*4");
    Test("1+2*3+4");
    Test("(1+2)*(3+4)");
    Test("1+(2*3)*(4+5)");
    Test("1+(2*3)/4+5");
    Test("5/(4+3)/2");
    Test("1 + 2.5");
    Test("125");
    Test("-1");
    Test("-1+(-2)");
    Test("-1+(-2.0)");

    Test("   1*2,5");
    Test("   1*2.5e2");
    Test("M1 + 2.5");
    Test("1 + 2&5");
    Test("1 * 2.5.6");
    Test("1 ** 2.5");
    Test("*1 / 2.5");

    return 0;
}
