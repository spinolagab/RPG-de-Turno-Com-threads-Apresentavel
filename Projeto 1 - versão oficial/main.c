#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <windows.h>

// Semáforos
sem_t turno_player;
sem_t turno_boss;

// Mutex
pthread_mutex_t turnoAtual;

// Boss
int HP_boss = 400;

// Player
int HP_player = 170;
int curaDisponivel = 3;

// Auxiliar para calcular o dano
int auxiliar = 0;

// Sinalizador para indicar o fim do combate
int combate_finalizado = 0;

void* boss(void* arg){
  srand(time(NULL));
  while(HP_player > 0 && !combate_finalizado){

    // Espera o boss poder agir
    sem_wait(&turno_boss);

    if(HP_boss > 0){
      // Proteger a sessão crítica
      pthread_mutex_lock(&turnoAtual);

      // 75% de chance de acertar o player
      if((rand()%99)+1 > 25){
        // Dano do Boss 2d20
        auxiliar = 10+((rand()%19)+1)+((rand()%19)+1);

        // Diminui o HP do player em caso de acerto
        HP_player -= auxiliar;
        printf("O Boss atacou e causou %d pontos de dano\n ", auxiliar);

      } else {
        printf("O Boss errou seu ataque!\n");
      }

      // Libera o mutex
      pthread_mutex_unlock(&turnoAtual);

      // Libera a ação do player
      sem_post(&turno_player);

      printf("HP Player: %d \nHP Boss: %d \n", HP_player, HP_boss);
      sleep(2);
    } else {
      printf("O Boss foi derrotado!\n");
      combate_finalizado = 1;
    }
  }

  return NULL;
}

void* player(void* arg){
  srand(time(NULL));
  printf("HP Player: %d \nHP Boss: %d \n", HP_player, HP_boss);
  while(HP_boss > 0 && !combate_finalizado){

    // Espera o player poder agir
    sem_wait(&turno_player);

    // Se o player estiver vivo pode agir
    if(HP_player > 0){
      // Proteger a sessão crítica
      pthread_mutex_lock(&turnoAtual);

      // 90% de chance de acertar o boss
      if((rand()%99)+1 > 20){
        // Dano do player 10 + 2d12
        auxiliar = 10 + ((rand()%11)+1)+((rand()%11)+1);

        // Diminui o HP do boss em caso de acerto
        HP_boss -= auxiliar;
        printf("O Player atacou e causou %d pontos de dano\n", auxiliar);

      } else {
        printf("O Player errou seu ataque!\n");
      }

      // Se a vida estiver muito baixa, o player se cura
      if(HP_player <= 40 && curaDisponivel > 0){
        HP_player += 90;
        curaDisponivel--;
        printf("O Player se curou em 90 pontos. HP atual: %d\nPocoes de vida restantes: %d\n", HP_player, curaDisponivel);
      }

      pthread_mutex_unlock(&turnoAtual);

      // Libera o turno do boss 
      sem_post(&turno_boss);
    
      // Tempo de intervalo entre ataques e se reposicionar
      sleep(2);
    } else {
      printf("O Player foi derrotado!\n");
      combate_finalizado = 1;
      break;
    }
  }

  return NULL;
}

int main(void){
  srand(time(NULL));
  pthread_t threadPlayer, threadBoss;

  // 1 turno por vez e o player começa atacando
  sem_init(&turno_player, 0, 1);
  sem_init(&turno_boss, 0, 0);

  pthread_mutex_init(&turnoAtual, NULL);

  pthread_create(&threadPlayer, NULL, player, NULL);
  pthread_create(&threadBoss, NULL, boss, NULL);

  // Esperar as threads para finalizar o código
  pthread_join(threadPlayer, NULL);
  pthread_join(threadBoss, NULL);

  printf("Fim do combate!\n");

  // Destruir os semáforos e mutex
  sem_destroy(&turno_player);
  sem_destroy(&turno_boss);
  pthread_mutex_destroy(&turnoAtual);

  printf("Memoria liberada...\n");

  return 0;
}
