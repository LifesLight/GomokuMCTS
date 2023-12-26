# Monte Carlo based Gomoku algorithm

This code is focused on pure speed, its remarkably not memory efficient.<br>
Implemented Transposition-Table on a "per state" value tracking, not per node.<br>
This allows for nodes to share data if they have the same state.<br>

## TODO:

- Multithreading
- Caching of previous simulations
- Custom memory allocation? -> faster freeing?