//Enrique Martin
//Fernando San Jose  

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

//Datos a compartir
int sig_llenar=0;
int sig_vaciar=0;
int tam_buffer;
int numnaves=0;
int contok=0;
int u;
char *B=NULL;
int *V=NULL;
FILE *fichOutput=NULL;


//Semaforos
sem_t hay_espacio;
sem_t hay_dato;
sem_t mutex_s_vaciar;
sem_t mutex_vector;

void *Disparador(void* arg){
    char a;
    int index=0;
    int contval=0;
    int continval=0;

    FILE* fichInput=NULL;
    fichInput = fopen((char*)arg, "r");

    //valida el fichero de entrada
    if(fichInput == NULL){
                printf("El archivo de entrada esta vacio\n");
                return -1;
    }

    while((a=fgetc(fichInput))!=EOF){
    contok++;
    if(a=='*'| a==' '){

        sem_wait(&hay_espacio);
        B[index]=a;
        index=(index+1)%tam_buffer;
        contval++;
        sem_post(&hay_dato);

    }else if(a=='b'){
        a=fgetc(fichInput);
        sem_wait(&hay_espacio);
        B[index]=a;
        index=(index+1)%tam_buffer;
        contval++;
        sem_post(&hay_dato);

    }else{continval++;}
    }
fclose(fichInput);

    sem_wait(&hay_espacio);
    B[index]='f';
    sem_post(&hay_dato);

    fprintf(fichOutput,"El numero de tokens validos procesados es %i",contval);
    fflush(stdout);
    fprintf(fichOutput,", El numero de tokens invalidos procesados es %i",continval);
    fflush(stdout);
    fprintf(fichOutput,", El numero de tokens procesados es %i\n ",contok);
    fflush(stdout);

    pthread_exit(NULL);

}

void *Nave(void* i){

    int *nave=(int*)i;
    int k=0;
    int contdispa=0;
    int contdispfall=0;
    int contbot=0;
    char e;

    int v=0;
    u=v;

    while(1){

    sem_wait(&hay_dato);
    sem_wait(&mutex_s_vaciar);
    k=sig_vaciar;
    e=B[k];

    if (e=='*'){
       sig_vaciar=(sig_vaciar+1)%tam_buffer;
       sem_post(&mutex_s_vaciar);
       contdispa++;
       sem_post(&hay_espacio);
}else if(e==' '){
        sig_vaciar=(sig_vaciar+1)%tam_buffer;
        sem_post(&mutex_s_vaciar);
        contdispfall++;
        sem_post(&hay_espacio);
}else if(e=='1'){
        sig_vaciar=(sig_vaciar+1)%tam_buffer;
        B[k]=B[k]-1;
        sem_post(&mutex_s_vaciar);
        contbot++;
        sem_post(&hay_espacio);
}else if(e=='2' || e=='3'){
        B[k]=B[k]-1;
        sem_post(&mutex_s_vaciar);
        contbot++;
        sem_post(&hay_dato);
}else if(e=='f'){
                sem_post(&mutex_s_vaciar);
                sem_post(&hay_dato);
                break;
       }
    }

///Guardar los elementos en el vector

    sem_wait(&mutex_vector);

    v=sig_llenar;
    sig_llenar=(sig_llenar+1)%(tam_buffer);
    V[v]=contdispa;

    v=sig_llenar;
    sig_llenar=(sig_llenar+1)%(tam_buffer);
    V[v]=contdispfall;

    v=sig_llenar;
    sig_llenar=(sig_llenar+1)%(tam_buffer);
    V[v]=contbot;

   v=sig_llenar;
   sig_llenar=(sig_llenar+1)%(tam_buffer);
   V[v]=*nave;

   sem_post(&mutex_vector);
    pthread_exit(NULL);
}




