#ifndef __LEXER_H
#define __LEXER_H

#define MAX_NUMBER_DATA 80
#define MAX_LABEL_LEN
/* Define the structure for the abstract syntax tree (AST) */
struct ast {
    char syntax_error[150]; // Error message associated with the AST
    char label_name[MAX_LABEL_LEN + 1];
    enum {
        ast_inst, 
        ast_dir,
        ast_syntax_error  
    } ast_opt;      // AST option: instruction or directive
    union {
        struct {

            enum {
                ast_dir_extern, // Directive: EXTERN
                ast_dir_entry,  // Directive: ENTRY
                ast_dir_string, // Directive: STRING
                ast_dir_data   // Directive: DATA
            } ast_dir_opt;     // Directive option
              
            union {
                char* label_name;        // Label name associated with the directive
                char* string;            // String value associated with the directive
                struct {
                    int data[MAX_NUMBER_DATA]; // Data values associated with the directive
                    int data_count;            // Number of data values
                } data;
            } dir_operands; // Directive operands

        } ast_dir;  // AST represents a directive

        struct {
            enum {
                ast_inst_mov,   // Instruction: MOV
                ast_inst_cmp,   // Instruction: CMP
                ast_inst_add,   // Instruction: ADD
                ast_inst_sub,   // Instruction: SUB
                ast_inst_lea,   // Instruction: LEA
                ast_inst_not,   // Instruction: NOT
                ast_inst_clr,   // Instruction: CLR
                ast_inst_dec,   // Instruction: DEC
                ast_inst_jmp,   // Instruction: JMP
                ast_inst_bne,   // Instruction: BNE
                ast_inst_red,   // Instruction: RED
                ast_inst_prn,   // Instruction: PRN
                ast_inst_jsr,   // Instruction: JSR
                ast_inst_rts,   // Instruction: RTS
                ast_inst_stop   // Instruction: STOP
            } ast_inst_opt;     // Instruction option
        } ast_inst;             // AST represents an instruction

        union {
            int const_number;       // Constant number associated with the instruction
            int register_number;    // Register number associated with the instruction
            char* label;            // Label associated with the instruction
        } ast_inst_operands[2];     // Instruction operands

    } dir_or_inst;                  // Union representing either directive or instruction
};

/* Define the typedef for the AST structure */
typedef struct ast ast;

/* Function declaration for lexer_get_ast */
ast lexer_get_ast(char* logical_line);

#endif
