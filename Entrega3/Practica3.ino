// Només fem servir el nucli app_cpu per simplicitat
// i tenint en compte que alguns esp32 són unicore
// unicore    -> app_cpu = 0
// 2 core  -> app_cpu = 1 (prog_cpu = 0)

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif
//#define INCLUDE_vTaskSuspend    1    //ja està posat per defecte sino descomentar es el temps de espera                //infinit als semàfors

/*************************** Variables Globals i definicions **************************************/

#define NUM_OF_PHILOSOPHERS 5  //Nombre de filòsofs
#define MAX_NUMBER_ALLOWED (NUM_OF_PHILOSOPHERS - 1)  // Màxim nombre de filòsofs a l'habitació  (un menys que el total per evitar deadlock)
#define ESPERA 200  //interval d'espera de vTaskDelay


// You'll likely need this on vanilla FreeRTOS
//#include <semphr.h>


// Settings
enum { TASK_STACK_SIZE = 2048 };  // Bytes in ESP32, words in vanilla FreeRTOS

// Globals
static SemaphoreHandle_t done_sem;  // Notifica que el programa principal ha terminado
static SemaphoreHandle_t chopstick[NUM_OF_PHILOSOPHERS];  // Array de semàforos de los palillos
static SemaphoreHandle_t asientos; // Semáforo contador para controlar los filósofos que están en la mesa a la vez
static SemaphoreHandle_t imprimir; // Semáforo mutex para controolar que se produzcan solapamientos
static char buf[50]; //Buffer para imprimir mensajes informativos sobre la ejecución
static bool finito = false; // Variable para poder seleccionar el modo de ejecución finita
static bool solape = false; // Variable parapoder seleccionar si se produce solapamiento en los mensajes
static int num_philosopher = 0; // Variable global para calcular el número de filósofos

//*****************************************************************************
// Tareas

