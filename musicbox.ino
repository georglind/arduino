// Musicbox 
// based somewhat on WaveHC code

#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include "WaveUtil.h"
#include "WaveHC.h"

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play

WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

#define DEBOUNCE 5  // button debouncer

char files[20][13];
uint8_t dirLevel; // indent level for file/dir names    (for prettyprinting)
dir_t dirBuf;     // buffer for directory reads

int playnext = 0;
int playing = 0;
int maxfiles = 0;

int buttonnext = 18;
int buttonstop = 19;
int onled = 6;   // on when the jukebox is on.

// Here is where we define the buttons that we'll use. button "1" is the first, button "6" is the 6th, etc
// In this modified version we __only__ use 2 buttons (play and stop: 18 and 19)
byte buttons[] = {14, 15, 16, 17, buttonnext, buttonstop};
// This handy macro lets us determine how big the array up above is, by checking the size
#define NUMBUTTONS sizeof(buttons)
// we will track if a button is just pressed, just released, or 'pressed' (the current state
volatile byte pressed[NUMBUTTONS], justpressed[NUMBUTTONS], justreleased[NUMBUTTONS];


// this handy function will return the number of bytes currently free in RAM, great for debugging!   
int freeRam(void)
{
  extern int  __bss_end; 
  extern int  *__brkval; 
  int free_memory; 
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end); 
  }
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval); 
  }
  return free_memory; 
} 

void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}

void setup() {
  byte i;
  
  // set up serial port
  Serial.begin(9600);
  putstring_nl("WaveHC with ");
  Serial.print(NUMBUTTONS, DEC);
  putstring_nl("buttons");
  
  putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(freeRam());      // if this is under 150 bytes it may spell trouble!
  
  // Set the output pins for the DAC control. This pins are defined in the library
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
 
  // pin13 LED
  pinMode(13, OUTPUT);
  
  // ON LED
  pinMode(onled, OUTPUT);
  digitalWrite(onled, HIGH);

  // Make input & enable pull-up resistors on switch pins
  for (i=0; i< NUMBUTTONS; i++) {
    pinMode(buttons[i], INPUT);
    digitalWrite(buttons[i], HIGH);
  }
  
  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
    putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
    sdErrorCheck();
    while(1);                            // then 'halt' - do nothing!
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
 
// Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
    putstring_nl("No valid FAT partition!");
    sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }
  
  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }
  
  // Whew! We got past the tough parts.
  putstring_nl("Ready!");
  
  TCCR2A = 0;
  TCCR2B = 1<<CS22 | 1<<CS21 | 1<<CS20;

  //Timer2 Overflow Interrupt Enable
  TIMSK2 |= 1<<TOIE2;

  indexFiles(root);
}

SIGNAL(TIMER2_OVF_vect) {
  check_switches();
}

void check_switches()
{
  static byte previousstate[NUMBUTTONS];
  static byte currentstate[NUMBUTTONS];
  byte index;

  for (index = 0; index < NUMBUTTONS; index++) {
    currentstate[index] = digitalRead(buttons[index]);   // read the button
    
    /*     
    Serial.print(index, DEC);
    Serial.print(": cstate=");
    Serial.print(currentstate[index], DEC);
    Serial.print(", pstate=");
    Serial.print(previousstate[index], DEC);
    Serial.print(", press=");
    */
    
    if (currentstate[index] == previousstate[index]) {
      if ((pressed[index] == LOW) && (currentstate[index] == LOW)) {
          // just pressed
          justpressed[index] = 1;
      }
      else if ((pressed[index] == HIGH) && (currentstate[index] == HIGH)) {
          // just released
          justreleased[index] = 1;
      }
      pressed[index] = !currentstate[index];  // remember, digital HIGH means NOT pressed
    }
    //Serial.println(pressed[index], DEC);
    previousstate[index] = currentstate[index];   // keep a running tally of the buttons
  }
}

/*
 * list recursively - possible stack overflow if subdirectories too nested
 */
void indexFiles(FatReader &d)
{
  int8_t r;                     // indicates the level of recursion
  int8_t i; 
  int8_t j;
  int count=0;
  
  while ((r = d.readDir(dirBuf)) > 0) {     // read the next file in the directory 
    // skip subdirs . and ..
    if (dirBuf.name[0] == '.') 
      continue;
    
    for (uint8_t i = 0; i < dirLevel; i++) 
      Serial.print(' ');        // this is for prettyprinting, put spaces in front
   // printName(dirBuf);          // print the name of the file we just found
    Serial.println();           // and a new line
    
    i = 0; j = 0;
    while (i < 11)
    {
     if (dirBuf.name[i] != ' ')
     { 
       if (i == 8) 
       {
           files[count][j] = '.';
           Serial.print('.');           // after the 8th letter, place a dot
           j++;
       }
       files[count][j] = dirBuf.name[i];
       Serial.print(dirBuf.name[i]);      // print the n'th digit
       j++;
     }
     i++;
    }
    
    count++;
  }
  
  maxfiles = count;
  
  sdErrorCheck();                  // are we doing OK?
}

void loop() {
  byte i;

  if (justpressed[0]) {
    justpressed[0] = 0;
    playfile(files[1]);
  }
  else if (justpressed[1]) {
      justpressed[1] = 0;
      playfile("RE.WAV");
  }
  else if (justpressed[2]) {
      justpressed[2] = 0;
      playfile("MI.WAV");
  }
  else if (justpressed[3]) {
      justpressed[3] = 0;
      playfile("FA.WAV");
  } 
  else if (justpressed[4]) {
      justpressed[4] = 0;
      if (wave.isplaying) {// already playing something, so stop it!
        wave.stop(); // stop it
        playing = 0;
        playnext = (playnext-1)%(maxfiles-1);
      }
  }
  else if (justpressed[5]) {
      justpressed[5] = 0;
      playnext();
  }
  
  if (playing==1 && !wave.isplaying) {
     playnext(); 
  }
}

// Plays the next song
void playnext() {
  playfile(files[playnext]);
  playnext = (playnext+1)%(maxfiles-1);
}

// Plays a full file from beginning to end with no pause.
void playcomplete(char *name) {
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
  // do nothing while its playing
  }
  // now its done playing
}

// Interrupt play and skip to next song
void playfile(char *name) {
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
    putstring("Couldn't open file "); Serial.print(name); Serial.println(); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV"); Serial.println(); return;
  }
  
  // ok time to play! start playback
  wave.play();
  playing = 1;
}