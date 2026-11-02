#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define LUNG_PAR 21 //lunghezza massima di una parola
#define DIM_HASH_RIC 1507 //lunghezza hash table ricettario
#define DIM_HASH_MAG 173 //lunghezza hash table magazzino

int tempo; //definiamo il tempo come variabile intera ma globale in modo da non avere problemi

/* OPERAZIONI E STRUTTURE DATI */

typedef struct struct_ingrediente { //generico nodo per l'ingrediente in una ricetta, contiene il nome, la quantità e il puntatore al prossimo ingrediente (se non c'è un altro ingrediente allora punta a NULL)
    char nome[LUNG_PAR];
    int quantita;
    struct struct_ingrediente *next;
} ingrediente;

typedef struct struct_ricetta { //generico nodo per una ricetta, contiene il nome, il puntatore al primo ingrediente, il puntatore per la ricetta seguente (serve solo per il ricettario)
    char nome[LUNG_PAR];
    ingrediente *ingredienti;
    struct struct_ricetta *next;
} ricetta;

typedef struct struct_lotto { //generico nodo per il lotto di un ingrediente, contiene il nome, la quantita, la scadenza, il puntatore al prossimo lotto (serve solo per il magazzino)
    char nome[LUNG_PAR];
    int quantita;
    int scadenza;
    struct struct_lotto *next;
} lotto;

typedef struct struct_ordine { //generico nodo per un ordine, contiene la ricetta (direttamente lei, così accedo direttamente agli ingredienti e non devo cercare ogni volta nel ricettario), la quantita ordinata e il puntatore al prossimo ordine (serve solo per le due code di ordini), il tempo di ordinazione e il peso totale (che viene calcolato solo nella funzione camioncino)
    ricetta *ricetta;
    int quantita;
    int tempo_ordinazione;
    int peso_totale;
    struct struct_ordine *next;
} ordine;

int funzione_hash_ricettario (char *nome); //funzione di hashing per il ricettario, usando il metodo della divisione (sul nome della ricetta)

int funzione_hash_magazzino (char *nome); //funzione di hashing per il magazzino, usando il metodo della divisione (sul nome dell'ingrediente)

void aggiunta_ricettario (ricetta *ricettario, ricetta *nuova, int hash); //funzione per aggiungere una ricetta nel ricettario

int ricerca_ricettario (ricetta *ricettario, int hash, char* nome_ricetta); //funzione di ricerca di una ricetta nel ricettario, restituisce 1 se la ricetta è presente, 0 altrimenti

ricetta* ricerca_ricettario_v2 (ricetta *ricettario, int hash, char* nome_ricetta); //funzione di ricerca di una ricetta nel ricettario, restituisce il riferimento se la ricetta è presente, NULL altrimenti (mi serve per la funzione "ordini")

void rimuovi_ricettario (ricetta *ricettario, int hash, char* nome_ricetta); //funzione per rimuovere una ricetta nel ricettario

void aggiunta_nome_ingrediente (char *parola, ricetta *nuova); //funzione per aggiungere il nome di un ingrediente ad una ricetta

void aggiunta_quantita_ingrediente (int quantita, ricetta *nuova); //funzione per aggiungere la quantità di un ingrediente ad una ricetta

int ricerca_ordine(char* nome_ricetta, ordine *testa); //funzione di ricerca di una ricetta in una qualsiasi delle code di ordini, restituisce 1 se la ricetta è presente nella coda, 0 altrimenti

void aggiunta_magazzino(lotto* magazzino, int hash, char* nome_ingrediente, int quantita, int scadenza); //funzione per aggiungere il lotto di un ingrediente al magazzino

void check_magazzino(lotto* magazzino); //funzione che passa tutte le liste del magazzino e controlla che i lotti non siano scaduti ed eventualmente li elimina

void inserimento_ordine_coda(ordine *da_preparare, ordine** testa, ordine** coda); //funzione che inserisce un ordine in coda alla queue passata come parametro (passo il riferimento al primo e ultimo elemento)

void prepara_ordine(lotto* magazzino, ordine *da_preparare, ordine **testa_pronti, ordine **coda_pronti, ordine **testa_attesa, ordine **coda_attesa); //funzione che prova a preparare l'ordine dato come parametro e lo inserisce nella rispettiva coda di ordini (pronti o in attesa)

void prepara_ordini_attesa(lotto* magazzino, ordine **testa_pronti, ordine **testa_attesa, ordine **coda_pronti, ordine **coda_attesa); //funzione che fa un check sugli ordini in attesa e prova a prepararli

int calcolo_peso(ordine **da_pesare); //funzione che calcola il peso di un ordine e lo restituisce

void svuota_strutture(lotto *magazzino, ricetta *ricettario, ordine **testa_pronti, ordine **testa_attesa); //funzione che alla fine di tutto libera le strutture dati

/* SWITCH CASES */

void aggiungi_ricetta(FILE *input, ricetta *ricettario);

void rimuovi_ricetta(FILE *input, ricetta *ricettario, ordine *ordini_pronti, ordine *ordini_attesa);

void rifornimento(FILE *input, lotto *magazzino, ordine **testa_pronti, ordine **testa_attesa, ordine **coda_pronti, ordine **coda_attesa);

void ordini(FILE *input, lotto *magazzino, ricetta *ricettario, ordine **testa_pronti, ordine **coda_pronti, ordine **testa_attesa, ordine **coda_attesa);

void camioncino(ordine **testa_pronti, ordine **coda_pronti, int capacita);

