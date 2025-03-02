#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "build_tree.h"

// Declare the function prototype
int is_power_of_2(uint32_t n);

int get_str(char s[], int lim)
{
    int c, i;
    
    for (i=0; i<lim-1 && (c=getchar())!=EOF && c!='\n'; ++i)
        s[i] = c;
    s[i] = '\0';
    return i;
}
  
void postorder(struct node *root)
{
    if (root) {
      postorder(root->left);
      postorder(root->right);
      if (root->type == UNARYOP || root->type == BINARYOP){
        printf("%s", optable[root->data].symbol);
      }
      else if (root->type == VAR){
        printf("%c", 'a' + (char)root->data); //deleted: 
        //global variable of array of registers
        //added: extern char *variableAtRegister[32];
        //added: printf("%s\n", variableAtRegister[root->data]);
      }
      else{
        printf("%d", root->data);
      }
    }
  }
  
int main()
{
    char expr1[MAXEXPRLENGTH];
    struct node *root;

    while (get_str(expr1, MAXEXPRLENGTH)) {   
        printf("Expression to parse: %s\n", expr1);
        root = build_tree(expr1);
        printvartable();
        printregtable();
        printf("Postfix: ");
        postorder(root);
        printf("\n");
        root = generate_code(root);
        if (root->type == REG)
          printf("root: x%d\n", root->data);
        else if (root->type == CONST)
          printf("root: %d\n", root->data);
        printregtable();
        printf("\n");
    }

    // unsigned int test_nums[] = {1, 2, 4, 8, 16, 31, 64, 128, 256, 500, 1024, 2048, 4096, 8192};
    // int expected[] = {0, 1, 2, 3, 4, 0, 6, 7, 8, 0, 10, 11, 12, 13};

    // printf("Testing is_power_of_2 function:\n");
    // for (int i = 0; i < sizeof(test_nums) / sizeof(test_nums[0]); i++) {
        // int result = is_power_of_2(test_nums[i]);
        // printf("is_power_of_2(%d) = %d (Expected: %d) %s\n", 
               // test_nums[i], result, expected[i], 
               // result == expected[i] ? "Success" : "Failure");
    //}

    return 0;
}