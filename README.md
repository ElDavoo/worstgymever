# Parziale Real Time 2023  
Esame di RealTime Embedded Systems, insegnamento di UniMORE, anno 22-23.  

# Traccia
Esame Real-Time Embedded Systems  
La palestra - 14/04/2023  

Una palestra possiede N tipologie di attrezzi. Ciascuna tipologia di attrezzo è presente in M copie identiche
(ex. M bilancieri, M cyclettes, ... ). La palestra contiene P persone che vogliono eseguire E esercizi usando gli
attrezzi in un ordine qualsiasi. Ciascuna persona è modellata con un thread.
Quando una persona usa un attrezzo prenota l'uso del successivo per non dover attendere una volta finito l'esercizio corrente.
La prenotazione può essere effettuata quando uno degli attrezzi del tipo richiesto è libero (ovvero non in uso
e non prenotato). La prenotazione è "per tipo" di attrezzo, tutti gli attrezzi dello stesso tipo sono equivalenti.
Se non ci sono attrezzi di quel tipo liberi la prenotazione non viene effettuata. Se presente, la prenotazione
viene rimossa quando la persona utilizza il tipo di attrezzo prenotato.
La funzione `usaattrezzo` non è bloccante se l'attrezzo è stato prenotato o se c'e' un attrezzo di quel tipo
libero.
Altrimenti, l'accesso alle macchine è in ordine di arrivo.
```c
struct palestra_t { ... } palestra;
void init_palestra(struct palestra_t *s);
void usaattrezzo(struct palestra_t *p, int numeropersona, int tipoattrezzo);
void prenota(struct palestra_t *p, int numeropersona, int tipoattrezzo);
void fineuso(struct palestra_t *p, int numeropersona, int tipoattrezzo);
void *persona(void *arg)
{ ...
attrezzocorrente = rand()%N;
prossimoattrezzo = rand()%N;
for (i=E; i>0; i--)
    {
    usaattrezzo(&palestra, numeropersona, attrezzocorrente);
    if (i!=0) prenota(&palestra, numeropersona, prossimoattrezzo);
    fineuso(&palestra, numeropersona, attrezzocorrente)
    if (i!=0) {
        attrezzocorrente = prossimoattrezzo;
        prossimoattrezzo = rand()%N;
                }
    }
}
```
Modellare il sistema in linguaggio C con la libreria pthread nel seguente modo utilizzando:  
[ ] semafori [ ] mutex + variabili condition
Scrivere il contenuto della struct `palestra` e le funzioni `init_palestra`, `usaattrezzo`, `prenota`,
`fineuso`.