#!/bin/bash

function print_report () {
    # Numero totale di test per la batteria
    total=$(grep -c "Test $1" testout.log)
    # Numero di test passati
    let passed=$(grep -c "Test $1: Success" testout.log)
    # Numero di test falliti
    let failed=$total-$passed
    # Percentuale di test passati e falliti
    let passed_perc=($passed/$total)*100
    let failed_perc=($failed/$total)*100
    # Stampa il sommario delle informazioni
    echo "Test $1: Lanciati $total, Passati $passed ($passed_perc%), Falliti $failed ($failed_perc%)"
}

# Numero totale di test
total=$(cat testout.log | wc -l)

# Stampa il report per ogni batteria
echo "Test lanciati: $total"
for ((i = 1; i <= 3; i++)); do
    print_report $i
done

# Manda un segnale di diagnostica al server
kill -USR1 $(pidof objectstore)
# Termina il server
kill -TERM $(pidof objectstore)