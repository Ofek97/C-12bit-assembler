    #include <stdio.h>
    #include <string.h>
    #include "lexer.h"
 
    int main(){
        char line_buffer[100] ={0};
        char save[100];
        int line_count =1;
        ast ast;
        while(fgets(line_buffer,sizeof(line_buffer),stdin)){
            strcpy(save,line_buffer);
            ast = lexer_get_ast(line_buffer);
            line_count++;
        }
        return 0;
    }