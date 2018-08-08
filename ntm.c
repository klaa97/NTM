/* Turing , ND
- Nastro singolo, accettori
- Nastro: char
- Stati: int
- "_" -> blank
- stdin
- controllare /r/n o /n
*/

/* Costanti moltiplicative */
#define STATES_S 10
#define STATES_E 2
#define HASH_MOD 256
#define S_FINAL 30
#define STR_IN 20
#define STR_RIGHT 64
#define STR_LEFT 64
#define STR_EXP 3

//libc libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//Se legge eof, NULL!
char prova = 'a';
char *hofinito = &prova;

//Variabile per sapere se ho già letto fino alla fine della stringa
bool isfinished=0;
//Variabile per sapere se qualcosa non terminerà mai
bool computing = false;
/* Struttura per transizioni
- 256 puntatori ad array, 0
- Ogni puntatore punta ad un array di puntatori di STATES_S elementi, espando se necessario di STATES_E
- Ogni puntatore di STATES_S elementi punta a una struct con lo stato di partenza, che a sua volta punta a cosa scrivere, R/S/L, stato  
- Tabella hash, 256, funzione hash: KNUTH modulo
*/

typedef struct dotrs{
    struct dotrs *next;
    int cwrite;
    char movement; 
    int states;
}dotrs;


typedef struct trs{
    struct trs *next;
    int startstate;
    dotrs *archi;
} trs;


#include <string.h>
trs **ingresso[127];

//Stati finali
int* final;
int nfinal;

//Puntatore a buffer di ingresso
char *buffer;
//Chunk letto
int lunghezzabuffer=0;


//Knuth method
int hash(int state){
    return (state*(state+3))%HASH_MOD;
}

long long int max;

/*Strutture per grafo di transizione:
indice - lista di stringhe che si espande a destra, grandezza iniziale: STR_IN
espansione a sinistra / destra: STR_EX*/
typedef struct str{
    char *text;
    int shared;
} str;

typedef struct str1{
    char *text;
    int shared;
    long lunghezza;
} str1;

typedef struct conf{
    str **sprev;
    short unsigned int lungsprev;
    str **snext;
    short unsigned int lungsnext;
    struct conf *next;
    int state;
    long current; // Puntatore alla posizone, negativo: prev | positivo: next
}conf;

conf *config = 0; //Mantiene la testa della lista vecchia
conf *newconfig = 0; //Mantiene testa lista nuova
conf *todelete = 0;

int elements = 0; // Nuovi
int elementstodelete = 0; // Inutili
int oldelements = 1; //Vecchi

//Copio conf da prev a dest
void memconf(conf* dest, conf* prev);

//Copio str da prev a dest
void memstr(str *dest, str *prev);

//Inserisco nella hash table una nuova transizione
void insert(int start, int h, int csub, char tr, int sd, int sstart);

//Leggo le transizioni per inserirle
void readtransaction();

//Leggo gli stati finali
void readfinal();

//Leggo il valore massimo (LONG)
void readmax();

//Inizializzo la prima configurazione
void startconfig();

//Computo una not-last transizione
void computeconfignotlast(dotrs *arco, conf *stato, int letto, int i, int where);

//Faccio il reset di tutte le conf a partire da cnf
void reset(conf *cnf);

//Computo una last transizione
void computeconfiglast(dotrs *arco, conf *stato, int letto, int i, int where);

//Cerco le transizioni e computo
bool searchandqueueandcompute(conf *cnf, conf **valore);

//Computo - loop principale
void compute();

//Rimappo gli stati finali a -1
void checkfinals();

int main() {
    //int *final;
    readtransaction();
    readfinal();
    readmax();
    checkfinals();
    while (!feof(stdin) && hofinito!=0) {
        compute();
    }
} 

void memconf(conf *dest, conf* prev) {
    int i, bigger;
    str **npointer;
    str **ppointer;
    dest->current=prev->current;
    dest->snext=malloc(sizeof(str)*prev->lungsnext);
    dest->sprev=malloc(sizeof(str)*prev->lungsprev);
    memcpy(dest->snext, prev->snext, prev->lungsnext*sizeof(str));
    memcpy(dest->sprev, prev->sprev, prev->lungsprev*sizeof(str));
    dest->lungsnext = prev->lungsnext;
    dest->lungsprev = prev->lungsprev;
    dest->state=prev->state;
    npointer = dest->snext;
    ppointer = dest->sprev;
    bigger = (prev->lungsnext >= prev->lungsprev) ? 1 : 0;
    switch (bigger) {
                    case 1: {
                        for (i=0;i<prev->lungsprev;i++){
                            npointer[i]->shared++;
                            ppointer[i]->shared++;
                        }
                        for (; i < prev->lungsnext; i++){
                            npointer[i]->shared++;
                        }
                        break;
                    }
                    case 0: {
                        for (i=0;i<prev->lungsnext;i++){
                            npointer[i]->shared++;
                            ppointer[i]->shared++;
                        }
                        for (; i < prev->lungsprev; i++){
                            ppointer[i]->shared++;
                        }
                        break;
                    }
                }
}