/* DEBUGGING */

void stampa_ricettario(ricetta *ricettario); //funzione per debugging, stampa i nomi di tutte le ricette con anche gli ingredienti e relative quantita

void stampa_ordini(ordine *testa); //funzione per debugging, stampa il nome della ricetta, la quantità ordinata e il tempo di ordinazione di tutti gli ordini della queue data in input

void stampa_magazzino(lotto *magazzino); //funzione per debugging, stampa tutti i lotti all'interno del magazzino

int main() {

    FILE *input = stdin; //fopen("test open/example.txt", "r"); //se scrivo stdin l'input sarà dalla mia tastiera, per testare al posto di stdin: fopen("test open/open_7.txt", "r");
    int peso_camion, periodo_camion, i; // i serve per inizializzare il ricettario e il magazzino, poi ci sono i due parametri del camion
    ricetta ricettario[DIM_HASH_RIC]; //tabella hash di ricette
    ordine *testa_ordini_pronti, *testa_ordini_attesa; //riferimenti ai primi elementi delle queue degli ordini
    ordine *coda_ordini_pronti, *coda_ordini_attesa; //riferimenti agli ultimi elementi delle queue degli ordini
    lotto magazzino[DIM_HASH_MAG]; //tabella hash che funge da magazzino dei lotti di ingredienti che arrivano
    char lettura[LUNG_PAR]; //mi serve solo per leggere il comando

    //inizializzo il tempo a 0 e i riferimenti delle queue a null
    tempo = 0;
    testa_ordini_pronti = NULL;
    coda_ordini_pronti = NULL;
    testa_ordini_attesa = NULL;
    coda_ordini_attesa = NULL;

    //inizializzo tutte le teste delle liste del ricettario e del magazzino a NULL (altrimenti sarebbero indefinite)
    for(i=0; i < DIM_HASH_RIC; i++) { 
        ricettario[i].next = NULL;
    }
    for(i=0; i < DIM_HASH_MAG; i++) {
        magazzino[i].next = NULL;
    }

    if(tempo == 0) {
        if(fscanf(input, "%d %d", &periodo_camion, &peso_camion) != 2) {
            printf("errore nella lettura dei parametri del camioncino");
        }
    }

    
    while(fscanf(input, "%s", lettura) != EOF) {

        if(tempo != 0 && tempo % periodo_camion == 0) {
            camioncino(&testa_ordini_pronti, &coda_ordini_pronti, peso_camion);
        }

        if(strcmp(lettura, "aggiungi_ricetta") == 0 ) {
            aggiungi_ricetta(input, ricettario);
            tempo++;
        }
        else if (strcmp(lettura, "rimuovi_ricetta") == 0) {
            rimuovi_ricetta(input, ricettario, testa_ordini_pronti, testa_ordini_attesa);
            tempo++;
        }
        else if (strcmp(lettura, "rifornimento") == 0) {
            rifornimento(input, magazzino, &testa_ordini_pronti, &testa_ordini_attesa, &coda_ordini_pronti, &coda_ordini_attesa);
            tempo++;
        }
        else if (strcmp(lettura, "ordine") == 0) {
            ordini(input, magazzino, ricettario, &testa_ordini_pronti, &coda_ordini_pronti, &testa_ordini_attesa, &coda_ordini_attesa);
            tempo++;
        }
        else {
            printf("comando non valido\n");
            tempo++;
        }
    }

    //è necessario anche un passaggio finale del camioncino dopo l'ultimo incremento del tempo (se tempo%periodo=0)
    if(tempo != 0 && tempo % periodo_camion == 0) {
            camioncino(&testa_ordini_pronti, &coda_ordini_pronti, peso_camion);
    }

    fclose(input);

    svuota_strutture(magazzino, ricettario, &testa_ordini_pronti, &testa_ordini_attesa);

    return 0;
}

int funzione_hash_ricettario (char *nome) {
    int n,i;
    n = 0;
    i = 0;

    while (nome[i] != '\0') {
        n = n + (int) nome[i];
        i++;
    }

    return n % DIM_HASH_RIC;
}

int funzione_hash_magazzino (char *nome) {
    int n,i;
    n = 0;
    i = 0;

    while (nome[i] != '\0') {
        n = n + (int) nome[i];
        i++;
    }

    return n % DIM_HASH_MAG;
}

void aggiunta_ricettario (ricetta *ricettario, ricetta *nuova, int hash) {
    ricetta *cursore;
    cursore = &ricettario[hash];
    nuova->next = cursore->next;
    cursore->next = nuova;
    return;
}

int ricerca_ricettario (ricetta *ricettario, int hash, char* nome_ricetta) {
    ricetta *cursore;
    cursore = ricettario[hash].next;
    while (cursore != NULL) {
        if(strcmp(cursore->nome,nome_ricetta) == 0) {
            return 1;
        }
        else {
            cursore = cursore->next;
        }
    }
    return 0;
}

ricetta* ricerca_ricettario_v2 (ricetta *ricettario, int hash, char* nome_ricetta) {
    ricetta *cursore;
    cursore = ricettario[hash].next;
    while (cursore != NULL) {
        if(strcmp(cursore->nome,nome_ricetta) == 0) {
            return cursore;
        }
        else {
            cursore = cursore->next;
        }
    }
    return NULL;
}

