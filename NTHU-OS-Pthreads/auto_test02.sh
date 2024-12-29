#!/bin/bash
echo "Running auto_gen_transformer..."
./scripts/auto_gen_transformer --input ./tests/02_spec.json --output transformer.cpp
echo "Running make clean..."
make clean
echo "Running make with devtoolset-8..."
scl enable devtoolset-8 'make'
echo "Running main with parameters 30000 ./tests/02.in ./tests/02.out..."
time ./main 30000 ./tests/02.in ./tests/02.out
echo "All steps completed."