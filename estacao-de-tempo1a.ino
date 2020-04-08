// Estacao de tempo v1
// Medição de temperatura e umidade com o DHT22
// Versão 1.01 - 08102019
// - Limpeza do código
// - Redução dos intervalos de envio de informações
// - Retirei os delays

#include "Adafruit_DHT_Particle.h"

#define DHTPIN D2     // what pin we're connected to

//sincronização do tempo
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
#define TEMPO_ENTRE_MEDIDAS_MILLIS (2 * 60 * 1000)
#define TEMPO_ECONOMIA_ENERGIA_MILLIS (5 * 60 * 1000)

unsigned long lastSync = millis();
unsigned long lastMed = millis();
unsigned long lastSLEEP = millis();
unsigned long lastdelay = millis();


#define DHTTYPE DHT22		// DHT 22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);
int loopCount;
float last_t =0;
float last_h =0;


void setup() {
	Particle.publish("state", "DHT22 inicio medidas");
	dht.begin();
}

void loop() {
  //Momento de sincronizar o relógio?
  if (millis() - lastSync > ONE_DAY_MILLIS) {
    // Request time synchronization from the Particle Cloud
    Particle.syncTime(); //sincroniza horário com a nuvem a cada 24h
    lastSync = millis();
  }

  // Momento de medir?
  //if (millis() - lastMed > TEMPO_ENTRE_MEDIDAS_MILLIS) { 
      
	float h = dht.getHumidity();
   	float t = dht.getTempCelcius();
	
    Time.zone(-3);
    String hora =Time.format(Time.now(), "%H:%M");
    String timeStamp = Time.timeStr();
	
	
    // Check if any reads failed and exit early (to try again).
	// if (isnan(h) || isnan(t) || isnan(f)) {
	if (isnan(h)) {
		Serial.println("Falha na leitura do sensor!");
	    h=t=0;
	}
	
	if (t==0 || h==0) 	{//Se houve erro na leitura, manter valor anterior
	    t = last_t;
	    h = last_h;
	} 
	    else // se leitura ok, atualizar valor
	{
	    last_t = t;
	    last_h = h;
	}
	
	String Data="{\"Value1\":\""+String(t,2)+"\","; //Temp
	Data+="\"Value2\":\""+String(h,2)+"\","; //Humdity
	Data+="\"Value3\":\""+String(timeStamp)+"\","; //data da medicao
	Data+="\"Value4\":\""+String(hora)+"\"}"; //hora

	if (t!=0 || h!=0 ){ //somente publica se o valor não for zero
	    Particle.publish("WeatherData", Data, PUBLIC);	//publica na nuvem
	    Particle.publish("state", "nova_medida");
	//lastMed = millis(); //Atualiza timer medidas
	}
	delay(2000);
	Particle.publish("state", "Economia de energia por 5 minutos");
    System.sleep(SLEEP_MODE_DEEP, TEMPO_ECONOMIA_ENERGIA_MILLIS/1000);
	
}

