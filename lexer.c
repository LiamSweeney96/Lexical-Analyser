/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
Lexer Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name:
Student ID:
Email:
Date Work Commenced:
*************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"


// YOU CAN ADD YOUR OWN FUNCTIONS, DECLARATIONS AND VARIABLES HERE

typedef struct {
  FILE *f;
  char name[50];
  int line;
} CurrentFile;

CurrentFile structFile, prevFile;

const char* KeyWords[21] = {"class", "method", "function", "constructor", "int", "boolean",
                            "char", "void", "var", "static", "field", "let", "do", "if", "else",
                            "while", "return", "true", "false", "null", "this"};

const char* Symbols[19] = {"{", "}", "(", ")", "[", "]", ".",
                           ",", ";", "+", "-", "*", "/", "&",
                           "|", "<", ">", "=", "~"};

char SingleLineComment (char c)
{
    while (c != '\n' && c != EOF)
    {
        c = getc(structFile.f);
    }
    structFile.line++;
    c = getc(structFile.f);
    return c;
}

char MultiLineComment (char c)
{
    char prevC = c;
    c = getc(structFile.f);
    while (!((prevC == '*' && c == '/') || c == EOF))
    {
        if (c == '\n')
        {
            structFile.line++;
        }
        prevC = c;
        c = getc(structFile.f);
    }
    c = getc(structFile.f);
    return c;
}

char ClearSpace (char c)
{
    while (isspace(c))
    {
        // Keep track of line number
        if (c == '\n')
        {
            structFile.line++;
        }
        c = getc(structFile.f);
    }
    return c;
}

Token FoundString (Token t, char c)
{
    char array[128];
    int i = 0;
    c = getc(structFile.f);
    // Loop until end of string
    while (c != '"')
    {
        if (c == '\n')
        {
            t.tp = ERR;
            t.ln = structFile.line;
            strcpy(t.lx, "Error: new line in string constant");
            strcpy(t.fl, structFile.name);
            return t;
        }
        if (c == EOF)
        {
            strcpy(t.lx, "Error: unexpected eof in string constant");
            strcpy(t.fl, structFile.name);
            t.ec = EofInStr;
            t.ln = structFile.line;
            return t;
        }
        array[i++] = c;
        c = getc(structFile.f);
    }
    array[i++] = '\0';
    t.tp = STRING;
    t.ln = structFile.line;
    strcpy(t.lx, array);
    strcpy(t.fl, structFile.name);
    return t;
}

Token EOFComment (Token t, char c)
{
    strcpy(t.lx, "Error: unexpected eof in comment");
    strcpy(t.fl, structFile.name);
    t.ec = EofInCom;
    t.ln = structFile.line;
    return t;
}

Token EOFFound (Token t, char c)
{
    strcpy(t.lx, "");
    strcpy(t.fl, structFile.name);
    t.ln = structFile.line;
    t.tp = EOFile;
    return t;
}

Token SymbolCheck (Token t, char c)
{
    char temp[2] = {c, '\0'};
    if (strcmp(temp, "?") == 0)
    {
        strcpy(t.lx, "Error: illegal symbol in source file");
        strcpy(t.fl, structFile.name);
        t.ln = structFile.line;
        t.tp = ERR;
        return t;
    }
    char prevC = c;
    c = getc(structFile.f);

    if ((prevC == '/' && c == '/') || (prevC == '/' && c == '*'))
    {
        ungetc(c, structFile.f);
        c = prevC;
    }
    else
    {
        for (int i = 0; i < 19; i++)
        {
            if (strcmp(temp, Symbols[i]) == 0)
            {
                t.tp = SYMBOL;
                t.ln = structFile.line;
                strcpy(t.lx, temp);
                strcpy(t.fl, structFile.name);
            }
        }
        ungetc(c, structFile.f);
        c = prevC;
    }
    if (c == EOF)
    {
        ungetc(c, structFile.f);
        c = prevC;
    }
    return t;
}

// IMPLEMENT THE FOLLOWING functions
//***********************************

// Initialise the lexer to read from source file
// file_name is the name of the source file
// This requires opening the file and making any necessary initialisations of the lexer
// If an error occurs, the function should return 0
// if everything goes well the function should return 1
int InitLexer (char* file_name)
{
    // Attempt to open the file
    structFile.f = fopen(file_name, "r");
    strcpy(structFile.name, file_name);

    // Check if still using the same file, if not set line count back to 1
    if (prevFile.name != structFile.name)
    {
        structFile.line = 1;
    }
    strcpy(prevFile.name, structFile.name);

    // Check if file opened correctly
    if (structFile.f == 0)
    {
        printf("Error: can't open file \n");
        return 0; // 0 means error
    }
    return 1;
}


