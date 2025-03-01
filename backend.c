#include <stdio.h>
#include <stdlib.h>
#include "build_tree.h"

int regtable[NUMREG];  // reg[i] contains current number of uses of register i
int vartable[NUMVAR];  // var[i] contains register to which var is assigned

void init_regtable(void) 
{
    for (int i = 0; i < NUMREG; i++)
        regtable[i] = 0;
}

void init_vartable(void)
{
    for (int i = 0; i < NUMVAR; i++)
        vartable[i] = -1;
}
    
int reuse_reg(int reg)
{
    if (regtable[reg] == 1)
        return(1);
    if (regtable[reg] > 1)
        return(0);
    printf("Error: called reuse_reg on unused register\n");
    return(-1);  // shouldn't happen
}

int assign_reg(int var)
{ 
        if ((var != -1) && (vartable[var] != -1)) {
            regtable[vartable[var]]++; // variable is already assigned a register
            return(vartable[var]);
        } else {
            for (int i = 5; i < NUMREG; i++)     // find unassigned register
                if (regtable[i] == 0) {
                    regtable[i]++;
                    if (var != -1) {
                        vartable[var] = i;
                    }
                    return(i);;
            }
            return(-1);  // out of registers
        }
}  

int release_reg(int reg)
{
    if (regtable[reg] > 0) {
        regtable[reg]--;
        return(0);
    } else
        return(-1);
}    

void printregtable(void)
{
    printf("register table -- number of uses of each register\n");
    for (int i = 0; i < NUMREG; i++)
      if (regtable[i])
        printf("register: x%d, uses: %d\n", i, regtable[i]);
}

void printvartable(void)
{
    printf("variable table -- register to which var is assigned\n");
    for (int i = 0; i < NUMVAR; i++)
      if (vartable[i] != -1)
        printf("variable: %c, register: x%d\n", 'a'+i, vartable[i]);
}

struct node *generate_code(struct node *root)
{
    struct node *left, *right;
    char instr[20];
    int destreg;

    if (root) {
        if (root->left)
            left = generate_code(root->left);
        if (root->right)
            right = generate_code(root->right);
        if (root->type == REG)
            return(root);
        else if (root->type == VAR) {
            root->type = REG;
            root->data = vartable[root->data];
            return(root);
        } else if (root->type == BINARYOP) {
            if((left->type == REG) && (right->type == REG)) {
                if (reuse_reg(left->data) == 1) {
                    destreg = left->data;
                    release_reg(right->data);
                }
                else if (reuse_reg(right->data) == 1) {
                    destreg = right->data;
                    release_reg(left->data);
                }
                else {
                    destreg = assign_reg(-1);
                    if (destreg == -1) {
                        printf("Error: out of registers\n");
                        exit(-1); 
                    }
                    release_reg(left->data);
                    release_reg(right->data);
                }
                sprintf(instr, "%s  x%d, x%d, x%d", optable[root->data].instr, destreg, 
                        left->data, right->data);
                printf("%s\n", instr);
                free(left);
                free(right);
                root->type = REG;
                root->data = destreg;
            }   
        } else if (root->type == UNARYOP) {
            if (root->data == UMINUS) {
                if (left->type == REG) {
                    if (reuse_reg(left->data)) {
                        destreg = left->data;
                    } else {
                        destreg = assign_reg(-1);
                        if (destreg == -1) {
                            printf("Error: out of registers\n");
                            exit(-1);
                        }
                        release_reg(left->data);
                    }
                    sprintf(instr, "sub  x%d, x0, x%d", destreg, left->data);
                    printf("%s\n", instr);
                    free(left);
                    root->type = REG;
                    root->data = destreg;
                }
            } else if (root->data == NOT) { // Handles Unary NOT
                if (left->type == REG) {
                    if (reuse_reg(left->data)) {
                        destreg = left->data;
                    } else {
                        destreg = assign_reg(-1);
                        if (destreg == -1) {
                            printf("Error: out of register\n");
                            exit(-1);
                        }
                        release_reg(left->data);
                    }
                    sprintf(instr, "xori x%d, x%d, -1", destreg, left->data);
                    printf("%s\n", instr);
                    free(left);
                    root->type = REG;
                    root->data = destreg;
                }
            }
        }
    }
    return(root);
}
