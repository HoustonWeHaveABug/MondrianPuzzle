# MondrianPuzzle

Solution to challenge https://www.reddit.com/r/dailyprogrammer/comments/9dv08q/20180907_challenge_367_hard_the_mondrian_puzzle/.

Parameters read on standard input of the mondrian program:
- Square order (>= 3)
- Delta A (>= 0)
- Delta B (>= 0)
- Verbose mode (1: on, 0: off)

The program will try to solve the puzzle searching from Delta A to Delta B.

If Delta A < Delta B, search is called for each Current in range \[ Delta A + 1, Delta B + 1 \] until a solution is found with Delta < Current. This is the preferred option when searching for an optimal solution.

If Delta A >= Delta B, search is called starting with Current = Delta A + 1. When a solution is found with Delta < Current, then Delta becomes the new Current. The search terminates when Current <= Delta B. This is the preferred option when searching for an approximate solution.
