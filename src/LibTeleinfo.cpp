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

#include "LibTeleinfo.h" 

/* ======================================================================
Class   : TInfo
Purpose : Constructor
Input   : -
Output  : -
Comments: -
====================================================================== */
TInfo::TInfo()
{
  // Init of our linked list
  _valueslist.name = NULL;
  _valueslist.value = NULL;

  _index_buff = 0;
  memset(&_teleinfo, 0, sizeof(_teleinfo));

}


/* ======================================================================
Function: valueAdd
Purpose : Add element to the Linked List of values
Input   : Pointer to the label name
          pointer to the value
          checksum value
          flag state of the label (modified by function)
Output  : pointer to the new node (or founded one)
Comments: - state of the label changed by the function
====================================================================== */
ValueList * TInfo::valueAdd(char * name, char * value)
{
  // Get our linked list 
  ValueList * me = &_valueslist;

  uint8_t lgname = strlen(name);
  uint8_t lgvalue = strlen(value);
  
    // Got one and all seems good ?
    if (me && lgname && lgvalue) {
      // Create pointer on the new node
      ValueList *newNode = NULL;
      ValueList *parNode = NULL ;

      // Loop thru the node
      while (me->next) {
        // save parent node
        parNode = me ;

        // go to next node
        me = me->next;

        // Check if we already have this LABEL (same name AND same size)
        if (lgname==strlen(me->name) && strcmp(me->name, name )==0) {

          // Already got also this value  return US
          if (lgvalue==strlen(me->value) && strcmp(me->value, value) == 0) {
            return ( me );
          } else {
            // Do we have enought space to hold new value ?
            if (strlen(me->value) >= lgvalue ) {
              // Copy it
              strlcpy(me->value, value , lgvalue + 1);
              // That's all
              return (me);
            } else {
              // indicate our parent node that the next node
              // is not us anymore but the next we have
              parNode->next = me->next;

              // free up this node
              free (me);

              // Return to parent (that will now point on next node and not us)
              // and continue loop just in case we have sevral with same name
              me = parNode;
            }
          }
        }
      }

      // Our linked list structure sizeof(ValueList)
      // + Name  + '\0'
      // + Value + '\0'
      size_t size ;
      #if defined (ESP8266) || defined (ESP32)
        lgname = ESP_allocAlign(lgname+1);   // Align name buffer
        lgvalue = ESP_allocAlign(lgvalue+1); // Align value buffer
        // Align the whole structure
        size = ESP_allocAlign( sizeof(ValueList) + lgname + lgvalue  )     ; 
      #else
        size = sizeof(ValueList) + lgname + 1 + lgvalue + 1  ;
      #endif

      // Create new node with size to store strings
      if ((newNode = (ValueList  *) malloc(size) ) == NULL) 
        return ( (ValueList *) NULL );

      // get our buffer Safe
      memset(newNode, 0, size);
      
      // Put the new node on the list
      me->next = newNode;

      // First String located after last struct element
      // Second String located after the First + \0
      newNode->name = (char *)  newNode + sizeof(ValueList);
      newNode->value = (char *) newNode->name + lgname + 1;

      // Copy the string data
      memcpy(newNode->name , name  , lgname );
      memcpy(newNode->value, value , lgvalue );

      TI_Debug(F("Added '"));
      TI_Debug(name);
      TI_Debug('=');
      TI_Debug(value);
      TI_Debugln(F("'"));

      // return pointer on the new node
      return (newNode);
    }


  // Error or Already Exists
  return ( (ValueList *) NULL);
}



/* ======================================================================
Function: valueGet
Purpose : get value of one element
Input   : Pointer to the label name
          pointer to the value where we fill data 
Output  : pointer to the value where we filled data NULL is not found
====================================================================== */
char * TInfo::valueGet(char * name, char * value)
{
  // Get our linked list 
  ValueList * me = &_valueslist;
  uint8_t lgname = strlen(name);

  // Got one and all seems good ?
  if (me && lgname) {

    // Loop thru the node
    while (me->next) {

      // go to next node
      me = me->next;

      // Check if we match this LABEL
      if (lgname==strlen(me->name) && strcmp(me->name, name)==0) {
        // this one has a value ?
        if (me->value) {
          // copy to dest buffer
          uint8_t lgvalue = strlen(me->value);
          strlcpy(value, me->value , lgvalue + 1 );
          return ( value );
        }
      }
    }
  }
  // not found
  return ( NULL);
}

/* ======================================================================
Function: getTopList
Purpose : return a pointer on the top of the linked list
Input   : -
Output  : Pointer 
====================================================================== */
ValueList * TInfo::getList(void)
{
  // Get our linked list 
  return &_valueslist;
}

