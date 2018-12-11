/*
 * menuHandler.c
 *
 *  Created on: 10 Dec 2018
 *      Author: joseph
 */

#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "lcdHandler.h"

void menuHandler(void) {
  setText (0, 0, "MENU",1,1);
  char *menuList[] = {"Option1", "Option2", "Option3", "Option4"};
  int position = 30;
  int selectedOption = 1;
  unsigned int i;
 //    int optionCount = (sizeof(menuList)/sizeof(menuList[0]));

  /*If option is selected reverse colours, also aligned to middle*/

  for (i = 0; i< 4; i++) {
    if ((selectedOption - 1) == i){
      setText (0, position, menuList[i], 0, 1);
    } else {
      setText (0, position, menuList[i], 1, 1);
    }
    position += 10;
  }
}


