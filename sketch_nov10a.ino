#include <WiFi.h>

String ssid;     
String password; 

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== Configuração de WiFi ===");
  
  Serial.print("Digite o nome da rede WiFi (SSID): ");
  while (ssid.length() == 0) {
    if (Serial.available()) {
      ssid = Serial.readStringUntil('\n'); 
      ssid.trim(); 
    }
  }

  Serial.print("Digite a senha: ");
  while (password.length() == 0) {
    if (Serial.available()) {
      password = Serial.readStringUntil('\n');
      password.trim();
    }
  }

  Serial.println("\nConectando ao WiFi...");
  WiFi.begin(ssid.c_str(), password.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Seu código principal aqui
}