void rimuovi_ricettario (ricetta *ricettario, int hash, char* nome_ricetta) {
    ricetta *curs_ric, *prec_ric;
    ingrediente *curs_ingr, *el_ingr;
    curs_ric = ricettario[hash].next;
    prec_ric = NULL;

    if(strcmp(curs_ric->nome,nome_ricetta) == 0) {
        ricettario[hash].next = curs_ric->next;
        curs_ingr = curs_ric->ingredienti;
        while(curs_ingr != NULL) {
            el_ingr = curs_ingr;
            curs_ingr = curs_ingr->next;
            free(el_ingr);
        }
        free(curs_ric);
        return;
    }
    else {
        while(strcmp(curs_ric->nome,nome_ricetta) != 0) {
            prec_ric = curs_ric;
            curs_ric = curs_ric->next;
        }
        prec_ric->next = curs_ric->next;
        curs_ingr = curs_ric->ingredienti;
        while(curs_ingr != NULL) {
            el_ingr = curs_ingr;
            curs_ingr = curs_ingr->next;
            free(el_ingr);
        }
        free(curs_ric);
        return;
    }
}

void aggiunta_nome_ingrediente (char *parola, ricetta *nuova) {
    ingrediente *nuovo;
    nuovo = (ingrediente*) malloc(sizeof(ingrediente));
    strcpy(nuovo->nome, parola);
    nuovo->next = nuova->ingredienti;
    nuova->ingredienti = nuovo;
    return;
}

void aggiunta_quantita_ingrediente (int quantita, ricetta *nuova) {
    nuova->ingredienti->quantita = quantita;
    return;
}

int ricerca_ordine(char* nome_ricetta, ordine *testa) {
    ordine *cursore;
    cursore = testa;
    while (cursore != NULL) {
        if(strcmp(cursore->ricetta->nome, nome_ricetta) == 0) {
            return 1;
        }
        else {
            cursore = cursore->next;
        }
    }
    return 0;
}

void aggiunta_magazzino(lotto* magazzino, int hash, char* nome_ingrediente, int quantita, int scadenza) {
    lotto *cursore, *prec;
    cursore = magazzino[hash].next;
    prec = NULL;

    while(cursore != NULL) { //devo per forza scorrere tutta la lista per essere sicuro che non ci sia l'ingrediente (di qualsiasi scadenza). Se esco dal while confermo che non c'è e quindi inserisco in coda
        if(strcmp(cursore->nome, nome_ingrediente) == 0) { //se trovo almeno il nome ingrediente entro, altrimenti continuo a cercare andando avanti
            if(scadenza < cursore->scadenza) { //se l'ho trovato ma la mia scadenza è minore, creo un nuovo nodo e lo inserisco prima
                lotto *nuovo;
                nuovo = (lotto*) malloc(sizeof(lotto));
                strcpy(nuovo->nome, nome_ingrediente);
                nuovo->quantita = quantita;
                nuovo->scadenza = scadenza;
                nuovo->next = cursore;
                if(prec == NULL) { //inserimento in testa ad una lista non vuota
                    magazzino[hash].next = nuovo;
                }
                else {
                    prec->next = nuovo;
                }
                return;
            }
            else if(scadenza == cursore->scadenza) { //se la scadenza è la stessa aggiorno solo la quantità senza creare un nuovo nodo
                cursore->quantita = cursore->quantita + quantita;
                return;
            } 
            else {
                if(cursore->next == NULL || strcmp(cursore->next->nome, nome_ingrediente) != 0){ //se la scadenza è maggiore, ma dopo c'è un ingrediente diverso allora creo un nuovo nodo e lo inserisco dopo il cursore, ovvero il nuovo è quello a scadenza più lunga. Altrimenti scorro in avanti perché potrebbe esserci qualcuno di più grande
                    //devo creare un nuovo nodo e inserirlo dopo il cursore 
                    lotto *nuovo;
                    nuovo = (lotto*) malloc(sizeof(lotto));
                    strcpy(nuovo->nome, nome_ingrediente);
                    nuovo->quantita = quantita;
                    nuovo->scadenza = scadenza;
                    nuovo->next = cursore->next;
                    cursore->next = nuovo;
                    return;
                }
            }
        }
        prec = cursore;
        cursore = cursore->next;
    }
    //si arriva qua se l'ingrediente non è presente in qualsiasi scadenza o se la lista è vuota, inserisco rispettivamente in coda o in testa
    lotto *nuovo;
    nuovo = (lotto*) malloc(sizeof(lotto));
    strcpy(nuovo->nome, nome_ingrediente);
    nuovo->quantita = quantita;
    nuovo->scadenza = scadenza;
    nuovo->next = NULL;

    if(prec == NULL) { //inserisco in testa
        magazzino[hash].next = nuovo;
    }
    else { //inserisco in coda
        prec->next = nuovo;
    }

    return; 
}

void check_magazzino(lotto* magazzino) {
    int i;
    lotto *cursore, *prec, *elimina;

    for(i=0; i < DIM_HASH_MAG; i++) {
        if(magazzino[i].next != NULL) { //controllo la lista solamente se non è vuota, altrimenti passo avanti
            cursore = magazzino[i].next;
            prec = NULL;
            while(cursore != NULL) {
                if(cursore->scadenza <= tempo) {
                    if(prec == NULL) { //devo rimuovere il primo nodo della lista
                        magazzino[i].next = cursore->next;
                    }
                    else {
                        prec->next = cursore->next;
                    }
                    elimina = cursore;
                    cursore = cursore->next;
                    free(elimina);
                }
                else {
                    prec = cursore;
                    cursore = cursore->next;
                }
            }
        }
    }
    return;
}