int main(int argc,char* argv []){

if(argc!=5){
    printf("Numero de parametros incorrecto\n");
    return -1;
}
 FILE* fichInput=NULL;
     fichInput = fopen((char*)argv[1], "r");

//valida el fichero de entra
if(fichInput == NULL){
    printf("El archivo de entrada esta vacio\n");
        return -1;
}

//asignamos tamaño al buffer circular
if((sscanf(argv[3], "%d",&tam_buffer)!=1)){
    printf("Dimension de buffer no valido\n");
        return -1;
}
if(tam_buffer<1){
        printf("El tamaño de buffer debe ser mayor o igual a 1\n");
        return -1;
}
B=(char*)malloc(tam_buffer*(sizeof(char)));

if(B==NULL){
printf("No se ha podido reservar memoria para el Buffer\n");
return -1;
}

//fichero de salida
 fichOutput = fopen(argv[2], "w");
//valida el fichero de salida
if(fichOutput == NULL){
    printf("El archivo de salida esta vacio\n");
    return -1;
}

//valida el argumento 4 del programa (numero de naves)
if(sscanf(argv[4], "%d", &numnaves) != 1){
    printf("Numero de Naves no valido\n");
    return -1;
}
if(numnaves<1){
        printf("El numero de naves debe ser mayor o igual a 1\n");
        return -1;
}
//Reservamos el espacio para el vector
 V=(int*)malloc(sizeof(int)*tam_buffer);
      if(V==NULL){
           printf("No ha sido posible reservar memoria para el vector");
           return -1;
}

//inicializacion de los semaforos
sem_init(&hay_espacio,0,tam_buffer);
sem_init(&hay_dato,0,0);
sem_init(&mutex_s_vaciar,0,1);
sem_init(&mutex_vector,0,1);

//definicion de los hilos
pthread_t Disparador_h, *Nave_h;

//Reserva espacio para el vector de hilos
Nave_h=(pthread_t*)malloc(sizeof(pthread_t)*numnaves);
if(Nave_h==NULL){
        printf("No ha sido posible reservar espacio para el vector de hilos");
        return -1;
}

//creamos los hilos para los procesos
pthread_create(&Disparador_h,NULL,Disparador,(void*)argv[1]);

//Vector que indicara el numero de nave
int *I;
I=(int*)malloc(sizeof(int)*numnaves);
if(I==NULL){
        printf("No ha sido posible reservar memoria para el vector que indica el numero de nave");
        return -1;
}

for(int i=0;i<numnaves;i++){
        I[i]=i;
        pthread_create(&Nave_h[i],NULL,Nave,(void*)&I[i]);
}

//Ambos procesos ejecutandose, main espera hasta que acaben
pthread_join(Disparador_h,NULL);

for(int i=0;i<numnaves;i++){
    pthread_join(Nave_h[i],NULL);
}

//Se destruyen los semaforos

sem_destroy(&hay_espacio);
sem_destroy(&hay_dato);
sem_destroy(&mutex_s_vaciar);
sem_destroy(&mutex_vector);

//Empieza el juez
int disparosAcertados=0;
int disparosFallados =0;
int botiquines =0;
int total=0;
//int disparos = 0;i
int nave =0;
int minimo_ganadora= contok;
int posicion_ganadora=0;
int minimo_subcampeona=contok;
int posicion_subcampeona=0;
int disparos_recibidost=0;
int disparos_fallidost=0;
int botiquines_t=0;
int tokens_t=0;

int posicion =u ;
int cont =0;
int i=0;

while(cont<numnaves){
i = posicion;
posicion=(posicion+1)%(tam_buffer);
disparosAcertados=V[i];
i = posicion;
posicion=(posicion+1)%(tam_buffer);
disparosFallados=V[i];
i = posicion;
posicion=(posicion+1)%(tam_buffer);
botiquines=V[i];
i = posicion;
posicion=(posicion+1)%(tam_buffer);
nave=V[i];

total= disparosAcertados- botiquines;
disparos_recibidost = disparos_recibidost + disparosAcertados;
disparos_fallidost =disparos_fallidost + disparosFallados;
botiquines_t = botiquines_t + botiquines;

//tengo que ver si tienen datos
if(disparosAcertados>0 || disparosFallados>0 || botiquines >0){
//miro el minimo total para ver la nave ganadora
if(total<minimo_ganadora){
minimo_subcampeona = minimo_ganadora;
posicion_subcampeona = posicion_ganadora;
minimo_ganadora = total;
posicion_ganadora = i;
}
//miro el minimo total para ver la nave subcampeona

if(minimo_ganadora<total && total<minimo_subcampeona ){
minimo_subcampeona = total;
posicion_subcampeona=i;
}
}

fprintf(fichOutput,"Nave: %i \n", nave);
fflush(stdout);
fprintf(fichOutput,"El numero de disparos acertados es %i \n",disparosAcertados);
fflush(stdout);
fprintf(fichOutput,"El numero de disparos fallados es %i \n",disparosFallados);
fflush(stdout);
fprintf(fichOutput,"El numero de botiquines es %i \n",botiquines);
fflush(stdout);
fprintf(fichOutput,"El total es %i \n",total);
fflush(stdout);
cont++;
}

//imprimo nave ganadora
i =posicion_ganadora;
posicion = posicion_ganadora;
posicion=(posicion-1)%(tam_buffer);
nave=V[i];
i = posicion;
posicion=(posicion-1)%(tam_buffer);
botiquines=V[i];
i = posicion;
posicion=(posicion-1)%(tam_buffer);
disparosFallados=V[i];
i = posicion;
posicion=(posicion-1)%(tam_buffer);
disparosAcertados=V[i];
total = disparosAcertados-botiquines;
fprintf(fichOutput,"Nave ganadora: %d \n",nave);
fflush(stdout);
fprintf(fichOutput,"El numero de disparos acertados es %d \n",disparosAcertados);
fflush(stdout);
fprintf(fichOutput,"El numero de disparos fallados es %d \n",disparosFallados);
fflush(stdout);
fprintf(fichOutput,"El numero de botiquines es %d \n",botiquines);
fflush(stdout);
fprintf(fichOutput,"El total es %d \n",total);
fflush(stdout);

//imprimo nave subcampeona
i = posicion_subcampeona;
posicion=posicion_subcampeona;
posicion=(posicion-1)%(tam_buffer);
nave=V[i];
i = posicion;
posicion=(posicion-1)%(tam_buffer);
botiquines=V[i];
i = posicion;
posicion=(posicion-1)%(tam_buffer);
disparosFallados=V[i];
i = posicion;
posicion=(posicion-1)%(tam_buffer);
disparosAcertados=V[i];
total = disparosAcertados-botiquines;

fprintf(fichOutput,"Nave subcampeona: %d \n",nave);
fflush(stdout);
fprintf(fichOutput,"El numero de disparos acertados es %d \n",disparosAcertados);
fflush(stdout);
fprintf(fichOutput,"El numero de disparos fallados es %d \n",disparosFallados);
fflush(stdout);
fprintf(fichOutput,"El numero de botiquines es %d \n",botiquines);
fflush(stdout);
fprintf(fichOutput,"El total es %d \n",total);
fflush(stdout);


//imprimo resumen
tokens_t = disparos_recibidost + disparos_fallidost + botiquines_t ;
fprintf(fichOutput,"================== RESUMEN ============== \n");
fprintf(fichOutput,"Disparos recibidos totales: %d \n",disparos_recibidost);
fprintf(fichOutput,"Disparos fallidos totales: %d \n",disparos_fallidost);
fprintf(fichOutput,"Botiquines obtenidos totales: %d \n",botiquines_t);
fprintf(fichOutput,"Tokens emitidos totales: %d \n",tokens_t);

//Cierra ficheros
fclose(fichOutput);

//Libera memoria
free(B);
free(V);
free(I);
free(Nave_h);
return 0;

}