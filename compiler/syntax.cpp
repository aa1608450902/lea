#include "syntax.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <map>

template<typename T, typename _> bool contains(std::map<T, _> map, T key) {return map.find(key) != map.end();}

/** type define */
struct VI_ATOM {
    int deep = 0;
    int type = -1;
    int args = 0;
    char name[512];
    /* record execute info */
};

struct EX_ATOM {
    int type = -1;
    char value[512];
    char label[512];
    EX_ATOM();
    EX_ATOM(int t, char* v);
    EX_ATOM(int t, const std::string& v);
};
EX_ATOM::EX_ATOM() {}
EX_ATOM::EX_ATOM(int t, char* v) {this->type = t;strcpy(this->value, v);}
EX_ATOM::EX_ATOM(int t, const std::string& v) {this->type = t;strcpy(this->value, v.c_str());}

class EX_DEEP_ATOM {
public:
    int deep = 0;
    std::vector<EX_ATOM> stack_ex;
};

struct SYMBOL {
    bool hasValue = false;
    int appType = 0;               // 1->variable 2->function
    std::string name;              // variable name/function name
    std::string value;             // value of string
    std::vector<std::string> sign; // variable->type function->ret_type,args_0_type,args_1_type...
    SYMBOL();
    SYMBOL(const std::string& n, const std::string& var_type);
    SYMBOL(const std::string& n, const std::string& var_type, const std::string& v);
    SYMBOL(const std::string& n, const std::vector<std::string>& sign);
    void clear();
};
SYMBOL::SYMBOL() {}
SYMBOL::SYMBOL(const std::string& n, const std::string& var_type) {
    this->appType = 1; this->name = n; this->sign.emplace_back(var_type); this->hasValue = false;
}
SYMBOL::SYMBOL(const std::string& n, const std::string& var_type, const std::string& v) {
    this->appType = 1; this->name = n; this->sign.emplace_back(var_type); this->value = v; this->hasValue = true;
}
SYMBOL::SYMBOL(const std::string& n, const std::vector<std::string>& sign) {
    this->appType = 2; this->name = n; this->sign = sign;
}
void SYMBOL::clear() {
    this->hasValue = false; this->appType = 0; this->name = ""; this->value = ""; this->sign.clear();
}
// -------------------------------------------------------------------------------------------

/** place for variable definition */
int lealine = 1;
int vi_deep = 0;
std::map<int, VI_ATOM> vis_m;
std::map<int, EX_DEEP_ATOM> stack_ex_m;
std::vector<std::string> asm_data_s;
std::vector<std::string> asm_text_s;
std::vector<std::string> scope_stack{"global"};
// map-map: scope -> (variable_name/function_name, symbol);
std::map<std::string, std::map<std::string, SYMBOL>> defines_of_scope;

#define A(x) asm_text_s.emplace_back(x)

void printf_empty(const char *__restrict __format, ...) {}

std::string record_scope() {
    std::string record;
    for (int i = 0; i < scope_stack.size(); i++) {
        if (i == scope_stack.size() - 1) {
            record.append(scope_stack[i]);
        } else {
            record.append(scope_stack[i]).append(".");
        }
    }
    return std::move(record);
}
/** scope control */
void scope_enter(char *scope) {
    scope_stack.emplace_back(std::string(scope));
}

void scope_exit() {
    if (scope_stack.back() == "global") 
        yyerror("Scope pop error!");
    scope_stack.pop_back();
}
/** symbol collect */
void var_def_end() {
    // SYMBOL symbol();
}

void var_ass_end() {

}

/** variable generation process */
SYMBOL building;

void bs_variable_name(char *name) {
    building.name = std::string(name);
    // printf(">> building.name : %s\n", building.name.c_str());
}

void bs_variable_type_sign(char *type) {
    // building.appType = app_type;
    building.sign.emplace_back(std::string(type)); 
}

void bs_variable_type(int app_type) {
    building.appType = app_type;
    // building.sign.emplace_back(std::string(type)); 
}

void bs_variable_type_judge() {
    // TODO: do judge
}

void bs_variable_value() {
    // TODO: take data from stack;
    //       the last data of stack.
    //       then -> do judge data type.
    bs_variable_type_judge();
}