void inserimento_ordine_coda(ordine *da_preparare, ordine** testa, ordine** coda) {
    if(*testa == NULL && *coda == NULL) { //la queue è vuota
        *testa = da_preparare;
        *coda = da_preparare;
    }
    else if(*testa != NULL && *coda == *testa) { //la queue ha un solo elemento
        (*testa)->next = da_preparare;
        *coda = da_preparare;
    }
    else { //la queue ha più di un elemento
        (*coda)->next = da_preparare;
        *coda = da_preparare;
    }

    da_preparare->next = NULL; //essendo inserimento alla fine non ha ordini dopo di esso

    return;
}

void prepara_ordine(lotto* magazzino, ordine *da_preparare, ordine **testa_pronti, ordine **coda_pronti, ordine **testa_attesa, ordine **coda_attesa) {
    ingrediente *curs_ingr; //cursore degli ingredienti nella ricetta
    lotto *curs_lott, *curs_lott_prec, *elimina; //cursori dei lotti nel magazzino
    int hash, quant_tot, quant_ris; //quant_tot è la quantità totale di un ingrediente nell'ordine (quantità in ricetta * quantità ordinata ricetta), quant_ris è invece la quantità di un ingrediente presente nel magazzino

    curs_ingr = da_preparare->ricetta->ingredienti;

    while(curs_ingr != NULL) { //se esco da questo while vuol dire che posso preparare l'ordine, vado ad eliminare la quantità richiesta dal magazzino e aggiungo l'ordine alla coda di ordini pronti (in coda)
        quant_tot = curs_ingr->quantita * da_preparare->quantita;
        quant_ris = 0; //altrimenti mi rimarrebbe salvato il valore dall'iterazione precedente
        hash = funzione_hash_magazzino(curs_ingr->nome); //calcolo la funzione hash sul nome dell'ingrediente per accedere alla giusta cella del magazzino
        curs_lott = magazzino[hash].next; 
        while(curs_lott != NULL && strcmp(curs_lott->nome, curs_ingr->nome) != 0) { //scorro la lista nel magazzino fino a che trovo l'ingrediente oppure finisce la lista
            curs_lott = curs_lott->next;
        }
        if(curs_lott == NULL) { //l'ingrediente non c'è nel magazzino, l'ordine va messo in coda a quelli in attesa
            inserimento_ordine_coda(da_preparare, testa_attesa, coda_attesa);
            return;
        }
        else { //ho trovato il primo nodo dell'ingrediente, controllo la quantità
            while(curs_lott != NULL && strcmp(curs_lott->nome, curs_ingr->nome) == 0 && quant_ris < quant_tot) { //calcolo la quantità dell'ingrediente che ho nel magazzino, continuo a sommarla fino a raggiungere (se posso) la quantità richiesta
                quant_ris = quant_ris + curs_lott->quantita;
                curs_lott = curs_lott->next;
            }
            if(quant_ris < quant_tot) { //non ho abbastanza quantità dell'ingrediente corrente per preparare tutto l'ordine, lo metto in attesa
                inserimento_ordine_coda(da_preparare, testa_attesa, coda_attesa);
                return;
            }
            else { //ho abbastanza quantità dell'ingrediente corrente per preparare l'ordine, vado avanti e verifico il prossimo ingrediente
                curs_ingr = curs_ingr->next;
            }
        }
    }
    
    inserimento_ordine_coda(da_preparare, testa_pronti, coda_pronti); //l'ordine è sicuramente il più recente, inserisco in fondo
    curs_ingr = da_preparare->ricetta->ingredienti;

    while(curs_ingr != NULL) { //scorro di nuovo gli ingredienti della ricetta per eliminare la quantità richiesta dal magazzino
        quant_tot = curs_ingr->quantita * da_preparare->quantita;
        hash = funzione_hash_magazzino(curs_ingr->nome);
        curs_lott = magazzino[hash].next;
        curs_lott_prec = NULL;
        while(strcmp(curs_lott->nome, curs_ingr->nome) != 0) { //vado a posizionarmi sul primo nodo nel magazzino dell'ingrediente corrente
            curs_lott_prec = curs_lott;
            curs_lott = curs_lott->next;
        }
        while(quant_tot > 0) { //finchè non ho tolto una quantità dal magazzino pari a quella richiesta dall'ordine vado avanti
            if(quant_tot >= curs_lott->quantita) { //esaurisco la quantità di tutto il nodo, vado ad eliminarlo
                quant_tot = quant_tot - curs_lott->quantita;
                if(curs_lott_prec == NULL) {
                    magazzino[hash].next = curs_lott->next;
                }
                else {
                    curs_lott_prec->next = curs_lott->next;
                }
                elimina = curs_lott;
                curs_lott = curs_lott->next;
                free(elimina);
            }
            else { //non esaurisco la quantità di tutto il nodo, non devo eliminarlo ma semplicemente dimuirne la quantità
                curs_lott->quantita = curs_lott->quantita - quant_tot;
                quant_tot = 0; //in modo da poter uscire dal while
            }
        }
        curs_ingr = curs_ingr->next;
    }

    return;
}

