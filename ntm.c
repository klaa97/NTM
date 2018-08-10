/* Turing , ND
- Nastro singolo, accettori
- Nastro: char
- Stati: int
- "_" -> blank
- stdin
- controllare /r/n o /n
*/

/* Costanti moltiplicative */
#define S_FINAL 30
#define STR_IN 20
#define STR_RIGHT 128
#define STR_LEFT 128
#define HASH_MODD 256

//libc libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//Vera per transizioni deterministiche
bool sonosola = true;

//Variabile per sapere se qualcosa non terminerà mai
bool computing = false;

//Stato pozzo?
bool pozzo = true;

/* Struttura per transizioni
- 127 puntatori ad array, 0
- Ogni puntatore punta ad un array di puntatori di HASH_MODD elementi, espando se necessario di HASH_MODD
- Ogni puntatore punta alla lista delle transizioni(cosa scrivere, R/S/L, stato)
- Tabella hash, 127, indirizzamento diretto
*/
typedef struct dotrs{
    struct dotrs *next;
    int cwrite;
    char movement; 
    int states;
}dotrs;

//Contiene le transizioni 
dotrs **ingresso[127]; 

int maxx[127]; // Massimo stato letto in entrata per carattere
int HASH_MOD[127]; //Lunghezza array per ogni stato letto

//Stati finali
int* final;
int nfinal;

//Puntatore a buffer di ingresso
char *buffer = 0;
//Chunk letto
int lunghezzabuffer=0;

//Numero di elementi nella nuova queue
long int elements = 0;

//Max mosse
long long int max;

/*Strutture per grafo di transizione:
indice - lista di stringhe che si espande a destra, grandezza iniziale: STR_IN
espansione a sinistra / destra: STR_EX*/
typedef struct str1{
    char *text;
    int shared;
    int lunghezza;
} str1;

//Configurazione della macchina
typedef struct conf{
    str1 *sprev;
    str1 *snext;
    struct conf *next;
    short int state;
    int current; // Puntatore alla posizone, negativo: prev | >=STR_IN: next
}conf;
conf *config = 0; //Mantiene la testa della lista vecchia
conf *newconfig = 0; //Mantiene testa lista nuova

void memconf(conf *dest, conf *prev);

void memstr1(str1 *dest, str1 *prev);

//Inserisco transizioni nell' HashTable
void insert(int start, int h, char csub, char tr, int sd, int sstart);

void readtransaction();

void readfinal();

void readmax();

void startconfig();

void freeconfig(conf *cnf);

//Modifico la config a seconda della transizione, aggiungendo in testa le nuove (non ultima)
void computeconfignotlast(dotrs *arco, conf *stato, int letto, int i, int where);

//Resetto le config successive a cnf 
void reset(conf *cnf);

//Modifico la config a seconda della transizione, aggiungendo in testa le nuove (ultima)
void computeconfiglast(dotrs *arco, conf *stato, int letto, int i, int where);

// Cerco le transizioni per config e computo
bool searchandqueueandcompute(conf *cnf, conf **valore);

/*Starto la configurazione, computo max-mosse
- se la config è null prima, termina 0 | se non è null dopo, è U */
void compute();

//Rimappo i finali a -1 in modo efficiente
void checkfinals();

int main() {
    //int *final;
    readtransaction();
    readfinal();
    readmax();
    checkfinals();
    while (!feof(stdin)) {
        compute();  
    }
} 

void memconf(conf *dest, conf* prev) {
    dest->current=prev->current;
    dest->snext=prev->snext;
    dest->sprev=prev->sprev;
    dest->next=prev->next;
    dest->state=prev->state;
}

void memstr1(str1 *dest, str1 *prev) {
    dest->lunghezza=prev->lunghezza;
    dest->shared=prev->shared;
    dest->text=prev->text;
}

void insert(int start, int h, char csub, char tr, int sd, int sstart){
    dotrs *el;
    el = malloc(sizeof(dotrs));
    el->cwrite=csub;
    el->movement=tr;
    el->states = sd;
    el->next = ingresso[start][h];
    ingresso[start][h] = el; 
}

