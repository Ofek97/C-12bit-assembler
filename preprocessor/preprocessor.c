#include "preprocessor.h"
#include "../data_structures/vector/vector.h"
#include "../data_structures/trie/trie.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#define as_file_ext ".as"
#define am_file_ext ".am"
#define MAX_MACRO_LEN 31
#define MAX_LINE_LEN 81
#define SPACE_CHARS " \t\n\f\r\v"
#define SKIP_SPACE(s)         \
    while (*s && isspace(*s)) \
    s++
#define SKIP_SPACE_R(s, base)              \
    while (*s && isspace(*s) && base != s) \
    s++

struct macro
{
    char m_name[MAX_MACRO_LEN + 1];
    Vector lines;
};
static void *line_ctor(const void *copy)
{
    const char *line = copy;
    return strcpy(malloc(strlen(line) + 1), line);
}
static void line_dtor(void *item)
{
    free(item);
}
static void *macro_ctor(const void *copy)
{
    const struct macro *copy1 = copy;
    struct macro *new_macro = malloc(sizeof(struct macro));
    strcopy(new_macro->m_name, copy1->m_name);
    new_macro->lines = new_vector(line_ctor, line_dtor);
    return new_macro;
}
static void macro_dtor(void *item)
{
    struct macro *macro = item;
    vector_destroy(&macro->lines);
    free((void *)macro);
}
enum preproc_line_detection
{
    null_line,
    macro_definition,
    end_macro_definition,
    other_line,
    macro_already_exists,
    bad_macro_definition,
    bad_endmacro_definition,
    macro_call,
    bad_macro_call
};
enum preproc_line_detection preproc_check_line(char *line, struct macro **macro, const Vector macro_table, const Trie macro_lookup)
{
    struct macro new_macro = {0};
    char *tok, *temp;
    tok = strchr(line, ';');
    if (tok)
        *tok = '\0';
    SKIP_SPACE(line);
    if (*line == '\0')
        return null_line;
    tok = strstr(line, "mcro");
    if (tok)
    {
        temp = tok;
        SKIP_SPACE_R(temp, line);
        if (temp != line)
        {
            return bad_macro_definition;
        }
        tok += 4;
        SKIP_SPACE(tok);
        line = tok;
        tok = strbrk(line, SPACE_CHARS);
        if (tok)
        {
            *tok = '\0';
            tok++;
            SKIP_SPACE(tok);
            if (*tok != '\0')
            {
                return bad_macro_definition;
            }
        }
        *macro = trie_exists(macro_lookup, line);
        if (*macro)
        {
            return macro_already_exists;
        }
        strcpy(new_macro.m_name, line);
        trie_insert(macro_lookup, line, vector_insert(macro_table, &new_macro));
        return macro_definition;
    }
    tok = strstr(line, "endmcro");
    if (tok)
    {
        temp = tok;
        SKIP_SPACE_R(temp, line);
        if (temp != line)
        {
            return bad_endmacro_definition;
        }
        tok += 7;
        SKIP_SPACE(tok);
        if (*tok != '\0')
            return bad_endmacro_definition;
        *macro = NULL;
        return end_macro_definition;
    }
    tok = strpbrk(line, SPACE_CHARS);
    if (tok)
        *tok = '\0';
    *macro = trie_exists(macro_lookup, line);
    if (*macro == NULL)
    {
        *tok = ' ';
        return other_line;
    }
    tok++;
    SKIP_SPACE(tok);
    if (*tok != '\0')
        return bad_macro_call;
    return macro_call;
}
const char *preproc(const char *file_name)
{
    char line_buff[MAX_LINE_LEN] = {0};
    size_t file_name_len;
    char *as_file_name;
    char *am_file_name;
    FILE *am_file;
    FILE *as_file;
    Vector macro_table;
    Trie macro_table_lookup;
    struct macro *macro;
    void *const *begin;
    void *const *end;
    file_name_len = strlen(file_name);

    as_file_name = strcat(strcpy(malloc(file_name_len + strlen(as_file_ext) + 1), file_name), as_file_ext);
    am_file_name = strcat(strcpy(malloc(file_name_len + strlen(am_file_ext) + 1), file_name), am_file_ext);

    as_file = fopen(as_file_name, "r");
    am_file = fopen(am_file_name, "w");
    if (as_file == NULL || am_file == NULL)
    {

        free(as_file_name);
        free(am_file_name);
        return NULL;
    }

    macro_table = new_vector(macro_ctor, macro_dtor);
    macro_table_lookup = trie();
    while (fgets(line_buff, sizeof(line_buff), as_file))
    {
        switch (preproc_check_line(line_buff, &macro, macro_table_lookup, macro_table))
        {
        case null_line:
            break;
        case macro_definition:
            break;
        case end_macro_definition:
            break;
        case macro_call:
            VECTOR_FOR_EACH(begin, end, macro->lines)
            {
                if (*begin)
                {
                    fputs((const char *)(*begin), am_file);
                }
            }
            macro = NULL;
            break;
        case other_line:
            if (macro)
            {
                vector_insert(macro->lines, &line_buff[0]);
            }
            else
            {
                fputs(line_buff, am_file);
            }
            break;
        case macro_already_exists:
            /*ERROR*/
            break;
        case bad_endmacro_definition:
            /*ERROR*/
            break;
        case bad_macro_call:
            /*ERROR*/
            break;
        case bad_macro_definition:
            /*ERROR*/
            break;
        }
    }
    vector_destroy(&macro_table);
    trie_destroy(&macro_table_lookup);
    free(as_file_name);
    fclose(as_file);
    fclose(am_file);
    return am_file_name;
}