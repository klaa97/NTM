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
#define HASH_MOD 33
#define S_FINAL 20
#define STR_IN 20
#define STR_RIGHT 8
#define STR_LEFT 8

//libc libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//Se legge eof, NULL!
char prova = 'a';
char *hofinito = &prova;

int numberconf=0;
//Variabile per sapere se ho già letto fino alla fine della stringa
bool isfinished=0;
//Variabile per sapere se qualche computazione è stata terminata per aver superato max
bool toomanycomputations=0;
//Variabile per sapere se qualcosa non terminerà mai
bool computing = 0;
/* Struttura per transizioni
- 256 puntatori ad array, 0
- Ogni puntatore punta ad un array di puntatori di STATES_S elementi, espando se necessario di STATES_E
- Ogni puntatore di STATES_S elementi punta a una struct con cosa scrivere, R/S/L, stato  
- Tabella hash, 256, funzione hash: modulo
*/
typedef struct trs{
    struct trs *next;
    int startstate;
    char cwrite;
    char movement; 
    int states;
} trs;

trs **ingresso[127];

//Stati finali
int* final;
int nfinal;


//Puntatore a buffer di ingresso
char *buffer = 0;
//Chunk letto
int lunghezzabuffer=0;

long elements = 0;

int hash(int state){
    return (state%HASH_MOD);
}

long max;

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
    int lunghezza;
} str1;
typedef struct conf{
    str1 *sprev;
    str1 *snext;
    struct conf *next;
    int state;
    int current; // Puntatore alla posizone, negativo: prev | >=STR_IN: next
}conf;
conf *config = 0; //Mantiene la testa della lista vecchia
conf *newconfig = 0; //Mantiene testa lista nuova
str1 *defaultnext; //Stringa dove mettere i next non modificati

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
    trs *el;
    el = malloc(sizeof(trs));
    el->startstate = sstart;
    el->cwrite=csub;
    el->movement=tr;
    el->states =  sd;
    el->next=ingresso[start][h];
    ingresso[start][h]=el;
    //printf("da scrivere %c\n movimento %c\n stato %d\n",el->cwrite, el->movement, el->states);
}



// Passo la posizione del cursore aumentata e char* da dove posso riempire per STR_EXP
/*void riempioadestra(char *dariempire, int cursor) {
    int i;
    // Non ho già letto il pezzo
    if (cursor > lunghezzabuffer) {
        if (isfinished == 1) {
            for (i=0;i<STR_EXP;i++)
                dariempire[i]="_";
        } else {
            if (buffer == 0)
                buffer= malloc(sizeof(char)*STR_EXP);
            else realloc(buffer,sizeof(char)*(lunghezzabuffer+STR_EXP));
            fgets(buffer+lunghezzabuffer, STR_EXP,stdin);
            for (i = lunghezzabuffer; i < lunghezzabuffer + STR_EXP; i++){
                        if (buffer[i] == "\0" && i == lunghezzabuffer + STR_EXP -1)
                            buffer[i] = getchar();
                        if (buffer[i]=='\r' || buffer[i]=='\n' || buffer[i]=='\0'){
                            if (buffer[i]=='\n')
                                isfinished=true;
                            if (buffer[i]=='\0') {
                               for (;i<STR_IN;i++){
                                    buffer[i]='_';
                                   }
                                 goto finish;
                            }
                        }
            }
            for (i=0;i<STR_EXP;i++)
                dariempire[i]=buffer[cursor+i-1];
        }
        finish: lunghezzabuffer += STR_EXP;
    }
    else {
        for (i=0;i<STR_EXP;i++)
            dariempire[i]=buffer[cursor+i-1];
        }       
}*/        


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
        insert((int)cstart,h,csubstitute,transaction,send, sstart);
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
    scanf("%ld", &max);
    scanf(" ");
    scanf("run ");
}

