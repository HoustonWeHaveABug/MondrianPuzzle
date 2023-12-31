# MondrianPuzzle

Solution to challenge https://www.reddit.com/r/dailyprogrammer/comments/9dv08q/20180907_challenge_367_hard_the_mondrian_puzzle/.

The mondrian program is the solver and expects the following parameters on standard input:
- Paint height (>= 1)
- Paint width (>= 3 and >= Paint height)
- Rotate flag (1: on, 0: off)
- Defect A (>= 0)
- Defect B (>= 0)
- Minimum number of options (>= 2)
- Verbose mode (1: on, 0: off)

When Rotate flag is on, the solver will consider tiles MxN and NxM identical. When it is off, they will be considered distinct.

The program will try to solve the puzzle searching from Defect A to Defect B.

If Defect A <= Defect B, the search is called for each Current in range \[ Defect A, Defect B \] until a solution is found where Defect = Current. This is the preferred option when searching for an optimal solution.

If Defect A > Defect B, the search is called starting with Current = Defect A. When a solution is found where Defect <= Current, then Defect-1 becomes the new Current. The search terminates when Current < Defect B. This is the preferred option when searching for an approximate solution (large paint areas).

Minimum number of options T greater than 2 can be specified to check only the sets containing at least T tiles.

The bash script mondrian_order_range.sh calls the solver for every order in the range specified.

The bash script mondrian_edpeggjr_ub.sh calls the solver from the upper bound for defect defined by Ed Pegg Jr. (n/log(n)+3 if Rotate flag is on, n/log(n) otherwise) to 0.

A text file mondrian_achievements.txt contains optimal defects, lower bounds and upper bounds with the corresponding solution found by this solver.

A text file mondrian_view_optimals.txt contains the optimal tilings found by this solver.

The mondrian_view program converts the output of the solver to the corresponding tiling. It expects the following parameters on standard input:
- Minimize flag (1: on, 0: off)
- Paint height (>= 1)
- Paint width (>= 3 and >= Paint height)
- Number of tiles (2 <= T <= 52)
- Solution output (T lines)

When Minimize flag is on, the program will use the least number of symbols possible, given that tiles sharing one edge or vertex cannot use the same symbol. When it is off, one symbol per tile will be used.