void memstr(str *dest, str *prev) {
    dest->shared=prev->shared;
    dest->text=prev->text;
}
void memstr1(str1 *dest, str1 *prev) {
    dest->lunghezza=prev->lunghezza;
    dest->shared=prev->shared;
    dest->text=prev->text;
}

void insert(int start, int h, int csub, char tr, int sd, int sstart){
    trs *temp = ingresso[start][h];
    while (temp != 0){
        if (sstart == temp->startstate)
            goto found;
        temp = temp->next;
    }
    // Se non ho trovato, creo trs
    temp = malloc(sizeof(trs));
    temp->startstate = sstart;
    temp->next=ingresso[start][h];
    ingresso[start][h]=temp;
    temp->archi = 0;
    found:;
    dotrs *el;
    el = malloc(sizeof(dotrs));
    el->cwrite=csub;
    el->movement=tr;
    el->states = sd;
    el->next = temp->archi;
    temp->archi = el; 
   //printf("da scrivere %c\n movimento %c\n stato %d\n",el->cwrite, el->movement, el->states);
}

void readtransaction(){
    int i;
    for (i=0;i<127;i++){
        ingresso[i]=0;
    }
    int h;
    int sstart;
    int send;
    char cstart;
    char csubstitute;
    char transaction;
    scanf("tr ");
    while (scanf("%d",&sstart)==1) {
        scanf(" %c %c %c %d", &cstart, &csubstitute,&transaction,&send);
        if (ingresso[(int)cstart]==0){
            ingresso[(int)cstart]=calloc(HASH_MOD, sizeof(trs));
        }
        h=hash(sstart);
        insert(cstart,h,csubstitute,transaction,send, sstart);
    }
}

void readfinal(){
    final=malloc(sizeof(int)*S_FINAL);
    nfinal = S_FINAL;
    int n=0;
    int temp;
    scanf("acc");
    while (scanf("%d",&temp)==1){
        if (n>=S_FINAL) {
            nfinal = nfinal+S_FINAL;
            final = realloc(final,sizeof(int)*nfinal);
        }
        final[n]=temp;
        n++;
    }
    nfinal=n;
}

void readmax() {
    scanf("max");
    scanf("%lli", &max);
    scanf(" ");
    scanf("run ");
}

//Inizializzo la prima configurazione
void startconfig() {
    str **p;
    str *testo;
    int i = 0;
    //Leggo tutta la stringa in ingresso con %ms
    scanf("%ms ", &buffer);
    lunghezzabuffer = strlen(buffer);
    config = malloc(sizeof(conf));
    config->next=0;
    config->state=0;
    config->current=0;
    p = config->snext;
    p=malloc(sizeof(*p));
    config->lungsnext = 1;
    testo = malloc(sizeof(*testo));
    testo->shared = 1;
    testo->text = malloc(sizeof(char)*STR_EXP);
    if (lunghezzabuffer>=STR_EXP) {
        strncpy(testo->text,buffer,STR_EXP);
        //strncpy(testo->text,buffer,STR_EXP);
    } else {
        strncpy(testo->text,buffer,lunghezzabuffer);
        for (i=lunghezzabuffer;i<STR_EXP;i++)
            testo->text[i]='_';
    }
    config->snext = malloc(sizeof(str));
    config->snext[0] = testo;
    p = config->sprev;
    p=malloc(sizeof(*p));
    config->lungsprev = 1;
    testo = *p;
    testo = malloc(sizeof(*testo));
    testo->shared = 1;
    testo->text = malloc(sizeof(char)*STR_EXP);
    for (i=0;i<STR_EXP;i++)
        testo->text[i]='_';
    config->sprev = malloc(sizeof(str));
    config->sprev[0] = testo;

}