/* ======================================================================
Function: valuesDump
Purpose : dump linked list content
Input   : -
Output  : total number of values
====================================================================== */
uint8_t TInfo::valuesDump(void)
{
  // Get our linked list 
  ValueList * me = &_valueslist;
  uint8_t index = 0;

  // Got one ?
  if (me) {
    // Loop thru the node
    while (me->next) {
      // go to next node
      me = me->next;

      index++;
      TI_Debug(index) ;
      TI_Debug(F(") ")) ;

      if (me->name) {
        TI_Debug(me->name) ;
      } else {
        TI_Debug(F("NULL")) ;
      }

      TI_Debug(F("=")) ;

      if (me->value) {
        TI_Debug(me->value) ;
      } else {
        TI_Debug(F("NULL")) ;
      }
      TI_Debug(F("' ")); 

      TI_Debugln() ;
    }
  }

  return index;
}

/* ======================================================================
Function: process
Purpose : teleinfo serial char received processing, should be called
          my main loop, this will take care of managing all the other
Input   : pointer to the serial used 
Output  : teleinfo global state
====================================================================== */
void TInfo::process(char in)
{
    in = (char)in & 127;  // seulement sur 7 bits

    switch (in) {
      case DEB_TRAME:
        _index_buff=0;
        break;

      case FIN_TRAME:
        _buffin[_index_buff]=0;
        
        // Test validité longueur trame
        if (_index_buff >= LG_TRAME_MIN-1 && _index_buff <= LG_TRAME_MAX-1) {
          //Serial.println(_buffin);
          //for (int i=0; i<_index_buff; i++) {
          //  Serial.print(".");
          //  Serial.print(_buffin[i], HEX);
          //}
          //Serial.println("");
          //Serial.print("CS=");
          //Serial.println(_buffin[_index_buff-1], HEX);
          if (ckecksum(_buffin, _index_buff-1) == _buffin[_index_buff-1]) { // Test du checksum
            traitement_trame(_buffin);
          } 
          _index_buff=0;
        }
        break;

      default:
        // stock buffer ------------------------
        if (_index_buff <= LG_TRAME_MAX) {
          _buffin[_index_buff] = in;
          _index_buff++;
        }
        else _index_buff=0;
        break;
    }     
}

char TInfo::ckecksum(char *buff, int len)
{
int i;
char sum = 0;

    for (i=0; i<len; i++) sum = sum + buff[i];
    sum = (sum & 0x3F) + 0x20;

    //Serial.print("CalCS=");
    //Serial.println(sum, HEX);
    return(sum);
}

