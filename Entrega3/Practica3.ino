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

#define NUM_OF_PHILOSOPHERS 5                       //Nombre de filòsofs
#define MAX_NUMBER_ALLOWED (NUM_OF_PHILOSOPHERS - 1)  // Màxim nombre de filòsofs a l'habitació  (un menys que el total per evitar deadlock)
#define ESPERA 200  //interval d'espera de vTaskDelay







// You'll likely need this on vanilla FreeRTOS
//#include <semphr.h>


// Settings
enum { TASK_STACK_SIZE = 2048 };  // Bytes in ESP32, words in vanilla FreeRTOS

// Globals
static SemaphoreHandle_t bin_sem;   // Wait for parameters to be read
static SemaphoreHandle_t done_sem;  // Notifies main task when done
static SemaphoreHandle_t chopstick[NUM_OF_PHILOSOPHERS];
static SemaphoreHandle_t asientos;
static SemaphoreHandle_t imprimir;
static char buf[50];
static bool finito = false;
static bool solape = false;

//*****************************************************************************
// Tasks

// The only task: eating
void eat(void *parameters) {

 
  int repeticiones = 1;
  int num;
  // Copy parameter and increment semaphore count
  num = *(int *)parameters;



  while (repeticiones) {

  if(finito==true){
    repeticiones--;
  }

    xSemaphoreGive(bin_sem);
    if (!solape) {
      xSemaphoreTake(imprimir, portMAX_DELAY);
    }
    sprintf(buf, "Filósofo %i: TOC TOC", num);
    Serial.println(buf);

    if (!solape) {
      xSemaphoreGive(imprimir);
    }

    xSemaphoreTake(asientos, portMAX_DELAY);
    if (!solape) {
      xSemaphoreTake(imprimir, portMAX_DELAY);
    }
    sprintf(buf, "Filósofo %i: |▄|", num);
    Serial.println(buf);
    if (!solape) {
      xSemaphoreGive(imprimir);
    }

    // Take left chopstick
    xSemaphoreTake(chopstick[num], portMAX_DELAY);
    if (!solape) {
      xSemaphoreTake(imprimir, portMAX_DELAY);
    }
    sprintf(buf, "Filósofo %i: ¡o", num);
    Serial.println(buf);
    if (!solape) {
      xSemaphoreGive(imprimir);
    }


    // Add some delay to force deadlock
    vTaskDelay(random(ESPERA));

    // Take right chopstick
    xSemaphoreTake(chopstick[(num + 1) % NUM_OF_PHILOSOPHERS], portMAX_DELAY);
    if (!solape) {
      xSemaphoreTake(imprimir, portMAX_DELAY);
    }
    sprintf(buf, "Filósofo %i: ¡o¡", num);
    Serial.println(buf);
    if (!solape) {
      xSemaphoreGive(imprimir);
    }
    vTaskDelay(random(ESPERA));

    // Do some eating
    if (!solape) {
      xSemaphoreTake(imprimir, portMAX_DELAY);
    }
    sprintf(buf, "Filósofo %i: /o\\ ÑAM", num);
    Serial.println(buf);
    if (!solape) {
      xSemaphoreGive(imprimir);
    }

    vTaskDelay(random(ESPERA));

    // Put down right chopstick
    xSemaphoreGive(chopstick[(num + 1) % NUM_OF_PHILOSOPHERS]);
    if (!solape) {
      xSemaphoreTake(imprimir, portMAX_DELAY);
    }
    sprintf(buf, "Filósofo %i: ¡o_", num);
    Serial.println(buf);
    if (!solape) {
      xSemaphoreGive(imprimir);
    }

    // Put down left chopstick
    xSemaphoreGive(chopstick[num]);
    if (!solape) {
      xSemaphoreTake(imprimir, portMAX_DELAY);
    }
    sprintf(buf, "Filósofo %i: _o", num);
    Serial.println(buf);
    if (!solape) {
      xSemaphoreGive(imprimir);
    }

    xSemaphoreGive(asientos);
    if (!solape) {
      xSemaphoreTake(imprimir, portMAX_DELAY);
    }
    sprintf(buf, "Filósofo %i: |_|", num);
    Serial.println(buf);
    if (!solape) {
      xSemaphoreGive(imprimir);
    }
  }
  // Notify main task and delete self
  xSemaphoreGive(done_sem);
  vTaskDelete(NULL);
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  char task_name[20];
  imprimir = xSemaphoreCreateMutex();
  String respuesta;
  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Dining Philosophers Challenge---");

  // Create kernel objects before starting tasks
  bin_sem = xSemaphoreCreateBinary();
  done_sem = xSemaphoreCreateCounting(NUM_OF_PHILOSOPHERS, 0);
  asientos = xSemaphoreCreateCounting(MAX_NUMBER_ALLOWED, MAX_NUMBER_ALLOWED);
  for (int i = 0; i < NUM_OF_PHILOSOPHERS; i++) {
    chopstick[i] = xSemaphoreCreateMutex();
  }

  Serial.println("Quieres que la ejecución sea finita? (S/N) ");
  respuesta=Serial.readStringUntil('\n');
  if(respuesta[0]=='S'||respuesta[0]=='s'){
    finito=true;
    Serial.println("Ejecución finita ");
  }
  
  Serial.println("Quieres que se superpongan las líneas de mensajes? (S/N) ");
  respuesta=Serial.readStringUntil('\n');
  if(respuesta[0]=='S'||respuesta[0]=='s'){
    solape=true;
    Serial.println("Solapamiento activado ");
  }

  Serial.println("Quieres que se produzca deadlock? (S/N) ");
  respuesta=Serial.readStringUntil('\n');
  if(respuesta[0]=='S'||respuesta[0]=='s'){
    asientos = xSemaphoreCreateCounting(NUM_OF_PHILOSOPHERS, NUM_OF_PHILOSOPHERS);
    Serial.println("Deadlock activado ");
  }
  
  // Have the philosphers start eating
  for (int i = 0; i < NUM_OF_PHILOSOPHERS; i++) {
    if (!solape) {
      xSemaphoreTake(imprimir, portMAX_DELAY);
    }
    sprintf(task_name, "@filósofo %i", i);
    Serial.println(task_name);
    if (!solape) {
      xSemaphoreGive(imprimir);
    }
    xTaskCreatePinnedToCore(eat,
                            task_name,
                            TASK_STACK_SIZE,
                            (void *)&i,
                            1,
                            NULL,
                            app_cpu);
    xSemaphoreTake(bin_sem, portMAX_DELAY);
  }


  // Wait until all the philosophers are done
  for (int i = 0; i < NUM_OF_PHILOSOPHERS; i++) {
    xSemaphoreTake(done_sem, portMAX_DELAY);
  }

  // Say that we made it through without deadlock
  Serial.println("Done! No deadlock occurred!");
}

void loop() {
  // Do nothing in this task
}
