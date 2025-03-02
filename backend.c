#include <stdio.h>
#include <stdlib.h>

#include "build_tree.h"
#include <math.h>
#include <stdint.h>

int regtable[NUMREG];  // reg[i] contains current number of uses of register i
int vartable[NUMVAR];  // var[i] contains register to which var is assigned

void init_regtable(void) {
  for (int i = 0; i < NUMREG; i++) regtable[i] = 0;
}

void init_vartable(void) {
  for (int i = 0; i < NUMVAR; i++) vartable[i] = -1;
}

int is_power_of_2(uint32_t n){
  if(n==0){
    return 0;
  }
  return (n&(n-1))==0 ? (int)log2(n) : 0;
}

static int __reuse_reg(const int reg) {
  if (regtable[reg] == 1) return 1;
  if (regtable[reg] > 1) return 0;

  fprintf(stderr, "Error: called __reuse_reg on unused register\n");

  // shouldn't happen
  return -1;
}

int assign_reg(const int var) {
  if ((var != -1) && (vartable[var] != -1)) {
    // variable is already assigned a register
    regtable[vartable[var]]++;
    return vartable[var];
  }

  // find unassigned register
  for (int i = 5; i < NUMREG; i++) {
    if (regtable[i] == 0) {
      regtable[i]++;
      if (var != -1) {
        vartable[var] = i;
      }
      return i;
    }
  }

  // out of registers
  return -1;
}


static int __release_reg(const int reg) {
  if (regtable[reg] > 0) {
    regtable[reg]--;
    return 0;
  }

  return -1;
}

void printregtable(void) {
  printf("register table -- number of uses of each register\n");

  for (int i = 0; i < NUMREG; i++)
    if (regtable[i]) printf("register: x%d, uses: %d\n", i, regtable[i]);
}

void printvartable(void) {
  printf("variable table -- register to which var is assigned\n");

  for (int i = 0; i < NUMVAR; i++)
    if (vartable[i] != -1)
      printf("variable: %c, register: x%d\n", 'a' + i, vartable[i]);
}

struct node *generate_code(struct node *root) {
  struct node *left, *right;
  char instr[20];
  int destreg;