void prepara_ordini_attesa(lotto* magazzino, ordine **testa_pronti, ordine **testa_attesa, ordine **coda_pronti, ordine **coda_attesa) {
    ordine *curs_ord, *curs_ord_prec, *da_inserire, *curs_ord_pronti, *curs_ord_pronti_prec;
    ingrediente *curs_ingr;
    lotto *curs_lott, *curs_lott_prec, *elimina;
    int hash, quant_tot, quant_ris, non_preparabile, inserito; //se non_preparabile vale 1 l'ordine non è preparabile e posso evitare di controllare eventuali altri ingredienti della ricetta. inserito vale 1 dopo che ho inserito l'ordine nella coda degli ordini pronti

    curs_ord = *testa_attesa;
    curs_ord_prec = NULL;

    while(curs_ord != NULL) { //devo scorrere tutti gli ordini in attesa e provare a prepararli
        non_preparabile = 0; //valore di default che si resetta ad ogni nuovo controllo di ordine
        curs_ingr = curs_ord->ricetta->ingredienti; 
        while(curs_ingr != NULL && non_preparabile != 1) {
            quant_tot = curs_ingr->quantita * curs_ord->quantita;
            quant_ris = 0;
            hash = funzione_hash_magazzino(curs_ingr->nome);
            curs_lott = magazzino[hash].next;
            while(curs_lott != NULL && strcmp(curs_lott->nome, curs_ingr->nome) != 0) {
                curs_lott = curs_lott->next;
            }
            if(curs_lott == NULL) {
                non_preparabile = 1;
            }
            else {
                while(curs_lott != NULL && strcmp(curs_lott->nome, curs_ingr->nome) == 0 && quant_ris < quant_tot) {
                    quant_ris = quant_ris + curs_lott->quantita;
                    curs_lott = curs_lott->next;
                }
                if(quant_ris < quant_tot) {
                    non_preparabile = 1;
                }
                else {
                    curs_ingr = curs_ingr->next;
                }
            }
        }
        if(non_preparabile != 1) { //posso preparare l'ordine, vado a togliere gli ingredienti dal magazzino e poi lo sposto nella coda degli ordini pronti (in maniera ordinata)
            curs_ingr = curs_ord->ricetta->ingredienti;
            while(curs_ingr != NULL) {
                quant_tot = curs_ingr->quantita * curs_ord->quantita;
                hash = funzione_hash_magazzino(curs_ingr->nome);
                curs_lott = magazzino[hash].next;
                curs_lott_prec = NULL;
                while(curs_lott != NULL && strcmp(curs_lott->nome, curs_ingr->nome) != 0) {
                    curs_lott_prec = curs_lott;
                    curs_lott = curs_lott->next;
                }
                while(quant_tot > 0 && curs_lott != NULL) {
                    if(quant_tot >= curs_lott->quantita) {
                        quant_tot = quant_tot - curs_lott->quantita;
                        if(curs_lott_prec == NULL) {
                            magazzino[hash].next = curs_lott->next;
                        }
                        else {
                            curs_lott_prec->next = curs_lott->next;
                        }
                        elimina = curs_lott;
                        curs_lott = curs_lott->next;
                        free(elimina);
                    }       
                    else {
                        curs_lott->quantita = curs_lott->quantita - quant_tot;
                        quant_tot = 0;
                    }
                }
                curs_ingr = curs_ingr->next;
            }
            //nella parte precedente ho tolto gli ingredienti dal magazzino, ora devo spostare l'ordine da in attesa a pronto (in maniera ordinata)
            if(curs_ord_prec == NULL) { //l'ordine da rimuovere era la testa, aggiorno la testa
                if(*testa_attesa == *coda_attesa) { //la lista aveva un solo elemento
                    *testa_attesa = NULL;
                    *coda_attesa = NULL;
                }
                else if ((*testa_attesa)->next == *coda_attesa) { //la lista aveva due elementi
                    *testa_attesa = curs_ord->next;
                    (*testa_attesa)->next = NULL;
                    *coda_attesa = curs_ord->next;
                    (*coda_attesa)->next = NULL;
                }
                else { //la lista aveva più di due elementi
                    *testa_attesa = curs_ord->next;
                    (*testa_attesa)->next = curs_ord->next->next;
                }
            }
            else { //l'ordine da rimuovere non era la testa, aggiorno solo il puntatore al precedente
                curs_ord_prec->next = curs_ord->next;
                if(curs_ord == *coda_attesa && (*testa_attesa)->next == curs_ord) { //se ho tolto l'ultimo ordine e la lista aveva due elementi, mi rimane un solo elemento che diventerà sia testa che coda
                    *coda_attesa = curs_ord_prec;
                    *testa_attesa = curs_ord_prec;
                    (*testa_attesa)->next = NULL;
                    (*coda_attesa)->next = NULL;
                }
                else if(curs_ord == *coda_attesa && (*testa_attesa)->next != curs_ord) { //se ho tolto l'ultimo ordine e la lista aveva più di due elementi, aggiorno solo il riferimento della coda (il caso di lista con un solo elemento viene gestito prima perchè sarà sia testa che coda)
                    *coda_attesa = curs_ord_prec;
                }
                else {} //non ho tolto l'ultimo ordine, non devo fare nulla
            }
            da_inserire = curs_ord; //mi salvo il riferimento all'ordine che ho tolto da quelli in attesa, andrò ad inserirlo in quelli pronti
            curs_ord = curs_ord->next; //per scorrere il successivo ordine in attesa nell'iterazione successiva del while più esterno
            curs_ord_pronti = *testa_pronti; //cursore per scorrere gli ordini pronti in modo da trovare la posizione corretta per l'inserimento
            curs_ord_pronti_prec = NULL;
            inserito = 0; //resetto il valore ad ogni nuovo ordine che devo inserire
            while(inserito != 1) {
                if(curs_ord_pronti == NULL) { 
                    if(curs_ord_pronti_prec == NULL) { //se la coda degli ordini pronti è vuota, inserisco subito
                        *testa_pronti = da_inserire;
                        *coda_pronti = da_inserire;
                        da_inserire->next = NULL;
                        inserito = 1;
                    }
                    else { //questo invece è il caso in cui arrivo fino all'ultimo ordine, ovvero devo inserire in coda
                        curs_ord_pronti_prec->next = da_inserire;
                        da_inserire->next = NULL;
                        *coda_pronti = da_inserire; //aggiorno il riferimento della coda degli ordini pronti
                        inserito = 1;
                    }
                }
                else if (da_inserire->tempo_ordinazione < curs_ord_pronti->tempo_ordinazione) { //se l'ordine da inserire è più vecchio del cursore lo inserisco prima
                    if(curs_ord_pronti_prec == NULL) { //devo inserire in testa e la lista non è vuota
                        *testa_pronti = da_inserire;
                        da_inserire->next = curs_ord_pronti;
                        inserito = 1;
                    }
                    else {
                        curs_ord_pronti_prec->next = da_inserire;
                        da_inserire->next = curs_ord_pronti;
                        inserito = 1;
                    }
                }
                else { //l'ordine da inserire è stato ordinato dopo quello puntato dal cursore, scorro in avanti il cursore
                        curs_ord_pronti_prec = curs_ord_pronti;
                        curs_ord_pronti = curs_ord_pronti->next;
                }
            }
        }
        else { //non posso preparare l'ordine, non devo fare niente se non spostarmi sull'ordine in attesa successivo
            curs_ord_prec = curs_ord;
            curs_ord = curs_ord->next;
        }
    }

    return;
}

