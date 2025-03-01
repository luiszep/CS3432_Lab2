#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "build_tree.h"

void remove_spaces(char* input){
    //tracks where to write characters, removing spaces through pointer to string.
    //moves through string reading characters one at a time
    char* write_pointer = input;
    char* read_pointer = input;

    //loop runs until the end of string, moving character by character on input
    //checks to see if characters are spaces, removing them if they are
    //copies non-space characters from pointers read to write to get stored
    while (*read_pointer != '\0'){
        if (*read_pointer != ' '){
            *write_pointer = *read_pointer;
            write_pointer++;
        }
        read_pointer++;
    }
    
    //goes through moving the write and read pointers until terminated by null.
    *write_pointer = '\0';
}

struct node *newNode(enum nodetype type, unsigned int data) {
    struct node *p = malloc(sizeof(struct node));
    p->type = type;
    p->data = data;
    p->left = NULL;
    p->right = NULL;
    return(p);
};

struct operator optable[NUMOPTYPES] = {
    {UNARYOP, 7, RL, "-",""},
    {BINARYOP, 5, LR, "+", "add"},
    {BINARYOP, 5, LR, "-", "sub"},
    {BINARYOP, 6, LR, "*", "mul"},
    {BINARYOP, 6, LR, "/", "div"},
    {BINARYOP, 3, LR, "&", "and"},
    {BINARYOP, 1, LR, "|", "or"},
    {BINARYOP, 2, LR, "^","xor"},
    {UNARYOP, 6, RL, "~", ""},
    {BINARYOP, 4, LR, "<<", "sll"},
    {BINARYOP, 4, LR, ">>", "srl"},
    {UNARYOP, 0, LR, "(", ""},
    {UNARYOP, 0, LR, ")", ""}
};
    
struct node *build_tree(char exprin[]) {

    remove_spaces(exprin);
    // printf("Expression after removing spaces: %s\n", exprin);
    
    struct operatorstack opstack;
    struct nodestack nstack;
    int length, numlength, num;
    struct node *t, *t1, *t2;
    int i;
    char numstr[MAXNUMLENGTH+1];
    enum ops op;
    char expr[MAXEXPRLENGTH+2];
    int var;
        
    init_vartable();
    init_regtable();
    expr[0] = '(';
    for (i = 0; i<strlen(exprin); i++)
        expr[i+1] = exprin[i];
    expr[i+1] = ')';
    expr[i+2] = '\0';
    opstack.top = 0;
    nstack.top = 0;
    length = strlen(expr);
    i = 0;
    while (i < length) {
        if (expr[i] == '(') {
            // push LPAREN on the opstack 
            if (opstack.top < MAXOPS) {
                opstack.ops[opstack.top] = LPAREN;
                opstack.top++;
            } else {
                printf("Error: too many operators\n");
                return(NULL);
            }
        } else if (isalpha(expr[i])) {
            var = (int) (expr[i] - 'a');
            assign_reg(var);
            t = newNode(VAR, var);
            // push node on the nodestack
            if (nstack.top < MAXNODES) {
                nstack.nodes[nstack.top] = t;
                nstack.top++;
            } else {
                printf("Error: too many nodes\n");
                return(NULL);
            }
        } else if (isdigit(expr[i])) {
            numstr[0] = expr[i];
            numlength = 1;
            while (isdigit(expr[i+1]) && (numlength < MAXNUMLENGTH)) {
               i++;
               numstr[numlength] = expr[i];
               numlength++;
            }
            numstr[numlength] = '\0';
            num = atoi(numstr);
            t = newNode(CONST, num);
            // push node on the nodestack
            if (nstack.top < MAXNODES) {
                nstack.nodes[nstack.top] = t;
                nstack.top++;
            } else {
                printf("Error: too many nodes\n");
                return(NULL);
            }
        } else {  // operator
            switch(expr[i]) {
                case('-'):
                   if ((i==0) || (!isalpha(expr[i-1]) && !isdigit(expr[i-1])))
                       op = UMINUS;
                   else
                       op = SUB;
                   break;
                case('+'):
                   op = ADD; break;
                case('*'):
                   op = MUL; break;
                case('/'):
                   op = DIV; break;
                case('&'):
                   op = AND; break;
                case('|'):
                   op = OR; break;
                case('^'):
                   op = XOR; break;
                case('~'):
                   op = NOT; break;
                case('<'):
                   if (expr[i+1] == '<') {
                      op = SLL; i++; break;
                   }
                case('>'):
                   if (expr[i+1] == '>') {
                       op = SRL; i++; break;
                   }
                case(')'):
                   op = RPAREN; break;
                default:
                   printf("Error: invalid character in expression: %c\n",
                         expr[i]);
                   return(NULL);
            }  // end switch
            if (optable[op].prec > 0) {
                while (opstack.top > 0 && opstack.ops[opstack.top-1] != LPAREN
                 && ((optable[op].assoc == LR 
                        && optable[opstack.ops[opstack.top-1]].prec >= optable[op].prec)
                     || (optable[op].assoc == RL
                        && optable[opstack.ops[opstack.top-1]].prec > optable[op].prec)))
                {
                   // Pop the operator stack and create node for popped op
                   t = newNode(optable[opstack.ops[opstack.top-1]].type, opstack.ops[opstack.top-1]); 
                   opstack.top--;
                   // Pop the node stack
                   if (t->type == UNARYOP) {
                       t2 = nstack.nodes[nstack.top-1];
                       nstack.top--;
                       t1 = NULL; 
                   } else {
                       t1 = nstack.nodes[nstack.top-1];
                       nstack.top--;
                       t2 = nstack.nodes[nstack.top-1];
                       nstack.top--;
                   }
                   // Add child/children and push new node on node stack
                   t->left = t2;
                   t->right = t1;
                   if (nstack.top < MAXNODES) {
                       nstack.nodes[nstack.top] = t;
                       nstack.top++;
                   } else {
                       printf("Error: too many nodes\n");
                       return(NULL);
                   }
               } // end while
               // Push op onto opstack
               if (opstack.top < MAXOPS) {
                   opstack.ops[opstack.top] = op;
                   opstack.top++;
               } else {
                   printf("Error: too many operators\n");
                   return(NULL);
               }
            } else if (op == RPAREN) {
                while (opstack.top > 0 && opstack.ops[opstack.top-1] != LPAREN) {
                   // Pop the operator stack and create node for popped op
                   t = newNode(optable[opstack.ops[opstack.top-1]].type, opstack.ops[opstack.top-1]); 
                   opstack.top--;
                   //if (t->data == 12) return(NULL);
                   // Pop the node stack
                   if (t->type == UNARYOP) {
                       t2 = nstack.nodes[nstack.top-1];
                       nstack.top--;
                       t1 = NULL; 
                   } else {
                       t1 = nstack.nodes[nstack.top-1];
                       nstack.top--;
                       t2 = nstack.nodes[nstack.top-1];
                       nstack.top--;
                   }
                   // Add child/children and push new node on node stack
                   t->left = t2;
                   t->right = t1;
                   if (nstack.top < MAXNODES) {
                       nstack.nodes[nstack.top] = t;
                       nstack.top++;
                   } else {
                       printf("Error: too many nodes\n");
                       return(NULL);
                   }
                } // end while
                if (opstack.top > 0)
                    opstack.top--;
            } // end if (op == RPAREN)
        }  // end operator
        i++;
    } // end while (i < length)
    if (nstack.top > 0)
      return(nstack.nodes[nstack.top-1]);
    else
      return(NULL);
}
