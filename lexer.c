#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexer.h"

static const char* keywords[MAX_KEYWORDS] = {
    "int", "float", "return", "if", "else", "while", "for", "do", "break", "continue",
    "char", "double", "void", "switch", "case", "default", "const", "static", "sizeof", "struct"
};

static const char* operators = "+-*/%=!<>|&";
static const char* specialCharacters = ",;{}()[]";

static FILE *fptr = NULL;

void initializeLexer(const char* filename)
{
    fptr = fopen(filename, "r");
    if(fptr == NULL)
    {
        printf("ERROR... File is not opened\n");
        return;
    }
    printf("File is opened successfully\n");
}

/*---------------------------------------------------------------------------------------------------------------------------------------*/
char *Token_Type(TokenType type)
{
    switch(type)
    {
        case KEYWORD:
            return "KEYWORD";
        case OPERATOR:
            return "OPERATOR";
        case SPECIAL_CHARACTER:
            return "SPECIAL_CHARACTER";
        case CONSTANT:
            return "CONSTANT";
        case IDENTIFIER:
            return "IDENTIFIER";
        case STRING_LITERAL:
            return "STRING_LITERAL";
        case CHAR_LITERAL:
            return "CHAR_LITERAL";
        case UNKNOWN:
            return "UNKNOWN";
        default:
            return "INVALID";
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------*/
Token getNextToken()
{
    int i = 0;
    char ch;
    int next;

    Token token;
    token.lexeme[0] = '\0';
    token.type = UNKNOWN;
    if(fptr == NULL)
    {
        return token;
    }
    //Skip spaces
    do
    {
        ch = fgetc(fptr);
    } while (ch != EOF && isspace(ch));

    if(ch == EOF)
        return token;

    //Preprocessor directive
    if(ch == '#')
    {
        while((ch = fgetc(fptr)) != '\n' && ch != EOF)  {}
        return getNextToken();
    }
    //Keyword or identifier
    if(isalpha(ch) || ch == '_')
    {
        do
        {
            if(i >= MAX_TOKEN_SIZE - 1)
                return token;
            
            token.lexeme[i++] = ch;
            ch = fgetc(fptr);
        } while(isalnum(ch) || ch == '_');

        token.lexeme[i] = '\0';

        if(ch != EOF)
        {
            ungetc(ch, fptr);
        }

        token.lexeme[i] = '\0';
        categorizeToken(&token);
        return token;
    }
    //Numeric constant
    else if(isdigit(ch))
    {
        do
        {
            if(i >= MAX_TOKEN_SIZE - 1)
                return token;

            token.lexeme[i++] = ch;
            ch = fgetc(fptr);
        } while (ch != EOF && (isalnum(ch) || ch == '.' || ch == '_'));
        
        //Exponent sign
        if((ch == '+' || ch == '-') && i > 0 && (token.lexeme[i - 1] == 'e' || token.lexeme[i - 1] == 'E'))
        {
            token.lexeme[i++] = ch;
            ch = fgetc(fptr);

            while(ch != EOF && (isalnum(ch) || ch == '.'))
            {
                if(i >= MAX_TOKEN_SIZE - 1)
                    return token;

                token.lexeme[i++] = ch;
                ch = fgetc(fptr);
            }
        }
        if(ch != EOF)
        {
            ungetc(ch, fptr);
        }

        token.lexeme[i] = '\0';

        if(isConstant(token.lexeme) || isfloatConstant(token.lexeme))
        {
            token.type = CONSTANT;
        }

        return token;
    }
    //Comment and division operator
    else if(ch == '/')
    {
        next = fgetc(fptr);
        //Single line comment
        if(ch == '/')
        {
            while((ch = fgetc(fptr)) != '\n' && ch != EOF)  {}
            return getNextToken();
        }
        //multi line comment
        if(ch == '*')
        {
            int found = 0;
            char prev = 0;

            while((ch = fgetc(fptr)) != EOF)
            {
                if(prev == '*' && ch == '/')
                {
                    found = 1;
                    break;
                }
                prev = ch;
            }
            if(!found)
            {
                strcpy(token.lexeme, "Unclosed comment");
                return token;
            }
            return getNextToken();
        }

        if(next != EOF)
        {
           ungetc(next, fptr);
        }
        strcpy(token.lexeme, "/");
        token.type = OPERATOR;
        return token;
    }
    //character literal
    else if(ch == '\'')
    {
        token.lexeme[i++] = '\'';
        ch = fgetc(fptr);

        if(ch == '\'' || ch == '\n' || ch == EOF)
        {
            return token;
        }

        token.lexeme[i++] = ch;

        if(ch == '\\')
        {
            ch = fgetc(fptr);
            if(ch == EOF || ch == '\n')
            {
                return token;
            }
            token.lexeme[i++] = ch;
        }
        ch = fgetc(fptr);
    
        if(ch != '\'')
        {
            while(ch != '\'' && ch != '\n' && ch != EOF)
            {
                ch = fgetc(fptr);
            }
            return token;
        }
        token.lexeme[i++] = '\'';
        token.lexeme[i] = '\0';
        token.type = CHAR_LITERAL;
        return token;
    }
    //string literal
    else if(ch == '"')
    {
        token.lexeme[i++] = '"';
        while(1)
        {
            ch = fgetc(fptr);
            if(ch == EOF || ch =='\n')
            {
                return token;
            }
            if(ch == '"')
            {
                token.lexeme[i++] = '"';
                token.lexeme[i] = '\0';
                token.type = STRING_LITERAL;
                return token;
            }
            
            if(i >= MAX_TOKEN_SIZE - 2)
                return token;
        
            token.lexeme[i++] = ch;

            if(ch == '\\')
            {
                ch = fgetc(fptr);
                if(ch == EOF || ch == '\n')
                    return token;

                token.lexeme[i++] = ch;
            }
        }
    }
    //operator
    else if(strchr(operators, ch) != NULL)
    {
        char op[4] = {ch, '\0', '\0', '\0'};
        next = fgetc(fptr);
        if((ch == '<' && next == '<') || (ch == '>' && next == '>'))
        {
            op[1] = next;
            int third = fgetc(fptr);
            if(third == '=')
            {
                op[2] = third;
            }
            else if(third != EOF)
            {
                ungetc(third, fptr);
            }
        }
        else if((ch == '+' && (next == '+' || next == '=')) || (ch == '-' && (next == '-' || next == '=' || next == '>')) ||
                (ch == '=' && next == '=') || (ch == '!' && next == '=') || (ch == '<' && next == '=') || (ch == '>' && next == '=') || 
                (ch == '&' && (next == '&' || next == '=')) || (ch == '|' && (next == '|' || next == '=')) ||
                ((ch == '*' || ch == '%' || ch == '^') && next == '='))
        {
            op[1] = next;
        }
        else
        {
            if(next != EOF)
            {
                ungetc(next, fptr);
            }
        }

        strcpy(token.lexeme, op);
        token.type = OPERATOR;
        return token;
    }
    //Special character
    else if(isSpecialCharacter(ch))
    {
        token.lexeme[0] = ch;
        token.lexeme[1] = '\0';
        token.type = SPECIAL_CHARACTER;
        return token;
    }
    //Unknown character
    token.lexeme[0] = ch;
    token.lexeme[1] = '\0';
    return token;
}

/*---------------------------------------------------------------------------------------------------------------------------------------*/
void categorizeToken(Token* token)
{
    if(isKeyword(token -> lexeme))
    {
        token -> type = KEYWORD;
    }
    else if(isConstant(token -> lexeme) || isfloatConstant(token -> lexeme))
    {
        token -> type = CONSTANT;
    }
    else if(isIdentifier(token -> lexeme))
    {
        token -> type = IDENTIFIER;
    }
    else if(isOperator(token -> lexeme))
    {
        token -> type = OPERATOR;
    }
    else
    { 
        token -> type = UNKNOWN;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------*/
int isKeyword(const char* str)
{
    for(int i = 0; i < MAX_KEYWORDS; i++)
    {
        if(strcmp(str, keywords[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------*/
int isOperator(const char* str)
{
    if(strlen(str) != 1)
        return 0;
    if(strchr(operators, str[0]) != NULL)
        return 1;
    
    return 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------*/
int isSpecialCharacter(char ch)
{
    if(strchr(specialCharacters, ch) != NULL)
        return 1;
    
    return 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------*/
int isConstant(const char* str)
{
    int i = 0;
    if(str == NULL || str[0] == '\0')
        return 0;
    //binary
    if(str[0] == '0' && (str[1] == 'b' || str[1] == 'B'))
    {
        if(str[2] == 0)
        {
            return 0;
        }
        i = 2;
        while(str[i] != 0)
        {
            if(str[i] != '0' && str[i] != '1')
            {
                return 0;
            }
            i++;
        }
        return 1;
    }
    //hexa
    if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        if(str[2] == 0)
        {
            return 0;
        }
        i = 2;
        while(str[i] != 0)
        {
            if(!isxdigit((unsigned char)str[i]))
            {
                return 0;
            }
            i++;
        }
        return 1;
    }
    //octal
    if(str[0] == '0' && str[1] != '0')
    {
        i = 1;
        while(str[i] != 0)
        {
            if(str[i] < '0' || str[i] > '7')
            {
                return 0;
            }
            i++;
        }
        return 1;
    }
    //decimal
    i = 0;
    if(str[i] == 0)
    {
        return 0;
    }
    while(str[i] != 0)
    {
        if(!isdigit(str[i]))
        {
            return 0;
        }
        i++;
    }
    return 1;
}

/*---------------------------------------------------------------------------------------------------------------------------------------*/
int isIdentifier(const char* str)
{
    int i = 0;
    if(!(isalpha(str[0]) || str[0] == '_'))
    {
        return 0;
    }

    while(str[i] != 0)
    {
        if(!(isalnum(str[i]) || str[i] == '_'))
        {
            return 0;
        }
        i++;
    }
    return 1;
}

/*---------------------------------------------------------------------------------------------------------------------------------------*/
int isfloatConstant(const char *str)
{
    int i = 0, dot_count = 0, digit_count = 0;
    while(str[i] != 0)
    {
        if(isdigit(str[i]))
        {
            digit_count++;
        }
        else if(str[i] == '.')
        {
            dot_count++;
            if(dot_count > 1)
            {
                return 0;
            }
        }
        else if(str[i] == 'f' || str[i] == 'l')
        {
            if(str[i+1] != 0)
                return 0;
            break;
        }
        else
        {
            return 0;
        }
        i++;
    }
    if(dot_count != 1)
    {
        return 0;
    }
    if(digit_count == 0)
    {
        return 0;
    }
    return 1;
}