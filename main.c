#include <stdio.h>
#include "lexer.h"

int main(int argc, char *argv[]) {
    initializeLexer(argv[1]);

    Token token;
    
    while ((token = getNextToken()).type != UNKNOWN) 
    {
        printf("Token: %-35s Type: %s\n", token.lexeme, Token_Type(token.type));
    }

    return 0;
}