//Inizializzo la prima configurazione
void startconfig() {
    //elements = 1;
    buffer = malloc(sizeof(char)*STR_RIGHT);
    hofinito = fgets(buffer, STR_RIGHT, stdin);
    if (hofinito==0)
        goto end;
    int i=0;
    int j;
    config = malloc(sizeof(conf));
    config->next=0;
    config->snext=malloc(sizeof(str1));
    config->snext->shared = 1;
    config->snext->text=malloc(sizeof(char)*STR_RIGHT);
    config->snext->lunghezza=STR_RIGHT;
    config->sprev=malloc(sizeof(str1));
    config->sprev->text=malloc(sizeof(char)*STR_LEFT);
    config->sprev->shared=1;
    config->sprev->lunghezza=STR_LEFT;
    char *control = config->sprev->text;
    for (i=0;i<STR_LEFT;i++)
        control[i]='_';
    lunghezzabuffer = STR_RIGHT;
    // Controllo se stringa finisce
    for (i=0; i < STR_RIGHT; i++){
        if (buffer[i]=='\r' || buffer[i]=='\n' || buffer[i]=='\0'){
            if (buffer[i]=='\n') {
                buffer[i]='_';
                isfinished=true;
            }
            if (buffer[i] == '\r') 
                                            buffer[i] = '_';
                                        if (buffer[i]=='\0'){
                                        if (isfinished != true) {
                                            buffer[i]=getchar();
                                            j=i+1;
                                            if (buffer[i]=='\n') {
                                                isfinished = true;
                                                j= i;
                                            } else if (buffer[i]=='\r')
                                                j=i;
                                        } else j=i;
                                        for (;j<lunghezzabuffer;j++)
                                            buffer[j]='_';
                                        
                                        goto inizializzato;
            }
        }
    }
    inizializzato: config->state=0;
    config->snext->text = malloc(sizeof(char)*lunghezzabuffer);
    strncpy(config->snext->text, buffer, STR_RIGHT);
    //printf("Letto %s\n", control);
    config->current=0;
    end: elements=0;
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

trs *addtoqueue(trs *queue, char cwr, char mov, int st){
    trs *el;
    el = malloc(sizeof(trs));
    el->startstate=0;
    el->cwrite=cwr;
    el->movement=mov;
    el->states =  st;
    el->next = queue;
    queue = el;
    return el;
}

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
void computeconfignotlast(trs *arco, conf *stato, int letto, int i, int where){
    if ((int)arco->cwrite == letto && arco->movement == 'S' && stato->state==arco->states) {
        computing = true;
        goto notlast;
    }
    int j;
    elements++;
    int cursor, ltmp, tmp;
    str1 *extrem = 0;
    char *control;
    //Creo una nuova config identica alla precedente
    conf *destinazione = malloc(sizeof(conf));
    memconf(destinazione,stato);
    //memmove(destinazione,stato,sizeof(conf));
    destinazione->next = 0;
    //funzione per fare un branch e creare una nuova stringa se necessario
    if ((int)arco->cwrite != letto) {
       switch (where) {
            case 1: {
                extrem=malloc(sizeof(str1));
                memstr1(extrem, stato->snext);
                //memmove(extrem,stato->snext,sizeof(str1));
                extrem->text=malloc(sizeof(char)*extrem->lunghezza);
                strncpy(extrem->text,stato->snext->text, extrem->lunghezza);
                extrem->text[i]=arco->cwrite;
                extrem->shared=1;
                destinazione->snext=extrem;
                destinazione->sprev->shared++;             
                break;
            }
            case 0: {
                extrem=malloc(sizeof(str1));
                memstr1(extrem,stato->sprev);
                //memmove(extrem,stato->sprev,sizeof(str1));
                extrem->text=malloc(sizeof(char)*extrem->lunghezza);
                strncpy(extrem->text,stato->sprev->text, extrem->lunghezza);
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
                if (ltmp>=lunghezzabuffer){
                    if (isfinished==1) {
                        destinazione->snext->lunghezza += STR_RIGHT;
                        destinazione->snext->text = realloc(destinazione->snext->text, sizeof(char)*destinazione->snext->lunghezza);
                        control = destinazione->snext->text;
                        for (; ltmp < destinazione->snext->lunghezza; ltmp++) 
                            control[ltmp]='_';
                    } else {
                        destinazione->snext->lunghezza += STR_RIGHT;
                        lunghezzabuffer += STR_RIGHT;
                        tmp = ltmp;
                        buffer = realloc(buffer, sizeof(char)*lunghezzabuffer);
                        fgets(buffer+ltmp, STR_RIGHT, stdin);
                        for (; ltmp < destinazione->snext->lunghezza; ltmp++){
                            if (buffer[ltmp]=='\r' || buffer[ltmp]=='\n' || buffer[ltmp]=='\0'){
                                if (buffer[ltmp]=='\n') {
                                    buffer[ltmp]='_';
                                    isfinished=true;
                                }
                                if (buffer[ltmp] == '\r') 
                                            buffer[ltmp] = '_';
                                        if (buffer[ltmp]=='\0'){
                                        if (isfinished != true) {
                                            buffer[ltmp]=getchar();
                                            j=ltmp+1;
                                            if (buffer[ltmp]=='\n') {
                                                isfinished = true;
                                                j= ltmp;
                                            } else if (buffer[ltmp]=='\r')
                                                j=ltmp;
                                        } else j=ltmp;
                                        for (;j<lunghezzabuffer;j++)
                                            buffer[j]='_';
                                        
                                        goto inizializzato;
                                }
                            }
                        }
                        inizializzato: destinazione->snext->text = realloc(destinazione->snext->text, sizeof(char)*destinazione->snext->lunghezza);
                        strncpy(destinazione->snext->text+tmp,buffer+tmp,STR_RIGHT);
                    }
                } else {
                    strncpy(destinazione->snext->text+ltmp,buffer+ltmp,STR_RIGHT);
                    destinazione->snext->lunghezza += STR_RIGHT;
                }
            }
            break;   
        }
    }
    //Aggiungo in testa alla lista delle config la nuova config creata
    destinazione->next=newconfig;
    newconfig=destinazione;  
    notlast:;  
}   
void reset(conf *cnf) {
    // reset delle config
    conf *temp = cnf;
    while (temp!=0 && elements!=0) {
        temp = cnf;
        temp=temp->next;
        freeconfig(cnf);
        elements--;
    }
    cnf = 0;
}

void computeconfiglast(trs *arco, conf *stato, int letto, int i, int where) {
    if ((int)arco->cwrite == letto && arco->movement == 'S' && stato->state==arco->states) {
        computing = true;
        freeconfig(stato);
        goto last;
    }
    elements++;
    str *new = 0;
    str1 *extrem = 0;
    int cursor, ltmp,j,tmp;
    char *control;
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
                    memstr1(extrem,stato->snext);
                    //memmove(extrem,stato->snext,sizeof(str1));
                    extrem->text=malloc(sizeof(char)*extrem->lunghezza);
                    strncpy(extrem->text,stato->snext->text, extrem->lunghezza);
                    extrem->text[i]=arco->cwrite;
                    extrem->shared=1;
                    stato->snext=extrem;
                    /*if (destinazione->sprev!=0) {
                        destinazione->sprev->shared++;
                    }*/
                }                
                break;
                }
            case 0: {
                if (stato->sprev->shared==1) {
                    stato->sprev->text[-i-1]=arco->cwrite;
                } else {
                    stato->sprev->shared--;
                    extrem=malloc(sizeof(str1));
                    memstr1(extrem, stato->sprev);
                    //memmove(extrem,stato->sprev,sizeof(str1));
                    extrem->text=malloc(sizeof(char)*extrem->lunghezza);
                    strncpy(extrem->text,stato->sprev->text, extrem->lunghezza);
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
                    if (isfinished==1) {
                        stato->snext->lunghezza += STR_RIGHT;
                        stato->snext->text = realloc(stato->snext->text, sizeof(char)*stato->snext->lunghezza);
                        control = stato->snext->text;
                        for (; ltmp < stato->snext->lunghezza; ltmp++) 
                            control[ltmp]='_';
                        } else {
                                stato->snext->lunghezza += STR_RIGHT;
                                lunghezzabuffer = stato->snext->lunghezza;
                                buffer = realloc(buffer, lunghezzabuffer);
                                fgets(buffer+ltmp, STR_RIGHT, stdin);
                                for (; ltmp < stato->snext->lunghezza; ltmp++){
                                    if (buffer[ltmp]=='\r' || buffer[ltmp]=='\n' || buffer[ltmp]=='\0'){
                                        if (buffer[ltmp]=='\n') {
                                            buffer[ltmp]='_';
                                            isfinished=true;
                                        }
                                        if (buffer[ltmp] == '\r') 
                                            buffer[ltmp] = '_';
                                        if (buffer[ltmp]=='\0'){
                                        if (isfinished != true) {
                                            buffer[ltmp]=getchar();
                                            j=ltmp+1;
                                            if (buffer[ltmp]=='\n') {
                                                isfinished = true;
                                                j= ltmp;
                                            } else if (buffer[ltmp]=='\n')
                                                j=ltmp;
                                        }
                                            
                                         else j=ltmp;
                                                for (;j<lunghezzabuffer;j++){
                                                    buffer[j]='_';
                                            }
                                        goto inizializzato;
                                    }
                                        
                                    }
                                }
                                inizializzato: stato->snext->text = realloc(stato->snext->text, sizeof(char)*stato->snext->lunghezza);
                                strncpy(stato->snext->text+tmp,buffer+tmp,STR_RIGHT);
                            }
                        } else {
                            stato->snext->lunghezza += STR_RIGHT;
                            stato->snext->text = realloc(stato->snext->text, sizeof(char)*stato->snext->lunghezza);
                            strncpy(stato->snext->text+tmp,buffer+tmp,STR_RIGHT);
                            
                        }
                    }
                    break;   
                }
        }
        //Aggiungo in testa 
        stato->next=newconfig;
        newconfig=stato;
        last:;
}


