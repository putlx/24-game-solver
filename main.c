#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define eq(x, y) (fabs((x) - (y)) < 0.01)

struct opr {
    double (*const fn)(double, double);
    const char ch;
    const int priority;
    const int reverse;
};

struct expr {
    const struct expr *lhs;
    const struct expr *rhs;
    const struct opr *op;
    double value;
    double hash;
};

struct node {
    double value;
    struct node *next;
};

static int N = 4;
static double X = 24.;
static struct node hashes = {0., NULL};

static double add(double x, double y) { return x + y; }
static double sub(double x, double y) { return x - y; }
static double rsub(double x, double y) { return y - x; }
static double mul(double x, double y) { return x * y; }
static double div_(double x, double y) { return x / y; }
static double rdiv(double x, double y) { return y / x; }

static void print(const struct expr *e) {
    if (!e->op) {
        printf(e->value < 0 ? "(%.lf)" : "%.lf", e->value);
        return;
    }

    const struct expr *lhs, *rhs;
    if (e->op->reverse) {
        lhs = e->rhs;
        rhs = e->lhs;
    } else {
        lhs = e->lhs;
        rhs = e->rhs;
    }

    if (lhs->op && lhs->op->priority < e->op->priority) {
        printf("(");
        print(lhs);
        printf(")");
    } else {
        print(lhs);
    }
    printf("%c", e->op->ch);
    if (rhs->op && rhs->op->priority < e->op->priority) {
        printf("(");
        print(rhs);
        printf(")");
    } else {
        print(rhs);
    }
}

static int included(double hash) {
    for (struct node *ptr = &hashes; (ptr = ptr->next);)
        if (eq(ptr->value, hash))
            return 1;
    return 0;
}

static void solve(const struct expr exprs[], int size) {
    static const struct opr oprs[] = {{add, '+', 0, 0},  {sub, '-', 0, 0},  {mul, '*', 1, 0},
                                      {div_, '/', 1, 0}, {rsub, '-', 0, 1}, {rdiv, '/', 1, 1}};

    if (size == 1) {
        if (eq(exprs[0].value, X) && !included(exprs[0].hash)) {
            struct node *ptr = malloc(sizeof(struct node));
            ptr->value = exprs[0].hash;
            ptr->next = hashes.next;
            hashes.next = ptr;
            print(&exprs[0]);
            printf("=%.lf\n", X);
        }
        return;
    }

    for (int x = 0; x < size; ++x) {
        for (int y = x + 1; y < size; ++y) {
            for (int k = 0; k < 6; ++k) {
                if (oprs[k].reverse && exprs[x].op && exprs[x].op->priority == oprs[k].priority)
                    continue;
                else if (!oprs[k].reverse && exprs[y].op && exprs[y].op->priority == oprs[k].priority)
                    continue;

                struct expr *gexprs = malloc((size - 1) * sizeof(struct expr));
                gexprs->lhs = &exprs[x];
                gexprs->rhs = &exprs[y];
                gexprs->op = &oprs[k];
                gexprs->value = oprs[k].fn(exprs[x].value, exprs[y].value);
                gexprs->hash = oprs[k].fn(exprs[x].hash, exprs[y].hash);
                for (int z = 0, i = 1; z < size; ++z)
                    if (z != x && z != y)
                        gexprs[i++] = exprs[z];
                solve(gexprs, size - 1);
                free(gexprs);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            printf("Usage: %s [-h, --help] [N] [X]\n"
                   "Arguments:\n"
                   "  N: number of operands involved (default 4)\n"
                   "  X: target value (default 24)\n",
                   argv[0]);
            return 0;
        }
    }
    if (argc > 1 && (N = atoi(argv[1])) <= 0)
        N = 4;
    if (argc > 2)
        X = (double)atoi(argv[2]);
    printf("Give %d integer%s to work out %.lf:\n\n", N, N < 2 ? "" : "s", X);

    srand((unsigned int)time(NULL));
    while (1) {
    begin:;
        struct expr *exprs = malloc(N * sizeof(struct expr));
        for (int i = 0; i < N; ++i) {
            if (scanf("%lf", &exprs[i].value) != 1) {
                for (char c; (c = getchar()) != '\n';) {
                    if (c == EOF) {
                        free(exprs);
                        return 0;
                    }
                }
                printf("[invalid input]\n\n");
                free(exprs);
                goto begin;
            }
            exprs[i].value = (double)(int)exprs[i].value;
            exprs[i].op = NULL;
            exprs[i].hash = rand() % 512;
            for (int j = 0; j < i; ++j) {
                if (exprs[i].value == exprs[j].value) {
                    exprs[i].hash = exprs[j].hash;
                    break;
                }
            }
        }
        solve(exprs, N);
        free(exprs);
        int n = 0;
        for (struct node *p; (p = hashes.next); free(p), ++n)
            hashes.next = p->next;
        printf("[%d solution%s]\n\n", n, n < 2 ? "" : "s");
    }
    return 0;
}
