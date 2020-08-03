# MondrianPuzzle

Solution to challenge https://www.reddit.com/r/dailyprogrammer/comments/9dv08q/20180907_challenge_367_hard_the_mondrian_puzzle/.

Parameters read on standard input of the mondrian program:
- Square order (>= 3)
- Minimum difference (>= 0)
- Maximum difference (>= Minimum difference)
- Verbose mode (1: on, 0: off)

The program will try to solve the puzzle from minimum difference to maximum difference incrementally until a solution is found.

The shell script mondrian_desc.sh encapsulates the solver to run a search by descending order.
