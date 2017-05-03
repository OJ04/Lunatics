#define SONAR_PWR_PIN 2

#define BUTTON_PIN 4

//internal button variables for debouncing
int btn;        //current state
int btn_t;      //time since state change

//vibration state
boolean button_state = false;
boolean vibrate_on = true;

//define vibrators: pin, last value given to the pin
int vibrator_right[] = {3,0};
int vibrator_front[] = {10,0};
int vibrator_left[] = {9,0};

// defines sonar pins trigger, echo respectively
int sonar_right[] = {8,7};
int sonar_front[] = {5,6};
int sonar_left[] = {12,11};

//max distance of measurement
const int max_dist = 412; //change treshold 10cm

//lenght of buffer containing last measurements(not in order)
const int buffer_len = 5;
//index where to put new distance reading
int buffer_index = 0;

//buffers containing last measurements(not in order)
//measurements are not in order to avoid shifting each buffer everytime a new
//reading is measured, which would be slow
int d_buffer_right[buffer_len];
int d_buffer_front[buffer_len];
int d_buffer_left[buffer_len];

//time since last change
unsigned long pulse_interval = 0;
unsigned long interval_start_time = 0;
const int pulse_duration = 100;

void setup() {
  //init sonar pins
  initSonar(sonar_right);
  initSonar(sonar_front);
  initSonar(sonar_left);

  //set vibrator pins as output
  pinMode(vibrator_right[0],OUTPUT);
  pinMode(vibrator_front[0],OUTPUT);
  pinMode(vibrator_left[0],OUTPUT);

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
  int d = getDistance(sonar_right);
  d_buffer_right[buffer_index] = d;
  //Serial.print(d);
  //Serial.print(" ");
  d = getDistance(sonar_front);
  d_buffer_front[buffer_index] = d;
  //Serial.print(d);
  //Serial.print(" ");
  d = getDistance(sonar_left);
  d_buffer_left[buffer_index] = d;
  //Serial.println(d);

  //Serial.println(read_button());

  //increase index to next slot keeping in range 0-buffer_len-1
  buffer_index = (buffer_index+1)%buffer_len;

  //when putton is released, change vibrate on/off
  if(button_state == true && read_button() == false) {
    vibrate_on = !vibrate_on;
    button_state = false;
  }
  else {
    button_state = read_button();
  }

  int min_distance = min(min(average(d_buffer_front),average(d_buffer_left)),average(d_buffer_right));
  Serial.print("Min distance: ");
  Serial.println(min_distance);
  pulse_interval = distToInterval(min_distance);


  //vibrate in "on" state
  if(vibrate_on) {

    if(pulse_interval > 0) {
        //Serial.print("Interval: ");
        //Serial.println(pulse_interval);

      //start pulsing
      if(interval_start_time == 0) interval_start_time = millis();
      //if the pulse sequence (interval+pulse duration) is over, stop vibration
      if((millis()-interval_start_time) > (pulse_interval+pulse_duration)) {
        vibrate(vibrator_right,0);
        vibrate(vibrator_front,0);
        vibrate(vibrator_left,0);
        Serial.println("End pulse.");
        //reset interval, set sequence start timestamp
        interval_start_time = millis();
      }
      //if the interval amount of time has passed, start pulse
      else if((millis()-interval_start_time) > pulse_interval) {
        vibrate(vibrator_right,distToStrength(average(d_buffer_right)));
        vibrate(vibrator_front,255);
        vibrate(vibrator_left,distToStrength(average(d_buffer_left)));
        Serial.println("Start pulse");
      }
    }
    else {
      interval_start_time = 0;
      vibrate(vibrator_right,0);
      vibrate(vibrator_front,0);
      vibrate(vibrator_left,0);

    }
  } 
  else {
    vibrate(vibrator_right,0);
    vibrate(vibrator_front,0);
    vibrate(vibrator_left,0);
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

//convert distance to pulse interval
int distToInterval(int d)
{
  //if no reading set strength to 0
  if(d == 0)
  {
    return 0;
  }

  if(d < 150) {
    return (d*8);
  }
  else return 0;
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