// Get the next token from the source file
Token GetNextToken ()
{
	Token t;
    t.tp = ERR;
    char temp[128], array[128];
    int c = getc(structFile.f), prevC = c, i = 0, j = 0, len = 0, count = 0;

    // Loop until a token has been found
    while (1)
    {
        if (c == 0)
        {
            exit(0);
        }
        // If end of file reached, return EOF token
        if (c == EOF)
        {
            strcpy(t.fl, structFile.name);
            t.ln = structFile.line;
            t.tp = EOFile;
            return t;
        }

        // Clear any white space
        if (isspace(c))
        {
            c = ClearSpace(c);
        }
        if (ispunct(c))
        {
            t = SymbolCheck(t, c);
            if (t.tp == SYMBOL)
            {
                return t;
            }
            if (strcmp(t.lx, "Error: illegal symbol in source file") == 0)
            {
                return t;
            }
            if (c == '/')
            {
                prevC = c;
                c = getc(structFile.f);
                if (c == '/')
                {
                    c = SingleLineComment(c);
                    if (c == EOF)
                    {
                        t = EOFComment(t, c);
                        return t;
                    }
                }
                else if (c == '*')
                {
                    c = MultiLineComment(c);
                    if (c == EOF)
                    {
                        t = EOFComment(t, c);
                        return t;
                    }
                }
                else
                {
                    ungetc(c, structFile.f);
                    c = prevC;
                    temp[0] = c;
                    temp[1] = '\0';
                    t.tp = SYMBOL;
                    t.ln = structFile.line;
                    strcpy(t.lx, temp);
                    strcpy(t.fl, structFile.name);
                    return t;
                }
            }
        }
        if (c == '"')
        {
            t = FoundString(t, c);
            return t;
        }

        // If EOF found, return error token
        if (c == EOF)
        {
            t = EOFFound(t, c);
            return t;
        }
        if (isalpha(c) || isdigit(c)  || c == '_')
        {
            // Start of a keyword or ID
            while (!isspace(c) && c != EOF)
            {
                if (ispunct(c) && c != '_')
                {
                    break;
                }
                array[j++] = c;
                temp[i++] = tolower(c);
                prevC = c;
                c = getc(structFile.f);
            }
            temp[i++] = '\0';
            array[j++] = '\0';
            ungetc(c, structFile.f);
            c = prevC;
            break;
        }
    }
    for (int i = 0; i < 21; i++)
    {
        if (strcmp(temp, KeyWords[i]) == 0)
        {
            t.tp = RESWORD;
            t.ln = structFile.line;
            strcpy(t.lx, array);
            strcpy(t.fl, structFile.name);
            return t;
        }
    }
    len = strlen(temp);
    for (int i = 0; i < len; i++)
    {
        if (i <= len)
        {
            // If all chars in token are digits, token type = integer
            if (isdigit(temp[i]))
            {
                count++;
                if (len == count)
                {
                    t.tp = INT;
                    t.ln = structFile.line;
                    strcpy(t.lx, array);
                    strcpy(t.fl, structFile.name);
                    return t;
                }
            }
        }
    }
    t.tp = ID;
    t.ln = structFile.line;
    strcpy(t.lx, array);
    strcpy(t.fl, structFile.name);
    return t;
}

// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken ()
{
  Token t;
    char temp[128];
    t = GetNextToken();
    strcpy(temp, t.lx);
    int len = strlen(t.lx);

    if (t.tp == STRING)
    {
        len++;
        for (int i = len; i >= 0; i--)
        {
            temp[i+1] = temp[i];
        }
        temp[0] = '"';
        temp[len] = '"';
    }

    for (int i = len; i >= 0; i--)
    {
        if (temp[i] == 0)
        {
            continue;
        }
        ungetc(temp[i], structFile.f);
    }
    strcpy(t.fl, structFile.name);
    t.ln = structFile.line;
    return t;
}

// clean out at end, e.g. close files, free memory, ... etc
int StopLexer ()
{
	return 0;
}

// do not remove the next line
#ifndef TEST
/*
int main (int argc, char* argv)
{
	// implement your main function here
  // NOTE: the autograder will not use your main function
    char f[50];
    strncpy(f, argv[1], 50);
    if (InitLexer(f) == 1)
    {
       GetNextToken();
    }
    return 0;
}
*/
// do not remove the next line
#endif
