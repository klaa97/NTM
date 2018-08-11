# NTM
Memory and time efficient non-deterministic Turing Machine single tape  simulator, with trivial loop checking, a stdin given template and a max number of steps before halting. 

## Running
It can be compiled with gcc easily.
```
gcc -o ntm ntm.c
./ntm.c < input.txt
```

## Assumptions
input.txt formatting and assumptions on TM are listed here.
* The machine starts at the initial state 0, the current tape symbol is the first char of the input string.
* The input string is made by the firsts 127 char of the ASCII table (you can easily tweak the code changing that).
* *_* is the blank symbol.  
### Formatting
Every input follows this format:
```
tr
S c w M E
acc
XX
max
YY
run
*****
```
* On the line after *tr* there is a list of 5-tuple, separated by a newline,describing all the transactions, where given the state *S* the machine is currently in *and* the symbol *c*, it replaces *c* with *w*, moves the head of *M* (three values, *L* (left), *R* (right) and *S* (stop)), and go to state *E*.
* On the line after *acc*, there is a the set of final states, separated by a newline. By convention, the machine crash on final states.
* On the line after *max*, there is the number of maximum step for each non-deterministic branch that the machine will compute.
* On the line after run, there is the set of the input string that the machine will tests, separated by a newline.

## Output
The machine can return three values:
* *1* , if the machine halts in a final state.
* *0*, if all the non-deterministic branches crash in a non-final state.
* *U*, if the machine doesn't halt in final state and at least one of the non-deterministic branches computes more than *max* step.


