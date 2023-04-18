#define N 1
#define M 1
#define P 3
#define E 2147483647
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

void pausetta(void){
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%100);
    nanosleep(&t, NULL);
}

typedef enum { FREE, BOOKED, BUSY} stato_t;
struct coda_t{
    sem_t mutex;
    int size;
    int coda[P];
};

int coda_isempty(struct coda_t *c){
    pausetta();
    int i;
    pausetta();
    sem_wait(&c->mutex);
    sem_getvalue(&c->mutex, &i);
    assert(i==0);
    pausetta();
    assert(c->size>=0);
    i=c->size==0;
    pausetta();
    sem_post(&c->mutex);
    pausetta();
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
    pausetta();
    sem_wait(&c->mutex);
    int i;
    sem_getvalue(&c->mutex, &i);
    assert(i==0);
    pausetta();
    c->coda[c->size]=persona;
    pausetta();
    c->size++;
    pausetta();
    sem_post(&c->mutex);
    pausetta();
}

int coda_rm(struct coda_t *c){
    pausetta();
    sem_wait(&c->mutex);
    int a;
    sem_getvalue(&c->mutex, &a);
    assert(a==0);
    pausetta();
    assert(c->size>0);
    int p=c->coda[0];
    pausetta();
    for (int i=0; i<c->size; i++){
        pausetta();
        c->coda[i]=c->coda[i+1];
    }
    pausetta();
    c->size--;
    pausetta();
    sem_post(&c->mutex);
    pausetta();
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
    pausetta();
    sem_wait(&p->mutex[tipoattrezzo]);
    int a;
    sem_getvalue(&p->mutex[tipoattrezzo], &a);
    assert(a==0);
    pausetta();
    for (int i=0; i<M; i++){
        pausetta();
        if (p->attrezzi[tipoattrezzo].persona[i]==numeropersona
        && p->attrezzi[tipoattrezzo].stato[i] == BOOKED){
            pausetta();
            p->attrezzi[tipoattrezzo].stato[i] = BUSY;
            pausetta();
            sem_post(&p->mutex[tipoattrezzo]);
            pausetta();
            return;
        }
    } // Teoricamente si potrebbe fare un unico ciclo...
    pausetta();
    for (int i=0; i<M; i++){
        pausetta();
        if (p->attrezzi[tipoattrezzo].stato[i] == FREE){
            pausetta();
            p->attrezzi[tipoattrezzo].stato[i] = BUSY;
            pausetta();
            p->attrezzi[tipoattrezzo].persona[i] = numeropersona;
            pausetta();
            sem_post(&p->mutex[tipoattrezzo]);
            pausetta();
            return;
        }
    }
    pausetta();
    // Non ci sono attrezzi liberi
    coda_add(&p->attrezzi[tipoattrezzo].coda, numeropersona);
    pausetta();
    // ERRORE 1: "Dimenticato" (era presente in brutta copia!! :-( ) post del mutex!
    sem_post(&p->mutex[tipoattrezzo]);
    pausetta();
    sem_wait(&p->persone[numeropersona]);
    sem_getvalue(&p->persone[numeropersona], &a);
    assert(a==0);
    pausetta();
    // Ora c'è un attrezzo, ripetiamo la stessa ricerca
    for (int i=0; i<M; i++){
        pausetta();
        if (p->attrezzi[tipoattrezzo].stato[i]==FREE){
            pausetta();
            p->attrezzi[tipoattrezzo].persona[i]=numeropersona;
            pausetta();
            p->attrezzi[tipoattrezzo].stato[i]=BUSY;
            pausetta();
            sem_post(&p->mutex[tipoattrezzo]);
            pausetta();
            return;
        }
    }
}