int calcolo_peso(ordine **da_pesare) {
    ingrediente *cursore;
    int peso;

    cursore = (*da_pesare)->ricetta->ingredienti;
    peso = 0;

    while(cursore != NULL) {
        peso = peso + (cursore->quantita * (*da_pesare)->quantita);
        cursore = cursore->next;
    }

    (*da_pesare)->peso_totale = peso;
    return peso;
}

void svuota_strutture(lotto *magazzino, ricetta *ricettario, ordine **testa_pronti, ordine **testa_attesa) {
    int i;
    ricetta *curs_ric, *el_ric;
    ingrediente *curs_ingr, *el_ingr;
    lotto *curs_lott, *el_lott;
    ordine *curs_ord, *el_ord;

    for(i=0; i < DIM_HASH_RIC; i++) {
        if(ricettario[i].next != NULL) {
            curs_ric = ricettario[i].next;
            while(curs_ric != NULL) {
                curs_ingr = curs_ric->ingredienti;
                while(curs_ingr != NULL) {
                    el_ingr = curs_ingr;
                    curs_ingr = curs_ingr->next;
                    free(el_ingr);
                }
                el_ric = curs_ric;
                curs_ric = curs_ric->next;
                free(el_ric);
            }
        }
    }

    for(i=0; i < DIM_HASH_MAG; i++) {
        if(magazzino[i].next != NULL) {
            curs_lott = magazzino[i].next;
            while(curs_lott != NULL) {
                el_lott = curs_lott;
                curs_lott = curs_lott->next;
                free(el_lott);
            }
        }
    }

    curs_ord = *testa_pronti;
    while(curs_ord != NULL) {
        el_ord = curs_ord;
        curs_ord = curs_ord->next;
        free(el_ord);
    }

    curs_ord = *testa_attesa;
    while(curs_ord != NULL) {
        el_ord = curs_ord;
        curs_ord = curs_ord->next;
        free(el_ord);
    }

    return;
}

void aggiungi_ricetta(FILE *input, ricetta *ricettario) {
    int hash, bool, pari, i, quantita; // hash è la funzione di hash calcolata sul nome della ricetta, bool vale 1 se la ricetta è già nel ricettario altrimenti 0, pari vale 1 se devo leggere un ingrediente mentre 0 se devo leggerne la quantità, i è l'indice per scorrere l'array di lettura, quantità è la variabile su cui metto la quantità trasformata in intero
    char nome_ricetta[LUNG_PAR], letto, spazio, parola[LUNG_PAR]; //letto è il singolo carattere che leggo mentre parola è il nome intero dell'ingrediente o la quantità di tale

    pari = 1;
    i = 0;
    quantita = 0;
    
    if(fscanf(input,"%s", nome_ricetta) != 1) {
        printf("errore in lettura");
    }
    hash = funzione_hash_ricettario(nome_ricetta);
    bool = ricerca_ricettario(ricettario, hash, nome_ricetta);
    if(bool == 1) {
        if(fscanf(input, "%c", &letto) != 1) {
            printf("errore in lettura");
        }
        while (letto != '\n') {
            if(fscanf(input, "%c", &letto) != 1) {
                printf("errore in lettura");
            }
        }
        printf("ignorato\n");
        return;
    }
    else {
        ricetta *nuova;
        nuova = (ricetta*) malloc(sizeof(ricetta));
        nuova->ingredienti = NULL;
        strcpy(nuova->nome, nome_ricetta);
        if(fscanf(input, "%c", &spazio) != 1) {
        printf("errore in lettura");
        }
        if(fscanf(input, "%c", &letto) != 1) {
        printf("errore in lettura");
        }      
        while(letto != '\n') {
            while(letto != spazio && letto != '\n') {
                parola[i] = letto;
                if(fscanf(input, "%c", &letto) != 1){
                    printf("errore in lettura");
                }
                i++;
            }
            parola[i] = '\0';
            if (pari == 1){
                pari = 0;
                aggiunta_nome_ingrediente(parola, nuova);
            }
            else {
                pari = 1;
                quantita = atoi(parola);
                aggiunta_quantita_ingrediente(quantita, nuova);
            }
            if(letto != '\n') {
                if(fscanf(input, "%c", &letto) != 1){
                printf("errore in lettura");
                }
            }
            i = 0;
        }
        aggiunta_ricettario(ricettario, nuova, hash);
        printf("aggiunta\n");
        return;
    }
}

