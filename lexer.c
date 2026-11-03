#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static const char* keywords[MAX_KEYWORDS] = {
    "int", "float", "return", "if", "else", "while", "for", "do", "break", "continue",
    "char", "double", "void", "switch", "case", "default", "const", "static", "sizeof", "struct"
};

static const char* operators = "+-*/%=!<>|&";
static const char* specialCharacters = ",;{}()[]";

static FILE *fptr;
static Token token;

void initializeLexer(const char* filename)
{
    fptr = fopen(filename, "r");
    if(fptr == NULL)
    {
        printf("ERROR... File is not opened\n");
        return;
    }
    printf("File is opened successfully\n");
    fclose(fptr);
}

Token getNextToken()
{
    int i = 0;
    char ch;
    token.lexeme[0] = '\0';

    do
    {
        ch = fgetc(fptr);
    } while (ch != EOF && isspace(ch));

    if(ch == EOF)
        return token;
    
    if(ch == '#')
    {
        while((ch = fgetc(fptr)) != '\n' && ch != EOF)  {}
        if(ch == '\n')
            return getNextToken();
    }
    else if(isalpha(ch) || ch == '_')
    {
        do
        {
            token.lexeme[i++] = ch;
            ch = fgetc(fptr);
        } while (isalnum(ch) || ch == '_');
        token.lexeme[i] = '\0';

        if(ch != EOF)
        {
            ungetc(ch, fptr);
        }
        categorizeToken(&token);
        return token;
    }
    else if(ch == '/')
    {
        char op = ch;
        ch = fgetc(fptr);
        if(ch == '/')
        {
            while((ch = fgetc(fptr)) != '\n' && ch != EOF)  {}
            if(ch == '\n')
            return getNextToken();
        }

        if(ch == '\n')
        {
            int next = 0;
            char prev = 0;

            while((ch = fgetc(fptr)) != EOF)
            {
                if(prev == '*' && ch == '/')
                {
                    next = 1;
                    break;
                }
                prev = ch;
            }
            if(!next)
            {
                token.type = UNKNOWN;
                printf("Lexical ERROR....\n");
                return token;
            }
            return getNextToken();
        }
        token.lexeme[0] = '/';
        token.lexeme[1] = '\0';
        token.type = OPERATOR;
        ungetc(ch, fptr);
        return token;
    }

    else if(ch == '\'')
    {
        i = 0;
        ch = fgetc(fptr);

        if(ch == '\'' || ch == '\n' || ch == EOF)
        {
            token.lexeme[i++] = ch;
            token.lexeme[i] = '\0';
            token.type = UNKNOWN;
            printf("ERROR....\n");
            return token;
        }
        token.lexeme[i++] = ch;
        if(ch == '\\')
        {
            ch = fgetc(fptr);
            if(ch == EOF || ch == '\n')
            {
                token.type = UNKNOWN;
                printf("ERROR....\n");
                return token;
            }
            token.lexeme[i++] = ch;
        }
        ch = fgetc(fptr);
        if(ch != '\'')
        {
            while(ch != '\'' && ch != '\n' && ch != EOF)
            {
                token.lexeme[i++] = ch;
                ch = fgetc(fptr);
            }
            token.lexeme[i] = '\0';
            token.type = UNKNOWN;
            printf("ERROR...\n");
            return token;
        }
        token.lexeme[i++] = '/';
        token.lexeme[i] = '\0';
        token.type = CHAR_LITERAL;
        return token;
    }
    else if(isdigit(ch))
    {
        do
        {
            token.lexeme[i++] = ch;
            ch = fgetc(fptr);
        } while (isdigit(ch) || ch == '.');
        
        if(isalpha(ch) || ch == '_')
        {
            while(isalnum(ch) || ch == '_')
            {
                token.lexeme[i++] = ch;
                ch = fgetc(fptr);
            }
            token.lexeme[i] = '\0';
            if(isConstant(token.lexeme) )
            {
                token.type = CONSTANT;
                return token;
            }
            token.type = UNKNOWN;
            printf("ERROR...\n");
            return token;
        }
        token.lexeme[i] = '\0';

        if(ch != EOF)
        {
            ungetc(ch, fptr);
        }

        if(isConstant(token.lexeme) )
        {
            token.type = CONSTANT;
        }
        else
        {
            printf("ERROR...\n");
            token.type = UNKNOWN;
        }
        return token;
    }
    else if(ch == '"')
    {
        ch = fgetc(fptr);
        do
        {
            if(ch == EOF || ch =='\n')
            {
                token.lexeme[i] = '\0';
                token.type = UNKNOWN;
                printf("ERROR....\n");
                return token;
            }
            if(ch == '\\')
            {
                token.lexeme[i++] = ch;
                ch = fgetc(fptr);
                if(ch ==EOF || ch == '\n')
                {
                    token.lexeme[i] = '\0';
                    token.type = UNKNOWN;
                    printf("ERROR...\n");
                    return token;
                }
            }
            token.lexeme[i++] = ch;
            ch = fgetc(fptr);
        } while (ch != '"');
        
        token.lexeme[i] = '\0';
        token.type = STRING_LITERAL;
        return token;
    }
}

void categorizeToken(Token* token)
{
    if(isKeyword(token -> lexeme))
        token -> type = KEYWORD;
    else if(isConstant(token -> lexeme))
        token -> type = CONSTANT;
    else if(isIdentifier(token -> lexeme))
        token -> type = IDENTIFIER;
    else if(isOperator(token -> lexeme))
        token -> type = OPERATOR;
    else if(isSpecialCharacter(token -> lexeme))
        token -> type = SPECIAL_CHARACTER;
    else 
        token -> type = UNKNOWN;
}

int isKeyword(const char* str)
{
    for(int i = 0; i < MAX_KEYWORDS; i++)
    {
        if(!strcmp(str, keywords[i]))
        {
            return 1;
        }
    }
    return 0;
}

int isOperator(const char* str)
{
    if(strlen(str) != 1)
        return 0;
    if(strchr(operators, str[0]) != NULL)
        return 1;
    
    return 0;
}

int isSpecialCharacter(char ch)
{
    if(strchr(specialCharacters, ch) != NULL)
        return 1;
    
    return 0;
}

int isConstant(const char* str)
{
    int i = 0;
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

    if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        if(str[2] == 0)
        {
            return 0;
        }
        i = 2;
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

    if(str[0] == '0' && str[1] != '0')
    {
        i = 2;
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