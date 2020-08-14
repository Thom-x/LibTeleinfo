// **********************************************************************************
// Driver definition for French Teleinfo
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// http://creativecommons.org/licenses/by-sa/4.0/
//
// For any explanation about teleinfo ou use , see my blog
// http://hallard.me/category/tinfo
//
// Code based on following datasheet
// http://www.erdf.fr/sites/default/files/ERDF-NOI-CPT_02E.pdf
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History : V1.00 2015-06-14 - First release
//
// All text above must be included in any redistribution.
//
// Edit : Tab size set to 2 but I converted tab to sapces
//
// **********************************************************************************

#ifndef LibTeleinfo_h
#define LibTeleinfo_h

#ifdef __arm__
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define boolean bool 
#endif

#ifdef ARDUINO
#include <Arduino.h>
#endif

// Using ESP8266 ?
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif


// Define this if you want library to be verbose
//#define TI_DEBUG

// I prefix debug macro to be sure to use specific for THIS library
// debugging, this should not interfere with main sketch or other 
// libraries
#ifdef TI_DEBUG
  #ifdef ESP8266
    #define TI_Debug(x)    Serial1.print(x)
    #define TI_Debugln(x)  Serial1.println(x)
    #define TI_Debugf(...) Serial1.printf(__VA_ARGS__)
    #define TI_Debugflush  Serial1.flush
  #else
    #define TI_Debug(x)    Serial.print(x)
    #define TI_Debugln(x)  Serial.println(x)
    #define TI_Debugf(...) Serial.printf(__VA_ARGS__)
    #define TI_Debugflush  Serial.flush
  #endif
#else
  #define TI_Debug(x)    
  #define TI_Debugln(x)  
  #define TI_Debugf(...) 
  #define TI_Debugflush  
#endif

#if defined (ESP8266) || defined (ESP32)
  // For 4 bytes Aligment boundaries
  #define ESP_allocAlign(size)  ((size + 3) & ~((size_t) 3))
#endif

#pragma pack(push)  // push current alignment to stack
#pragma pack(1)     // set alignment to 1 byte boundary

// Linked list structure containing all values received
typedef struct _ValueList ValueList;
struct _ValueList 
{
  ValueList *next; // next element
  char  * name;    // LABEL of value name
  char  * value;   // value 
};

struct teleinfo_s {
  char ADSC[13]="";
  char VTIC[3]="";
  char NGTF[17]="";
  char LTARF[16]="";
  unsigned long EAST=0;
  unsigned long EAIT=0;
  unsigned int IRMS1=0;
  unsigned int IRMS2=0;
  unsigned int IRMS3=0;
  unsigned int URMS1=0;
  unsigned int URMS2=0;
  unsigned int URMS3=0;
  unsigned int PREF=0;
  unsigned int PCOUP=0;
  unsigned int SINSTS=0;
  unsigned int SINSTS1=0;
  unsigned int SINSTS2=0;
  unsigned int SINSTS3=0;
  unsigned int SINSTI=0;
  char STGE[9]="";
  char MSG1[32]="";
  char NTAF[2]="";
  char NJOURF[2]="";
  char NJOURF1[2]="";
};

#define LG_TRAME_MAX  125
#define LG_TRAME_MIN  5
#define DEB_TRAME 0x0A
#define FIN_TRAME 0x0D



#pragma pack(pop)

class TInfo
{
  public:
    TInfo();
    void          process (char c);
    ValueList *   getList(void);
    char *        valueGet(char * name, char * value);
    uint8_t       valuesDump(void);

  private:
    ValueList *   valueAdd (char * name, char * value);
    ValueList     _valueslist;   // Linked list of teleinfo values
    char          _buffin[LG_TRAME_MAX];
    int           _index_buff = 0;
    void          traitement_trame(char *buff);
    char          ckecksum(char *buff, int len);
    teleinfo_s    _teleinfo;
    char*         replace_char(char* str, char find, char replace);

};

#endif
