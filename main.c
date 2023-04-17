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

void coda_add(struct coda_t *c, int persona){
    sem_wait(&c->mutex);
    c->coda[c->size]=p;
    c->size++;
    sem_post(&c->mutex);
    return;
}

int coda_rm(struct coda_t *c){
    sem_wait(&c->mutex);
    int p=c->coda[0];
    for (int i=0; i<c->size; i++){
        c->coda[i]=c->coda[i+1];
    }
    c->size--;
    sem_signal(&c->mutex);
    return p;
}

struct attrezzo_t {
    stato_t stato[M];
    int persona[M]; // Chi ha l'attrezzo
    coda_t coda; // Chi sta aspettando l'attrezzo
};

struct palestra_t {
    attrezzo_t attrezzi[N];
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
        sem_init(&s->attrezzi[i]->coda->mutex, 0, 1);
        sem_init(&s->mutex[i], 0, 1);
    }
    for (int i=0; i<P; i++){
        sem_init(&s->persone[i], 0, 0);
    }
}

void usaatrrezzo(struct palestra_t *p,
        int numeropersona, int tipoattrezzo){
    sem_wait(&p->mutex[tipoattrezzo]);
    for (int i=0; i<M; i++){
        if (p->attrezzi[tipoattrezzo]->persona[i]==numeropersona
        && p->attrezzi[tipoattrezzo]->stato[i] == BOOKED){
            p->attrezzi[tipoattrezzo]->stato[i] = BUSY;
            sem_post(&p->mutex[tipoattrezzo]);
            return;
        }
    } // Teoricamente si potrebbe fare un unico ciclo...
    for (int i=0; i<M; i++){
        if (p->attrezzi[tipoattrezzo]->stato[i] == FREE){
            p->attrezzi[tipoattrezzo]->stato[i] = BUSY;
            p->attrezzi[tipoattrezzo]->persona[i] = numeropersona;
            sem_post(&p->mutex[tipoattrezzo]);
            return;
        }
    }
    // Non ci sono attrezzi liberi
    coda_add(&p->attrezzi[tipoattrezzo]->coda, numeropersona);
    sem_wait(&p->persone[numeropersona]);
    // Ora c'è un attrezzo, ripetiamo la stessa ricerca
    for (int i=0; i<M; i++){
        if (p->attrezzi[tipoattrezzo]->stato[i]==FREE){
            p->attrezzi[tipoattrezzo]->persona[i]=numeropersona;
            p->attrezzi[tipoattrezzo]->stato[i]=BUSY;
            sem_post(&p->mutex[tipoattrezzo]);
            return;
        }
    }
}

void prenota(struct palestra_t *p, int numeropersona,
        int tipoattrezzo){
    sem_wait(&p->mutex[tipoattrezzo]);
    for (int i=0; i<M; i++){
        if (p->attrezzi[tipoattrezzo]->stato[i]==FREE){
            p->attrezzi[tipoattrezzo]->stato[i]=BOOKED;
            p->attrezzi[tipoattrezzo]->persona[i]=numeropersona;
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
        if (p->attrezzi[tipoattrezzo]->persona[i]==numeropersona
        && p->attrezzi[tipoattrezzo]->stato[i] == BUSY){
            p->attrezzi[tipoattrezzo]->persona[i]=-1;
            p->attrezzi[tipoattrezzo]->stato[i]=FREE;
            break;
        }
    }
    if (coda_isempty(p->attrezzi[tipoattrezzo]->coda)){
        sem_post(&p->mutex[tipoattrezzo]);
    } else {
        sem_post(&p->persone[coda_rm(p->attrezzi[tipoattrezzo]->coda)]);
    }
}