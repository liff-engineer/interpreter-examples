
#include <exception>
#include <string>
#include <sstream>
#include <iostream>

enum ASTNodeType
{
    Undefined,
    OperatorPlus,
    OperatorMinus,
    OperatorMul,
    OperatorDiv,
    UnaryMinus,
    NumberValue
};

struct ASTNode
{
    ASTNodeType Type = Undefined;
    double Value = 0;
    ASTNode *Left = nullptr;
    ASTNode *Right = nullptr;

    ~ASTNode()
    {
        delete Left;
        delete Right;
    }
};

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
    ASTNode *CreateNode(ASTNodeType type, ASTNode *left, ASTNode *right)
    {
        ASTNode *node = new ASTNode;
        node->Type = type;
        node->Left = left;
        node->Right = right;
        return node;
    }
    ASTNode *CreateUnaryNode(ASTNode *left)
    {
        ASTNode *node = new ASTNode;
        node->Type = UnaryMinus;
        node->Left = left;
        node->Right = nullptr;
        return node;
    }

    ASTNode *CreateNodeNumber(double value)
    {
        ASTNode *node = new ASTNode;
        node->Type = NumberValue;
        node->Value = value;
        return node;
    }

    ASTNode *Expression()
    {
        ASTNode *tnode = Term();
        ASTNode *e1node = Expression1();
        return CreateNode(OperatorPlus, tnode, e1node);
    }

    ASTNode *Expression1()
    {
        ASTNode *tnode;
        ASTNode *e1node;
        switch (m_crtToken.type)
        {
        case Plus:
            GetNextToken();
            tnode = Term();
            e1node = Expression1();
            return CreateNode(OperatorPlus, e1node, tnode);
        case Minus:
            GetNextToken();
            tnode = Term();
            e1node = Expression1();
            return CreateNode(OperatorMinus, e1node, tnode);
        }
        return CreateNodeNumber(0);
    }

    ASTNode *Term()
    {
        ASTNode *fnode = Factor();
        ASTNode *t1node = Term1();
        return CreateNode(OperatorMul, fnode, t1node);
    }

    ASTNode *Term1()
    {
        ASTNode *fnode;
        ASTNode *t1node;
        switch (m_crtToken.type)
        {
        case Mul:
            GetNextToken();
            fnode = Factor();
            t1node = Term1();
            return CreateNode(OperatorMul, t1node, fnode);
        case Div:
            GetNextToken();
            fnode = Factor();
            t1node = Term1();
            return CreateNode(OperatorDiv, t1node, fnode);
        }
        return CreateNodeNumber(1);
    }

    ASTNode *Factor()
    {
        ASTNode *node;
        switch (m_crtToken.type)
        {
        case OpenParenthesis:
            GetNextToken();
            node = Expression();
            Match(')');
            return node;
        case Minus:
            GetNextToken();
            node = Factor();
            return CreateUnaryNode(node);
        case Number:
        {
            double value = m_crtToken.value;
            GetNextToken();
            return CreateNodeNumber(value);
        }

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
    ASTNode *Parse(const char *text)
    {
        m_Text = text;
        m_Index = 0;
        GetNextToken();
        return Expression();
    }
};

class EvaluatorException : public std::exception
{
public:
    EvaluatorException(const std::string &message) : std::exception(message.c_str())
    {
    }
};
class Evaluator
{
    double EvaluateSubtree(ASTNode *ast)
    {
        if (ast == nullptr)
            throw EvaluatorException("Incorrect syntax tree!");

        if (ast->Type == NumberValue)
        {
            return ast->Value;
        }
        else if (ast->Type == UnaryMinus)
        {
            return -EvaluateSubtree(ast->Left);
        }
        else
        {
            double v1 = EvaluateSubtree(ast->Left);
            double v2 = EvaluateSubtree(ast->Right);
            switch (ast->Type)
            {
            case OperatorPlus:
                return v1 + v2;
            case OperatorMinus:
                return v1 - v2;
            case OperatorMul:
                return v1 * v2;
            case OperatorDiv:
                return v1 / v2;
            }
        }

        throw EvaluatorException("Incorrect syntax tree!");
    }

public:
    double Evalute(ASTNode *ast)
    {
        if (ast == nullptr)
            throw EvaluatorException("Incorrect abstract syntax tree");
        return EvaluateSubtree(ast);
    }
};

void Test(const char *text)
{
    Parser parser;
    try
    {
        auto result = parser.Parse(text);
        std::cout << "\"" << text << "\"\t OK\n";

        try
        {
            Evaluator eval;
            double val = eval.Evalute(result);
            std::cout << text << " = " << val << std::endl;
        }
        catch (EvaluatorException &ex)
        {
            std::cout << text << " \t " << ex.what() << std::endl;
        }
        delete result;
    }
    catch (ParserException &ex)
    {
        std::cout << "\"" << text << "\"\t " << ex.what() << "\n";
    }
}
int main()
{
    Test("1+2*3");
    return 0;
}