void freeconfig(conf *cnf) {
    //Riduco di 1 il contatore shared di tutte le stringhe in conf, libero le stringhe se è zero
    int i,bigger;
    str **npointer=cnf->snext;
    str **ppointer=cnf->sprev;
    bigger = (cnf->lungsnext >= cnf->lungsprev) ? 1 : 0;
    switch (bigger) {
                    case 1: {
                        for (i=0;i<cnf->lungsprev;i++){
                            npointer[i]->shared--;
                            ppointer[i]->shared--;
                            if (npointer[i]->shared==0)
                                free(npointer[i]);
                            if (ppointer[i]->shared==0)
                                free(ppointer[i]);
                        }
                        for (; i < cnf->lungsnext; i++){
                            npointer[i]->shared--;
                            if (npointer[i]==0)
                                free(npointer[i]);
                        }
                        break;
                    }
                    case 0: {
                        for (i=0;i<cnf->lungsnext;i++){
                            npointer[i]->shared--;
                            ppointer[i]->shared--;
                            if (npointer[i]->shared==0)
                                free(npointer[i]);
                            if (ppointer[i]==0)
                                free(ppointer[i]);
                        }
                        for (; i < cnf->lungsprev; i++){
                            ppointer[i]->shared--;
                            if (ppointer[i]->shared==0)
                                free(ppointer[i]);
                        }
                        break;
                    }
                }
    free(cnf->sprev);
    free(cnf->snext);
    free(cnf);
}


//Modifico la config a seconda della transizione, aggiungendo in testa le nuove
void computeconfignotlast(dotrs *arco, conf *stato, int letto, int i, int where){
    int j;
    elements++;
    char *control;
    int cursor, ltmp, tmp, nchunk, block, relativepos, bigger,startpoint;
    str *current = 0;
    str *new = 0;
    str **npointer;
    str **ppointer;
    //Creo una nuova config identica alla precedente
    conf *destinazione = malloc(sizeof(conf));
    memconf(destinazione,stato);
    //memmove(destinazione,stato,sizeof(conf));
    //funzione per fare un branch e creare una nuova stringa se necessario
    if ((int)arco->cwrite != letto) {
       switch (where) {
            case 1: {
                block = i/STR_EXP;
                relativepos = i%STR_EXP;
                current = stato->snext[block];
                current->shared--;
                new=malloc(sizeof(str));
                new->text=malloc(sizeof(char)*STR_EXP);
                strncpy(new->text,current->text, STR_EXP);
                new->text[relativepos]=arco->cwrite;
                new->shared=1;
                destinazione->snext[block]=new;
                break;
            }
            case 0: {
                block = (-i-1)/STR_EXP;
                relativepos = (-1-i)%STR_EXP;
                current = stato->sprev[block];
                current->shared--;
                new=malloc(sizeof(str));
                new->text=malloc(sizeof(char)*STR_EXP);
                strncpy(new->text,current->text, STR_EXP);
                new->text[relativepos]=arco->cwrite;
                new->shared=1;
                destinazione->sprev[block]=new;
                break;
                }
            break;
        }           
    }
    
    //Cambio stato
    destinazione->state=arco->states;
    //Cambio posizione
    switch(arco->movement) {
        case 'L': {
            destinazione->current--;
            cursor = destinazione->current;
            ltmp = STR_EXP * destinazione->lungsprev;
            if (where == 0 && cursor < -ltmp) {
                destinazione->lungsprev += 1;
                block = destinazione->lungsprev;
                destinazione->sprev = realloc(destinazione->sprev, sizeof(str)*block);
                destinazione->sprev[block-1] = malloc(sizeof(str));
                destinazione->sprev[block-1]->shared = 1;
                destinazione->sprev[block-1]->text = malloc(STR_EXP);
                control = destinazione->sprev[block-1]->text;
                for (i = 0; i < STR_EXP; i++) {
                    control[i]='_';
                }
            }                    
            break;
        }
        case 'R': {
            destinazione->current++;
            cursor = destinazione->current;
            ltmp = STR_EXP * destinazione->lungsnext;
            if (where == 1 && cursor == ltmp) {
                destinazione->lungsnext++;
                block = destinazione->lungsnext;
                destinazione->snext = realloc(destinazione->snext, sizeof(str)*block);
                if (ltmp>=lunghezzabuffer){
                    destinazione->snext[block-1] = malloc(sizeof(str));
                    destinazione->snext[block-1]->shared = 1;
                    destinazione->snext[block-1]->text = malloc(STR_EXP);
                    control = destinazione->snext[block-1]->text;
                    for (i = 0; i < STR_EXP; i++) {
                        control[i]='_';
                    }
                } else {
                    destinazione->snext[block-1] = malloc(sizeof(str));
                    destinazione->snext[block-1]->shared = 1;
                    destinazione->snext[block-1]->text = malloc(STR_EXP);
                    control = destinazione->snext[block-1]->text;
                    for (i = 0; i < STR_EXP && (ltmp+i) < lunghezzabuffer; i++) {
                        control[i]=buffer[ltmp+i];
                    }
                    for (; i < STR_EXP; i++) 
                        control[i] = '_';
                }
            }   
            break;
        }
    }
    //Aggiungo in testa alla lista delle config la nuova config creata
    destinazione->next=newconfig;
    newconfig=destinazione;  
}   
void reset(conf *cnf) {
    // reset delle config
    conf *temp = cnf;
    while (temp!=0) {
        //elements--;
        cnf = temp;
        temp = temp->next;
        freeconfig(cnf);
        
    }
}