// Función a ejecutar por cada tarea(filósofo)
void eat(void *parameters) {

  // Variable para poder controlar las repeticiones de la ejecución
  int repeticiones = 1;
  //Variable que almacena el número de filósofos
  int num;
  num = num_philosopher++;


  //Bucle con el algoritmo de los filósofos
  while (repeticiones) {
    //Si se selecciona la opción finito se decrementa el número de repeticiones, rompiendo el bucle
    if (finito == true) {
      repeticiones--;
    }

    //Para cada mensaje impreso,se controla con el sémaforo imprimir el acceso al serial
    if (!solape) {

      if ( xSemaphoreTake(imprimir, portMAX_DELAY) == pdFALSE )
      {
        Serial.println("Error al capturar el semáforo iprimir ");
        return ;
      }

    }
    sprintf(buf, "Filósofo %i: TOC TOC", num);
    Serial.println(buf);

    if (!solape) {

      if ( xSemaphoreGive(imprimir) == pdFALSE )
      {
        Serial.println("Error al liberar el semáforo imprimir ");
        return ;
      }
    }
    //Por cada filósofo que quiere comer se bloquea el semáforo asientos.
    //Con este semáforo se evita el deadlock al no permitir que todos los filósofos coman a la vez

    if ( xSemaphoreTake(asientos, portMAX_DELAY) == pdFALSE )
    {
      Serial.println("Error al capturar el semáforo asientos ");
      return ;
    }

    if (!solape) {

      if ( xSemaphoreTake(imprimir, portMAX_DELAY) == pdFALSE )
      {
        Serial.println("Error al capturar el semáforo iprimir ");
        return ;
      }
    }
    sprintf(buf, "Filósofo %i: |▄|", num);
    Serial.println(buf);
    if (!solape) {

      if ( xSemaphoreGive(imprimir) == pdFALSE )
      {
        Serial.println("Error al liberar el semáforo iprimir ");
        return ;
      }
    }

    // Coge el palillo de la izquierda

    if (xSemaphoreTake(chopstick[num], portMAX_DELAY) == pdFALSE )
    {
      Serial.println("Error al capturar el semáforo palillo ");
      return ;
    }

    if (!solape) {

      if (xSemaphoreTake(imprimir, portMAX_DELAY) == pdFALSE )
      {
        Serial.println("Error al capturar el semáforo imprimir ");
        return ;
      }
    }
    sprintf(buf, "Filósofo %i: ¡o", num);
    Serial.println(buf);
    if (!solape) {

      if (xSemaphoreGive(imprimir) == pdFALSE )
      {
        Serial.println("Error al liberar el semáforo imprimir ");
        return ;
      }
    }


    // Tiempo aleatorio entre 0 y ESPERA(200ms) para pensar
    vTaskDelay(random(ESPERA));

    // Coge el palillo de la derecha

    if (xSemaphoreTake(chopstick[(num + 1) % NUM_OF_PHILOSOPHERS], portMAX_DELAY) == pdFALSE )
    {
      Serial.println("Error al capturar el semáforo palillo ");
      return ;
    }


    if (!solape) {

      if (xSemaphoreTake(imprimir, portMAX_DELAY) == pdFALSE )
      {
        Serial.println("Error al capturar el semáforo imprimir ");
        return ;
      }
    }
    sprintf(buf, "Filósofo %i: ¡o¡", num);
    Serial.println(buf);
    if (!solape) {

      if (xSemaphoreGive(imprimir) == pdFALSE )
      {
        Serial.println("Error al liberar el semáforo imprimir ");
        return ;
      }
    }
    vTaskDelay(random(ESPERA));

    // Comer
    if (!solape) {

      if (xSemaphoreTake(imprimir, portMAX_DELAY) == pdFALSE )
      {
        Serial.println("Error al capturar el semáforo imprimir ");
        return ;
      }
    }
    sprintf(buf, "Filósofo %i: /o\\ ÑAM", num);
    Serial.println(buf);
    if (!solape) {

      if (xSemaphoreGive(imprimir) == pdFALSE )
      {
        Serial.println("Error al liberar el semáforo imprimir ");
        return ;
      }
    }

    vTaskDelay(random(ESPERA));

    //Suelta el palillo de la derecha

    if ( xSemaphoreGive(chopstick[(num + 1) % NUM_OF_PHILOSOPHERS]) == pdFALSE )
    {
      Serial.println("Error al liberar el semáforo palillo ");
      return ;
    }
    if (!solape) {

      if ( xSemaphoreTake(imprimir, portMAX_DELAY) == pdFALSE )
      {
        Serial.println("Error al capturar el semáforo imprimir ");
        return ;
      }
    }
    sprintf(buf, "Filósofo %i: ¡o_", num);
    Serial.println(buf);
    if (!solape) {

      if (xSemaphoreGive(imprimir) == pdFALSE )
      {
        Serial.println("Error al liberar el semáforo imprimir ");
        return ;
      }
    }

    //Suelta el palillo de la izquierda

    if (xSemaphoreGive(chopstick[num]) == pdFALSE )
    {
      Serial.println("Error al liberar el semáforo palillo ");
      return ;
    }
    if (!solape) {

      if (xSemaphoreTake(imprimir, portMAX_DELAY) == pdFALSE )
      {
        Serial.println("Error al capturar el semáforo imprimir ");
        return ;
      }
    }
    sprintf(buf, "Filósofo %i: _o", num);
    Serial.println(buf);
    if (!solape) {

      if (xSemaphoreGive(imprimir) == pdFALSE )
      {
        Serial.println("Error al liberar el semáforo imprimir ");
        return ;
      }
    }
    //El filósofo se levanta de la mesa dejando un asiento disponible
    if (xSemaphoreGive(asientos) == pdFALSE )
    {
      Serial.println("Error al liberar el semáforo asientos ");
      return ;
    }

    if (!solape) {

      if (xSemaphoreTake(imprimir, portMAX_DELAY) == pdFALSE )
      {
        Serial.println("Error al capturar el semáforo imprimir ");
        return ;
      }


    }
    sprintf(buf, "Filósofo %i: |_|", num);
    Serial.println(buf);
    if (!solape) {

      if (xSemaphoreGive(imprimir) == pdFALSE )
      {
        Serial.println("Error al liberar el semáforo imprimir ");
        return ;
      }
    }
  }
  //Notifica al hilo principal que ha terminado de comer y por último elimina la tarea

  if ( xSemaphoreGive(done_sem) == pdFALSE )
  {
    Serial.println("Error al liberar el semáforo done_sem ");
    return ;
  }
  vTaskDelete(NULL);
}

//*****************************************************************************
// Main

