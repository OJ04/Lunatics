#define SONAR_PWR_PIN 2

//define vibrators: pin, last value given to the pin
int vibrator_1[] = {3,0};
int vibrator_2[] = {10,0};
int vibrator_3[] = {9,0};


// defines sonar pins trigger, echo respectively
int sonar_1[] = {8,7};
int sonar_2[] = {5,6};
int sonar_3[] = {12,11};

//lenght of buffer containing last measurements(not in order)
const int buffer_len = 10;
//index where to put new distance reading
int buffer_index = 0;

//buffers containing last measurements(not in order)
//measurements are not in order to avoid shifting each buffer everytime a new
//reading is measured, which would be slow
int d_buffer_1[buffer_len];
int d_buffer_2[buffer_len];
int d_buffer_3[buffer_len];


void setup() {
  //init sonar pins
  initSonar(sonar_1);
  initSonar(sonar_2);
  initSonar(sonar_3);

  //set vibrator pins as output
  pinMode(vibrator_1[0],OUTPUT);
  pinMode(vibrator_2[0],OUTPUT);
  pinMode(vibrator_3[0],OUTPUT);

  //set sonar powering pin as output and set high to power the sonars
  pinMode(SONAR_PWR_PIN, OUTPUT);
  digitalWrite(SONAR_PWR_PIN,HIGH);

  //serial output for debugging
  Serial.begin(115200);
  
}

void loop() {
  //limit to make buffer readings span longer time period and make
  //serial output more readable
  delay(50);
  //read distances from sonars and print them to ouput and put them to
  //corresponding buffers
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

  //increase index to next slot keeping in range 0-buffer_len-1
  buffer_index = (buffer_index+1)%buffer_len;

  //calculate vibration strength from average of last measuremnts
  //and update vibrators accordingly
  vibrate(vibrator_1,distToStrength(average(d_buffer_1)));
  vibrate(vibrator_2,distToStrength(average(d_buffer_2)));
  vibrate(vibrator_3,distToStrength(average(d_buffer_3)));
  
}

//convert distance to vibration stregnth
int distToStrength(int d)
{
  //if no reading set strength to 0
  if(d == 0)
  {
    return 0;
  }

  //distance is mapped to vibration using an exponential
  //function, which gives maximum vibration at 0cm and
  //minimum at 263cm
  return min(max((20000/(d+70)-60),0),255);
}

//update vibrator with given strength
void vibrate(int vibrator[], int strength)
{

  //if vibrator turned of, give 1ms pulse of highest strength
  //to start the motor
  if(vibrator[1] == 0 && strength > 0)
  {
    analogWrite(vibrator[0],255);
    delay(1);
  }

  //write strength to the vibrator pin and
  //to the vibrator array
  analogWrite(vibrator[0],strength);
  vibrator[1] = strength;
}

//computes the average of all values in the distance buffer
int average(int bufr[])
{
  int sum = 0;
  for(int n=0;n<buffer_len;n++)
  {
    sum += bufr[n];
  }
  return sum/buffer_len;
}

//set sonar pins output and input properly
void initSonar(int sonar[])
{
  pinMode(sonar[0],OUTPUT);
  pinMode(sonar[1],INPUT);
}

//read distance from given sonar, returns distance in cm
int getDistance(int sonar[]) 
{
long duration, distance;

//pulse to start measurement
digitalWrite(sonar[0], HIGH);
delayMicroseconds(10);
digitalWrite(sonar[0], LOW);

//wait for echo with timeout 20ms
duration = pulseIn(sonar[1], HIGH, 20000);

//if timeout, reset the sonars to get the
//sonas stop waiting for echo
if(duration == 0)
{
  digitalWrite(SONAR_PWR_PIN,LOW);
  delay(1);
  digitalWrite(SONAR_PWR_PIN,HIGH);
}

//calculate the distance (sound speed in air is aprox. 291m/s), /2 because of the pulse going and coming
distance = (duration/2) / 29.1;

return distance; //return the distance. 0 if we timed out
}
