#include <RadioLib.h>

// =====================================
// LoRa SX1276
// NSS, DIO0, RST, DIO1
// =====================================
SX1276 radio = new Module(5, 2, 14, 13);

// =====================================
// VARIABLES SIMULADAS
// =====================================
int humedad = 0;
float temperatura = 0;

int nitrogeno = 0;
int fosforo = 0;
int potasio = 0;

int cultivo = 1;

// =====================================
void setup() {

  Serial.begin(115200);

  // =====================================
  // Inicializar LoRa
  // =====================================
  Serial.print("Iniciando LoRa...");

  int estado = radio.begin(915.0);

  if (estado != RADIOLIB_ERR_NONE) {

    Serial.print("Error LoRa: ");
    Serial.println(estado);

    while (true);
  }

  Serial.println("OK");

  // Semilla aleatoria
  randomSeed(analogRead(34));
}

// =====================================
void loop() {

  generarDatos();

  // =====================================
  // MENSAJE JSON
  // Compatible con Firebase/index.html
  // =====================================
  String mensaje =
    "{"
    "\"id\":1,"
    "\"humedad_vwc\":" + String(humedad) + ","
    "\"temp_x10\":" + String((int)(temperatura * 10)) + ","
    "\"n_mg\":" + String(nitrogeno) + ","
    "\"p_mg\":" + String(fosforo) + ","
    "\"k_mg\":" + String(potasio) + ","
    "\"cultivo\":" + String(cultivo) + ","
    "\"alerta\":0"
    "}";

  // =====================================
  // Mostrar datos
  // =====================================
  Serial.println("================================");
  Serial.println("DATOS GENERADOS");
  Serial.println();

  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.println("%");

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" C");

  Serial.print("N: ");
  Serial.println(nitrogeno);

  Serial.print("P: ");
  Serial.println(fosforo);

  Serial.print("K: ");
  Serial.println(potasio);

  Serial.println();
  Serial.println("Enviando LoRa:");
  Serial.println(mensaje);

  // =====================================
  // Enviar LoRa
  // =====================================
  int estado = radio.transmit(mensaje);

  if (estado == RADIOLIB_ERR_NONE) {

    Serial.println("Mensaje enviado correctamente");

  } else {

    Serial.print("Error LoRa: ");
    Serial.println(estado);
  }

  Serial.println();

  delay(5000);
}

// =====================================
// GENERAR DATOS REALISTAS
// =====================================
void generarDatos() {

  // Humedad típica agrícola
  humedad = random(35, 85);

  // Temperatura suelo
  temperatura = random(180, 360) / 10.0;

  // Nutrientes típicos
  nitrogeno = random(20, 80);
  fosforo   = random(10, 60);
  potasio   = random(40, 120);

  // =====================================
  // ALERTAS SIMULADAS
  // =====================================

  // Suelo seco
  if (humedad < 45) {

    Serial.println("ALERTA: Suelo seco");
  }

  // Exceso de agua
  if (humedad > 75) {

    Serial.println("ALERTA: Exceso de agua");
  }

  // Bajo nitrógeno
  if (nitrogeno < 30) {

    Serial.println("ALERTA: N bajo");
  }
}