char* TInfo::replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos){
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

// Traitement trame teleinfo ------------------------------------------
void TInfo::traitement_trame(char *buff)
{

    //Serial.println(buff);
    
    if (strncmp("ADSC", &buff[0] , 4)==0) {
      strncpy(_teleinfo.ADSC, &buff[5], 12);
      _teleinfo.ADSC[12] = '\0';
      replace_char(_teleinfo.ADSC, (char)0x09, (char)0x20);
      valueAdd("ADSC", _teleinfo.ADSC);
      return;
    }
    if (strncmp("VTIC", &buff[0] , 4)==0) {
      strncpy(_teleinfo.VTIC, &buff[5], 2);
      _teleinfo.VTIC[2] = '\0';
      replace_char(_teleinfo.VTIC, (char)0x09, (char)0x20);
      valueAdd("VTIC", _teleinfo.VTIC);
      return;
    }
    if (strncmp("NGTF", &buff[0] , 4)==0) {
      strncpy(_teleinfo.NGTF, &buff[5], 16);
      _teleinfo.NGTF[16] = '\0';
      replace_char(_teleinfo.NGTF, (char)0x09, (char)0x20);
      valueAdd("NGTF", _teleinfo.NGTF);
      return;
    }
    if (strncmp("LTARF", &buff[0] , 4)==0) {
      strncpy(_teleinfo.LTARF, &buff[5], 16);
      _teleinfo.LTARF[16] = '\0';
      replace_char(_teleinfo.LTARF, (char)0x09, (char)0x20);
      valueAdd("LTARF", _teleinfo.LTARF);
      return;
    }
    if (strncmp("EAST", &buff[0] , 4)==0) {
      _teleinfo.EAST = atol(&buff[5]);
      valueAdd("EAST", (char *)(String(_teleinfo.EAST).c_str()));
      return;
    }
    if (strncmp("EAIT", &buff[0] , 4)==0) {
      _teleinfo.EAIT = atol(&buff[5]);
      valueAdd("EAIT", (char *)(String(_teleinfo.EAIT).c_str()));
      return;
    }
    if (strncmp("IRMS1", &buff[0] , 5)==0) {
      _teleinfo.IRMS1 = atoi(&buff[6]);
      valueAdd("IRMS1", (char *)(String(_teleinfo.IRMS1).c_str()));
      return;
    }
    if (strncmp("IRMS2", &buff[0] , 5)==0) {
      _teleinfo.IRMS2 = atoi(&buff[6]);
      valueAdd("IRMS2", (char *)(String(_teleinfo.IRMS2).c_str()));
      return;
    }
    if (strncmp("IRMS3", &buff[0] , 5)==0) {
      _teleinfo.IRMS3 = atoi(&buff[6]);
      valueAdd("IRMS3", (char *)(String(_teleinfo.IRMS3).c_str()));
      return;
    }
    if (strncmp("URMS1", &buff[0] , 5)==0) {
      _teleinfo.URMS1 = atoi(&buff[6]);
      valueAdd("URMS1", (char *)(String(_teleinfo.URMS1).c_str()));
      return;
    }
    if (strncmp("URMS2", &buff[0] , 5)==0) {
      _teleinfo.URMS2 = atoi(&buff[6]);
      valueAdd("URMS2", (char *)(String(_teleinfo.URMS2).c_str()));
      return;
    }
    if (strncmp("URMS3", &buff[0] , 5)==0) {
      _teleinfo.URMS3 = atoi(&buff[6]);
      valueAdd("URMS3", (char *)(String(_teleinfo.URMS3).c_str()));
      return;
    }
    if (strncmp("PREF", &buff[0] , 4)==0) {
      _teleinfo.PREF = atoi(&buff[5]);
      valueAdd("PREF", (char *)(String(_teleinfo.PREF).c_str()));
      return;
    }
    if (strncmp("PCOUP", &buff[0] , 5)==0) {
      _teleinfo.PCOUP = atoi(&buff[6]);
      valueAdd("PCOUP", (char *)(String(_teleinfo.PCOUP).c_str()));
      return;
    }
    if (strncmp("SINSTS", &buff[0] , 6)==0) {
      _teleinfo.SINSTS = atoi(&buff[7]);
      valueAdd("SINSTS", (char *)(String(_teleinfo.SINSTS).c_str()));
      return;
    }
    if (strncmp("SINSTS1", &buff[0] , 7)==0) {
      _teleinfo.SINSTS1 = atoi(&buff[8]);
      valueAdd("SINSTS1", (char *)(String(_teleinfo.SINSTS1).c_str()));
      return;
    }
    if (strncmp("SINSTS2", &buff[0] , 7)==0) {
      _teleinfo.SINSTS2 = atoi(&buff[8]);
      valueAdd("SINSTS2", (char *)(String(_teleinfo.SINSTS2).c_str()));
      return;
    }
    if (strncmp("SINSTS3", &buff[0] , 7)==0) {
      _teleinfo.SINSTS3 = atoi(&buff[8]);
      valueAdd("SINSTS3", (char *)(String(_teleinfo.SINSTS3).c_str()));
      return;
    }
    if (strncmp("SINSTI", &buff[0] , 6)==0) {
      _teleinfo.SINSTI = atoi(&buff[7]);
      valueAdd("SINSTI", (char *)(String(_teleinfo.SINSTI).c_str()));
      return;
    }
    if (strncmp("STGE", &buff[0] , 4)==0) {
      strncpy(_teleinfo.STGE, &buff[5], 8);
      _teleinfo.STGE[8] = '\0';
      replace_char(_teleinfo.STGE, (char)0x09, (char)0x20);
      valueAdd("STGE", _teleinfo.STGE);
      return;
    }
    if (strncmp("MSG1", &buff[0] , 4)==0) {
      strncpy(_teleinfo.MSG1, &buff[5], 32);
      _teleinfo.MSG1[32] = '\0';
      replace_char(_teleinfo.MSG1, (char)0x09, (char)0x20);
      valueAdd("MSG1", _teleinfo.MSG1);
      return;
    }
    if (strncmp("NTAF", &buff[0] , 4)==0) {
      strncpy(_teleinfo.NTAF, &buff[5], 2);
      _teleinfo.NTAF[2] = '\0';
      replace_char(_teleinfo.NTAF, (char)0x09, (char)0x20);
      valueAdd("NTAF", _teleinfo.NTAF);
      return;
    }
    if (strncmp("NJOURF", &buff[0] , 6)==0) {
      strncpy(_teleinfo.NJOURF, &buff[7], 2);
      _teleinfo.NJOURF[2] = '\0';
      replace_char(_teleinfo.NJOURF, (char)0x09, (char)0x20);
      valueAdd("NJOURF", _teleinfo.NJOURF);
      return;
    }
    if (strncmp("NJOURF1", &buff[0] , 7)==0) {
      strncpy(_teleinfo.NJOURF1, &buff[8], 2);
      _teleinfo.NJOURF1[2] = '\0';
      replace_char(_teleinfo.NJOURF1, (char)0x09, (char)0x20);
      valueAdd("NJOURF1", _teleinfo.NJOURF1);
      return;
    }
}
