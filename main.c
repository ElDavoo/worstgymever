#define N 1
#define M 1
#define P 3
#define E 2147483647
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum { FREE, BOOKED, BUSY} stato_t;
struct coda_t{
    sem_t mutex;
    int size;
    int coda[P];
};

int coda_isempty(struct coda_t *c){
    int i;
    sem_wait(&c->mutex);
    i=c->size==0;
    sem_post(&c->mutex);
    return i;
}

void coda_dump(struct coda_t *c){
    printf("%d: ", c->size);
    for (int i=0; i<c->size; i++){
        printf("%d ", c->coda[i]);
    }
    printf("\n");
}

void coda_add(struct coda_t *c, int persona){
    sem_wait(&c->mutex);
    c->coda[c->size]=persona;
    c->size++;
    sem_post(&c->mutex);
}

int coda_rm(struct coda_t *c){
    sem_wait(&c->mutex);
    int p=c->coda[0];
    for (int i=0; i<c->size; i++){
        c->coda[i]=c->coda[i+1];
    }
    c->size--;
    sem_post(&c->mutex);
    return p;
}

struct attrezzo_t {
    stato_t stato[M];
    int persona[M]; // Chi ha l'attrezzo
    struct coda_t coda; // Chi sta aspettando l'attrezzo
};

struct palestra_t {
    struct attrezzo_t attrezzi[N];
    sem_t mutex[N]; // Per ogni categoria di attrezzi
    sem_t persone[P]; // Semafori privati x aspettare
} palestra;

void init_palestra(struct palestra_t *s){
    for (int i=0; i<N; i++){
        for (int j=0; j<M; j++){
            s->attrezzi[i].stato[j]=FREE;
            s->attrezzi[i].persona[j]=-1;
        }
        s->attrezzi[i].coda.size=0;
        sem_init(&s->attrezzi[i].coda.mutex, 0, 1);
        sem_init(&s->mutex[i], 0, 1);
    }
    for (int i=0; i<P; i++){
        sem_init(&s->persone[i], 0, 0);
    }
}

void usaattrezzo(struct palestra_t *p,
        int numeropersona, int tipoattrezzo){
    sem_wait(&p->mutex[tipoattrezzo]);
    for (int i=0; i<M; i++){
        if (p->attrezzi[tipoattrezzo].persona[i]==numeropersona
        && p->attrezzi[tipoattrezzo].stato[i] == BOOKED){
            p->attrezzi[tipoattrezzo].stato[i] = BUSY;
            sem_post(&p->mutex[tipoattrezzo]);
            return;
        }
    } // Teoricamente si potrebbe fare un unico ciclo...
    for (int i=0; i<M; i++){
        if (p->attrezzi[tipoattrezzo].stato[i] == FREE){
            p->attrezzi[tipoattrezzo].stato[i] = BUSY;
            p->attrezzi[tipoattrezzo].persona[i] = numeropersona;
            sem_post(&p->mutex[tipoattrezzo]);
            return;
        }
    }
    // Non ci sono attrezzi liberi
    coda_add(&p->attrezzi[tipoattrezzo].coda, numeropersona);
    sem_wait(&p->persone[numeropersona]);
    // Ora c'è un attrezzo, ripetiamo la stessa ricerca
    for (int i=0; i<M; i++){
        if (p->attrezzi[tipoattrezzo].stato[i]==FREE){
            p->attrezzi[tipoattrezzo].persona[i]=numeropersona;
            p->attrezzi[tipoattrezzo].stato[i]=BUSY;
            sem_post(&p->mutex[tipoattrezzo]);
            return;
        }
    }
}

void prenota(struct palestra_t *p, int numeropersona,
        int tipoattrezzo){
    sem_wait(&p->mutex[tipoattrezzo]);
    for (int i=0; i<M; i++){
        if (p->attrezzi[tipoattrezzo].stato[i]==FREE){
            p->attrezzi[tipoattrezzo].stato[i]=BOOKED;
            p->attrezzi[tipoattrezzo].persona[i]=numeropersona;
            sem_post(&p->mutex[tipoattrezzo]);
            return;
        }
    }
    sem_post(&p->mutex[tipoattrezzo]);
}

void fineuso(struct palestra_t *p, int numeropersona, int
        tipoattrezzo){
    sem_wait(&p->mutex[tipoattrezzo]);
    for (int i=0;i<M;i++){
        if (p->attrezzi[tipoattrezzo].persona[i]==numeropersona
        && p->attrezzi[tipoattrezzo].stato[i] == BUSY){
            p->attrezzi[tipoattrezzo].persona[i]=-1;
            p->attrezzi[tipoattrezzo].stato[i]=FREE;
            break;
        }
    }
    if (coda_isempty(&p->attrezzi[tipoattrezzo].coda)){
        sem_post(&p->mutex[tipoattrezzo]);
    } else {
        sem_post(&p->persone[coda_rm(&p->attrezzi[tipoattrezzo].coda)]);
    }
}

void *persona(void *arg)
{
    int i;
    int numeropersona = (int)arg;
    int attrezzocorrente = rand()%N;
    int prossimoattrezzo = rand()%N;
    for (i=E-1; i>=0; i--)
    {
        printf("%d usa %d\n", numeropersona, attrezzocorrente);
        usaattrezzo(&palestra, numeropersona, attrezzocorrente);
        printf("%d ha usato %d\n", numeropersona, attrezzocorrente);
        printf("%d prenota %d\n", numeropersona, prossimoattrezzo);
        if (i!=0) prenota(&palestra, numeropersona, prossimoattrezzo);
        printf("%d ha prenotato %d\n", numeropersona, prossimoattrezzo);
        printf("%d finisce %d\n", numeropersona, attrezzocorrente);
        fineuso(&palestra, numeropersona, attrezzocorrente);
        printf("%d ha finito %d\n", numeropersona, attrezzocorrente);
        if (i!=0) {
            attrezzocorrente = prossimoattrezzo;
            prossimoattrezzo = rand()%N;
        }
    }
    printf("%d ha finito di usare la palestra!", numeropersona);
    return NULL;
}

int main()
{
    srand(123);
    setvbuf(stdout, NULL, _IONBF, 0);
    pthread_attr_t a;
    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);
    int i;
    pthread_t thread[P];
    init_palestra(&palestra);
    for (i=0; i<P; i++)
    {
        pthread_create(&thread[i], 0, persona, (void *)i);
    }
    pthread_join(thread[0], 0);
    return 0;
}