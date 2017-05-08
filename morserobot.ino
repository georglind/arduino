// Morse Robot
//

//   Board                     SERVO_PIN_A   SERVO_PIN_B   SERVO_PIN_C
//   -----                     -----------   -----------   -----------
//   Arduino Uno, Duemilanove       9            10          (none)
//   Arduino Mega                  11            12            13
//   Sanguino                      13            12          (none)
//   Teensy 1.0                    17            18            15
//   Teensy 2.0                    14            15             4
//   Teensy++ 1.0 or 2.0           25            26            27

#include <PWMServo.h> 
 
PWMServo larm;
PWMServo rarm;

// iterators
int i = 0;
int j = 0;
int k = 0;

// servo pars
int lpos = 0;
int rpos = 0;
int l0 = 0;
int l1 = 0;
int r0 = 0;
int r1 = 0;
 
char message[] = "tak";

void setup() 
{ 
  Serial.begin(9600);
  larm.attach(SERVO_PIN_A);  // attach servo pin 9 
  rarm.attach(SERVO_PIN_B);
}

void loop() 
{ 
   writemessage();
   newword(); 
} 
void writemessage() 
{
   for (k=0; k<sizeof(message) - 1; k+=1)
   {
     Serial.print(i);
     Serial.print(message[k]);
     if (message[i] == ' ')
     {
       newword();
     }
     else
     {
       char letter[] = "      "; 

       lettertomorse(message[k], letter);
       
       Serial.print(letter);
       
       for (j=0; j<sizeof(letter)-1; j+=1)
       {
         if (letter[j] == '-')
         {
           dash();
         }
         if (letter[j] == '.')
         {
           dot();
         }
       }
       newchar();
     }
   }
} 

char lettertomorse(char ch, char *m)
{
   switch(ch) {
      case 'a': 
//          m = ".-    ";
          m[0] = '.';
          m[1] = '-';
      break;
      case 'b':
//          m = "-...  ";
          m[0] = '-';
          m[1] = '.';
          m[2] = '.';
          m[3] = '.';
      break;
      case 'c': 
//          m = "-.-.  ";
          m[0] = '-';
          m[1] = '.';
          m[2] = '-';
          m[3] = '.';
      break;
      case 'd': 
//          m = "-..   ";
          m[0] = '-';
          m[1] = '.';
          m[2] = '.';
      break;
      case 'e': 
//          m = ".     ";
          m[0] = '.';
      break;
      case 'f': 
//          m = "..-.  ";
          m[0] = '.';
          m[1] = '.';
          m[2] = '-';
          m[3] = '.';
      break;
      case 'g': 
//          m = "--.   ";
          m[0] = '-';
          m[1] = '-';
          m[2] = '.';
      break;
      case 'h': 
//          m = "....  ";
          m[0] = '.';
          m[1] = '.';
          m[2] = '.';
          m[3] = '.';
      break;
      case 'i': 
//          m = "..    ";
          m[0] = '.';
          m[1] = '.';        
      break;
      case 'j': 
//          m = ".---  ";
          m[0] = '.';
          m[1] = '-';
          m[2] = '-';
          m[3] = '-';
      break;
      case 'k': 
//          m = "-.-   ";
          m[0] = '-';
          m[1] = '.';
          m[2] = '-';
      break;
      case 'l': 
//          m = ".-..  ";
          m[0] = '.';
          m[1] = '-';
          m[2] = '.';
          m[3] = '.';
      break;
      case 'm': 
//          m = "--    ";
          m[0] = '-';
          m[1] = '-';
      break;
      case 'n': 
//          m = "-.    ";
          m[0] = '-';
          m[1] = '.';
      break;
      case 'o': 
//          m = "---   ";
          m[0] = '-';
          m[1] = '-';
          m[2] = '-';
      break;
      case 'p': 
//          m = ".--.  ";
          m[0] = '.';
          m[1] = '-';
          m[2] = '-';
          m[3] = '.';
      break;
      case 'q': 
//          m = "--.-  ";
          m[0] = '-';
          m[1] = '-';
          m[2] = '.';
          m[3] = '-';
      break;
      case 'r': 
//          m = ".-.   ";
          m[0] = '.';
          m[1] = '-';
          m[2] = '.';
      break;
      case 's': 
//          m = "...   ";
          m[0] = '.';
          m[1] = '.';
          m[2] = '.';
      break;
      case 't': 
//          m = "-     ";
          m[0] = '-';
      break;
      case 'u': 
//          m = "..-   ";
          m[0] = '.';
          m[1] = '.';
          m[2] = '-';
      break;
      case 'v': 
//          m = "...-  ";
          m[0] = '.';
          m[1] = '.';
          m[2] = '.';
          m[3] = '-';
      break;
      case 'w': 
//          m = ".--   ";
          m[0] = '.';
          m[1] = '-';
          m[2] = '-';
      break;  
      case 'x': 
//          m = "-..-  ";
          m[0] = '-';
          m[1] = '.';
          m[2] = '.';
          m[3] = '-';
      break;
      case 'y': 
//          m = "-.--  ";
          m[0] = '-';
          m[1] = '.';
          m[2] = '-';
          m[3] = '-';
      break;
      case 'z': 
//          m = "--..  ";
          m[0] = '-';
          m[1] = '-';
          m[2] = '.';
          m[3] = '.';
      break;
      case '.':
//          m = ".-.-.-";
          m[0] = '.';
          m[1] = '-';
          m[2] = '.';
          m[3] = '-';
          m[4] = '.';
          m[5] = '-';
      break;
      case '?':
//          m = "..--..";
          m[0] = '.';
          m[1] = '.';
          m[2] = '-';
          m[3] = '-';
          m[4] = '.';
          m[5] = '.';
      break;
   }
}

void moveleftarm(int l0, int l1)
{
   float dl = (l1 - l0)/100.0;
   
   for(i = 0; i<=100; i+=1)     // goes from 180 degrees to 0 degrees 
   {           
     lpos = (int) i*dl;
     lpos += l0;
     larm.write(lpos);              // tell servo to go to position in variable 'pos' 
     delay(6);
   }
}


void movebotharms(int l0, int l1, int r0, int r1)
{
   float dl = (l1 - l0)/100.0;
   float dr = (r1 - r0)/100.0;
   
   for(i = 0; i<=100; i+=1)     // goes from 180 degrees to 0 degrees 
   {           
     lpos = (int) i*dl;
     lpos += l0;
     larm.write(lpos);              // tell servo to go to position in variable 'pos' 
     rpos = (int) i*dr;
     rpos += r0;
     rarm.write(rpos);              // tell servo to go to position in variable 'pos' 
     delay(6);
   }
}

void dot()
{
  int l0 = 110;
  int l1 = 10;

   moveleftarm(l0, l1);
   delay(400);
   moveleftarm(l1, l0);
   delay(400); 
}

void dash()
{
  int l0 = 110;
  int l1 = 10;
  int r0 = 110;
  int r1 = 10;

  movebotharms(l0, l1, r0, r1);
  delay(400);
  movebotharms(l1, l0, r1, r0);
  delay(400);
}

void newchar()
{
  delay(1800);
}

void newword()
{
  delay(8000);
}