    
    #include <ESP8266WiFi.h>
    #include <PubSubClient.h>
    #include <DHT.h>
    
    int Relay = D1;
    
    #define DHTPIN D4
    #define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
    DHT dht(DHTPIN, DHTTYPE);
    
    int Mode;
    const char *ssid = "Wasan_Show_2.4G"; 
    const char *password = "wasanfern23170";  
     
    
    const char *mqtt_broker = "broker.hivemq.com"; 
    const char *topic_control = "Pump/Control";
    const char *topic_temp = "temp";
    const char *topic_humi = "humi";
    const char *topic_pump = "Pump/State";
    const char *topic_state = "soil/State";
    const int mqtt_port = 1883;
    WiFiClient espClient;
    PubSubClient client(espClient);
    
    void setup() {
      
     Serial.begin(115200);
     dht.begin();
     pinMode(Relay,OUTPUT);
     WiFi.begin(ssid, password);
     while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
     }
     
     Serial.println("Connected to the WiFi network");
     
     client.setServer(mqtt_broker, mqtt_port);
     client.setCallback(Callback);
     
     while (!client.connected()) {
     String client_id = "esp8266-client-";
     client_id += String(WiFi.macAddress());
     
     Serial.printf("The client %s connects to mosquitto mqtt broker\n", client_id.c_str());
     
     if (client.connect(client_id.c_str())) {
      Serial.println("Public emqx mqtt broker connected");
     } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
     }
    }
     
     client.publish(topic_control, "อุปกรณ์เชือมต่อเเล้ว");
     client.subscribe(topic_control);
     }
    
    
    void Callback(char *topic_control, byte *payload, unsigned int length) {
    String string;
     
     Serial.print(topic_control);
     for (int i = 0; i < length; i++) {
      string+=((char)payload[i]);  
      
      Serial.println(string);
      
      if(((char) payload[i] == '1') and (Mode == 1)){
       digitalWrite(Relay,1);
       client.publish(topic_state,String(1).c_str());
       client.publish(topic_pump,String("ปั๊มกำลังทำงาน").c_str());
      }
     if(((char) payload[i] == '0') and (Mode == 1)){
       digitalWrite(Relay,0);
      client.publish(topic_state,String(0).c_str());
      client.publish(topic_pump,String("ปั๊มหยุดทำงาน").c_str());
      }
      if((char) payload[i] == 'A'){
       Mode = 1;
      }
      if((char) payload[i] == 'M'){
       Mode = 0;
      }

     }
     Serial.println(string);
     Serial.println(" - - - - - - - - - - - -");
    }

    
    
    void loop() {
       client.loop();
       
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        if (isnan(h) || isnan(t)) {
          Serial.println(F("Failed to read from DHT sensor!"));
          return;
        }
       client.publish(topic_temp,String(t).c_str());
       client.publish(topic_humi,String(h).c_str());
       
       if( t >= 30 and Mode == 0){
        digitalWrite(Relay,1);
        client.publish(topic_state,String(1).c_str());
        client.publish(topic_pump,String("ปั๊มกำลังทำงาน").c_str());
       }
        if( t <= 30 and Mode == 0){
         digitalWrite(Relay,0);
         client.publish(topic_state,String(0).c_str());
         client.publish(topic_pump,String("ปั๊มหยุดทำงาน").c_str());
       }
       delay(1000);
    }
