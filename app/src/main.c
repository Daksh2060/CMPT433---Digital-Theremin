
#include <stdio.h>
#include "knob_controls.h"
#include "utils.h"
#include "rotary_encoder.h"
#include "lcd.h"

int main()
{   
    knob_controls_init();

    while (1)
    {
       sleep_for_ms(100);
    }
    knob_controls_cleanup();
    
    return 0;
}