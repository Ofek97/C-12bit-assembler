#ifndef __LEXER_H
#define __LEXER_H

#define MAX_NUMBER_DATA 80
struct ast{
    char ast_error_msg[150];
    enum {
        ast_inst,
        ast_dir
    }ast_opt;
    union 
    {
        struct {
            enum{
                ast_dir_extern,
                ast_dir_entry,
                ast_dir_string,
                ast_dir_data
            }ast_dir_opt;
        union{
            char* label_name;
            char* string;
            struct 
            {
                int data[MAX_NUMBER_DATA];
                int data_count;
            }data;
          }dir_operands;
        }ast_dir;
        struct{
            enum{
                ast_inst_mov,
                ast_inst_cmp,
                ast_inst_add,
                ast_inst_sub,
                ast_inst_lea,

                ast_inst_not,
                ast_inst_clr,
                ast_inst_dec,
                ast_inst_jmp,
                ast_inst_bne,
                ast_inst_red,
                ast_inst_prn,
                ast_inst_jsr,

                ast_inst_rts,
                ast_inst_stop
            }ast_inst_opt;
        }ast_inst;
        union
        {
            int const_number;
            int register_number;
            char* label;
        }ast_inst_operands[2];
        
    }dir_or_inst;
};
typedef struct ast ast;
 
 ast lexer_get_ast(char* logical_line);




#endif