void computeconfiglast(dotrs *arco, conf *stato, int letto, int i, int where) {
    oldelements--;
    elements++;
    str *current = 0;
    str * new = 0;
    int cursor, ltmp,j,tmp,nchunk;
    char *control;
    int block;
    int relativepos;
    //Modifico il carattere da modificare, se necessario;
    //Se shared=1, non è necessario fare un branch ma basta modificare
    if ((int)arco->cwrite != letto) {
        switch (where) {
            case 1: {
                block = i/STR_EXP;
                relativepos = i%STR_EXP;
                current = stato->snext[block];
                if (current->shared==1) {
                    current->text[relativepos]=arco->cwrite;
                } else {
                    current->shared--;
                    new=malloc(sizeof(str));
                    memstr(new,current);
                    new->text=malloc(sizeof(char)*STR_EXP);
                    strncpy(new->text,current->text, STR_EXP);
                    new->text[relativepos]=arco->cwrite;
                    new->shared=1;
                    stato->snext[block]=new;
                    /*if (destinazione->sprev!=0) {
                        destinazione->sprev->shared++;
                    }*/
                }                
                break;
                }
            case 0: {
                block = (-i-1)/STR_EXP;
                relativepos = (-i-1)%STR_EXP;
                current = stato->sprev[block];
                if (current->shared==1) {
                    current->text[relativepos]=arco->cwrite;
                } else {
                    current->shared--;
                    new=malloc(sizeof(str));
                    nchunk = (stato->current / STR_RIGHT +1)*STR_RIGHT;
                    memstr(new,current);
                    new->text=malloc(sizeof(char)*STR_EXP);
                    strncpy(new->text,current->text, STR_EXP);
                    new->text[relativepos]=arco->cwrite;
                    new->shared=1;
                    stato->sprev[block]=new;
                    /*if (destinazione->snext!=0) {
                        destinazione->snext->shared++;
                    }*/
                }
                break;       
            }
        }
    }
    stato->state=arco->states;
    //Cambio posizione
    switch (arco->movement) {
        case 'L': {
            stato->current--;
            cursor = stato->current;
            ltmp = STR_EXP * stato->lungsprev;
            if (where == 0 && cursor < -ltmp) {
                stato->lungsprev += 1;
                block = stato->lungsprev;
                stato->sprev = realloc(stato->sprev, sizeof(str)*block);
                stato->sprev[block-1] = malloc(sizeof(str));
                stato->sprev[block-1]->shared = 1;
                stato->sprev[block-1]->text = malloc(sizeof(char)*STR_EXP);
                control = stato->sprev[block-1]->text;
                for (i = 0; i < STR_EXP; i++) {
                    control[i]='_';
                }
            }                    
            break;
        }
        case 'R': {
            stato->current++;
            cursor = stato->current;
            ltmp = STR_EXP * stato->lungsnext;
            if (where == 1 && cursor == ltmp) {
                stato->lungsnext++;
                block = stato->lungsnext;
                stato->snext = realloc(stato->snext, sizeof(str)*block);
                if (ltmp>=lunghezzabuffer){
                    stato->snext[block-1] = malloc(sizeof(str));
                    stato->snext[block-1]->shared = 1;
                    stato->snext[block-1]->text = malloc(sizeof(char)*STR_EXP);
                    control = stato->snext[block-1]->text;
                    for (i = 0; i < STR_EXP; i++) {
                        control[i]='_';
                    }
                } else {
                    stato->snext[block-1] = malloc(sizeof(str));
                    stato->snext[block-1]->shared = 1;
                    stato->snext[block-1]->text = malloc(sizeof(char)*STR_EXP);
                    control = stato->snext[block-1]->text;
                    for (i = 0; i < STR_EXP && (ltmp+i) < lunghezzabuffer; i++) {
                        control[i]=buffer[ltmp+i];
                    }
                    for (; i < STR_EXP; i++) 
                        control[i] = '_';
                }
            }   
            break;
        }
    }
    //Aggiungo in testa 
    stato->next=newconfig;
    newconfig=stato;
}