void readtransaction(){
    int i,tmp;
    for (i=0; i<127;i++){
        HASH_MOD[i] = HASH_MODD;
        maxx[i] = 0;
        ingresso[i] = 0;
    }
    conf **temp;
    int h;
    int sstart;
    int send;
    char cstart;
    char csubstitute;
    char transaction;
    scanf("tr ");
    while (scanf("%d",&sstart)==1) {
        scanf(" %c %c %c %d", &cstart, &csubstitute,&transaction,&send);
        if (ingresso[cstart]==0){
            ingresso[cstart]=calloc(HASH_MODD, sizeof(*temp));
        }
        if (sstart >= maxx[cstart]) {
            tmp = (sstart>send) ? sstart : send;
            maxx[cstart] = sstart+1;
            if (maxx[cstart] >= HASH_MOD[cstart]) {
                i = HASH_MOD[cstart];
                HASH_MOD[cstart]= maxx[cstart]*2;
                ingresso[cstart] = realloc(ingresso[cstart], sizeof(*temp)*HASH_MOD[cstart]);
                for (;i < HASH_MOD[cstart]; i++)
                    ingresso[cstart][i] = 0;
            }
        }
        insert(cstart,sstart,csubstitute,transaction,send, sstart);
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
    int i=0;
    char *control;
    scanf("%ms ", &buffer);
    lunghezzabuffer = strlen(buffer);
    config = malloc(sizeof(conf));
    config->next=0;
    config->snext=malloc(sizeof(str1));
    config->snext->shared = 1;
    config->snext->lunghezza=STR_RIGHT;
    config->sprev=malloc(sizeof(str1));
    config->sprev->text=malloc(sizeof(char)*STR_LEFT);
    config->sprev->shared=1;
    config->sprev->lunghezza=STR_LEFT;
    control = config->sprev->text;
    for (i=0;i<STR_LEFT;i++)
        control[i]='_';
    config->state=0;
    config->snext->text = malloc(sizeof(char)*STR_RIGHT);
    control = config->snext->text;
    for (i=0; i<lunghezzabuffer && i<STR_RIGHT;i++)
        control[i] = buffer[i];   
    for (; i < STR_RIGHT; i++)
        control[i] = '_';
    config->current=0;
    elements=0;

}

void freeconfig(conf *cnf) {
    //Riduco di 1 il contatore shared di tutte le stringhe in conf, libero le stringhe se è zero
    cnf->sprev->shared--;
    cnf->snext->shared--;
    if (cnf->sprev->shared==0) {
        free(cnf->sprev->text);
        free(cnf->sprev);
        
        
    }
    if (cnf->snext->shared==0) {
        free(cnf->snext->text);
        free(cnf->snext);
        
    }
    
    free(cnf);
}

//Modifico la config a seconda della transizione, aggiungendo in testa le nuove (non ultima)
void computeconfignotlast(dotrs *arco, conf *stato, int letto, int i, int where){
    int lunghezza,ltmp, tmp, cursor;
    str1 *extrem = 0;
    char *control;
    //Creo una nuova config identica alla precedente
    conf *destinazione = malloc(sizeof(conf));
    memconf(destinazione,stato);
    //memmove(destinazione,stato,sizeof(conf));
    //funzione per fare un branch e creare una nuova stringa se necessario
    if ((int)arco->cwrite != letto) {
       switch (where) {
            case 1: {
                extrem=malloc(sizeof(str1));
                lunghezza = stato->snext->lunghezza;
                extrem->lunghezza = lunghezza;
                extrem->text=malloc(sizeof(char)*lunghezza);
                strncpy(extrem->text,stato->snext->text, lunghezza);
                extrem->text[i]=arco->cwrite;
                extrem->shared=1;
                destinazione->snext=extrem;
                destinazione->sprev->shared++;             
                break;
            }
            case 0: {
                extrem=malloc(sizeof(str1));
                lunghezza = stato->sprev->lunghezza;
                extrem->lunghezza = lunghezza;
                extrem->text=malloc(sizeof(char)*lunghezza);
                strncpy(extrem->text,stato->sprev->text, lunghezza);
                extrem->text[-i-1]=arco->cwrite;
                extrem->shared=1;
                destinazione->sprev=extrem;
                destinazione->snext->shared++;
                break;
            }
        }
    }
    else {
            destinazione->snext->shared++;
            destinazione->sprev->shared++;
    }
    //Cambio stato
    destinazione->state=arco->states;
    //Cambio posizione
    switch(arco->movement) {
        case 'L': {
            destinazione->current--;
            cursor = destinazione->current;
            ltmp = destinazione->sprev->lunghezza;
            if (where == 0 && cursor < -ltmp) {
                destinazione->sprev->lunghezza += STR_LEFT;
                destinazione->sprev->text = realloc(destinazione->sprev->text, sizeof(char)*destinazione->sprev->lunghezza);
                control = destinazione->sprev->text;
                for (; ltmp < destinazione->sprev->lunghezza; ltmp++) {
                    control[ltmp]='_';
                }
            }                    
            break;
        }
        case 'R': {
            destinazione->current++;
            cursor = destinazione->current;
            ltmp = destinazione->snext->lunghezza;
            if (where == 1 && cursor == ltmp) {
                if (ltmp >= lunghezzabuffer){
                    destinazione->snext->lunghezza += STR_RIGHT;
                    destinazione->snext->text = realloc(destinazione->snext->text, sizeof(char)*destinazione->snext->lunghezza);
                    control = destinazione->snext->text;
                    for (; ltmp < destinazione->snext->lunghezza; ltmp++) 
                        control[ltmp]='_';
                } else {
                    destinazione->snext->lunghezza += STR_RIGHT;
                    tmp = ltmp;
                    destinazione->snext->text = realloc(destinazione->snext->text, sizeof(char)*destinazione->snext->lunghezza);
                    control = destinazione->snext->text;
                    for (; ltmp<lunghezzabuffer && ltmp<tmp+STR_RIGHT;ltmp++)
                        control[ltmp] = buffer[ltmp];
                    for (; ltmp < tmp+STR_RIGHT; ltmp++)
                        control[ltmp] = '_'; 
                }
            }
            break;   
        }
    }
    //Aggiungo in testa alla lista delle config la nuova config creata
    destinazione->next=newconfig;
    newconfig=destinazione;  
}   

//Resetto le config successive a cnf 
void reset(conf *cnf) {
    conf *temp = cnf;
    while (temp!=0) {
        cnf = temp;
        temp=temp->next;
        freeconfig(cnf);
    }
}

//Modifico la config a seconda della transizione, aggiungendo in testa le nuove (ultima)
void computeconfiglast(dotrs *arco, conf *stato, int letto, int i, int where) {
    str1 *extrem = 0;
    int  lunghezza;
    long ltmp,tmp,cursor;
    char *control;
    int oldstate = stato->state;
    //Modifico il carattere da modificare, se necessario;
    //Se shared=1, non è necessario fare un branch ma basta modificare
    if ((int)arco->cwrite != letto) {
        switch (where) {
            case 1: {
                if (stato->snext->shared==1) {
                    stato->snext->text[i]=arco->cwrite;
                } else {
                    stato->snext->shared--;
                    extrem=malloc(sizeof(str1));
                    lunghezza = stato->snext->lunghezza;
                    extrem->lunghezza = lunghezza;
                    extrem->text=malloc(sizeof(char)*lunghezza);
                    strncpy(extrem->text,stato->snext->text, lunghezza);
                    extrem->text[i]=arco->cwrite;
                    extrem->shared=1;
                    stato->snext=extrem;
                }                
                break;
                }
            case 0: {
                if (stato->sprev->shared==1) {
                    stato->sprev->text[-i-1]=arco->cwrite;
                } else {
                    stato->sprev->shared--;
                    extrem=malloc(sizeof(str1));
                    lunghezza = stato->sprev->lunghezza;
                    extrem->lunghezza = lunghezza;
                    extrem->text=malloc(sizeof(char)*lunghezza);
                    strncpy(extrem->text,stato->sprev->text,lunghezza);
                    extrem->text[-i-1]=arco->cwrite;
                    extrem->shared=1;
                    stato->sprev=extrem;
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
            if (where == 0) {
                ltmp = stato->sprev->lunghezza;
                if (cursor<-ltmp) {
                    if (sonosola==true && oldstate == arco->states && letto == '_'  /*&& letto == arco->cwrite*/) {
                        computing = true;
                        freeconfig(stato);
                        goto fine;
                    }
                    stato->sprev->lunghezza += STR_LEFT;
                    stato->sprev->text = realloc(stato->sprev->text, sizeof(char)*stato->sprev->lunghezza);
                    control = stato->sprev->text;
                    for (; ltmp<stato->sprev->lunghezza; ltmp++) {
                        control[ltmp]='_';
                    }
                }
            }
            break;
        }
        case 'R': {
            stato->current++;
            cursor = stato->current;
            ltmp = stato->snext->lunghezza;
            tmp = ltmp;
            if (where == 1 && cursor == ltmp) {
                if (ltmp >= lunghezzabuffer){
                    if (sonosola==true && oldstate == arco->states && letto == '_' /*&& letto == arco->cwrite*/) {
                        computing = true;
                        freeconfig(stato);
                        goto fine;
                    }
                    stato->snext->lunghezza += STR_RIGHT;
                    stato->snext->text = realloc(stato->snext->text, sizeof(char)*stato->snext->lunghezza);
                    control = stato->snext->text;
                    for (; ltmp < stato->snext->lunghezza; ltmp++) 
                        control[ltmp]='_';
                } else {
                    stato->snext->lunghezza += STR_RIGHT;
                    stato->snext->text = realloc(stato->snext->text, sizeof(char)*stato->snext->lunghezza);
                    control = stato->snext->text;
                    tmp = ltmp;
                    for (; ltmp < lunghezzabuffer && ltmp<tmp+STR_RIGHT;ltmp++)
                        control[ltmp] = buffer[ltmp];
                    for (; ltmp < tmp+STR_RIGHT; ltmp++)
                        control[ltmp]='_';
                    
                }
            }
                    break;   
        }
    }
        //Aggiungo in testa 
        stato->next=newconfig;
        newconfig=stato;
        elements++;
        fine:;
    }


bool searchandqueueandcompute(conf *cnf, conf **valore){
    sonosola = true;
    str1 *extrem=0;
    pozzo = true;
    dotrs *dafare = 0;
    char control;
    int h = cnf->state;
    int i = cnf->current;
    bool where;
    if (i >= 0){
        where = 1;
        extrem=cnf->snext;
        control = cnf->snext->text[i];
    }else if (i < 0) {
        where = 0;
        extrem=cnf->sprev;
        control = cnf->sprev->text[-i-1];        
    }
    int letto = (int) control;
    if (ingresso[letto]!=0 && maxx[letto] >= cnf->state)
        dafare = ingresso[letto][h];
    else goto nonaccettata;
    while (dafare!=0) {
        pozzo = false;
        if (dafare->states == cnf->state && dafare->movement == 'S' && letto == dafare->cwrite)
            computing = true;
        else if (dafare->states == -1) {
            printf("1\n");
            return true;
        }
        else if (dafare->next!=0) {
            computeconfignotlast(dafare, cnf, letto, i, where);
            sonosola=false;
        }
        else computeconfiglast(dafare, cnf, letto, i, where);
        dafare = dafare->next;
    }
            
    //Se non ci sono possibili transizioni, è pozzo (non accettata)
    if (pozzo == true) {
        nonaccettata: freeconfig(cnf);
    }
    return false;
}

/*Starto la configurazione, computo max-mosse
- se la config è null prima, termina 0 | se non è null dopo, è U */
void compute(){
    int tmp;
    startconfig();
    long long int i = 0;
    conf *temp=config;
    conf *prec = 0;
    newconfig = 0;
    while (i < max) {
        while (temp!=0) {
            prec = temp;
            temp = temp->next;
            if (searchandqueueandcompute(prec,&prec)) {
                reset(newconfig);
                reset(prec);
                goto reset1;
                
            }
        }
        if (elements==0 && computing==false) {
            printf("0\n");
            goto reset1;
        } else if (elements == 0 && computing ==true) {
            printf("U\n");
            goto reset1;
        }
        elements=0;
        config = newconfig;
        newconfig = 0;
        temp = config;
        i++;      
    }
    printf("U\n");
    reset(config);
    reset1:elements=0;
    computing=false;
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
    for (i=0;i<127;i++) {
        if (ingresso[i]!=0) {
            for (j=0;j<maxx[i];j++) {
                dotrstemp = ingresso[i][j];
                while (dotrstemp != 0) {
                        for (k=0;k<nfinal;k++)
                            if (dotrstemp->states==final[k]){
                                dotrstemp->states = -1;
                            }
                        dotrstemp=dotrstemp->next;
                }
            }
        }   
    }
}