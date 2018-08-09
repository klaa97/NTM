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
#define S_FINAL 30
#define STR_IN 20
#define STR_RIGHT 3
#define STR_LEFT 3
#define HASH_MOD 2048

//libc libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//Se legge eof, NULL!
char prova = 'a';
char *hofinito = &prova;
bool sonosola = true;

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

int maxstate = 0;

//Stati finali
int* final;
int nfinal;

bool pozzo = true;

//Puntatore a buffer di ingresso
char *buffer = 0;
//Chunk letto
int lunghezzabuffer=0;

long int elements = 0;

//Knuth method
int hash(int state){
    return (state%1024);
}

int oldelements = 0;

long long int max;

/*Strutture per grafo di transizione:
indice - lista di stringhe che si espande a destra, grandezza iniziale: STR_IN
espansione a sinistra / destra: STR_EX*/
typedef struct str{
    char *text;
    long shared;
} str;
typedef struct str1{
    char *text;
    int shared;
    int lunghezza;
} str1;
typedef struct conf{
    str1 *sprev;
    str1 *snext;
    struct conf *next;
    short int state;
    int current; // Puntatore alla posizone, negativo: prev | >=STR_IN: next
}conf;
conf *config = 0; //Mantiene la testa della lista vecchia
conf *newconfig = 0; //Mantiene testa lista nuova

void memconf(conf *dest, conf* prev) {
    dest->current=prev->current;
    dest->snext=prev->snext;
    dest->sprev=prev->sprev;
    dest->next=prev->next;
    dest->state=prev->state;
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

void insert(int start, int h, char csub, char tr, int sd, int sstart){
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
            ingresso[cstart]=calloc(HASH_MOD, sizeof(trs));
        }
        h=sstart;
        insert((int)cstart,h,(int)csubstitute,transaction,send, sstart);
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
    //elements = 1;
    scanf("%ms ", &buffer);
    lunghezzabuffer = strlen(buffer);
    int i=0;
    int j;
    char *control;
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

//Libero una lista di transazioni (utile?)
void freetrs(trs *list) {
    trs *prev = 0;
    while (list!=0){
        prev=list;
        free(prev);
        prev=0;
        list=list->next;
    }
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

/*trs *addtoqueue(trs *queue, char cwr, char mov, int st){
    trs *el;
    el = malloc(sizeof(trs));
    el->startstate=0;
    el->cwrite=cwr;
    el->movement=mov;
    el->states =  st;
    el->next = queue;
    queue = el;
    return el;
}*/

//Pop di una transizione una volta eseguita
trs *deletefirstinqueue(trs *queue){
    trs *el = queue->next;
    free (queue);
    return el;
}

//Inizializza una stringa nuova creata con BLANK // DA RIFARE!
void inizializza(str1 *new, int lunghezza){
    int i;
    char *s = new->text;
    for (i=0; i<lunghezza; i++)
        s[i]='_';
    new->shared=1;
}

//Modifico la config a seconda della transizione, aggiungendo in testa le nuove
void computeconfignotlast(dotrs *arco, conf *stato, int letto, int i, int where){
    int j, lunghezza;
    elements++;
    int nchunk = 0;
    long ltmp, tmp, cursor;
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
void reset(conf *cnf, int downelements) {
    // reset delle config
    conf *temp = cnf;
    while (temp!=0) {
        cnf = temp;
        temp=temp->next;
        freeconfig(cnf);
        
    }

    
}

void computeconfiglast(dotrs *arco, conf *stato, int letto, int i, int where) {
    str *new = 0;
    str1 *extrem = 0;
    int  j,lunghezza;
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
        oldelements--;
}


bool searchandqueueandcompute(conf *cnf, conf **valore){
    // Aggiungo alla Queue le transizioni possibili per la transizione
    sonosola = true;
    str1 *extrem=0;
    pozzo = true;
    int tmp;
    char control;
    trs *queue=0;
    int h = hash(cnf->state);
    int i = cnf->current;
    bool where;
    dotrs *dafare;
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
    //("è la %d conf che computo, e ho letto %c\n", numberconf, letto);
    trs *list = (ingresso[letto]!=0) ? ingresso[letto][h] : 0;
    while (list!=0) {
        if (list->startstate==cnf->state){
            dafare = list->archi;
            pozzo = false;
            while (dafare!=0){
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
            goto computata;
        }
        list=list->next;
    }
    computata:;
    //Se non ci sono possibili transizioni, o è accettazione o è pozzo (non accettata)
    if (pozzo == true) {
        /*for (tmp=0;tmp<nfinal;tmp++)
            if (cnf->state==final[tmp]){
                printf("1\n");
                return true;
            }*/
        // Se sono qui, è da cancellare la config, non accettata!
        freeconfig(cnf);
    }
    
    
    //Per ogni config presente nella lista, computo; l'ultima ha una computazione speciale (non libero)
    /*trs *now = 0;
    while (queue!=0) {
        now = queue;
        if (now->next!=0)
            computeconfignotlast(now, cnf, letto, i, where);
        else computeconfiglast(now, cnf, letto, i, where);
        queue = deletefirstinqueue(queue);
    }*/
    return false;
}
/*Starto la configurazione, computo max-mosse
- se la config è null prima, termina 0 | se non è null dopo, è U */
void compute(){
    
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


int main() {
    //int *final;
    readtransaction();
    readfinal();
    readmax();
    checkfinals();
    while (!feof(stdin)) {
        char *srt;
    int tmp;
    startconfig();
    long long int i = 0;
    conf *temp=config;
    conf *prec = 0;
    newconfig = 0;
    oldelements = 1;
    while (i < max) {
        while (temp!=0) {
            prec = temp;
            temp = temp->next;
            if (searchandqueueandcompute(prec,&prec)) {
                reset(newconfig, elements);
                reset(prec, oldelements);
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
        oldelements = elements;
        elements=0;
        config = newconfig;
        newconfig = 0;
        temp = config;
        i++;      
    }
    printf("U\n");
    reset(config, oldelements);

    
    reset1:elements=0;
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


} 