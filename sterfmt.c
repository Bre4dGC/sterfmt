// #pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

static enum token_type {
    /* service */
    SERV_ILLEGAL, SERV_EOF,
    SERV_OPEN,    SERV_CLOSE,  // < >
    SERV_LBRACE,  SERV_RBRACE, // { }
    SERV_COLON,   SERV_DELIM,  // : ,
    SERV_RESET,                // /
    
    /* direction */
    DIRECT_UP, DIRECT_DOWN, 
    DIRECT_LEFT, DIRECT_RIGHT,

    /* decoration */
    DECOR_NONE, DECOR_BOLD, DECOR_UNDERLINE, 
    DECOR_ITALIC, DECOR_BLINK, DECOR_INVERT, DECOR_STRIKE,

    /* alignment */
    ALIGN_DEFAULT, ALIGN_JUSTIFY, ALIGN_CENTER,

    /* color */
    COLOR_DEFAULT, COLOR_BLACK,  COLOR_RED,
    COLOR_GREEN,   COLOR_YELLOW, COLOR_BLUE,
    COLOR_MAGENTA, COLOR_CYAN,   COLOR_WHITE,
};

static struct lexer {
    char* input;
    char ch;
    uint16_t pos;
    uint16_t nextpos;
    uint16_t paren_balance;
};

static struct token {
    enum token_type type;
    char* literal;
};

static struct keyword {    
    const char *literal;
    uint8_t type;
};

static const struct keyword keywords[] = {
    /* direction */
    {"up",    DIRECT_UP},
    {"down",  DIRECT_DOWN},
    {"left",  DIRECT_LEFT},
    {"right", DIRECT_RIGHT},

    /* decoration */
    {"bold",      DIRECT_UP},
    {"underline", DIRECT_DOWN},
    {"italic",    DIRECT_LEFT},
    {"blink",     DIRECT_RIGHT},
    {"invert",    DIRECT_RIGHT},
    {"strike",    DIRECT_RIGHT},

    /* colors */
    {"default", COLOR_DEFAULT},
    {"red",     COLOR_RED},
    {"green",   COLOR_GREEN},
    {"blue",    COLOR_BLUE},
    {"black",   COLOR_BLACK},
    {"white",   COLOR_WHITE},
    {"magenta", COLOR_MAGENTA},
    {"cyan",    COLOR_CYAN},
    {"yellow",  COLOR_YELLOW},
};

static const int keywords_count = sizeof(keywords) / sizeof(struct keyword);

static void read_ch(struct lexer* lex)
{
    if(lex->nextpos >= strlen(lex->input)) {
        lex->ch = 0;
    }
    else {
        lex->ch = lex->input[lex->nextpos];
    }
    lex->pos = lex->nextpos;
    lex->nextpos++;
}

static struct lexer* new_lexer(const char* input)
{
    struct lexer* lex = (struct lexer*)malloc(sizeof(struct lexer));
    if (!lex) return NULL;

    lex->input = (char*)malloc((strlen(input) + 1) * sizeof(char));
    if (!lex->input){free(lex); return NULL; }

    strcpy(lex->input, input);
    lex->input[strlen(input)] = L'\0';

    lex->pos = 0; lex->nextpos = 1; lex->ch = input[0];
    return lex;
}

static struct token new_token(const enum token_type type, const char *literal)
{
    struct token tok = { .type = type, .literal = NULL };
    if(literal){ 
        tok.literal = strdup(literal);
        if (!tok.literal){ tok.type = SERV_ILLEGAL; return tok; }
    }
    return tok;
}

static void free_lexer(struct lexer *lex) 
{
    if(lex){free(lex->input); free(lex);}
}

static void free_token(struct token *tok)
{
    if(tok){free(tok->literal);}
}

static const struct keyword *find_keyword(const char *ident)
{
    for (size_t i = 0; i < keywords_count; ++i) {
        if (strcmp(ident, keywords[i].literal) == 0) return &keywords[i];
    }
    return NULL;
}

static struct token handle_paren(struct lexer* lex)
{
    enum token_type type;
    switch (lex->ch) {
        case '<': type = SERV_OPEN; break;
        case '>': type = SERV_CLOSE; break;
        case '{': type = SERV_LBRACE; break;
        case '}': type = SERV_RBRACE; break;
    }
    struct token tok = new_token(type, (char[]){lex->ch, L'\0'});
    read_ch(lex);
    return tok;
}

static struct token handle_ident(struct lexer* lex)
{
    const int kwsize = 32;
    char ident[kwsize];
    
    int i = 0;
    while(isalpha(lex->ch)){
        if(i == kwsize - 1) break;
        ident[i++] = lex->ch;
        read_ch(lex);
    }
    ident[i] = '\0';

    const struct keyword *keyw = find_keyword(ident);
    if (keyw) return new_token(keyw->type, keyw->literal);

    return new_token(SERV_ILLEGAL, keyw->literal);
}

static struct token next_token(struct lexer* lex) 
{
    if(isspace(lex->ch)) read_ch(lex);

    char literal[2] = {lex->ch, '\0'};
    struct token tok;
    
    switch(lex->ch){
        case '<': case '>':
        case '{': case '}':
            if (lex->ch == '<' || lex->ch == '{') lex->paren_balance++;
            else if (lex->ch == '>' || lex->ch == '}') {
                if (lex->paren_balance == 0) printf("error at %d\n", lex->pos); 
                else lex->paren_balance--;
            }
            tok = handle_paren(lex);
            break;
        case ':': 
            tok = new_token(SERV_COLON, literal);
            read_ch(lex);
            break;
        case ',': 
            tok = new_token(SERV_DELIM, literal);
            read_ch(lex);
            break;
        case '/': 
            tok = new_token(SERV_RESET, literal);
            read_ch(lex);
            break;
        case '\0': 
            tok = new_token(SERV_EOF, "EOF");
            break;
        default:
            if(isalpha(lex->ch)) tok = handle_ident(lex);
            else tok = new_token(SERV_ILLEGAL, "ILLEGAL");
    }
    return tok;
}

int main()
{
    const char* input = "<bold, red></>";

    printf("%s\n\n", input);
    struct lexer *lex = new_lexer(input);
    
    struct token tok;
    while((tok = next_token(lex)).type != SERV_EOF) {
        printf("%s\n", tok.literal);
        free_token(&tok);
    }
    free_lexer(lex);
    free_token(&tok);

    return 0;
}

int sfinput(const char *fmt, ...);
int sfprint(const char *fmt, ...);
int sfreg(const char *name, void (*func)(void*));