bool searchandqueueandcompute(conf *cnf, conf **valore){
    // Aggiungo alla Queue le transizioni possibili per la transizione
    str1 *extrem=0;
    int tmp;
    char control;
    trs *queue=0;
    int h = hash(cnf->state);
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
    //("è la %d conf che computo, e ho letto %c\n", numberconf, letto);
    trs *list = (ingresso[letto]!=0) ? ingresso[letto][h] : 0;
    while (list!=0){
        if (list->startstate==cnf->state){
            //("per la %d conf scriverò %c\n", numberconf, list->cwrite);
            queue = addtoqueue(queue, list->cwrite, list->movement, list->states);
        }
        list=list->next;
    }
    //Se non ci sono possibili transizioni, o è accettazione o è pozzo (non accettata)
    if (queue==0) {
        for (tmp=0;tmp<nfinal;tmp++)
            if (cnf->state==final[tmp]){
                printf("1");
                return true;
            }
        // Se sono qui, è da cancellare la config, non accettata!
        freeconfig(cnf);
    }
    //Per ogni config presente nella lista, computo; l'ultima ha una computazione speciale (non libero)
    trs *now = 0;
    while (queue!=0) {
        now = queue;
        if (now->next!=0)
            computeconfignotlast(now, cnf, letto, i, where);
        else computeconfiglast(now, cnf, letto, i, where);
        queue = deletefirstinqueue(queue);
    }
    return false;
}
/*Starto la configurazione, computo max-mosse
- se la config è null prima, termina 0 | se non è null dopo, è U */
void compute(){
    char *srt;
    startconfig();
    if (hofinito==0)
        goto fine;
    int i = 0;
    conf *temp=config;
    conf *prec = 0;
    while (i < max) {
        //("sono alla %d comp", i);
        while (temp!=0) {
            prec = temp;
            temp = temp->next;
            if (searchandqueueandcompute(prec,&prec))
                goto reset1;
        }
        if (elements==0) {
            printf("0");
            goto reset2;
        }
        elements=0;
        config = newconfig;
        newconfig = 0;
        temp = config;
        i++;      
    }
    printf("U");
    reset1: reset(newconfig);
    
    //Controllo se la stringa è finita
     reset2: if (isfinished==false) {
        srt = malloc(sizeof(char)*200);
        fgets(srt,200,stdin);
        free (srt);
        srt=0;
    }
    printf("\n");
    fine: elements=0;
    isfinished=0;
    if (buffer!=0)
        free(buffer);
    buffer = 0;
    lunghezzabuffer=0;
    config = 0;
    newconfig=0;
}


int main() {
    //int *final;
    readtransaction();
    readfinal();
    readmax();
    while (!feof(stdin) && hofinito!=0) {
        compute();
    }
    return 0;

} 