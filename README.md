# MondrianPuzzle

Solution to challenge https://www.reddit.com/r/dailyprogrammer/comments/9dv08q/20180907_challenge_367_hard_the_mondrian_puzzle/.

The mondrian program is the solver and expects the following parameters on standard input:
- Square order (>= 3)
- Rotate flag (1: on, 0: off)
- Delta A (>= 0)
- Delta B (>= 0)
- Minimum number of options (>= 0)
- Verbose mode (1: on, 0: off)

When Rotate flag is on, the solver will consider tiles MxN and NxM identical. When it is off, they will be considered distinct.

The program will try to solve the puzzle searching from Delta A to Delta B.

If Delta A < Delta B, search is called for each Current in range \[ Delta A, Delta B \] until a solution is found where Delta = Current. This is the preferred option when searching for an optimal solution.

If Delta A >= Delta B, search is called starting with Current = Delta A. When a solution is found where Delta <= Current, then Delta-1 becomes the new Current. The search terminates when Current < Delta B. This is the preferred option when searching for an approximate solution (large orders).

Minimum number of options T greater than 0 can be specified to check only the sets containing at least T tiles.

The bash script mondrian_order_range.sh calls the solver for every order in the range specified.

A text file mondrian_achievements.txt contains optimal defects, lower bounds and upper bounds with corresponding tilings that were not yet discovered.

The mondrian_view program converts the output of the solver to the resulting square display. It expects the following parameters on standard input:
- Optimize flag (1: on, 0: off)
- Square order (>= 3)
- Number of tiles (2 <= T <= 52)
- Solution output (T lines)

When Optimize flag is on, the program will use the least number of symbols possible, with the constraint that tiles sharing one edge or vertex cannot use the same symbol. When it is off, one symbol per tile will be used.
