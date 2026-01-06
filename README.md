# MondrianPuzzle

Solution to challenge https://www.reddit.com/r/dailyprogrammer/comments/9dv08q/20180907_challenge_367_hard_the_mondrian_puzzle/.

The mondrian program is the solver and expects the following parameters on standard input:
- Request (1: squares, 2: rectangles, other: unique)
- Order low (>= 1)
- Order high (>= Order low)
- Rotate flag (1: on, 0: off)
- Defect A (>= 0)
- Defect B (>= 0)
- Options low (>= 2)
- Options high (>= Options low)
- Verbose mode (1: on, 0: off)

When Request is squares, the program will iterate on every squares in range \[ \<Order low\>x\<Order low\>, \<Order high\>x\<Order high\> \].

When Request is rectangles, the program will iterate on every rectangles in range \[ 1x\<Order low\>, \<Order high\>x\<Order high\> \].

Otherwise, the program will solve the \<Order low\>x\<Order high\> rectangle.

When Rotate flag is on, the program will consider tiles MxN and NxM identical. When it is off, they will be considered distinct.

The program will try to solve the problem searching from Defect A to Defect B.

If Defect A <= Defect B, the search is called for each Current in range \[ Defect A, Defect B \] until a solution is found where Defect = Current. This is the preferred option when searching for an optimal solution.

If Defect A > Defect B, the search is called starting with Current = Defect A. When a solution is found where Defect <= Current, then Defect-1 becomes the new Current. The search terminates when Current < Defect B. This is the preferred option when searching for an approximate solution (large paint areas).

Options low greater than or equal to 2 can be specified to check only the sets containing at least that number of tiles.
Options high greater than or equal to Options low can be specified to check only the sets containing at most that number of tiles.

The bash script mondrian_squares.sh calls the solver for every squares in the order range specified.

The bash script mondrian_rectangles.sh calls the solver for every rectangles in the width range specified.

The bash script mondrian_edpeggjr_ub.sh calls the solver from the upper bound for defect defined by Ed Pegg Jr. (n/log(n)+3 if Rotate flag is on, n/log(n) otherwise) to 0.

A text file mondrian_achievements.txt contains optimal defects, lower bounds and upper bounds with the corresponding solution found by this solver.

The text files mondrian_view_A276523.txt and mondrian_view_A279596.txt contain the optimal tilings found by this solver and related to the respective OEIS sequences.

The mondrian_view program converts the output of the solver to the corresponding tiling. It expects the following parameters on standard input:
- Minimize flag (1: on, 0: off)
- Paint height (>= 1)
- Paint width (>= 3 and >= Paint height)
- Number of tiles (2 <= T <= 52)
- Solution output (T lines)

When Minimize flag is on, the program will use the least number of symbols possible, given that tiles sharing one edge or vertex cannot use the same symbol. When it is off, one symbol per tile will be used.
