#define SONAR_PWR_PIN 2

#define BUTTON_PIN 4

//internal button variables for debouncing
int btn;        //current state
int btn_t;      //time since state change

//vibration state
boolean button_state = false;
boolean vibrate_on = false;

//define vibrators: pin, last value given to the pin
int vibrator_1[] = {3,0};
int vibrator_2[] = {10,0};
int vibrator_3[] = {9,0};


// defines sonar pins trigger, echo respectively
int sonar_1[] = {8,7};
int sonar_2[] = {5,6};
int sonar_3[] = {12,11};

//max distance of measurement
const int max_dist = 412; //change treshold 10cm

//lenght of buffer containing last measurements(not in order)
const int buffer_len = 5;
//index where to put new distance reading
int buffer_index = 0;

//buffers containing last measurements(not in order)
//measurements are not in order to avoid shifting each buffer everytime a new
//reading is measured, which would be slow
int d_buffer_1[buffer_len];
int d_buffer_2[buffer_len];
int d_buffer_3[buffer_len];

//buffers containing changes since last (about) 2 seconds (a loop takes about 83ms)
const int c_treshold = 15; //change treshold 10cm
const int c_buffer_length = 28;
int c_buffer_1[c_buffer_length];
int c_buffer_2[c_buffer_length];
int c_buffer_3[c_buffer_length];
int c_buffer_index = 0;

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

  button_state = read_button();
}

void loop() {
  //limit to make buffer readings span longer time period and make
  //serial output more readable
  //delay(10);
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

  Serial.println(read_button());

  //increase index to next slot keeping in range 0-buffer_len-1
  buffer_index = (buffer_index+1)%buffer_len;

  c_buffer_index = (c_buffer_index+1)%c_buffer_length;
  c_buffer_1[c_buffer_index] = average(d_buffer_1);
  c_buffer_2[c_buffer_index] = average(d_buffer_2);
  c_buffer_3[c_buffer_index] = average(d_buffer_3);

  //when putton is released, change vibrate on/off
  if(button_state == true && read_button() == false) {
    vibrate_on = !vibrate_on;
    button_state = false;
  }
  else {
    button_state = read_button();
  }

  //vibrate in "on" state
  if(vibrate_on) {

    //calculate vibration strength from average of last measuremnts
    //and update vibrators accordingly
    vibrate(vibrator_1,distToStrength(average(d_buffer_1)));
    vibrate(vibrator_2,distToStrength(average(d_buffer_2)));
    vibrate(vibrator_3,distToStrength(average(d_buffer_3)));
  }
  else {
    vibrate(vibrator_1,0);
    vibrate(vibrator_2,0);
    vibrate(vibrator_3,0);
  }
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
  //value between 0 and 255
  //return min(max((20000/(d+70)-60),0),255);
  return min(max((20000/(d+20)-50),0),255);
}

//update vibrator with given strength
void vibrate(int vibrator[], int strength)
{

  //if vibrator turned off, give 1ms pulse of highest strength
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
  int amt = 0;
  for(int n=0;n<buffer_len;n++)
  {
    amt++;
    if(bufr[n] < max_dist) sum += bufr[n];
  }
  return sum/amt;
}

boolean significant_change(int bufr[])
{
  int min_value = bufr[0];
  int max_value = bufr[0];

  for(int n=1;n<c_buffer_length;n++)
  {
    min_value = min(bufr[n],min_value);
    max_value = max(bufr[n],max_value);
  }

  if((max_value-min_value) > c_treshold) {
    Serial.print("significant: ");
    Serial.println((max_value-min_value));
  }
  else return false;
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

//wait for echo with timeout 24ms
duration = pulseIn(sonar[1], HIGH, 24000);

//if timeout, reset the sonars to get the
//sonars stop waiting for echo
if(duration == 0)
{
  digitalWrite(SONAR_PWR_PIN,LOW);
  delay(1);
  digitalWrite(SONAR_PWR_PIN,HIGH);
}

if(duration > 0) {
  //calculate the distance (sound speed in air is aprox. 291m/s), /2 because of the pulse going and coming
  distance = (duration/2) / 29.1;
  
  return distance; //return the distance. 0 if we timed out
}
else return max_dist; //max distance with timeout 24ms
}

boolean read_button()
{
  int b = digitalRead(BUTTON_PIN);
  if(btn != b)
  {
    if(btn_t == -1)
    {
      btn_t = millis();
    }
    else if(millis()-btn_t > 50)
    {
      //if reading has not changed in 50ms, then accept it as the current state
      btn = b;
      btn_t = -1;
    }
  }
  return (boolean)btn;
}