  if (root) {
    if (root->left) left = generate_code(root->left);

    if (root->right) right = generate_code(root->right);

    // if (root->type == REG) do nothing
    //if node is variable
    if (root->type == VAR) {
      //convert node type to register
      root->type = REG;
      //sets data to coressponding register number
      root->data = vartable[root->data];
    } 
    //if node is a binary operator
    else if (root->type == BINARYOP) {
      //check if both children nodes are registers, ex: reg ['binaryOp'] reg
      if ((left->type == REG) && (right->type == REG)) {
        //if left has only been used once(1 occurence)
        if (__reuse_reg(left->data) == 1) {
          destreg = left->data;
          //subtract number of ocurrences for right register by 1
          __release_reg(right->data);
        } 
        //if right has only been used once(1 occurence)
        else if (__reuse_reg(right->data) == 1) {
          destreg = right->data;
          //subtract number of ocurrences for left register by 1
          __release_reg(left->data);
        } 
        //both left and right children are either used more than once, or never used
        else {
          //find the next available register and assign it to destination register
          destreg = assign_reg(-1);
          //if there are no more available registers, print error
          if (destreg == -1) {
            printf("Error: out of registers\n");
            exit(-1);
          }
          //subtract number of ocurrences for left register by 1
          __release_reg(left->data);
          //subtract number of ocurrences for right register by 1
          __release_reg(right->data);
        }
        //print assembly instr: instr, destinatipn register, left register, right register
        printf("%s x%d, x%d, x%d\n", optable[root->data].instr, destreg,left->data, right->data); 
        //free children nodes
        free(left);
        free(right);
        //override root from binary operator to register
        root->type = REG;
        root->data = destreg;
      }//if left child node is a register and right child node is a constant
      else if ((left->type == REG) && (right->type == CONST)){
        //if left has only been used once(1 occurence)
        if (__reuse_reg(left->data) == 1) {
          //destination register = right data
          destreg = left->data;
          //subtract number of ocurrences for left register by 1
          __release_reg(right->data);
        } 
        //right node is either used more than once
        else {
          //find the next available register and assign it to destination register
          destreg = assign_reg(-1);
          //if there are no more available registers, print error
          if (destreg == -1) {
            printf("Error: out of registers\n");
            exit(-1);
          }
          //subtract number of ocurrences for left register by 1
          __release_reg(left->data);
          
        }
        //if assembly instruction is mul and the right operand is a power of 2, implement strength reduction by leftshiting 
        if(root->data == MUL && is_power_of_2((uint32_t)right->data)!=0){
          //note: we dont override operator node from mul to shift, we just change the print statement
          int reducedOperand;
          //if right node is a power of 2, we will reduce the right child
          if (is_power_of_2((uint32_t)right->data)!=0) {
            reducedOperand = is_power_of_2((uint32_t)right->data);
            //if right child is a power of 2, we will reduce the right child and print assembly instr in new format with reduced operand
            printf("slli x%d, x%d, %d\n", destreg,left->data, reducedOperand);
          } 
        }//if assembly instruction is div and the right operand is a power of 2, implement strength reduction by leftshiting 
        else if(root->data == DIV && ( is_power_of_2((uint32_t)right->data))){
          int reducedOperand;
          //if right child is a power of 2, we will reduce the right child
          if (is_power_of_2((uint32_t)right->data)) {
            reducedOperand = is_power_of_2((uint32_t)right->data);
            //if right child is a power of 2, we will reduce the right child and print assembly instr in new format with reduced operand
            printf("srai x%d, x%d, %d\n", destreg,left->data, reducedOperand);
          } 
        }//instruction is of normal immediate format
        else{
          //switch case for each different type of instruction
          switch(root->data) {
            case MUL:
              //add constant + x0 into destination register with addi instruction
              printf("addi x%d, x0, %d\n", destreg,  right->data);
              left->data=destreg;
              //get new destination register
              destreg=assign_reg(-1);
              //print immediate assembly instr
              printf("%s x%d, x%d, x%d\n", optable[root->data].instr, destreg,left->data, destreg);
              root->data = destreg;
              break;
            case SUB:
              //add -(constant) + x0 into destination register with addi instruction
              printf("addi x%d, x%d, -%d\n", destreg, left->data, right->data);
              root->data = destreg;
              break;
            case ADD:
              //add constant + x0 into destination register with addi instruction
              printf("addi x%d, x%d, %d\n", destreg, left->data, right->data);
              root->data = destreg;
              break;
            case XOR:
              //add constant + x0 into destination register with addi instruction
              printf("addi x%d, x0, %d\n", destreg,  right->data);
              //print immediate assembly instr
              printf("xori x%d, x%d, %d\n", destreg, left->data, right->data);
              root->data = destreg;
              break;
            case SLL:
              //print immediate assembly instr
              printf("slli x%d, x%d, %d\n", destreg, left->data, right->data);
              root->data = destreg;
              break;
            case SRL:
              //print immediate assembly instr
              printf("srli x%d, x%d, %d\n", destreg, left->data, right->data);
              root->data = destreg;
              break;
           
          }
        }
        //free left and right children nodes
        free(left);
        free(right);
        //override root data to a register
        root->type = REG;
        root->data = destreg;
        
      }//if left child node is a constant and right child node is a register
      else if ((left->type == CONST) && (right->type == REG)){
        //if left is a constant and we are multiplying, we can rearrange the register if strength reduction is valid
        if(root->data == MUL && is_power_of_2((uint32_t)left->data)){
          //note: we dont override operator node from mul to shift, we just change the print statement
          int reducedOperand;
          //if left node is a power of 2, we will reduce the left child
          if (is_power_of_2((uint32_t)left->data)) {
            reducedOperand = is_power_of_2((uint32_t)left->data);
            //if register only has 1 ocurrence
            if (__reuse_reg(right->data)) {
              destreg = right->data;
              __release_reg(left->data);
            } 
            else {//if register has more than 1 ocurrence
              //find a new register
              destreg = assign_reg(-1);
              if (destreg == -1) {
                printf("Error: out of registers\n");
                exit(-1);
              }
             //subtract number of ocurrences by 1
              __release_reg(right->data);
            }
            //if left child is a power of 2, we will reduce the right child and print assembly instr in new format with reduced operand
            printf("slli x%d, x%d, %d\n", destreg,right->data, reducedOperand);
            //free children nodes
            free(left);
            free(right);
            //override root to a register
            root->type=REG; 
            root->data=destreg;
          } 
        }
        else{
          //find a new register and add constant to new register using addi
          destreg = assign_reg(-1);
          printf("addi x%d, x0, %d\n",destreg,left->data);
          //override left constant to a register and make recursive call to go into reg-reg case
          left->type=REG;
          left->data=destreg;
          root = generate_code(root);
        }

      }//if both left and right child nodes are constants
      else if ((left->type == CONST) && (right->type == CONST)){
        //find a new register to store them in
        assign_reg(-1);
        root->type = CONST;

        //add if statements for all operators and do operations for both constants and assign to destreg
        int result=-1;
        //switch case that applies different operations to each constant
        switch(root->data) {
          case ADD:
            result = left->data + right->data;
            break;
          case SUB:
            result = left->data - right->data;
            break;
          case MUL:
            result = left->data * right->data;
            break;
          case DIV:
            result = left->data / right->data;
            break;
          case OR:
            result = left->data | right->data;
            break;
          case XOR:
            result = left->data ^ right->data;
            break;
          case SLL:
            result = left->data << right->data;
            break;
          case SRL:
            result = left->data >> right->data;
            break;
          default:
            break;
        }
        free(left);
        free(right);
        
        root->type = CONST;
        root->data=result;
        
    
      }
    } 

    else if (root->type == UNARYOP) {
      if (root->data == UMINUS) {
        if (left->type == REG) {
       
       
          if (__reuse_reg(left->data)) {
            destreg = left->data;
          } else {
            destreg = assign_reg(-1);
            if (destreg == -1) {
              printf("Error: out of registers\n");
              exit(-1);
            }
           
            __release_reg(left->data);
          }
         
          printf("sub x%d, x0, x%d\n", destreg, left->data);
          free(left);
          root->type = REG;
          root->data = destreg;
        }
        else if (left->type == CONST) {
          int result = 0 - left->data;
          free(left);
          root->type = CONST;
          root->data = result;
          
        }
      }
      else if (root->data == NOT) {
        if (left->type == REG) {
       
     
          //if register only has 1 ocurrence
          if (__reuse_reg(left->data)) {
            destreg = left->data;
          } 
          else {//if register has more than 1 ocurrence
            //find a new register
            destreg = assign_reg(-1);
            if (destreg == -1) {
              printf("Error: out of registers\n");
              exit(-1);
            }
            __release_reg(left->data);
          }
          printf("xori x%d, x%d, -1\n", destreg, left->data);
      
          free(left);
          root->type = REG;
          root->data = destreg;
        }
        else if (left->type == CONST) {
          int result = left->data ^ -1;
          free(left);
          root->type = CONST;
          root->data = result;
          
        }
      }
      

    }
  }
 

  return root;
}
