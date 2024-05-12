### TODO

### Now

- Float equality / not equal operations
- More float tests

### Later
- [ ] Compile time -- real	0m33.827s
    - but this is linking with google test too.
- [ ] Handle variables outside of functions
- [ ] Implement multiplication
- [ ] Implement division
- [ ] Implement <=
- [ ] Implement >=
- [ ] Implement prefix / postfix increment / decrement
- [ ] Implement char
- [ ] Handle return types of functions that aren't int
- [ ] Implement VLA's
- [ ] Implement structs
- [ ] Handle expressions in the first part of a for loop
    - Looks like `for(a; a < 5; a = a + 1)`
- [ ] Add a direct IR instruction for adding to a stack location
- [ ] Add a direct IR instruction for subtracting from a stack location
- [ ] Refactor data type for AST data types
- [ ] Fix `result = result + 1` assembly generation
    - Currently is load, add, store
    - This happens because all bin ops 'create' their destination in `assem.cpp`. Would either need to change there
        or add a pass to rewrite temp destinations with their actual destinations.
- [ ] Handle single void parameter `int main(void) {}`
