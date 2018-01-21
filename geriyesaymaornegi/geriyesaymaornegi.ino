
//Relay Data  --only need these 2 -this replaces HIGH -LOW for facility since these relays
//have LOW as normally on
#define RELAY_ON 0
#define RELAY_OFF 1

/* Declare variable pins- for me at this time*/
#define Relay_1 A1  //analog 1

static boolean pinOnA1 = false; //these pin choices are merely for easy id by me to keep up with what I'm doing
//and how my relays are connected

static unsigned long  offpinTimeA1 = 0; //variable to hold a large time value number


//more variables, declarations, definitions, and libraries here of course-left out for brevity

void setup(){
//other code here, including other initialization sequences

  //initalize pins so relays are inactive  
  digitalWrite(Relay_1, RELAY_OFF);    
  //add other relays as needed
  
  //set pins as outputs, 1 unique pin per relay 
  pinMode(Relay_1, OUTPUT);
  //add other pins as needed

}


/*----Timer section   -sensor activated by temperature variable f, to control a fan
for my personal use, I wanted my fanRuntime to be 900000 millis, or 15 minutes on and 3 hours 45 minutes fanOfftime for a total time of 4 hour cycles to be repeated constantly during a 24 hour period
Time holding variables are all unsigned longs----*/

//pinOnA1 is assigned to pin A1 for this example only and initially set to "false"--could be another pin assignment
//my if statements again are geared for my own unique needs and will be changed by you.

void loop() {

      if (f > 75 && f < 95 && pinOnA1 == false && millis() - offpinTimeA1 > fanOfftime)  
   {    
    digitalWrite(Relay_1, RELAY_ON);  //force relay 1 to on 
    pinOnA1 = true; //change pin state-- need to know pin state for setting On/Off (if) conditions       
    onpinTimeA1 = 0; //clear any old onpinTimeA1 value
    onpinTimeA1 = millis(); //current millis 
   }
       
   //must force the relay off by using this if statement--I am using a set length of time--
   //could be setup to be purely sensor based as well if needed or desired

   if (pinOnA1 == true && millis() - onpinTimeA1 > fanRuntime)  
   {
      digitalWrite(Relay_1, RELAY_OFF);  //force to off              
      pinOnA1 = false;  //reset pin state to false for re-evaluation in the "if" statements
      offpinTimeA1 = 0; //clear any old offpinTimeA1 value     
      offpinTimeA1 = millis(); //current millis 
    } 

//add any other code as appropriate to your situation


}
