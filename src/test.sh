#!/bin/bash

# Redirige standard input ed error sul file
exec &> testout.log

obj_pid=$(pidof objectstore)

# Fa partire i 50 clients e poi aspetta la loro terminazione
for ((i = 0; i < 50; i++)); do
    ./client user$i 1 &
done
wait

# Fa partire un'altra volta i 50 clients per i test di tipo 2 e 3 e poi attende la loro terminazione
for ((i = 0; i < 30; i++)); do
    ./client user$i 2 &
done
for ((i = 30; i < 50; i++)); do
    ./client user$i 3 &
done
wait