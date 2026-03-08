# Custom malloc implementation

## TODO
- Implement splitting
- Implement realloc
- Write tests

## Takeaways
- free never "frees" memory, it stays mapped but becomes marked as reusable
- The metadata is in the heap itself. A pointer to the head and clever pointer arithmetics are enough to manage all the blocks.
- Adding a footer that is the same as the header at the bottom of each "block". This trick was discovered by Donald Knuth and allows us to traverse backwards.
- Pointer arithmetic is hard... lots of type casting and thinking about what an increment really is (+1 for a char* is different to a +1 for a uint32\_t*)
