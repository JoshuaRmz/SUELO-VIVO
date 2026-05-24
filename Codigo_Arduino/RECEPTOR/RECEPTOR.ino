#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <RadioLib.h>
#include <ArduinoJson.h>

// =====================================================
// WIFI
// =====================================================
const char* ssid = "UAM-ROBOTICA";
const char* password = "m4nt32024uat";

// =====================================================
// FIREBASE
// =====================================================
#define API_KEY "AIzaSyAQWfoyt1xUJ5B600cVSUuGeBcapTAGrLw"
#define DATABASE_URL "https://suelo-vivo-default-rtdb.firebaseio.com/"

// =====================================================
// FIREBASE OBJETOS
// =====================================================
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// =====================================================
// LoRa SX1276
// NSS, DIO0, RST, DIO1
// =====================================================
SX1276 radio = new Module(5, 2, 14, 13);

// =====================================================
void setup() {

  Serial.begin(115200);

  // =====================================================
  // WIFI
  // =====================================================
  Serial.println();
  Serial.print("Conectando WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());

  // =====================================================
  // FIREBASE
  // =====================================================
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase conectado");

  // =====================================================
  // LORA
  // =====================================================
  Serial.print("Iniciando LoRa...");

  int estado = radio.begin(915.0);

  if (estado != RADIOLIB_ERR_NONE) {

    Serial.print("Error LoRa: ");
    Serial.println(estado);

    while (true);
  }

  Serial.println("OK");
  Serial.println("Esperando datos LoRa...");
}

// =====================================================
void loop() {

  String mensaje;

  // =====================================================
  // RECIBIR LORA
  // =====================================================
  int estado = radio.receive(mensaje);

  if (estado == RADIOLIB_ERR_NONE) {

    Serial.println("================================");
    Serial.println("MENSAJE RECIBIDO");
    Serial.println(mensaje);

    // =====================================================
    // JSON
    // =====================================================
    DynamicJsonDocument doc(512);

    DeserializationError error =
      deserializeJson(doc, mensaje);

    if (error) {

      Serial.println("Error JSON");
      return;
    }

    // =====================================================
    // LEER DATOS
    // =====================================================
    int id            = doc["id"];
    int humedad       = doc["humedad_vwc"];
    int temp_x10      = doc["temp_x10"];
    int n_mg          = doc["n_mg"];
    int p_mg          = doc["p_mg"];
    int k_mg          = doc["k_mg"];
    int cultivo       = doc["cultivo"];
    int alerta        = doc["alerta"];

    // =====================================================
    // MOSTRAR
    // =====================================================
    Serial.print("Nodo: ");
    Serial.println(id);

    Serial.print("Humedad: ");
    Serial.println(humedad);

    Serial.print("Temperatura: ");
    Serial.println(temp_x10 / 10.0);

    Serial.print("N: ");
    Serial.println(n_mg);

    Serial.print("P: ");
    Serial.println(p_mg);

    Serial.print("K: ");
    Serial.println(k_mg);

    // =====================================================
    // RUTA FIREBASE
    // =====================================================
    String rutaActual =
      "/suelo_nodos/" + String(id) + "/ultima_lectura";

    String rutaHistorial =
      "/suelo_lecturas/" + String(id);

    // =====================================================
    // CREAR JSON FIREBASE
    // =====================================================
    FirebaseJson json;

    json.set("humedad_vwc", humedad);
    json.set("temp_x10", temp_x10);
    json.set("n_mg", n_mg);
    json.set("p_mg", p_mg);
    json.set("k_mg", k_mg);
    json.set("cultivo", cultivo);
    json.set("alerta", alerta);
    json.set("timestamp", millis());

    // =====================================================
    // SUBIR ULTIMA LECTURA
    // =====================================================
    bool ok1 =
      Firebase.RTDB.setJSON(
        &fbdo,
        rutaActual.c_str(),
        &json
      );

    // =====================================================
    // SUBIR HISTORIAL
    // =====================================================
    bool ok2 =
      Firebase.RTDB.pushJSON(
        &fbdo,
        rutaHistorial.c_str(),
        &json
      );

    // =====================================================
    // RESULTADO
    // =====================================================
    if (ok1 && ok2) {

      Serial.println("Datos enviados a Firebase");

    } else {

      Serial.println("Error Firebase");
      Serial.println(fbdo.errorReason());
    }

    Serial.println();

  } else if (estado == RADIOLIB_ERR_RX_TIMEOUT) {

    // nada recibido

  } else {

    Serial.print("Error LoRa: ");
    Serial.println(estado);
  }
}