void prenota(struct palestra_t *p, int numeropersona,
        int tipoattrezzo){
    pausetta();
    sem_wait(&p->mutex[tipoattrezzo]);
    int a;
    sem_getvalue(&p->mutex[tipoattrezzo], &a);
    assert(a==0);
    pausetta();
    for (int i=0; i<M; i++){
        pausetta();
        if (p->attrezzi[tipoattrezzo].stato[i]==FREE){
            pausetta();
            p->attrezzi[tipoattrezzo].stato[i]=BOOKED;
            pausetta();
            p->attrezzi[tipoattrezzo].persona[i]=numeropersona;
            pausetta();
            sem_post(&p->mutex[tipoattrezzo]);
            pausetta();
            return;
        }
    }
    pausetta();
    sem_post(&p->mutex[tipoattrezzo]);
}

void fineuso(struct palestra_t *p, int numeropersona, int
        tipoattrezzo){
    pausetta();
    sem_wait(&p->mutex[tipoattrezzo]);
    int a;
    sem_getvalue(&p->mutex[tipoattrezzo], &a);
    assert(a==0);
    pausetta();
    for (int i=0;i<M;i++){
        pausetta();
        if (p->attrezzi[tipoattrezzo].persona[i]==numeropersona
        && p->attrezzi[tipoattrezzo].stato[i] == BUSY){
            pausetta();
            p->attrezzi[tipoattrezzo].persona[i]=-1;
            pausetta();
            p->attrezzi[tipoattrezzo].stato[i]=FREE;
            pausetta();
            break;
        }
    }
    pausetta();
    if (coda_isempty(&p->attrezzi[tipoattrezzo].coda)){
        pausetta();
        sem_post(&p->mutex[tipoattrezzo]);
    } else {
        pausetta();
        sem_post(&p->persone[coda_rm(&p->attrezzi[tipoattrezzo].coda)]);
    }
    pausetta();
}

void *persona(void *arg)
{
    int i;
    int numeropersona = (int)arg;
    int attrezzocorrente = rand()%N;
    int prossimoattrezzo = rand()%N;
    for (i=E-1; i>=0; i--)
    {
        //printf("%d usa %d\n", numeropersona, attrezzocorrente);
        pausetta();
        usaattrezzo(&palestra, numeropersona, attrezzocorrente);
        pausetta();
        //printf("%d ha usato %d\n", numeropersona, attrezzocorrente);
        //printf("%d prenota %d\n", numeropersona, prossimoattrezzo);
        pausetta();
        if (i!=0) prenota(&palestra, numeropersona, prossimoattrezzo);
        pausetta();
        //printf("%d ha prenotato %d\n", numeropersona, prossimoattrezzo);
        //printf("%d finisce %d\n", numeropersona, attrezzocorrente);
        pausetta();
        fineuso(&palestra, numeropersona, attrezzocorrente);
        pausetta();
        //printf("%d ha finito %d\n", numeropersona, attrezzocorrente);
        if (i!=0) {
            attrezzocorrente = prossimoattrezzo;
            prossimoattrezzo = rand()%N;
            if (i%50 == 0) {
                printf(".");
            }
        }
    }
    printf("!");
    return NULL;
}

int main()
{
    srand(time(NULL));
    setvbuf(stdout, NULL, _IONBF, 0);
    int i;
    pthread_t thread[P];
    init_palestra(&palestra);
    for (i=0; i<P; i++)
    {
        pthread_create(&thread[i], 0, persona, (void *)i);
    }
    for (i=0; i<P; i++)
    {
        pthread_join(thread[i], NULL);
    }
    // Assert
    for (i=0; i<N; i++)
    {
        assert(coda_isempty(&palestra.attrezzi[i].coda));
        int a;
        sem_getvalue(&palestra.mutex[i], &a);
        assert(a==1);
        sem_getvalue(&palestra.attrezzi[i].coda.mutex, &a);
        assert(a==1);
        for (int j=0; j<M; j++)
        {
            assert(palestra.attrezzi[i].stato[j]==FREE);
            assert(palestra.attrezzi[i].persona[j]==-1);
        }
    }
    for (i=0; i<P; i++)
    {
        int a;
        sem_getvalue(&palestra.persone[i], &a);
        assert(a==0);
    }
    printf("\n");
    return 0;
}