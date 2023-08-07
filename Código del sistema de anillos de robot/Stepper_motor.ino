#include <WiFi.h>
#include <PubSubClient.h>
#include <Stepper.h>

//Motor1
#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 17
//Motor2
#define IN5 12
#define IN6 14
#define IN7 27
#define IN8 26

// Replace the next variables with your SSID/Password combination
const char* ssid = "Tec-IoT";
const char* password = "spotless.magnetic.bridge";

int motor_direction=0;
int motor_state=0;// 0 off, 1 on
int stepCount_m1=0;
int stepCount_m2=0;

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "10.25.64.102";

const int stepsPerRevolution=100;
Stepper myStepper (stepsPerRevolution, IN1, IN3, IN2, IN4);
Stepper myStepper2 (stepsPerRevolution, IN5, IN7, IN6, IN8);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  myStepper.setSpeed(150);
  myStepper2.setSpeed(150);
}

void setup_wifi() {
  delay(100);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Command from MQTT broker is: [ ");
  Serial.print(topic);
  //Serial.print(". Message: ");

  int p =(char)payload[0]-'0';
  
  if (p==0){
    motor_state=0;
  }
 
  if (String(topic)=="Zoom"){
    if (p==3){
      motor_state=1;
      motor_direction=3;
    }
    else if (p==4){
      motor_state=1;
      motor_direction=4;
    }
  }

  if (String(topic)=="Focus"){
    if (p==1){
      motor_state=1;
      motor_direction=1;
    }
    else if (p==2){
      motor_state=1;
      motor_direction=2;
    }
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("Focus");
      client.subscribe("Zoom");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (motor_state==0){
    myStepper.step(0);
    myStepper2.step(0);
  }
  else if (motor_state==1 && motor_direction==1){
    myStepper.step(stepsPerRevolution);
    Serial.print("motor_1clockwise \n");
    Serial.print("steps: ");
    Serial.println(stepCount_m1);
    stepCount_m1++;
  }
  else if (motor_state==1 && motor_direction==2){
    myStepper.step(-stepsPerRevolution);
    Serial.print("motor_1counterclockwise \n");
    Serial.print("steps: ");
    Serial.println(stepCount_m1);
    stepCount_m1--;
  }
  else if (stepCount_m1==16){
    myStepper.step(0);
  }
  else if (motor_state==1 && motor_direction==3){
    myStepper2.step(stepsPerRevolution);
    Serial.print("motor_2clockwise \n");
    Serial.print("steps: ");
    Serial.println(stepCount_m2);
    stepCount_m2++;
    if (stepCount_m2==10){
      myStepper.step(0);
    }
  }
  else if (motor_state==1 && motor_direction==4){
    myStepper2.step(-stepsPerRevolution);
    Serial.print("motor_2counterclockwise \n");
    Serial.print("steps: ");
    Serial.println(stepCount_m2);
    stepCount_m2--;
    if (stepCount_m2==0){
      myStepper.step(0);
    }
  }
  if (stepCount_m1==21){
    motor_state=0;
  }
  else if (stepCount_m1==0){
    motor_state=0;
  }

  delay(0);

}