void bs_variable_record() {
    // TODO: record and clear <building>
    std::string scope_sign = record_scope();
    if (contains(defines_of_scope, scope_sign)) {
        if (contains(defines_of_scope[scope_sign], building.name)) {
            yyerror(("Symbol "+building.name+" already exists.").c_str());
        } else {
            defines_of_scope[scope_sign][building.name] = building;
            building.clear();
        }
    } else {
        // printf("++record : %s ; scope : %s\n", building.name.c_str(), scope_sign.c_str());
        defines_of_scope[scope_sign] = std::map<std::string, SYMBOL>();
        defines_of_scope[scope_sign][building.name] = building;
        building.clear();
    }
}

void print_symbols() {
    printf("** symbol **\n");
    printf("\033[32;1m%-15s%-15s\033[0m\n", "Name", "Scene");
    printf("\033[32;1m%-15s%-15s\033[0m\n", "----", "-----");
    std::map<int, std::string> scenes{
        {1, "declare"}, {2, "declare & assign"}, {3, "assign"}, {4, "variable use"}, {5, "function invoke"},
    };
    for (const auto& scope_kv : defines_of_scope) {
        printf("[scope] %s\n", scope_kv.first.c_str());
        for (const auto& symbol_kv : scope_kv.second) {
            printf("%-15s%-20s\n", symbol_kv.first.c_str(), scenes[symbol_kv.second.appType].c_str());
        }
    }
}

/** variable invoking */
void vi_register(char* name) {
    vi_deep++;
    VI_ATOM vi_a;
    vi_a.deep = vi_deep;
    strcpy(vi_a.name, name);
    vis_m[vi_deep] = vi_a;
}
void vi_args() {
    vis_m[vi_deep].args++;
}
void vi_end_var() {
    // SYMBOL symbol(2, std::string(vis_m[vi_deep].name), );
    vis_m[vi_deep].type = 5;
    vi_deep--;
    ex_push_vi(vis_m[vi_deep+1]);
}
void vi_end_inv() {
    vis_m[vi_deep].type = 6;
    ex_invoke(vis_m[vi_deep].args, vis_m[vi_deep].name);
    vi_deep--;
    ex_push_vi(vis_m[vi_deep+1]);
}

