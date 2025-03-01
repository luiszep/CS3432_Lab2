#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "build_tree.h"

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
    //Checks to see if node is empty. If empty, does nothing. Serves as final stopping condition.
    if (root == NULL) return;

    //Visit Left and Right Subtrees
    //Moves in post order direction recursively
    postorder(root->left);
    postorder(root->right);

    //Print the Current Node 
    //After completing the left and right subtrees, we print the root node. 

    //Prints a character as variable
    if (root->type == VAR) {
        printf("%c", 'a' + root->data);
    }
    //Prints number as constant
    else if (root->type == CONST) {
        printf("%d", root->data);
    }
    //Prints operator depending in Unary or Binaryop on the optable
    else {
        printf("%s", optable[root->data].symbol);
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
    return 0;
}