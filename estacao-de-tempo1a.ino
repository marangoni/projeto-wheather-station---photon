/// Estacao de tempo v1
// Medição de temperatura e umidade com o DHT22
// Versão 1.03 - 10042020
// Enviará todas as informacoes para o thingspeak - nao fará limpeza dos dados
// Fluxo do programa: Desperta -> Sincroniza relógio -> Mede -> Verifica se
// 										medida esta ok -> Se Sim publica -> Dorme

#include "Adafruit_DHT_Particle.h"

#define DHTPIN D2     // Saída do DHT -> pino D2

#define TEMPO_ECONOMIA_ENERGIA_MILLIS (12 * 60 * 1000)
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)

unsigned long lastSync = millis();

#define DHTTYPE DHT22		// DHT 22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);
int loopCount;

// ThingSpeak
// Ref: https://community.particle.io/t/how-to-set-up-a-json-for-multiple-variables-in-a-webhook-integration/33172/4
// Deve casar com o nome do evento utilizado no webhook
const char * eventName = "weather";
// Informacao do canal - ThingSpeak
unsigned long myChannelNumber =1034429; //HomeAlone2
const char * myWriteAPIKey = "97I0Q70ITCMGMQ6U";

void setup() {
	Particle.publish("state", "DHT22 inicio medidas");
	dht.begin();
	//sincroniza horário com a nuvem a cada nova medida
	if (Particle.connected())                     //Only time sync when in connected operation mode and Cloud connected
	    {
	        time_t lastSyncTimestamp;
	        unsigned long lastSync = Particle.timeSyncedLast(lastSyncTimestamp);
	        if (millis() - lastSync >= ONE_DAY_MILLIS)                              //More than one day since last time sync
	        {
	            unsigned long cur = millis();
	            Particle.syncTime();                                                //Request time synchronization from Particle Device Cloud
	            waitUntil(Particle.syncTimeDone);                                   //Wait until Photon receives time from Particle Device Cloud (or connection to Particle Device Cloud is lost)
	            Particle.publish("state", "Relógio sincronizado!");
	        }
	    }
}

void loop() {

  // Realiza as medidas e obtem "timestamp" e a hora da medida
	float h = dht.getHumidity();
  float t = dht.getTempCelcius();
	Time.zone(-3);
  String hora =Time.format(Time.now(), "%H:%M");
  String timeStamp = Time.timeStr();

	// Verifica se houve falha na medida - se h=0 ou t=0 -> houve erro
	// Se houve erro publica mensagem na serial e na nuvem
	if (isnan(h) || isnan(t)) {
		Serial.println("Falha na leitura do sensor!");
	  Particle.publish("state", "Falha na leitura do sensor!");
	}
	else
	{//Se ocorreu a leitura - segue o programa

		// Cria o JSON para envio
		String Data="{\"Data_med\":\""+String(timeStamp)+"\","; //timestamp
	  Data+="\"temperatura\":\""+String(t,2)+"\","; 				 //Temperatura
		Data+="\"umidade\":\""+String(h,2)+"\",";         		//Umidade
	  Data+="\"key\":\""+String(myWriteAPIKey)+"\"}";				//api_key thingspeak

		//Publicação na nuvem do JSON
		Particle.publish("state", "Nova_medida");
		Particle.publish(eventName, Data, PUBLIC);
	}

  //delay enquanto para facilitar o debug
  RGB.control(true);
  RGB.color(255, 255, 255);
  RGB.brightness(64);
  Particle.publish("state", "Aguardando 15s nova gravacao");
  delay(15000);
  RGB.control(false);
  //Entra no modo de economia de energia 5 min
  Particle.publish("state", "Economia de energia por 12 minutos");
  System.sleep(SLEEP_MODE_DEEP, TEMPO_ECONOMIA_ENERGIA_MILLIS/1000);

}