void setup() {

  //Se crea el semáforo mutex imprimir
  imprimir = xSemaphoreCreateMutex();
  if ( imprimir == NULL )
  {
    Serial.println("Error al crear semáforo imprimir");
    return ;
  }
  //Se crea el String para leer las respuestas a las preguntas interactivas
  String respuesta;
  //String para imprimir el nombre del filósofo craedo
  char task_name[20];
  // Configure Serial
  Serial.begin(115200);

  // Tiempo de espera para que se inicialice el serial
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---  CENA DE LOS FILÓSOFOS ---");

  //Se inicializa el semáforo de las tareas
  done_sem = xSemaphoreCreateCounting(NUM_OF_PHILOSOPHERS, 0);
  if ( done_sem == NULL )
  {
    Serial.println("Error al crear semáforo done_sem");
    return ;
  }
  //Se inicializa el semáforo de asientos con el número de filósofos menos 1
  asientos = xSemaphoreCreateCounting(MAX_NUMBER_ALLOWED, MAX_NUMBER_ALLOWED);
  if ( asientos == NULL )
  {
    Serial.println("Error al crear semáforo asientos");
    return ;
  }


  //Se inicializan los semáforos que controlan los palillos
  for (int i = 0; i < NUM_OF_PHILOSOPHERS; i++) {

    chopstick[i] = xSemaphoreCreateMutex();
    if ( chopstick[i] == NULL )
    {
      Serial.println("Error al crear semáforo chopstick[i]");
      return ;
    }

  }
  //Se inicia el diálogo interactivo con el usuario
  Serial.println("Quieres que la ejecución sea finita? (S/N) ");
  //Se lee la respuesta
  respuesta = Serial.readStringUntil('\n');
  //Solo en caso de respuesta afirmativa se activa la ejecución finita
  if (respuesta[0] == 'S' || respuesta[0] == 's') {
    finito = true;
    Serial.println("Ejecución finita ");
  }

  Serial.println("Quieres que se superpongan las líneas de mensajes? (S/N) ");
  //Se lee la respuesta
  respuesta = Serial.readStringUntil('\n');
  //Solo en caso de respuesta afirmativa se activa el solapamiento de mensajes
  if (respuesta[0] == 'S' || respuesta[0] == 's') {
    solape = true;
    Serial.println("Solapamiento activado ");
  }

  Serial.println("Quieres que se produzca deadlock? (S/N) ");
  //Se lee la respuesta
  respuesta = Serial.readStringUntil('\n');
  //Solo en caso de respuesta afirmativa se provoca deadlock
  if (respuesta[0] == 'S' || respuesta[0] == 's') {
    //Para ello configuramos el semáforo de asientos con el número total de los filósofos,
    //permitiendo que todos cojan el palillo a la vez
    asientos = xSemaphoreCreateCounting(NUM_OF_PHILOSOPHERS, NUM_OF_PHILOSOPHERS);
    Serial.println("Deadlock activado ");
  }

  //Bloqueamos los mensajes hasta haber creado todos los filósofos
  if (!solape) {

    if (xSemaphoreTake(imprimir, portMAX_DELAY) == pdFALSE )
    {
      Serial.println("Error al capturar el semáforo iprimir ");
      return ;
    }
  }
  //Bucle de creación de filósofos
  for (int i = 0; i < NUM_OF_PHILOSOPHERS; i++) {

    sprintf(task_name, "@filósofo %i", i);
    Serial.println(task_name);
    //Se crea el filósofo
    xTaskCreatePinnedToCore(eat,
                            task_name,
                            TASK_STACK_SIZE,
                            NULL,
                            1,
                            NULL,
                            app_cpu);

  }
  //Se desbloquean los mensajes, permitiendo que los filósofos puedan ejecutarse
  if (!solape) {

    if ( xSemaphoreGive(imprimir) == pdFALSE )
    {
      Serial.println("Error al liberar el semáforo iprimir ");
      return ;
    }
  }


  //Se espera a que todas la tareas hayan finalizado para terminar la ejecución del main
  for (int i = 0; i < NUM_OF_PHILOSOPHERS; i++) {

    if (xSemaphoreTake(done_sem, portMAX_DELAY) == pdFALSE )
    {
      Serial.println("Error al capturar el semáforo iprimir ");
      return ;
    }
  }

  //Se imprime el mensaje de que no se ha producido deadlock
  Serial.println("Fin! Sin bloqueo!");
}

void loop() {

}
