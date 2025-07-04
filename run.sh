#/bin/bash

if [[ "$1" == "" ]]; then
    echo give example no. to run as argument...
    echo run.sh 1
    exit 1
fi
src="./examples/example$1.cpp"
if [[ -f "$src" ]]; then
    g++ "$src" -o main
    ./main
    rm main
    exit 0
fi

echo bruh ts example doesnt exist
