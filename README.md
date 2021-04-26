## Usage

```
Usage: main [-h, --help] [N] [X]
Arguments:
  N: number of operands involved (default 4)
  X: target value (default 24)
```

```
$ main
Give 4 numbers to work out 24:

1 2 3 4
(1+2+3)*4=24
1*2*3*4=24
2/1*3*4=24
(2+4)*(1+3)=24
[4 solutions]

1 5 5 5
(5-1/5)*5=24
[1 solution]
```

```
$ main 5 128
Give 5 numbers to work out 128:

-4 6 10 13 7
(13+7)*(6-(-4)/10)=128
10*7-(-4)*13+6=128
(13-(-4))*10-6*7=128
10*13-6/((-4)+7)=128
[4 solutions]
```

## Algorithm

Represent the expression as a binary tree. Find all solutions by backtracking. Output the in-order traversal sequence of each solution.

If the priority of left child node operator is lower than that of the parent node, or the priority of the right child operator is lower than or equal to that of the parent node, you need to add parentheses to the output sub-expression. For equivalent expressions like a/(b/c) and a/b*c, only the one whose parentheses cannot be removed by any transformation is output. A necessary and sufficient condition for the existence of parentheses is that there is an operator inside the parentheses whose priority is lower than that of one of the operators adjacent to the parentheses. The operator with the lowest priority inside the parentheses must be at the root of the subtree, and the operator with the highest priority adjacent to the parentheses must be at the parent node of the root of the subtree. It can be derived that when the priority of the right child node operator is equal to that of the parent node, the child search tree is skipped.

Consider the deduplication algorithm. Make each input operand map to a random number, and equal operands map to the same random number. Applying different functions to the same set of operands may work out the same target value, but their corresponding random numbers often do not. Otherwise, these functions are likely to be equivalent, i.e., duplicated. Known issue: Expressions like (a+b-b)/c and a/(c+b-b) are considered equivalent, because +b and -b are reciprocal regardless of the value of b. `strict.cpp` uses a deterministic method to determine whether two expressions are equivalent.
