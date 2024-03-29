#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "estructuras.h"
#include "funciones.h"


//Variables globales
Hebra** hebra_array = NULL;
Matriz* matriz = NULL;
pthread_mutex_t lockPrint;
int dflag = 0; //bandera

//ENTRADA: Void (nada)
//SALIDA: Entero
//Función se encarga de obtener y retornar el valor del id de la hebra. 
int getIndex(){
    int index;
    int i=0;
    while(hebra_array[i]!=NULL){
        if(hebra_array[i]->thread == pthread_self()){
            index = i;
        }
        i++;
    }
    return index;
}

//ENTRADA: Argumentos casteados
//SALIDA: void (Nada)
//Se encarga de que la hebra que llame a esta función ubique una palabra
void* ubicar(void* args){
    int index=getIndex();
    int i;
    int not_next;
    Position pos;
    char buffer[250];
    int counter;
    int e;
    int status;
    for (i=0;i<hebra_array[index]->int_words;i++){
        not_next = 1;
        counter = 0;
        while(not_next){
            status = 1;
            position_rand(&pos,matriz->row,matriz->col);
            for(e=0;e<strlen(hebra_array[index]->words[i]);e++){
                if(pos.col+e>=matriz->col){
                    status = 0;
                    break;
                }
                pthread_mutex_lock(&matriz->locks[pos.row][pos.col+e]);
                if((matriz->data[pos.row][pos.col+e]) != (' ')){
                    status = 0;
                    break;   
                }
            }
            if (status == 1){
                    memset(buffer,0,250);
                    strcpy(buffer,hebra_array[index]->words[i]);
                    string_upper(buffer);
                    insert_word(matriz,buffer,pos);
                    not_next = 0;
                if(dflag == 1){
                    pthread_mutex_lock(& lockPrint);
                    printf("Hebra: %d\n",index);
                    printf("Insertando palabra: %s\n",buffer);
                    position_show(pos);
                    matriz_show(matriz); 
                    printf("\n\n");  
                    pthread_mutex_unlock(&lockPrint);
                }

            }
            while(e>=0){
                pthread_mutex_unlock(&matriz->locks[pos.row][pos.col+e]);
                e--; 
            }
            if(counter == (matriz->row*matriz->col)*50){
                not_next =0;
            }
            counter ++;
        }
    }
    void* ret = NULL;
    return ret;
}


int main(int argc, char** argv){

    char ivalue[300]; //nombre archivo entrada
    int hvalue; //cantidad de hebras
    int cvalue; //cantidad de palabras
    int nvalue; //ancho matriz
    int mvalue; //largo matriz
    char svalue[300]; //nombre archivo salida

    if(!opt_get( argc, argv, ivalue,&hvalue,&cvalue,&nvalue,&mvalue,svalue,&dflag))
        return 0;  
   
    if(verifyArguments(ivalue,hvalue,cvalue,nvalue,mvalue,svalue) == 0) 
        return 0;


    pthread_mutex_init(&lockPrint, NULL);
    srand(time(NULL));
    
    /*
    int hvalue=hvalue;
    int mvalue = mvalue;
    int nvalue= nvalue;
    int cvalue = cvalue;
    */  


    //if(hvalue > cvalue)
      //  hvalue = cvalue;
    hebra_array = hebra_array_init(hvalue,cvalue,ivalue);
    if(dflag == 1){
        hebra_array_show(hebra_array);
    }
    
    matriz = matriz_create(mvalue,nvalue);
    int i;

    for(i=0;i<hvalue;i++){
        pthread_create(&hebra_array[i]->thread , NULL , ubicar , NULL );
    }
    for(i=0;i<hvalue;i++){
        pthread_join ( hebra_array[i]->thread , NULL ) ;
    } 

    FILE* archivoSalida = fopen(svalue,"w");
    matriz_fill(matriz);
    if(dflag == 1){
        printf("Matriz final: \n");
        matriz_show(matriz);
    }
    matriz_print(matriz, archivoSalida);
    matriz_destroy(matriz);
    hebra_array_destroy(hebra_array);
    
    return 1;
}
