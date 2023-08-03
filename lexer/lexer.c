#include "lexer.h"
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "../data_structures/trie/trie.h"
#define MAX_REG 7
#define MIN_REG 0
#define MAX_CONST_NUM 511
#define MIN_CONST_NUM -512
#define SPACE_CHARS " \t\n\f\r\v"
#define SKIP_SPACE(s)         \
    while (*s && isspace(*s)) \
    s++
#define SKIP_SPACE_R(s, base)              \
    while (*s && isspace(*s) && base != s) \
    s++

Trie instruction_lookup = NULL;
Trie directive_lookup = NULL;
static int is_trie_inited = 0;
static struct asm_instruction_binding{
    const char* inst_name;
    int key;
    const char* src_operand_options; /* I - immediate(1), L - label ( direct adressing)(3), R - register(5)*/
    const char* dest_operand_options;/* I - immediate(1), L - label ( direct adressing)(3), R - register(5)*/
    
} asm_instruction_binding[16] = {
    {"mov",ast_inst_mov,"ILR","LR"},
    {"cmp",ast_inst_cmp,"ILR","ILR"},
    {"add",ast_inst_add,"ILR","LR"},
    {"sub",ast_inst_sub,"ILR","LR"},
    {"lea",ast_inst_lea,"L","LR"},

    {"not",ast_inst_not,NULL,"LR"},
    {"clr",ast_inst_clr,NULL,"LR"},
    {"inc",ast_inst_inc,NULL,"LR"},
    {"dec",ast_inst_dec,NULL,"LR"},
    {"jmp",ast_inst_jmp,NULL,"LR"},
    {"bne",ast_inst_bne,NULL,"LR"},
    {"red",ast_inst_red,NULL,"LR"},
    {"prn",ast_inst_prn,NULL,"ILR"},
    {"jsr",ast_inst_jsr,NULL,"LR"},

    {"rts",ast_inst_rts,NULL,NULL},
    {"stop",ast_inst_mov,NULL,NULL}
};
static struct asm_directive_binding{
    const char* directive_name;
    int key;
}asm_directive_binding[4] = {
    {"data",ast_dir_data},
    {"string",ast_dir_string},
    {"extern",ast_dir_extern},
    {"entry",ast_dir_entry}
};
static void lexer_trie_init(){
    int i;
    instruction_lookup = trie();
    directive_lookup = trie();
    for(i=0;i<16;i++){
        trie_insert(instruction_lookup,asm_instruction_binding[i].inst_name,&asm_instruction_binding[i]);
    }
    for(i=0;i<4;i++){
        trie_insert(directive_lookup,asm_directive_binding[i].directive_name,&asm_directive_binding[i]);
    }
    is_trie_inited=1;
}
enum lexer_valid_label_err{
    label_ok,
    starts_without_alpha,
    contains_none_alpha_numeric,
    label_is_too_long

};
static enum lexer_valid_label_err lexer_is_valid_label(const char* label){
    int char_count = 0;
    if (!isalpha(*label)){
        return starts_without_alpha;
    }
    label++;
    while(*label && isalpha(*label)){
        char_count++;
        label++;
    }
    if(*label !='\0'){
        return contains_none_alpha_numeric;
    }
    if(char_count > MAX_LABEL_LEN){
        return label_is_too_long;
    }
    return label_ok;

}

static int lexer_parse_number( const char* num_str, char** endptr, long *num, long min, long max){
    char *my_end;
    *num = strtol(num_str,&my_end,10);
     errno = 0;
    while(isspace(*my_end)) my_end++;
    if(*my_end != '\0'){
        return -1;
    }
    if(errno == ERANGE){
        return -2;
    }
    if(*num > max || *num < min){
        return -3;
    }
    if(endptr)
        *endptr = my_end;
    return 0;
    
}
/**
 * @brief
 * @param operand_string
 * 
 * @return char returns I,L,R. N if unknown operand or E if empty, or F if constant number overflow
*/
static char lexer_parse_operand_i(char* operand_string, char** label,int* const_number, int* register_number){
    char* temp;
    char* temp2;
    long num;
    int ret;

    if(operand_string == NULL)
        return 'E';
    SKIP_SPACE(operand_string);
    if(*operand_string =='\0')
        return 'E';
    if(*operand_string == '@'){
        if((*operand_string + 1) == 'r'){
            if(*(operand_string + 2) == '+'|| *(operand_string + 2) == '-'){
                return 'N';
            }
            if(lexer_parse_number(operand_string+2,NULL,&num,MIN_REG,MAX_REG)!= 0){
                return 'N';
            }
            if(register_number)
                *register_number = (int)num;
            return 'R';
        }
        return 'N';
    }
    if(isalpha(*operand_string)){
        temp2 = temp = strpbrk(operand_string,SPACE_CHARS);
        if(temp) {
        *temp ='\0';
        temp++;
        SKIP_SPACE(temp);
        if(*temp !='\0'){
            *temp2 = ' ';
            return 'N';
            }
        }
        if(lexer_is_valid_label(operand_string) != label_ok){
            return 'N';
        }
        if(label)
            (*label) = operand_string;
        return 'L';
    }
    if((ret =lexer_parse_number(operand_string,NULL,&num,MIN_CONST_NUM,MAX_CONST_NUM)) < -2 ){
        return 'F';
    }else if(ret == 0){
        if(const_number)
            (*const_number) = num;
        return 'I';
    }
    return 'N';

    
}

