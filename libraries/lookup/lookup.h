/* 
 * File:   lookup.h
 * Author: Gustav Öst
 *
 * Created on October 30, 2014
 */

#ifndef LOOKUP_H
#define	LOOKUP_H

#include <avr/pgmspace.h>
class Lookup {
  
  static const unsigned int etable[] PROGMEM;
  
  public:
    unsigned int get(uint8_t i);
};

#endif
