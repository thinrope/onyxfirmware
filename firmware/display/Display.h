#include "oled.h"

extern uint8_t _binary_image_data_start;
extern uint8_t _binary_image_data_size;

class Display {

public:

  Display() {
  }

  void initialise() {
    oled_init();
  }

  void test() {

    dump_image();
 /*
    uint8 data[100];
    for(int n=0;n<100;n++) data[n]=n;

    oled_draw_rect(10,10,20,20,data);
*/
  }

  void dump_image() {
    oled_draw_rect(0,0,128,128,(uint8_t *) &_binary_image_data_start);
  }

};