bool searchandqueueandcompute(conf *cnf, conf **valore){
    // Aggiungo alla Queue le transizioni possibili per la transizione
    str1 *extrem=0;
    bool pozzo = true;
    int tmp;
    char control;
    trs *queue=0;
    int h = hash(cnf->state);
    int i = cnf->current;
    bool where;
    dotrs *dafare;
    if (i >= 0){
        where = 1;
        control = cnf->snext[i/STR_EXP]->text[i%STR_EXP];
    }else if (i < 0) {
        where = 0;
        control = cnf->sprev[(-i-1)/STR_EXP]->text[(-i-1)%STR_EXP];        
    }
    int letto = (int) control;
    trs *list = (ingresso[letto]!=0) ? ingresso[letto][h] : 0;
    while (list!=0) {
        if (list->startstate==cnf->state){
            dafare = list->archi;                
            pozzo = true;
            while (dafare!=0){
                /*if (dafare->states == cnf->state && dafare->movement == 'S' && letto == dafare->cwrite) {
                    computing = true;
                    goto daeliminare;
                }*/
                if (dafare->states == -1) {
                    printf("1\n");
                    return true;
                }
                else if (dafare->next!=0)
                    computeconfignotlast(dafare, cnf, letto, i, where);
                else computeconfiglast(dafare, cnf, letto, i, where);
                dafare = dafare->next;
            }
            goto computata;
        }
        list=list->next;
    }
    computata:;
    //Se non ci sono possibili transizioni, è pozzo (non accettata)
    if (pozzo == true) {
        daeliminare:;
        freeconfig(cnf);
    }

    return false;
}
/*Starto la configurazione, computo max-mosse
- se la config è null prima, termina 0 | se non è null dopo, è U */
void compute(){
    char *srt;
    int tmp, tmpelements;
    startconfig();
    if (hofinito==0)
        goto fine;
    long long int i = 0;
    conf *temp=config;
    conf *prec = 0;
    while (i < max) {
        while (temp!=0) {
            prec = temp;
            temp = temp->next;
            if (searchandqueueandcompute(prec,&prec)) {
                config = prec;
                goto reset1;
            }
        }
        if (elements==0 && computing==false) {
            printf("0\n");
            goto reset2;
        } else if (elements == 0 && computing ==true) 
            goto nonfiniromai;
        elements=0;
        config = newconfig;
        newconfig = 0;
        temp = config;
        i++;      
    }
    nonfiniromai: printf("U\n");
    reset1:;
    reset(newconfig);
    reset(config);
    /*reset1: reset(newconfig, elements);
    reset(todelete, elementstodelete);
    reset(config, oldelements);*/
    //reset(config);
    reset2:;
    /*Controllo se la stringa è finita, non uso per ora
     reset2: if (isfinished==false) {
        srt = malloc(sizeof(char)*200000);
        hofinito = fgets(srt,200000,stdin);
        free (srt);
        srt=0;
    }*/

    elements=0;
    computing=false;
    isfinished=false;
    if (buffer!=0)
        free(buffer);
    buffer = 0;
    lunghezzabuffer=0;
    config = 0;
    newconfig=0;
    fine:;
}

void checkfinals() {
    long int i,j, k;
    dotrs *dotrstemp;
    trs *trstemp;
    for (i=0;i<127;i++) {
        if (ingresso[i]!=0) {
            for (j=0;j<HASH_MOD;j++) {
                trstemp = ingresso[i][j];
                while (trstemp != 0) {
                    dotrstemp = trstemp->archi;
                    while (dotrstemp!=0) {
                        for (k=0;k<nfinal;k++)
                            if (dotrstemp->states==final[k]){
                                dotrstemp->states = -1;
                            }
                        dotrstemp=dotrstemp->next;
                    }
                    trstemp=trstemp->next;
                }
            }   
        }
    }
}


