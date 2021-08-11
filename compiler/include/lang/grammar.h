//
// Created by Mr.Jun on 2021/8/6.
//

#ifndef __grammar_h__
#define __grammar_h__

#ifdef __cplusplus
#  define __export_c       extern "C"
#  define __export_c_begin extern "C" {
#  define __export_c_end   }
/**
 * Inner use for cpp
 */
void declare(const char* _identifier);
void declare(const char* _identifier, const char* context);
#else
#  define __export_c
#  define __export_c_begin
#  define __export_c_end
extern int yyparse(void);

#endif

/**
 * Prepared for 'lea.y'
 */
#define _p_lex printf
#define _p_yacc printf_empty

extern long lea_line;
extern char output_file[100];
extern int n_pro;
extern int n_com;

__export_c void printf_empty(const char *__restrict __format, ...);

__export_c void _lea(char *name);
__export_c void _line_inc();

__export_c void _push(char *s);
__export_c void _pull(char *s);

__export_c void _open_mc();
__export_c void _close_mc();

__export_c void _scope_ano(long line);
__export_c void _scope_begin(char *scope);
__export_c void _scope_end(char *scope);

__export_c void _if();
__export_c void _elif();
__export_c void _else();
__export_c void _for();
__export_c void _match();
__export_c void _case();
__export_c void _def(char* fun_name);
__export_c void _def_();
__export_c void _block();
__export_c void _block_();
__export_c void _add_arg();
__export_c void _add_arg_str(char* s);
__export_c void _call(int args);
__export_c void _var_def(char* var_name);
__export_c void _var_ass(char* var_name);
__export_c void _var_def_ass(char* var_name);

__export_c void clean();
__export_c void complete();

__export_c void add_deep();
__export_c void rv_enter();
__export_c void rv_exit();
__export_c void rv_mul();
__export_c void rv_div();
__export_c void rv_add();
__export_c void rv_sub();
__export_c void rv_arg_add(char* val, char* type);
__export_c void rv_boo();
__export_c void rv_boo_ops(char* op);
__export_c void rv_cal_com();
__export_c void rv_cal_pro();

struct RV_ARG {
    char raw[100];
    char type[50];
};

#endif//__grammar_h__