/* Justin Nguyen
 *  4/7/2017
 */
 
#include <Keypad.h>
#include <WaveHC.h>
#include <WaveUtil.h>

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the volumes root directory
FatReader f;
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

uint8_t dirLevel; // indent level for file/dir names    (for prettyprinting)
dir_t dirBuf;     // buffer for directory reads

#define error(msg) error_P(PSTR(msg))

int hookSwitch = A5;
const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns0

//define the cymbols on the buttons of the keypads
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

int cap = 4;
int funCap = 3;
char sol[5] = {'\0', '\0', '\0', '\0', '\0'};
char fun[4] = {'\0', '\0', '\0', '\0'};
String solution = "5561";
String funny = "911";
int solPointer = 0;
int funPointer = 0;
boolean dialing = false;
char* ringer = "Ringer.WAV";
char* message = "SODM1.WAV";
char* easterEgg = "test1.WAV";
char* buzzing = "buzzing.WAV";

// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins
byte rowPins[ROWS] = {6, 7, 8, 9}; //connect to the row pinouts of the keypad
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = {A0, A1, A2}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS); 

void setup(){
  Serial.begin(9600);           // set up Serial library at 9600 bps for debugging

  putstring_nl("\nWave test!");  // say we woke up!
  
  putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(FreeRam());
  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you

  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
    error("Card init. failed!");  // Something went wrong, lets print out why
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
  
  // Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {   // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                           // we found one, lets bail
  }
  if (part == 5) {                     // if we ended up not finding one  :(
    error("No valid FAT partition!");  // Something went wrong, lets print out why
  }
  
  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(), DEC);     // FAT16 or FAT32?
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    error("Can't open root dir!");      // Something went wrong,
  }
  
  // Whew! We got past the tough parts.
  putstring_nl("Files found (* = fragmented):");

  // Print out all of the files in all the directories.
  root.ls(LS_R | LS_FLAG_FRAGMENTED);

  //Initialize hook switch
  pinMode(hookSwitch, INPUT);
  customKeypad.addEventListener(keypadEvent);
}
  
void loop(){
  int conn = digitalRead(hookSwitch);
  char customKey = customKeypad.getKey();

  if(!conn){
    reset(&solPointer, sol, cap);
    reset(&solPointer, fun, funCap);
    dialing = false;
  }
  else{
    if(!dialing){
      playfile2(buzzing);
    }
    else{
      if(solPointer < cap && customKey != '\0') {
        sol[solPointer] = customKey;
        solPointer++;
      }
      else if(solPointer == cap){
        solPointer = 0;
      }
    
      if(funPointer < funCap && customKey != '\0') {
        fun[funPointer] = customKey;
        funPointer++;
      }
      else if(funPointer == funCap){
        funPointer = 0;
      }
      
      switch(customKey){
        case '0':
          playfile("0.WAV");
          break;
        case '1':
          playfile("1.WAV");
          break;
        case '2':
          playfile("2.WAV");
          break;
        case '3':
          playfile("3.WAV");
          break;
        case '4':
          playfile("4.WAV");
          break;
        case '5':
          playfile("5.WAV");
          break;
        case '6':
          playfile("6.WAV");
          break;
        case '7':
          playfile("7.WAV");
          break;
        case '8':
          playfile("8.WAV");
          break;
        case '9':
          playfile("9.WAV");
      }
  
      String answer = String(sol);
      String police = String(fun);
      Serial.println("Answer = ");
      Serial.println(answer);
      Serial.println("Police = ");
      Serial.println(police);
      if(answer.equals(solution)){
        playcomplete(ringer);
        playcomplete(message);
        reset(&solPointer, sol, cap);
        reset(&funPointer, fun, funCap);
      }
    
      if(police.equals(funny)){
        playcomplete(ringer);
        playcomplete(easterEgg);
        reset(&solPointer, sol, cap);
        reset(&funPointer, fun, funCap);
      }
      //delay(1000);
    }
  }
}

/////////////////////////////////// HELPERS
/*
 * print error message and halt
 */

void reset(int* pointer, char* solution, int capacity){
  *pointer = 0;
  for(int k = 0; k < capacity; k++){
    solution[*pointer] = '\0';
    *pointer = *pointer + 1;
  }
  *pointer = 0;
  dialing = false;
}

void keypadEvent(KeypadEvent key){
  switch (customKeypad.getState()){
    case PRESSED:
      dialing = true;
      wave.stop();
      break;
    case RELEASED:
    break;
    case HOLD:
    break;
  }
}

void error_P(const char *str) {
  PgmPrint("Error: ");
  SerialPrint_P(str);
  sdErrorCheck();
  while(1);
}
/*
 * print error message and halt if SD I/O error, great for debugging!
 */
void sdErrorCheck(void) {
  if (!card.errorCode()) return;
  PgmPrint("\r\nSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  PgmPrint(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}
/*
 * play recursively - possible stack overflow if subdirectories too nested
 */

void play(FatReader &dir) {
  FatReader file;
  while (dir.readDir(dirBuf) > 0) {    // Read every file in the directory one at a time
  
    // Skip it if not a subdirectory and not a .WAV file
    if (!DIR_IS_SUBDIR(dirBuf)
         && strncmp_P((char *)&dirBuf.name[8], PSTR("WAV"), 3)) {
      continue;
    }

    Serial.println();            // clear out a new line
    
    for (uint8_t i = 0; i < dirLevel; i++) {
       Serial.write(' ');       // this is for prettyprinting, put spaces in front
    }
    if (!file.open(vol, dirBuf)) {        // open the file in the directory
      error("file.open failed");          // something went wrong
    }
    
    if (file.isDir()) {                   // check if we opened a new directory
      putstring("Subdir: ");
      printEntryName(dirBuf);
      Serial.println();
      dirLevel += 2;                      // add more spaces
      // play files in subdirectory
      play(file);                         // recursive!
      dirLevel -= 2;    
    }
    else {
      // Aha! we found a file that isnt a directory
      putstring("Playing ");
      printEntryName(dirBuf);              // print it out
      if (!wave.create(file)) {            // Figure out, is it a WAV proper?
        putstring(" Not a valid WAV");     // ok skip it
      } else {
        Serial.println();                  // Hooray it IS a WAV proper!
        wave.play();                       // make some noise!
        
        uint8_t n = 0;
        while (wave.isplaying) {// playing occurs in interrupts, so we print dots in realtime
          putstring(".");
          if (!(++n % 32))Serial.println();
          delay(100);
        }       
        sdErrorCheck();                    // everything OK?
        // if (wave.errors)Serial.println(wave.errors);     // wave decoding errors
      }
    }
  }
}

void playcomplete(char *name) {
  // call our helper to find and play this name
  Serial.println(name);
  playfile(name);
  while (wave.isplaying) {
  // do nothing while its playing
  }
  // now its done playing
}
 
void playfile(char *name) {
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
    putstring("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
}

void playfile2(char *name) {
  // see if the wave object is currently doing something
  /*if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }*/
  // look in the root directory and open the file
  if (!f.open(root, name)) {
    putstring("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
}