static void lexer_parse_instruction_operands(ast* ast,char* operands_string, struct asm_instruction_binding* aib){
    char operand_i_option;
    char* aux1;
    char* sep = NULL;
    if(operands_string)
      sep = strchr(operands_string,',');
    if(sep){
        aux1 = strchr(sep+1,',');
        if(aux1){
            strcpy(ast->syntax_error,"Syntax error: found two or more ',' tokens. ");
    
            return;
        }
        if(aib->src_operand_options == NULL){
            sprintf(ast->syntax_error,"instruction: '%s' expects one operand, but has two",aib->inst_name);
    
            return;
        }
        *sep ='\0';

         operand_i_option = lexer_parse_operand_i(operands_string,&ast->dir_or_inst.ast_inst.ast_inst_operands[0].label,
         &ast->dir_or_inst.ast_inst.ast_inst_operands[0].const_number,&ast->dir_or_inst.ast_inst.ast_inst_operands[0].register_number);

         if(operand_i_option == 'N'){
            sprintf(ast->syntax_error,"unknown operand: '%s' for source",operands_string);
    
            return;
         }
         if(operand_i_option == 'F'){
            sprintf(ast->syntax_error,"overflowing immediate operand: '%s' for source",operands_string);
    
            return;
         }
         if(operand_i_option == 'E'){
            sprintf(ast->syntax_error,"got no operand for source");
    
            return;
         }
         if(strchr(aib->src_operand_options,operand_i_option) == NULL){
            sprintf(ast->syntax_error,"source operand is not supported");
    
            return; 
         }
        ast->dir_or_inst.ast_inst.ast_inst_operand_opt[0] = operand_i_option == 'I' ? ast_operand_opt_const_num : operand_i_option == 'R' ? ast_operand_opt_register : ast_operand_opt_label;
         operands_string = sep+1;
         operand_i_option = lexer_parse_operand_i(operands_string,&ast->dir_or_inst.ast_inst.ast_inst_operands[1].label,
         &ast->dir_or_inst.ast_inst.ast_inst_operands[1].const_number,&ast->dir_or_inst.ast_inst.ast_inst_operands[1].register_number);

         if(operand_i_option == 'N'){
            sprintf(ast->syntax_error,"unknown operand: '%s' for destination",operands_string);
            return;
         }
         if(operand_i_option == 'F'){
            sprintf(ast->syntax_error,"overflowing immediate operand: '%s' for destination",operands_string);
            return;
         }
         if(operand_i_option == 'E'){
            sprintf(ast->syntax_error,"got no operand for destination");
            return;
         }
          if(strchr(aib->dest_operand_options,operand_i_option) == NULL){
            sprintf(ast->syntax_error,"destination operand is not supported");
            return; 
         }
        ast->dir_or_inst.ast_inst.ast_inst_operand_opt[1] = operand_i_option == 'I' ? ast_operand_opt_const_num : operand_i_option == 'R' ? ast_operand_opt_register : ast_operand_opt_label;

    }else{
        if(aib->src_operand_options != NULL){
            sprintf(ast->syntax_error,"instruction:'%s' expects seperator token: ','",aib->inst_name);
    
            return;
        }
        operand_i_option = lexer_parse_operand_i(operands_string,&ast->dir_or_inst.ast_inst.ast_inst_operands[1].label,
         &ast->dir_or_inst.ast_inst.ast_inst_operands[1].const_number,&ast->dir_or_inst.ast_inst.ast_inst_operands[1].register_number);

         if(operand_i_option != 'E' && aib->dest_operand_options == NULL){
            sprintf(ast->syntax_error,"instruction:'%s' expects no operands",aib->inst_name);
            return;
         }
         if(operand_i_option == 'E'){
            sprintf(ast->syntax_error,"instruction:'%s' expects one operand",aib->inst_name);
            return; 
         }
         if(operand_i_option == 'F'){
            sprintf(ast->syntax_error,"overflowing immediate operand: '%s' for destination",operands_string);
            return;
         }
         if(operand_i_option == 'N'){
            sprintf(ast->syntax_error,"unknown operand: '%s' for destination",operands_string);
            return;
         }
         if(strchr(aib->dest_operand_options,operand_i_option) == NULL){
            sprintf(ast->syntax_error,"destination operand is not supported");
            return; 
         }
         ast->dir_or_inst.ast_inst.ast_inst_operand_opt[1] = operand_i_option == 'I' ? ast_operand_opt_const_num : operand_i_option == 'R' ? ast_operand_opt_register : ast_operand_opt_label;

    }
    
}
static void lexer_parse_directive(ast* ast,char* operand_string,struct asm_directive_binding* adb){
    char* sep;
    char* sep2;
    int num_count = 0;
    int curr_num;
    if(adb->key <=ast_dir_entry){
        if(lexer_parse_operand_i(operand_string,&ast->dir_or_inst.ast_dir.dir_operand.label_name,NULL,NULL) != 'L'){
            sprintf(ast->syntax_error,"directive:'%s' with invalid operand:'%s'",adb->directive_name,operand_string);
    
            return;
        }
    }
        else if(adb->key == ast_dir_string){
            sep= strchr(operand_string,'"');
            if(!sep){
                sprintf(ast->syntax_error,"directive: '%s' has no opening '\"' : '%s'",adb->directive_name,operand_string);
            }
            sep++;
            sep2 = strchr(sep,'"');
            if(!sep2){
                sprintf(ast->syntax_error,"directive: '%s' has no closing '\"' : '%s'",adb->directive_name,operand_string);
            }
            *sep2 = '\0';
            sep2++;
            SKIP_SPACE(sep2);
            if(*sep2 != '\0'){
                sprintf(ast->syntax_error,"directive: '%s' has extra text after the string '%s' ",adb->directive_name,sep2);
            }
            ast->dir_or_inst.ast_dir.dir_operand.string = sep;
    }
        else if(adb->key == ast_dir_data){
        do{
            sep = strchr(operand_string,',');
            if(sep){ 
            *sep = '\0';
            }
            switch(lexer_parse_operand_i(operand_string,NULL,&curr_num,NULL)){
                case 'I':
                    ast->dir_or_inst.ast_dir.dir_operand.data.data[num_count] = curr_num;
                    num_count++;
                    ast->dir_or_inst.ast_dir.dir_operand.data.data_count = num_count;
                break;
                case 'F':
                    sprintf(ast->syntax_error,"directive: '%s' overflow number: '%s'",adb->directive_name,operand_string);
                    return;
                break;
                case 'E':
                    sprintf(ast->syntax_error,"directive: '%s' got an empty string while expecting a number",adb->directive_name);
                break;
                default:
                    sprintf(ast->syntax_error,"directive: '%s' unsupported string:'%s'",adb->directive_name,operand_string);
                break;
            }
            if(sep){
                operand_string = sep+1;
            }
            else{
                break;
            }
        }while(1);
    }
}
void lexer_dealloc(){
    is_trie_inited = 0;
    trie_destroy(&directive_lookup);
    trie_destroy(&instruction_lookup);
}
ast lexer_get_ast(char* logical_line){
    ast ast = {0};
    enum lexer_valid_label_err lvle;
    struct asm_instruction_binding* aib = NULL;
    struct asm_directive_binding* adb = NULL;
    char* aux1,* aux2;
    if(!is_trie_inited){
        lexer_trie_init();
    }
    logical_line[strcspn(logical_line, "\r\n")] = 0;
    SKIP_SPACE(logical_line);
    aux1 = strchr(logical_line,':');
    if(aux1){
        aux2 = strchr(aux1+1,':');
        if(aux2){
            strcpy(ast.syntax_error,"token ':' appears twice in this line");
            return ast;
        }
        (*aux1) = '\0';
        switch(lvle = lexer_is_valid_label(logical_line)){
            case starts_without_alpha:
                sprintf(ast.syntax_error,"label '%s' starts with non alpha-numeric characters",logical_line);
            break;
            case contains_none_alpha_numeric:
                sprintf(ast.syntax_error,"label '%s' contains non alpha-numeric characters",logical_line);
            break;
            case label_is_too_long:
                sprintf(ast.syntax_error,"label '%s' is longer than %d",logical_line,MAX_LABEL_LEN);
            break;
            case label_ok:
                strcpy(ast.label_name,logical_line);
            break;
        }
        if(lvle != label_ok){
            
            return ast;
        }
        logical_line = aux1+1;
        SKIP_SPACE(logical_line);
    }

    
    if(*logical_line == '\0' && ast.label_name[0] != '\0'){
        sprintf(ast.syntax_error,"line contains only a label:'%s'",ast.label_name);
        return ast;
    }
    
    aux1 = strpbrk(logical_line,SPACE_CHARS);
    if(aux1) {
        *aux1 = '\0';
        aux1++;
        SKIP_SPACE(aux1);
    }
    if(*logical_line == '.'){
        adb = trie_exists(directive_lookup,logical_line+1);
        if(!adb){
            sprintf(ast.syntax_error,"unknown directive:'%s'",logical_line + 1);
            return ast;
        }
        ast.ast_opt = ast_dir;
        ast.dir_or_inst.ast_dir.ast_dir_opt = adb->key;
        lexer_parse_directive(&ast, aux1,adb);
        return ast;
    }
    aib = trie_exists(instruction_lookup, logical_line);
    if(!aib){
       sprintf(ast.syntax_error,"unknown keyword: '%s'",logical_line);
       return ast;
    }
    ast.ast_opt = ast_inst;
    ast.dir_or_inst.ast_inst.ast_inst_opt = aib->key;
    lexer_parse_instruction_operands(&ast,aux1,aib);

    return ast;
}