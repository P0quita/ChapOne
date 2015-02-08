
// Chapters Programming Language for Arduino

// Copyright 2014, 2015 David Locke
// This program is distributed under the terms of the GNU General Public License

/*

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
/*

#include <String.h>
#include <SD.h>

//General purpose File variable 
File myFile;

// SD Card Present?
boolean sdCardFlag = true;

// Arduino SD Card Shield: pin 4
const int chipSelect = 4;     // SD Card Shield Pin   

// Sensor pins
int sensor0 = A0;
int sensor1 = A1;
int sensor2 = A2;
int sensor3 = A3;
int sensor4 = A4;
int sensor5 = A5;
int sensor6 = A6;

// TextLine
String textLine;             // Current textline from any input file
String lcLine = "";          // Lowercase for matching strings

// Array of all code lines
String codeArray[35];         // Big code array
int codeLines = 0;           // Number of code lines

// Synonyms Array
String synonyms[35];
byte synonymLines = 0;

// General variables
boolean firstPass = true;     // First time through?
int multiplier = 0;  // Multiplies input if > 0

// Chapters
int chapters[35];
int chapter = 1;     // Current chapter

float pins[54];


// IF THEN - if sensor <comparison> threshold then variable = value

// Conditional Variables
String conditional[35];
byte sensor[35];     // Input pin
int comparison[35];  // Comparison operator < or >
int threshold[35];   // Threshold for action

// Assignment Variables
String assignment[35];
String variable[35]; // Variable to assign  
String value[35];    // Value for variable

int inputEcho = 10;    // Pin echos the input - used for meter
int chapterCount = 9;  // Pin shows chapter number in volts

// Variables user can set in Code.txt file
boolean kidCodeFlag = false;  // Set true if there is a kidcode flag in code.txt
float sampleRate = 1000;       // How often to delay between sensor readings
byte logFrequency = 5;        // How often to write to log
int logThreshold = 0;        // If > 0 then only log input changes > threshold

// System variables
float lastMillis;
float lastTime = millis();
int sensorValue = -1000;     // Main sensor value
int lastSensorValue = 2000;  // Previous sensor value


// Pin Map

// Global variables for pulsing pin fo blinking lights
int pulsePin[15];                   // 5 pins
unsigned long pulseHighLength[15];  // How long does it stay high?
unsigned long pulseLowLength[15];   // How long does it stay low?
unsigned long pulseUntil[15];       // Pulse until this time comes
boolean pulseMode[15];              // High (TRUE) or Low (FALSE)
int pulseCount = 0;                // How many pulses are we tracking?

int echoSensor1 = 44;              // Echo Sensor1
int echoSensor2 = 45;              // Echo Sensor2
int echoSensor3 = 46;              // Echo Sensor3




// -------- ************************* -------
// -------- *** UTILITY FUNCTIONS *** -------
// -------- ************************* -------

// Load pulse arrays for blinking pins
void registerPulse(int pin, int highLength, int lowLength) {
  // 

  pulsePin[pulseCount] = pin;
  pulseHighLength[pulseCount] = highLength;
  pulseLowLength[pulseCount] = lowLength;
  pulseUntil[pulseCount] = lowLength + millis();
  pulseMode[pulseCount] = false;
  pulseCount++;
}


// Make certain pins blink in patterns
void pulse() {
  for (int i=0; i< pulseCount; i++) {
    if (pulseUntil[i] < millis()) {
      pulseMode[i] = !pulseMode[i];
      digitalWrite(pulsePin[i], pulseMode[i]);  
      if (pulseMode[i]) 
        pulseUntil[i] = pulseHighLength[i] + millis(); 
      else  
        pulseUntil[i] = pulseLowLength[i] + millis(); 
    }
  }
}



// _________ PRINT FUNCTIONS   -----------

// Print Function
void pp(String myString){
  Serial.print(myString);
}

// Printline Function
void pl(String myString){
  Serial.println(myString);
}


// _________ OPEN FILE -----------

// Open the file fileNameString
// Only works for read not write
void openFile(String fileNameString) {

  char fileName[40];

  fileNameString.toCharArray(fileName, 40);

  // Open the file for reading:
  myFile = SD.open(fileName);
  if (myFile) {
    Serial.println("  " + fileNameString + " open");
  }
  else {
    // if the file didn't open, print an error:
    Serial.println("Error opening " + fileNameString);
    return; 
  }
}


// _________ GET REAL -----------

float getReal(String numString) {
  // Converts passed in string to float

  char number[10];

  // Move the string into a char array
  numString.toCharArray(number, 10);

  // Convert char array to float
  float f = atof(number);

  // Return the float
  return f; 
}


// ----------------- GET LINE----------------------

String getLine()
// Get a line from current file

{
  String newLine = "";
  char c = ' ';
  boolean goFlag = 1;

  // read from the file until there's nothing else in it:
  while (myFile.available() and goFlag) {
    c  = char(myFile.read());

    if (c != '\n') newLine += c;
    else goFlag = 0;        
  }

  newLine.trim();    
  return newLine;
}



// -------------------- POP ONE -------------------------------

String popOne()
// Get one word from textLine 
// Destructive - reduces global variable textLine
// Returns string

{
  char c;
  String tempLine;

  textLine += "  /";

  for ( int i = 0; i < textLine.length(); i++ ) {
    c = char(textLine[i]); 
    if ((c == '\n') || (c == '\t') || (c == ' ')) {
      tempLine = textLine.substring(0, i);
      textLine = textLine.substring(i);
      textLine.trim();
      return tempLine;
    }  
  }     
  return " ";
}



// _________ MATCH STRING -----------


boolean matchString(String keyword,  String myString) {
  // Check if keyword is found in myString
  // Handles one "*" wildcard character

  String part1 = "";
  String part2 = "";
  int p;

  boolean matchFlag = false;  
  boolean part1Flag = false;
  boolean part2Flag = false;

  if ((myString.indexOf("chapter") > -1) and (keyword.indexOf("chapter") > -1))
    pl("Found Chapter Search");

  // Look for wildcard
  p = keyword.indexOf("*");

  // if beginning or end of keyword - delete it
  if (p == 0) keyword = keyword.substring(1);
  else if (p == keyword.length()) 
    keyword.substring(0, keyword.length() - 1); 

  if (myString.indexOf(keyword) > -1) 
    matchFlag = true;

  // if no wildcard return the match flag
  if (p == -1) return matchFlag;

  // handle wildcards which means two parts
  else { 
    part1 = keyword.substring(0, p);
    part2 = keyword.substring(p+1);

    // match lefthand part
    if (myString.indexOf(part1) > -1) part1Flag = true;

    // match righthand part
    if (part2.length() == 0) part2Flag = true;
    else if (myString.indexOf(part2) > -1) part2Flag = true;
  }

  // If both parts are true - return true
  if ((part1Flag == 1) and (part2Flag == 1)) return true;
  else return false;
}



// _________ WRITE DEBUG FILER -----------

void writeDebugFile()
// Write information to the debug file

{
  int i =0;

  pl("");
  Serial.println(F("Write Debug File"));
  pl("");

  SD.remove("DEBUG.TXT");
  File myFile = SD.open("DEBUG.TXT", FILE_WRITE);

  if (myFile) {

    myFile.println(F("Debug File"));
    myFile.println();
    myFile.println(F("Assignments"));
    myFile.println(F("-----------"));
    myFile.print (F("sampleRate = "));
    myFile.println (sampleRate);
    myFile.print (F("kidCodeFlag = "));
    myFile.println (kidCodeFlag);
    myFile.print (F("logThreshold = "));
    myFile.println (logThreshold);
    myFile.print (F("logFrequency = "));
    myFile.println (logFrequency);
    myFile.print (F("multiplier = "));
    myFile.println (multiplier);
    myFile.println();
    myFile.println(F("Rules"));
    myFile.println(F("-----"));
    for (i = 0; i < codeLines; i++ )  { 
      // Print values

      if (variable[i].length() == 0) { 
        myFile.print ("Chapter ");
        myFile.print (chapter);
      }
      if ((comparison[i] == 1) or (comparison[i] == 2)) {
        myFile.print (F("If (Sensor "));
        if (comparison[i] == 1) myFile.print ("< ");
        else if (comparison[i] == 2) myFile.print ("> ");
        myFile.print (threshold[i]);
        myFile.print (F(") Then "));
      }      
      if (variable[i].length() > 0) {
        myFile.print (variable[i]);
        myFile.print (" = ");
        myFile.print (value[i]);
        myFile.println();
      }
    }
    myFile.close();
  }
}



// -------- ********************** -------
// -------- *** MAIN FUNCTIONS *** -------
// -------- ********************** -------




// ---------- LOAD CODE ARRAY ------------

void loadCodeArray() {
  // Load code.txt file into the code array

    int i = 0;
  int p = -1;
  String lcLine;

  Serial.println();
  Serial.println("Entering Load Code Array");
  Serial.println("------------------------");

  openFile("code.txt");

  // Load the code file into codeArray
  while (myFile.available()) {
    // Read a line from code.txt
    textLine = getLine();
    textLine.trim();
    lcLine = textLine;
    lcLine.toLowerCase();

    // Check to see whether Kidcode is turned on
    if ((lcLine.indexOf("kidcode") > -1) and (lcLine.indexOf("true")) > -1){
      kidCodeFlag = true;
      Serial.println();
      Serial.println(F("--Kidcode On--"));
    }
    // Only put it in the code array if it is an assignment or conditional
    else if ((lcLine.indexOf("=") > -1) or 
      (lcLine.indexOf("then ") > -1) or 
      (lcLine.indexOf("message") > -1) or 
      (lcLine.indexOf("exit") > -1) or 
      (lcLine.indexOf("chapter") > -1))  
    {
      p = textLine.indexOf("//");
      if (p > -1) textLine = textLine.substring(0, p);
      codeArray[i] = textLine;
      i++;
    }
  }

  // cdLines is the number of lines in the code array
  codeLines = i;

  myFile.close();
}



// ---------- LOAD KID CODE ------------

void loadKidCode() {
  // Load kidcode.txt file into the end of the code array

  int i = codeLines;
  String lcLine;

  Serial.println();
  Serial.println(F("Entering Load Kid Code"));
  Serial.println(F("----------------------"));

  openFile("kidcode.txt");

  // Load the kidecode file into the end of codeArray
  while (myFile.available()) {
    // read a line from kidcode.txt
    textLine = getLine();
    textLine.trim();
    lcLine = textLine;
    lcLine.toLowerCase();
    textLine = lcLine;

    int p = textLine.indexOf("//");
    if (p > -1) textLine = textLine.substring(0, p);

    textLine.replace("wait", "delay =");
    textLine.replace("the end", "exit");
    textLine.replace(" seconds", "000");
    textLine.replace(" second", "000");
    textLine.replace("sensor ", "sensor1 ");
    textLine.replace("step ", "chapter ");
    textLine.replace("part ", "chapter ");
    textLine.replace("goto ", "go to ");
    textLine.replace(" turn ", " then turn ");
    textLine.replace(" go to ", " then go to ");
    textLine.replace("then then ", "then ");
    textLine.replace("if if ", "if ");
    textLine.replace("go to chapter", "chapter = ");
    textLine.replace("when ", "if ");
    textLine.replace("message ", "topline =");
    textLine.replace("  ", " ");
    textLine.replace("  ", " ");
    textLine.replace("  ", " ");
    textLine.replace("  ", " ");
    textLine.trim();

    // Turn on pin - set to 5 volts
    if (matchString("turn on ", textLine) == true) {
      textLine.replace("turn on ", "");
      textLine.concat(" = 5.0");
    }

    // Turn off pin - set to 5 volts
    if (matchString("turn off ", textLine) == true) {
      textLine.replace("turn off ", "");
      textLine.concat(" = 0.0");
    }

    // Ignore blank lines
    if (textLine.length() > 0) {
      codeArray[i] = textLine;
      i++;
    }
  }

  // codeLines is the number of lines in the code array
  codeLines = i;

  myFile.close();
}



// ---------- LOAD SYNONYMS ------------

void loadSynonyms() {
  // Load synonyms.txt into the synonyms array

  Serial.println();
  Serial.println(F("Entering Load Synonyms"));
  Serial.println(F("----------------------"));

  openFile("synonyms.txt");

  // Load the code file into codeArray
  while (myFile.available()) {
    // read a line from synonyms.txt
    textLine = getLine();
    textLine.trim();
    synonyms[synonymLines] = textLine;
    if (textLine.length() > 0) {
      pp("  ");
      Serial.println(textLine);
    }
    synonymLines++;
  }
  myFile.close();
}




// ---------- PROCESS SYNONYMS ------------


void processSynonyms() {

  // Parse through each of the codeLine statements and compare them to 
  // the synonyms in the synonyms array

  int i;
  int j;
  int divide = -1;
  int found = -1;
  int foundTwo = -1;

  String keyword = "";
  String synonym = "";
  String tmpLine = "";


  pl("  ");
  Serial.println(F("Entering Process Synonyms"));
  Serial.println(F("-------------------------"));

  for (i=0; i< codeLines; i++) {

    codeArray[i].replace("exit", "exit = true");
    if ((codeArray[i].indexOf("hapter") > -1) and (codeArray[i].indexOf("=") == -1)) pp("  ");   
    else pp("    ");
    Serial.println(codeArray[i]);

    // Blank out conditional and assignment values
    conditional[i] = "";
    assignment[i] = "";
    sensor[i] = 0;
    comparison[i] = 0;
    threshold[i] = 0;
    variable[i] = "";
    value[i] = "";

    chapters[i] = chapter;

    // Get line from code array
    textLine = codeArray[i];

    textLine.toLowerCase();

    // Is there a conditional?
    divide = textLine.indexOf("then ");

    // If there is a conditional...
    if (divide > -1) {
      // Divide the if-then statement at then
      conditional[i] = codeArray[i].substring(0, divide);
      assignment[i] = codeArray[i].substring(divide + 5);
    }

    // No condiitonal so just assignment
    else {
      assignment[i] = codeArray[i];
      conditional[i] = "";
    }

    // Lower case so we can match
    assignment[i].toLowerCase();
    conditional[i].toLowerCase();
    chapters[i] = chapter;

    // Parse synonyms array and pull out keyword & synonym
    for (j = 0; j < synonymLines; j++) {

      // Get a line from synonyms array
      textLine = synonyms[j];
      textLine.trim();
      textLine.toLowerCase();

      // See if it has a double colon in it - means replace whole phrase
      foundTwo = textLine.indexOf("::");
      textLine.replace("::", ":");

      // See if it has a colon in it - valid synonyms line
      divide = textLine.indexOf(":");

      // If not, ignore it
      if (divide == -1) ;

      // Else, it has synonyms
      else {

        keyword = textLine.substring(0, divide);
        synonym = textLine.substring(divide + 1);

        keyword.trim();

        // Handle wild card
        int foundStar = textLine.indexOf("*");

        // CONDITIONALS

        // Substitute synonyms for IF part
        boolean foundFlag = matchString(keyword, conditional[i]);

        // Replace whole line - foundTwo "::"
        if ((foundFlag) and (foundTwo > -1))
          conditional[i] = synonym;

        // Match found and "*" replace whole line
        else if ((foundFlag) and (foundStar > -1))
          conditional[i] = synonym;

        // Single ":" and no "*" - just replace keyword with synonym  
        else if (foundFlag)
          conditional[i].replace(keyword, synonym);  

        // ASSIGNMENTS

        // Substitute synonyms for THEN part
        foundFlag = matchString(keyword, assignment[i]);

        // Replace whole line - foundTwo "::"
        if ((foundFlag) and (foundTwo > -1))
          assignment[i] = synonym;

        // Match found and "*" replace whole line
        else if ((foundFlag) and (foundStar > -1))
          assignment[i] = synonym;

        // Single ":" and no "*" - just replace keyword with synonym  
        else if (foundFlag)
          assignment[i].replace(keyword, synonym);  

      }  
    }
  }

  Serial.println();
  Serial.println();
  Serial.println(F("Code with Synonyms"));
  Serial.println(F("------------------"));

  for (i=0; i< codeLines; i++) {

    // Look for output pins used - if found, change from -1 to 0  
    for (int j = 0; j < 54; j++) {
      tmpLine = "pin "; 
      tmpLine.concat(j);  
      tmpLine.concat(" ");  
      // pl(tmpLine);   
      if ((assignment[i].indexOf(tmpLine)) > -1) {
        pinMode(i, OUTPUT);
        // pl(tmpLine);
        pins[j] = 0;
      }
    }   

    // Serial print with indents based on chapters
    if ((assignment[i].indexOf("chapter") > -1) and (assignment[i].indexOf("=") == -1)) pp("  ");   
    else pp("    ");
    if (conditional[i].length() > 2) {
      conditional[i].trim();
      Serial.print(conditional[i]);
      Serial.print(" then ");
    }

    Serial.println(assignment[i]);
  }
}



// ---------------- Interpret Code -----------------------

void interpretCode()
// Process and interpret all codelines

{ 
  int i=0;   // Array counter
  int p=-1;  // Used for position
  int p1 = -1;
  int p2;
  int tmpInt=0;
  String tmpLine="";

  Serial.println();
  Serial.println();
  Serial.println(F("Identifying Chapters"));
  Serial.println(F("--------------------"));

  // while ((textLine != "END") && (myFile.available())) {
  for (int i=0; i < codeLines; i++) {

    // Look for chapter statement
    textLine = codeArray[i];
    textLine.toLowerCase();
    p = textLine.indexOf("chapter");
    p1 = textLine.indexOf("=");

    // Found a chapter statement
    if ((p > -1) and (p1 < 0)) {
      pp("  ");
      pl(textLine);
      textLine = textLine.substring(p +7);
      textLine.trim();

      // Get chapter number
      tmpLine = popOne();
      chapter = tmpLine.toInt();
      codeArray[i] = "";
      textLine = "";
    } 

    chapters[i] = chapter;  

    // CONDITIONAL

    // Interpret Conditional Statement - Put into arrays
    if (conditional[i].length() > 2) {
      textLine = conditional[i];
      textLine.replace("(", "  ");
      textLine.replace("  ", " ");
      textLine.replace(")", "  ");
      textLine.replace("pin", "  ");
      if (textLine.indexOf("=") == -1) textLine.replace("timer", "sensor6");
      textLine.replace("sensor", "  ");
      textLine.replace("if ", "  ");
      textLine.trim();

      // Get sensor pin
      tmpLine = popOne();
      sensor[i] = tmpLine.toInt();

      // Get comparison operator
      tmpLine = popOne(); 
      comparison[i] = -1;
      if (tmpLine[0] == '=') comparison[i] = 0;
      else if (tmpLine[0] == '<') comparison[i] = 1;
      else if (tmpLine[0] == '>') comparison[i] = 2;
      else (comparison[i] = -1); 

      // Get threshold
      tmpLine = popOne();
      threshold[i] = tmpLine.toInt();
    }


    // ASSIGNMENTS 

    // Now work on the assignment statement
    textLine = assignment[i];

    // Find the position of the = sign
    p = textLine.indexOf("=");

    // If there is an "="
    if (p > -1)  {

      // FIND VALUE

      // Get the value from after "=" sign
      textLine = textLine.substring(p+1);
      textLine.toLowerCase();
      textLine.trim();

      // Read the value into tmpLine
      tmpLine = popOne();                 

      // Restore the textLine
      textLine = assignment[i];  

      // FIND VARIABLE

      // Find the VARIABLE for the assignment
      if (textLine.indexOf("delay") > -1)  {
        variable[i] = "delay"; 
        value[i] = tmpLine;
      }
      else if (textLine.indexOf("quency = ") > -1)  {
        variable[i] = "logfrequency"; 
        value[i] = tmpLine;
        logFrequency = tmpLine.toInt();
      }
      else if (textLine.indexOf("pin ") > -1)  {
        textLine.replace("pin ", "pin");
        p2 = textLine.indexOf("pin");
        textLine = textLine.substring(p2, p2+5); 
        textLine.trim();      
        variable[i] = textLine; 
        value[i] = tmpLine;
      }
      else if (textLine.indexOf("samplerate") > -1)  {
        variable[i] = "samplerate"; 
        value[i] = tmpLine;
      }
      else if (textLine.indexOf("logthresh") > -1) {
        variable[i] = "logthreshold"; 
        value[i] = tmpLine;
        logThreshold = tmpLine.toInt();
      }
      else if (textLine.indexOf("exit") > -1) {
        variable[i] = "exit"; 
        value[i] = tmpLine;
      }
      else if (textLine.indexOf("kidcode") > -1) ;

      else if (textLine.indexOf("multip") > -1) {
        variable[i] = "multiplier"; 
        value[i] = tmpLine;
        multiplier = tmpLine.toInt();
      }
      else if (textLine.indexOf("timer") > -1) {
        variable[i] = "timer"; 
        value[i] = tmpLine;
      }
      else if (textLine.indexOf("sensor0") > -1) {
        sensor0 = int(tmpLine[1]);
        sensor0 = sensor0 + 6;
      }
      else if (textLine.indexOf("sensor1") > -1) {
        sensor1 = int(tmpLine[1]);
        sensor1 = sensor1 + 6;
      }
      else if (textLine.indexOf("sensor2") > -1) {
        sensor2 = int(tmpLine[1]);
        sensor2 = sensor2 + 6;
      }
      else if (textLine.indexOf("sensor3") > -1) {
        sensor3 = int(tmpLine[1]);
        sensor3 = sensor3 + 6;
      }
      else if (textLine.indexOf("sensor4") > -1) {
        sensor4 = int(tmpLine[1]);
        sensor4 = sensor4 + 6;
      }
      else if (textLine.indexOf("sensor5") > -1) {
        sensor5 = int(tmpLine[1]);
        sensor5 = sensor5 + 6;
      }

      // Sensor 6 is the timer
      else if (textLine.indexOf("sensor6") > -1) ; 

      else if (textLine.indexOf("chapter") > -1) {
        variable[i] = "chapter"; 
        value[i] = tmpLine;
      }
      else { 

        // Else can't find variable 
        Serial.print(F(" - Unknown Assignment -")); 
        Serial.println(assignment[i]);
      }
    }
  }


  // INTERPRETER ARRAY VALUES

  Serial.println();
  Serial.println();
  Serial.println(F("Variable Assignments"));
  Serial.println(F("--------------------"));
  Serial.println();
  Serial.println(F("Chapter\tSensor\tOp\tThresh\tVariable\tValue"));
  Serial.println(F("------------------------------------------------------------------------"));
  for (i = 0; i< codeLines; i++){
    if ((sensor[i] != 0) or (variable[i] != "")) { 
      Serial.print(chapters[i]); 
      Serial.print("\t");
      Serial.print(sensor[i]); 
      Serial.print("\t");
      Serial.print(comparison[i]); 
      Serial.print("\t");
      Serial.print(threshold[i]); 
      Serial.print("\t");
      Serial.print(variable[i]); 
      Serial.print("\t\t");
      Serial.print(value[i]); 
      Serial.println();
    }
  }

  // Close the file:
  myFile.close();
}



// _________ OUTPUT VALUES -----------

void outputValues() {

  int i;

  // Check to see how big the sensor jump is. Log only if over threshold
  if (abs (lastSensorValue - sensorValue) > (logThreshold - 1)) {

    if (firstPass) {
      Serial.println();
      Serial.println(F("Chapter\tTime\tSensor1\tSensor2\tOut1\tOut1\tOut2\tOut2\tOut3\tOut3"));
      Serial.println(F("\tElapsed\tValue\tValue\tPin\tValue\tPin\tValue\tPin\tValue"));
      Serial.println(F("----------------------------------------------------------------------"));
    } 

    // Log the data to serial
    Serial.print(chapter);
    Serial.print("\t");
    Serial.print(millis()/1000);
    Serial.print("\t");

    // Log the first two sensor values to serial
    Serial.print(analogRead(sensor1));  
    Serial.print("\t");
    Serial.print(analogRead(sensor2));  
    Serial.print("\t");

    // Put in labels and values for output pins that are being used
    for (i = 0; i < 54; i++) 
      if (pins[i] > -1) {
        String tmpString;
        tmpString = "";
        tmpString.concat(i);
        tmpString.concat("\t");
        char charVal[10];
        dtostrf(pins[i],2,1, charVal);
        tmpString.concat(charVal);
        tmpString.concat("\t");
        Serial.print(tmpString);
      }  


    // Timer info
    Serial.print(millis());
    Serial.print("\t");
    Serial.print(lastTime);
    Serial.print("\t");
    Serial.print(millis() - lastTime);
    Serial.println();

  }

  // Output the echo value and chapter count

  for (int i = 2; i < 54; i++) {

    String tmpString;

    // If pin has been set, write to it
    if (pins[i] > -1) 
      analogWrite(i, pins[i] * 51);

    // Else use the pin map defaults

    // Pin 2 mirrors sensor 1
    
    else if (i == 2) analogWrite(2, (analogRead(A1)) / 4);

    // Pin 3 mirrors output pin 13  
    else if (i == 3) analogWrite(3, pins[13] * 51);  

    // Pin 5 shows the chapter in voltage
    else if (i == 5) analogWrite(5, chapter * 51);  

    // 44-46 mirror input1 - input 3
    else if (i == 44) analogWrite(44, (analogRead(A1)) / 4);  
    else if (i == 45) analogWrite(45, (analogRead(A2)) / 4);    
    else if (i == 46) analogWrite(46, (analogRead(A3)) / 4);    

    // 47-49 Always on - 5V
    else if (i == 47) analogWrite(47, 5.0 * 51);  
    else if (i == 48) analogWrite(48, 5.0 * 51);  
    else if (i == 49) analogWrite(49, 5.0 * 51);  
  }
}




// -------- ************************** -------
// -------- ********** SETUP ********* -------
// -------- ************************** -------



//----------------------- SETUP-----------------------
// Main Setup Function

void setup(){

  int i;

  Serial.begin(9600);            // Initialize serial port for debugging
  Serial.flush();                // Flush the serial buffer

  // Set up SD Card Reader

  Serial.println();
  Serial.println(F("INITIALIZING SD READER"));
  Serial.println(F("______________________"));
  Serial.println(F("Init SD"));
  Serial.println();

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("No SD Card");
    Serial.println("___________");
    Serial.println("");
    Serial.println("Activate Pin Map");
    Serial.println("----------------");
    Serial.println("Pin - 2 - Mirror Sensor 1");
    Serial.println("Pin - 3 - Mirror Pin 13");
    Serial.println("Pin - 5 - Echo Chapter Number as Volts");
    Serial.println("Pins - 30-37 - Paired blink patterns");
    Serial.println("Pins - 38-41 - Paired night light");
    Serial.println("Pins - 42-43 - Paired reverse night light");
    Serial.println("Pins - 44-46 - Mirror Sensor 1 - Sensor 3");
    Serial.println("Pins - 47-49 - Always on - 5V");


    sdCardFlag = false;
    // don't do anything more:

    // ****** PIN MAP ******

    // initialize the digital pin as an output.
    for (i = 2; i < 50; i++)
      pinMode(i, OUTPUT);     

    registerPulse(30, 500, 500);
    registerPulse(31, 500, 500);
    registerPulse(32, 1000, 1000);
    registerPulse(33, 1000, 1000);
    registerPulse(34, 2000, 1000);
    registerPulse(35, 2000, 1000);
    registerPulse(36, 200, 200);
    registerPulse(37, 200, 200);
    
    
    registerPulse(3, 500, 500);
    registerPulse(2, 3000, 1000);
    registerPulse(13, 500, 500);
    registerPulse(12, 1000, 1000);
    registerPulse(11, 3000, 1000);
    registerPulse(10, 200, 200);
   

    return;
  }

  else {

    // Set input and output pins  
    // initialize the digital pin as an output.
    for (i = 44; i < 50; i++)
      pinMode(i, OUTPUT);     


    pinMode(inputEcho, OUTPUT);
    pinMode(9, OUTPUT);

    // Set 40-49 default HIGH
    for (i = 47; i < 50; i++) {
      pinMode(i, OUTPUT);
      digitalWrite(i, HIGH); 
    }

    // Initialize pins array to -1 
    for (i = 0; i < 54; i++) 
      pins[i] = -1;

    // Load code.txt into the code array
    loadCodeArray();

    // Copy kidcode.txt to end of code array
    if (kidCodeFlag) loadKidCode();

    Serial.flush();
    Serial.println();
    Serial.println(F("Printing Code Array with Kidcode"));
    Serial.println(F("--------------------------------"));

    for (int i=0; i < codeLines; i++) {
      if ((codeArray[i].indexOf("hapter") > -1) and (codeArray[i].indexOf("=") == -1)) pp("  ");   
      else pp("    ");
      Serial.println(codeArray[i]);
    }
    Serial.println();


    // Load synonyms into synonyms arrary
    loadSynonyms();

    // Apply synonyms to the code arrary
    processSynonyms();

    // Load data from the code array into the Interpreter array
    interpretCode();

    // Remove the old log.txt file
    SD.remove("LOG.TXT");

    // Write information to the log.txt file
    writeDebugFile();

    // Set to first chapter
    chapter = 1;

  }


}

// -------- ************************* -------
// -------- ********** LOOP ********* -------
// -------- ************************* -------


//---------------- LOOP ---------------------------
// Main Loop Function

void loop() 

{
  String tempString;
  int tempInt;

  if (sdCardFlag) {

    // Do the conditions match?
    boolean thresholdMet = false; 

    // Get the sensor value
    sensorValue = analogRead(sensor1);

    if (firstPass) {
      // Print the title lines
      Serial.println(F("Sending Output Data"));
      Serial.println(F("-------------------"));
    }  

    /* 
     // Sensor information 
     pp("Sensors: ");
     Serial.print(sensor0 - 54); Serial.print("="); Serial.print(analogRead(sensor0)); pp("  ");
     Serial.print(sensor1 - 54); Serial.print("="); Serial.print(analogRead(sensor1)); pp("  ");
     Serial.print(sensor2 - 54); Serial.print("="); Serial.print(analogRead(sensor2)); pp("  ");
     Serial.print(sensor3 - 54); Serial.print("="); Serial.print(analogRead(sensor3)); pp("  ");
     Serial.print(sensor4 - 54); Serial.print("="); Serial.print(analogRead(sensor4)); pp("  ");
     Serial.print(sensor5 - 54); Serial.print("="); Serial.print(analogRead(sensor5)); pp("  ");
     Serial.println();
     */


    // CONDITIONAL

    // For each codeline check to see whether its condition is met
    for ( int i = 0; i < codeLines; i++ )  { 

      // Read the appropriate Sensor depending on what is in the if statement
      if (sensor[i] == 0) sensorValue =  analogRead(sensor0);
      else if (sensor[i] == 1) sensorValue =  analogRead(sensor1);
      else if (sensor[i] == 2) sensorValue =  analogRead(sensor2);
      else if (sensor[i] == 3) sensorValue =  analogRead(sensor3);
      else if (sensor[i] == 4) sensorValue =  analogRead(sensor4);
      else if (sensor[i] == 5) sensorValue =  analogRead(sensor5);  
      else if (sensor[i] == 6) sensorValue =  millis() - lastTime;  



      // Does comparison for this if statement meet the threshold "<"
      if ((comparison[i] == 2) && (sensorValue > threshold[i])) thresholdMet = true;

      // Does comparison for this if statement meet the threshold ">"
      if ((comparison[i] == 1) && (sensorValue < threshold[i]))  thresholdMet = true;

      // Does comparison for this if statement meet the threshold "="
      if (comparison[i] == 0) thresholdMet = true;

      if (chapters[i] != chapter) thresholdMet = false;

      Serial.flush();

      if (firstPass) {
        outputValues();
        firstPass = false;      
      }


      // ASSIGNMENT

      // If threshold is met - figure out the assignment
      if (thresholdMet == true) {

        // Check to see which variable we are assigning tmpInt to.
        if (variable[i].indexOf("delay") > -1) { 
          outputValues(); 
          delay(value[i].toInt()); 
          // Get the sensor value
          sensorValue = analogRead(sensor1);
        }

        // Exit if exit
        else if (variable[i].indexOf("exit") > -1) { 
          outputValues(); 
          Serial.println(F("----------------------------------------------------------------------"));
          Serial.println("Exit Program"); 
          while(1) {
          }   
        }

        // Else search for matching variable
        else if (variable[i].indexOf("quency") > -1) logFrequency = value[i].toInt();
        else if (variable[i].indexOf("logthresh") > -1) logThreshold = value[i].toInt();
        else if (variable[i].indexOf("samplerate") > -1) sampleRate = value[i].toInt();
        else if (variable[i].indexOf("kidcode") > -1) ;
        else if (variable[i].indexOf("chapt") > -1) chapter = value[i].toInt();
        else if (variable[i].indexOf("timer") > -1) lastTime = millis() + value[i].toInt();
        else if (variable[i].indexOf("multip") > -1) multiplier = value[i].toInt();
        else if (variable[i].indexOf("action") > -1) ;
        else if (variable[i].indexOf("sensor") > -1) ;

        // Handle pins
        else if (variable[i].indexOf("pin") > -1) {
          tempString = variable[i];
          tempString.replace("pin", "");
          float tmpFloat;
          tmpFloat = getReal(tempString);
          tempInt = int(tmpFloat);
          pins[tempInt] = getReal(value[i]);
          // pp(tempString); pp("+++++++++++"); pl(value[i]);
        }

        else if (variable[i].length() < 1) ;

        else { 
          Serial.print(" - Unknown Assignment -"); 
          Serial.println(variable[i]);
        }
      }
      // reset variable
      thresholdMet = false;                                                                                                                   
    }

    // Send values to outputs
    outputValues();

    // Turn off first pass
    firstPass = false;

    // Set lastSensor so we can do jump logging
    lastSensorValue = sensorValue;

    // Reset the text line
    textLine = "";

    // Delay until next read
    // delay(2000);
    delay(sampleRate);

  }

  else {

    // ***********************************
    // **** No Card - Turn on Pin Map ****
    // ***********************************

    float volts; 

    // Blink Lights
    // 2      3/1 seconds
    // 3      .5 seconds
    // 10     .2 seconds
    // 11     3 seconds
    // 12     1 second
    // 13, 30-31  .5 seconds
    // 32-33  1 second
    // 34-35  2/1 seconds
    // 36-37  .2 seconds
    pulse();

    // Get volts for night lights
    volts = sensorValue / 205.0;

    // Night Light - Pins 6 - 7
    if (volts < 3) digitalWrite(6, HIGH);
    else digitalWrite(6, LOW);

    if (volts < 1) digitalWrite(7, HIGH);
    else digitalWrite(7, LOW);

    // Night Light - Pins 38 - 40
    if (volts < 1) digitalWrite(38, HIGH);
    else digitalWrite(38, LOW);

    if (volts < 2) digitalWrite(39, HIGH);
    else digitalWrite(39, LOW);

    if (volts < 3) digitalWrite(40, HIGH);
    else digitalWrite(40, LOW);


    // Reverse Night Light - Pins 4
    if (volts > 2) digitalWrite(5, HIGH);
    else digitalWrite(5, LOW);

    // Reverse Night Light - Pins 41 - 43
    if (volts > 1) digitalWrite(41, HIGH);
    else digitalWrite(41, LOW);

    if (volts > 2) digitalWrite(42, HIGH);
    else digitalWrite(42, LOW);

    if (volts > 3) digitalWrite(43, HIGH);
    else digitalWrite(43, LOW);

    // Mirror Sensor 1 - Sensor 3
    // Mirror A1 to 44
    sensorValue = analogRead(A1);
    analogWrite(44, sensorValue / 4);
    analogWrite(8, sensorValue / 4);
    analogWrite(9, sensorValue / 4);

    // Mirror A2 to 45
    sensorValue = analogRead(A2);
    analogWrite(45, sensorValue / 4);

    // Mirror A3 to 46
    sensorValue = analogRead(A3);
    analogWrite(46, sensorValue / 4);


    // Set 47 -50 Hight
    for (int i = 47; i < 50; i++)
      digitalWrite(i, HIGH);
    
    digitalWrite(4, HIGH);

    delay(10);

  }


}




