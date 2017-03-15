#define SONAR_PWR_PIN 2

int vibrator_1[] = {3,0};
int vibrator_2[] = {10,0};
int vibrator_3[] = {9,0};


// defines sonar pins trigger, echo respectively
int sonar_1[] = {8,7};
int sonar_2[] = {5,6};
int sonar_3[] = {12,11};

const int buffer_len = 10;
int buffer_index = 0;

int d_buffer_1[buffer_len];
int d_buffer_2[buffer_len];
int d_buffer_3[buffer_len];


void setup() {
  initSonar(sonar_1);
  initSonar(sonar_2);
  initSonar(sonar_3);

  pinMode(vibrator_1[0],OUTPUT);
  pinMode(vibrator_2[0],OUTPUT);
  pinMode(vibrator_3[0],OUTPUT);
  
  pinMode(SONAR_PWR_PIN, OUTPUT);
  Serial.begin(115200);
  digitalWrite(SONAR_PWR_PIN,HIGH);
}



void loop() {
  delay(50);
  int d = getDistance(sonar_1);
  d_buffer_1[buffer_index] = d;
  Serial.print(d);
  Serial.print(" ");
  d = getDistance(sonar_2);
  d_buffer_2[buffer_index] = d;
  Serial.print(d);
  Serial.print(" ");
  d = getDistance(sonar_3);
  d_buffer_3[buffer_index] = d;
  Serial.println(d);
  
  buffer_index = (buffer_index+1)%buffer_len;

  
  
  vibrate(vibrator_1,distToStrength(average(d_buffer_1)));
  vibrate(vibrator_2,distToStrength(average(d_buffer_2)));
  vibrate(vibrator_3,distToStrength(average(d_buffer_3)));
  
}
int distToStrength(int d)
{
  if(d == 0)
  {
    return 0;
  }
  return max(200-d,0);
}
void vibrate(int vibrator[], int strength)
{
  
  if(vibrator[1] == 0 && strength > 0)
  {
    analogWrite(vibrator[0],255);
    delay(1);
  }
  analogWrite(vibrator[0],strength);
  vibrator[1] = strength;
}


int average(int bufr[])
{
  int sum = 0;
  for(int n=0;n<buffer_len;n++)
  {
    sum += bufr[n];
    
  }
 
  return sum/buffer_len;
}

void initSonar(int sonar[])
{
  pinMode(sonar[0],OUTPUT);
  pinMode(sonar[1],INPUT);
}





int getDistance(int sonar[]) // returns the distance (cm)
{
long duration, distance;

digitalWrite(sonar[0], HIGH);
delayMicroseconds(10);
digitalWrite(sonar[0], LOW);

duration = pulseIn(sonar[1], HIGH, 20000); // We wait for the echo to come back, with a timeout of 20ms, which corresponds approximately to 3m

// pulseIn will only return 0 if it timed out. (or if echoPin was already to 1, but it should not happen)
if(duration == 0) // If we timed out
{
  digitalWrite(SONAR_PWR_PIN,LOW);
  delay(1);
  digitalWrite(SONAR_PWR_PIN,HIGH);
}

distance = (duration/2) / 29.1; // We calculate the distance (sound speed in air is aprox. 291m/s), /2 because of the pulse going and coming

return distance; //We return the result. Here you can find a 0 if we timed out
}