void rimuovi_ricetta(FILE *input, ricetta *ricettario, ordine *ordini_pronti, ordine *ordini_attesa) {
    char nome_ricetta[LUNG_PAR];
    int hash, bool_1, bool_2;

    if(fscanf(input,"%s", nome_ricetta) != 1) {
        printf("errore in lettura");
    }
    hash = funzione_hash_ricettario(nome_ricetta);
    bool_1 = ricerca_ricettario(ricettario, hash, nome_ricetta);
    if(bool_1 == 0) {
        printf("non presente\n");
        return;
    }
    else {
        bool_1 = ricerca_ordine(nome_ricetta, ordini_pronti);
        if (bool_1 == 1) {
            printf("ordini in sospeso\n");
            return;
        }
        else {
            bool_2 = ricerca_ordine(nome_ricetta, ordini_attesa);
            if (bool_2 == 1) {
                printf("ordini in sospeso\n");
                return;
            }
            else {
                rimuovi_ricettario(ricettario, hash, nome_ricetta);
                printf("rimossa\n");
                return;
            }
        }
    }
}

void rifornimento(FILE *input, lotto *magazzino, ordine **testa_pronti, ordine **testa_attesa, ordine **coda_pronti, ordine **coda_attesa) {
    int hash, quantita, scadenza, i, basetre; // hash è la funzione di hash calcolata sul nome dell'ingrediente, basetre serve per direzionare il caso in cui sto leggendo il nome dell'ingrediente (=0) o la quantità (=1) o la scadenza (=2), i è l'indice per scorrere l'array di lettura, quantità è la variabile su cui metto la quantità trasformata in intero, scadenza è la variabile su cui metto la scadenza trasformata in intero
    char parola[LUNG_PAR], nome_ingrediente[LUNG_PAR], spazio, letto; //letto è il singolo carattere che leggo mentre parola è il nome intero dell'ingrediente o la quantità o la scadenza di tale

    quantita = 0;
    scadenza = 0;
    i = 0;
    basetre = 0; //la prima cosa che leggo è il nome dell'ingrediente

    if(fscanf(input, "%c", &spazio) != 1) {
        printf("errore in lettura");
    }
    if(fscanf(input, "%c", &letto) != 1) {
        printf("errore in lettura");
    }
    while(letto != '\n') {
        while(letto != spazio && letto != '\n') {
            parola[i] = letto;
            if(fscanf(input, "%c", &letto) != 1) {
                printf("errore in lettura");
            }
            i++;
        }
        parola[i] = '\0';
        if (basetre == 0) { //ho letto il nome dell'ingrediente, dopo leggerò la quantità
            basetre = 1;
            strcpy(nome_ingrediente, parola);
        }
        else if (basetre == 1) { //ho letto la quantità, dopo leggerò la scadenza
            basetre = 2;
            quantita = atoi(parola);
        }
        else { //ho letto la scadenza, provo ad aggiungere il lotto al magazzino
            basetre = 0;
            scadenza = atoi(parola);
            if (scadenza > tempo) { //il lotto non è già scaduto, devo inserirlo nel magazzino
                hash = funzione_hash_magazzino(nome_ingrediente);
                aggiunta_magazzino(magazzino, hash, nome_ingrediente, quantita, scadenza);
            }
        }
        if (letto != '\n') {
            if(fscanf(input, "%c", &letto) != 1) {
                printf("errore in lettura");
            }
        }
        i = 0;
    }
    check_magazzino(magazzino);
    prepara_ordini_attesa(magazzino, testa_pronti, testa_attesa, coda_pronti, coda_attesa);
    printf("rifornito\n");

    return;
}

void ordini(FILE *input, lotto *magazzino, ricetta *ricettario, ordine **testa_pronti, ordine **coda_pronti, ordine **testa_attesa, ordine **coda_attesa) {
    int quantita, hash;
    char nome_ricetta[LUNG_PAR], spazio;
    ricetta *riferimento;

    if(fscanf(input, "%s", nome_ricetta) != 1) { //leggo il nome della ricetta
        printf("errore in lettura");
    }
    if(fscanf(input, "%c", &spazio) != 1) { //leggo lo spazio tra nome ricetta e quantità ordinata
        printf("errore in lettura");
    }
    if(fscanf(input, "%d", &quantita) != 1) { //leggo la quantità ordinata
        printf("errore in lettura");
    }
    hash = funzione_hash_ricettario(nome_ricetta);
    riferimento = ricerca_ricettario_v2(ricettario, hash, nome_ricetta); //ricerco la ricetta nel ricettario, se viene trovata ho direttamente il riferimento da poter inserire nell'ordine
    if(riferimento != NULL) { //ricetta trovata, posso accettare l'ordine
        printf("accettato\n");
        ordine *nuovo;
        nuovo = (ordine*) malloc(sizeof(ordine));
        nuovo->ricetta = riferimento;
        nuovo->quantita = quantita;
        nuovo->tempo_ordinazione = tempo;
        nuovo->peso_totale = 0;
        nuovo->next = NULL;
        check_magazzino(magazzino);
        prepara_ordine(magazzino, nuovo, testa_pronti, coda_pronti, testa_attesa, coda_attesa);
    }
    else { //ricetta non trovata, devo rifiutare l'ordine
        printf("rifiutato\n");
    }
    return;
}