// -1->byte 0->int 1->double 2->bool 3->char 4->string 5->variable 6->function invoke 7->asm label
void ex_close() {ex_show();g_print();}
void ex_push(EX_ATOM ea) {
    int vi_deep_p = vi_deep == -1 ? 0 : vi_deep;
    if (stack_ex_m.end() == stack_ex_m.find(vi_deep_p)) {
        EX_DEEP_ATOM eda;
        eda.deep = vi_deep_p;
        eda.stack_ex.emplace_back(ea);
    }
    stack_ex_m[vi_deep_p].stack_ex.emplace_back(ea);
}
void ex_push_v(EX_ATOM ea) {
    int vi_deep_p = vi_deep == -1 ? 0 : vi_deep;
    if (stack_ex_m.end() == stack_ex_m.find(vi_deep_p)) {
        EX_DEEP_ATOM eda;
        eda.deep = vi_deep_p;
        eda.stack_ex.emplace_back(ea);
    }
    stack_ex_m[vi_deep_p].stack_ex.emplace_back(ea);
}
void ex_push_i(int i) {
    EX_ATOM ex_a;
    sprintf(ex_a.value, "%d", i);
    ex_a.type = 0;
    ex_push(ex_a);
    ex_show();
}
void ex_push_d(double d) {
    EX_ATOM ex_a;
    sprintf(ex_a.value, "%f", d);
    ex_a.type = 1;
    ex_push(ex_a);
    ex_show();
}
void ex_push_s(char* s) {
    EX_ATOM ex_a;
    strcpy(ex_a.value, s);
    ex_a.type = 1;
    ex_push(ex_a);
    ex_show();
}
void ex_push_c(char c) {
    EX_ATOM ex_a;
    sprintf(ex_a.value, "%c", c);
    ex_a.type = 1;
    ex_push(ex_a);
    ex_show();
}
void ex_push_vi(const VI_ATOM& vi) {
    EX_ATOM ex_a;
    sprintf(ex_a.value, "%s", vi.name);
    ex_a.type = vi.type;
    ex_push_v(ex_a);
    ex_show();
}
void ex_invoke(int n, char *fun) {
    // pop args from stack_ex
    std::vector<EX_ATOM>& stack_ex = stack_ex_m[vi_deep <= -1 ? 0 : vi_deep].stack_ex;
    std::string args[n];
    for (int i = 0; i < n; i++) {
        args[i] = std::string(stack_ex.back().value);
        stack_ex.pop_back();
    }
    g_invoke(std::string(fun), args, n);
}
void ex_calculate(int n, char *op) {
    std::vector<EX_ATOM>& stack_ex = stack_ex_m[vi_deep <= -1 ? 0 : vi_deep].stack_ex;

    int op_n = n; std::string op_s(op);
    std::map<std::string, int> support_op {
        {"*", 0}, {"/", 1}, {"+", 2}, {"-", 3}, 
        {">", 4}, {"<", 5}, {">=", 6}, {"<=", 7}, 
        {"==", 8}, {"!=", 9}, {"&&", 10}, {"||", 11}
    };
    if (op_n == 1 && op_s == "!") {
        if (stack_ex.size() < 1)
            yyerror(std::string("Operator [" + op_s + "] needs two parameters! stack rest : 0").c_str());
        EX_ATOM p0 = stack_ex[stack_ex.size() - 1];
        if (p0.type > 4) {
            EX_ATOM _r(7, p0.value);
            stack_ex.pop_back(); stack_ex.emplace_back(_r);
        } else if (p0.type < 4) {
            int boo = p0.type < 3 ? atoi(p0.value) : (int) p0.value[0];
            EX_ATOM _r(2, std::to_string(boo == 0 ? 1 : 0));
            stack_ex.pop_back(); stack_ex.emplace_back(_r);
        } else {
            yyerror("Operator [!] don't support string type!");
        }
    } else if (op_n == 2 && support_op.find(op_s) != support_op.end()) {
        if (stack_ex.size() < 2) 
            yyerror(std::string("Operator [" + op_s + "] needs two parameters! stack rest : " + std::to_string(stack_ex.size())).c_str());
        EX_ATOM& p0 = stack_ex[stack_ex.size() - 2]; EX_ATOM& p1 = stack_ex[stack_ex.size() - 1];
        EX_ATOM _r;
        g_asm(op_s, p0, p1, _r);
        if (p0.type > 4) {
            EX_ATOM _r(7, p0.value);
            stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            return;
        } else if (p1.type > 4) {
            EX_ATOM _r(7, p1.value);
            stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            return;
        }

        if (support_op[op_s] == 0) {
            if (p0.type == 0 && p1.type == 0) {
                EX_ATOM _r(0, std::to_string(atoi(p0.value) * atoi(p1.value)));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type + p1.type == 1) {
                EX_ATOM _r(1, std::to_string(atof(p0.value) * atof(p1.value)));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else {yyerror("Unsupported type of [*]!");}
        } else if (support_op[op_s] == 1) {
            if (p0.type == 0 && p1.type == 0) {
                EX_ATOM _r(0, std::to_string(atoi(p0.value) / atoi(p1.value)));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type + p1.type == 1) {
                EX_ATOM _r(1, std::to_string(atof(p0.value) / atof(p1.value)));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else {yyerror("Unsupported type of [/]!");}
        } else if (support_op[op_s] == 2) {
            if (p0.type == 0 && p1.type == 0) {
                EX_ATOM _r(0, std::to_string(atoi(p0.value) + atoi(p1.value)));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type + p1.type == 1) {
                EX_ATOM _r(1, std::to_string(atof(p0.value) + atof(p1.value)));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else {yyerror("Unsupported type of [+]!");}
        } else if (support_op[op_s] == 3) {
            if (p0.type == 0 && p1.type == 0) {
                EX_ATOM _r(0, std::to_string(atoi(p0.value) - atoi(p1.value)));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type + p1.type == 1) {
                EX_ATOM _r(1, std::to_string(atof(p0.value) - atof(p1.value)));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else {yyerror("Unsupported type of [-]!");}
        } else if (support_op[op_s] == 4) {
            if (p0.type == 0 && p1.type == 0) {
                EX_ATOM _r(2, std::to_string(atoi(p0.value) - atoi(p1.value) > 0 ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type + p1.type == 1) {
                EX_ATOM _r(2, std::to_string(atof(p0.value) - atof(p1.value) > 0 ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else {yyerror("Unsupported type of [>]!");}
        } else if (support_op[op_s] == 5) {
            if (p0.type == 0 && p1.type == 0) {
                EX_ATOM _r(2, std::to_string(atoi(p0.value) - atoi(p1.value) < 0 ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type + p1.type == 1) {
                EX_ATOM _r(2, std::to_string(atof(p0.value) - atof(p1.value) < 0 ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else {yyerror("Unsupported type of [<]!");}
        } else if (support_op[op_s] == 6) {
            if (p0.type == 0 && p1.type == 0) {
                EX_ATOM _r(2, std::to_string(atoi(p0.value) - atoi(p1.value) >= 0 ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type + p1.type == 1) {
                EX_ATOM _r(2, std::to_string(atof(p0.value) - atof(p1.value) >= 0 ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else {yyerror("Unsupported type of [>=]!");}
        } else if (support_op[op_s] == 7) {
            if (p0.type == 0 && p1.type == 0) {
                EX_ATOM _r(2, std::to_string(atoi(p0.value) - atoi(p1.value) <= 0 ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type + p1.type == 1) {
                EX_ATOM _r(2, std::to_string(atof(p0.value) - atof(p1.value) <= 0 ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else {yyerror("Unsupported type of [<=]!");}
        } else if (support_op[op_s] == 8) {
            if ((p0.type == 0 || p0.type == 2) && (p1.type == 0 || p1.type == 2)) {
                EX_ATOM _r(2, std::to_string(atoi(p0.value) == atoi(p1.value) ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type == 1 && p1.type == 1) {
                EX_ATOM _r(2, std::to_string(atof(p0.value) == atof(p1.value) ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type == 3 && p1.type == 3) {
                EX_ATOM _r(2, std::to_string(p0.value[0] == p1.value[0] ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type == 4 && p1.type == 4) {
                EX_ATOM _r(2, std::to_string(std::string(p0.value) == std::string(p1.value) ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else {yyerror("Unsupported type of [==]!");}
        } else if (support_op[op_s] == 9) {
            if ((p0.type == 0 || p0.type == 2) && (p1.type == 0 || p1.type == 2)) {
                EX_ATOM _r(2, std::to_string(atoi(p0.value) != atoi(p1.value) ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type == 1 && p1.type == 1) {
                EX_ATOM _r(2, std::to_string(atof(p0.value) != atof(p1.value) ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type == 3 && p1.type == 3) {
                EX_ATOM _r(2, std::to_string(p0.value[0] != p1.value[0] ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else if (p0.type == 4 && p1.type == 4) {
                EX_ATOM _r(2, std::to_string(std::string(p0.value) != std::string(p1.value) ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else {yyerror("Unsupported type of [!=]!");}
        } else if (support_op[op_s] == 10) {
            if (p0.type == 2 && p1.type == 2) {
                EX_ATOM _r(2, std::to_string(atoi(p0.value) + atoi(p1.value) >= 2 ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else {yyerror("Unsupported type of [&&]!");}
        } else if (support_op[op_s] == 11) {
            if (p0.type == 2 && p1.type == 2) {
                EX_ATOM _r(2, std::to_string(atoi(p0.value) + atoi(p1.value) > 0 ? 1 : 0));
                stack_ex.pop_back(); stack_ex.pop_back(); stack_ex.emplace_back(_r);
            } else {yyerror("Unsupported type of [||]!");}
        }
    } else {
        yyerror(std::string("Unsupported operator " + op_s).c_str());
    }
}
void ex_show() {
    // for (const auto& stack : stack_ex_m) {
    //     printf("stack#%d#%d: ", stack.first, stack.second.stack_ex.size());
    //     for (const auto& item : stack.second.stack_ex) {
    //         printf("%s | ", item.value);
    //     }
    //     printf("\n");
    // }
    // printf("---\n");
}

/** data generation */
/** write many of validators */
/** validators' framework */
#define VA_DEF(x, y, z) int va_##x(const std::string& op, EX_ATOM& p0, EX_ATOM& p1, EX_ATOM& _r) \
{if (y) return 1; z; return 0;}
#define VA_CALL(x) va_##x(op, p0, p1, _r)
#define G(x) asm_text_s.emplace_back(x)
#define D(x) asm_data_s.emplace_back(x)
#define S(x) std::string(x)

VA_DEF(add_int, op == "+" && p0.type == 0 && p1.type == 0, {
    G("movl $"+S(p0.value)+",%eax");
    G("addl $"+S(p1.value)+",%eax");
    G("movl %eax,$"+S(_r.value));
})

VA_DEF(add_double, op == "+" && p0.type == 1 && p1.type == 1, {
    // fadd %st(1),%st(0)
    // fadd value
    G("flds "+S(p0.label));
    G("flds "+S(p1.label));
    G("faddp %st(1),%st(0)");
    G("fstpl "+S(_r.value));
})

VA_DEF(add_double_int, op == "+" && p0.type == 1 && p1.type == 0, {
    // fadd value
    G("flds "+S(p0.label));
    G("fadd "+S(p1.label));
    G("fstpl "+S(_r.value));
})

VA_DEF(add_int_double, op == "+" && p0.type == 1 && p1.type == 0, {
    // fadd value
    G("flds "+S(p1.label));
    G("fadd "+S(p0.label));
    G("fstpl $"+S(_r.value));
})

VA_DEF(sub_int, op == "-" && p0.type == 0 && p1.type == 0, {
    G("movl $"+S(p0.value)+",%eax");
    G("subl $"+S(p1.value)+",%eax");
    G("movl %eax,$"+S(_r.value));
})

VA_DEF(sub_double, op == "-" && p0.type == 1 && p1.type == 1, {
    G("flds "+S(p0.label));
    G("flds "+S(p1.label));
    G("fsubp %st(1),%st(0)");
    G("fstpl "+S(_r.value));
})

VA_DEF(sub_int_double, op == "-" && p0.type == 0 && p1.type == 1, {
    G("flds "+S(p0.label));
    G("flds "+S(p1.label));
    G("fsubp %st(1),%st(0)");
    G("fstpl "+S(_r.value));
})

VA_DEF(sub_double_int, op == "-" && p0.type == 1 && p1.type == 0, {
    G("flds "+S(p0.label));
    G("fsub "+S(p1.label));
    G("fstpl "+S(_r.value));
})

VA_DEF(gt, op == "-" && p0.type == 0 && p1.type == 0, {
    G("flds "+S(p0.label));
})

void g_data_def(const std::string& label, const std::string& asm_type, const std::string& value) {
    D(label+":");
    D(asm_type+" "+value);
}

void g_asm(const std::string& op, EX_ATOM& p0, EX_ATOM& p1, EX_ATOM& _r) {
    // asm_text_s.emplace_back("cmd["+op+"]\t"+std::string(p0.value)+","+std::string(p1.value));
    if (!VA_CALL(add_int)) return;
    if (!VA_CALL(add_double)) return;
    if (!VA_CALL(add_int_double)) return;
    if (!VA_CALL(add_double_int)) return;

    if (!VA_CALL(sub_int)) return;
    if (!VA_CALL(sub_double)) return;
    if (!VA_CALL(sub_int_double)) return;
    if (!VA_CALL(sub_double_int)) return;

}



void g_invoke(const std::string& fun, std::string args[], int size) {
    if (size > 5) {
        printf("Beyond args limitation!\n");
        exit(1);
    }
    std::string reg_seq[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    std::string pargs;
    for (int i = 0; i < size; i++) {
        pargs.append(args[i]).append(",");
        // A("mov $" + args[i] + ",%" + reg_seq[i]);
    }
    // A("call "+fun);
    A("call "+fun+" ("+pargs+")");
}

void g_print() {
    // printf(".text:\n");
    // for (auto& s : asm_text_s) {
    //     printf("\t%s\n", s.c_str());
    // }
}

