#define MAXOPS 100
#define MAXNODES 100
#define MAXNUMLENGTH 12
#define MAXEXPRLENGTH 100
#define LR 0
#define RL 1
#define NUMOPTYPES 13
#define NUMREG 32
#define NUMVAR 10

enum nodetype{VAR, REG, CONST, UNARYOP, BINARYOP};
enum ops{UMINUS, ADD, SUB, MUL, DIV, AND, OR, XOR, NOT, SLL, SRL, LPAREN, RPAREN};

void init_regtable(void);
void init_vartable(void);
int assign_reg(int var);
void printregtable(void);
void printvartable(void);
struct node *generate_code(struct node*);

struct operator {
    enum nodetype type;
    int  prec;
    int  assoc;
    char symbol[3];
    char instr[4];
};

extern struct operator optable[];

struct node {
    enum nodetype type;
    int data;
    struct node *left, *right;
};

void postorder(struct node *root);
struct node *build_tree(char exprin[]);

struct operatorstack {
    unsigned int top;
    int ops[MAXOPS];
};

struct nodestack {
    unsigned int top;
    struct node *nodes[MAXNODES];
};