void camioncino(ordine **testa_pronti, ordine **coda_pronti, int capacita) {
    ordine *curs_ord, *camion, *curs_cam, *prec_cam, *da_inserire, *da_eliminare, *cursore;
    int pieno, peso, caricato; //pieno è un flag che mi fa uscire dal while quando il camioncino è riempito al massimo, caricato è un flag che mi fa uscire dal while quando ho caricato l'ordine nel camioncino

    curs_ord = *testa_pronti;
    camion = NULL;
    pieno = 0;

    while(curs_ord != NULL && pieno != 1) {
        peso = calcolo_peso(&curs_ord);
        if(peso <= capacita) { //posso caricare l'ordine nel camion
            capacita = capacita - peso;
            if(*coda_pronti == *testa_pronti) { //la lista ha un solo elemento, quindi diventerà vuota
                *testa_pronti = NULL;
                *coda_pronti = NULL;
            }
            else if(*coda_pronti == (*testa_pronti)->next) { //la lista ha solo due elementi, sto rimuovendo il primo
                *testa_pronti = curs_ord->next;
                (*testa_pronti)->next = NULL;
                *coda_pronti = curs_ord->next;
                (*coda_pronti)->next = NULL;
            }
            else { //la lista ha più di due elementi, devo aggiornare solo il riferimento alla testa
                *testa_pronti = curs_ord->next;
            }
            da_inserire = curs_ord;
            curs_cam = camion;
            prec_cam = NULL;
            caricato = 0;
            while(caricato != 1) {
                if(camion == NULL) { //il camion è ancora vuoto, inserisco subito 
                    camion = da_inserire;
                    da_inserire->next = NULL;
                    caricato = 1;
                }
                else {
                    if(curs_cam == NULL) { //ho fatto lo scorrimento di tutti gli ordini del camion, inserisco in coda
                        prec_cam->next = da_inserire;
                        da_inserire->next = NULL;
                        caricato = 1;
                    }
                    else if(da_inserire->peso_totale > curs_cam->peso_totale || (da_inserire->peso_totale == curs_cam->peso_totale && da_inserire->tempo_ordinazione < curs_cam->tempo_ordinazione)) { //devo inserire in questa posizione
                        if(prec_cam == NULL) { //inserisco in testa
                            da_inserire->next = curs_cam;
                            camion = da_inserire;
                        }
                        else {
                            prec_cam->next = da_inserire;
                            da_inserire->next = curs_cam;
                        }
                        caricato = 1;
                    }
                    else { //ancora non ho trovato la posizione giusta, scorro il cursore del camioncino
                        prec_cam = curs_cam;
                        curs_cam = curs_cam->next;
                    }
                }
            }
            curs_ord = *testa_pronti;              
        }
        else { //non posso caricare l'ordine nel camion perché il prossimo ordine non ci sta dentro
            pieno = 1;
        }
    }

    if(camion == NULL) { //il camioncino è vuoto
        printf("camioncino vuoto\n");
    }
    else { //il camioncino non è vuoto, stampo gli ordini che ho caricato e poi libero il camioncino
        cursore = camion;
        while(cursore != NULL) {
            printf("%d %s %d\n", cursore->tempo_ordinazione, cursore->ricetta->nome, cursore->quantita);
            da_eliminare = cursore;
            cursore = cursore->next;
            free(da_eliminare);
        }

    }

    return;
}

void stampa_ricettario(ricetta *ricettario) {
    int i;
    ricetta *cursore_1;
    ingrediente *cursore_2;

    for(i=0; i < DIM_HASH_RIC; i++) {
        cursore_1 = ricettario[i].next;
        while (cursore_1 != NULL) {
            printf("nome: %s | ", cursore_1->nome);
            cursore_2 = cursore_1->ingredienti;
            while(cursore_2 != NULL) {
                printf("%s %d | ", cursore_2->nome, cursore_2->quantita);
                cursore_2 = cursore_2->next;
            }
            cursore_1 = cursore_1->next;
            printf("\n");
        }
    }
    return;
}

void stampa_ordini(ordine *testa) {
    ordine *cursore;
    cursore = testa;
    while(cursore != NULL) {
        printf("%s %d %d | ", cursore->ricetta->nome, cursore->quantita, cursore->tempo_ordinazione);
        cursore = cursore->next;
    }
    printf("\n");
    return;
}

void stampa_magazzino(lotto *magazzino) {
    int i;
    lotto *cursore;

    for(i=0; i < DIM_HASH_MAG; i++) {
        cursore = magazzino[i].next;
        if (cursore != NULL) {
            while(cursore != NULL) {
                printf("nome: %s qta: %d scad: %d | ", cursore->nome, cursore->quantita, cursore->scadenza);
                cursore = cursore->next;
            }
            printf("\n");
        }
    }

    return;
}