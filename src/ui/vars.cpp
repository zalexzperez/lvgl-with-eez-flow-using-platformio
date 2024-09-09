/* File created by user */

#include <stdio.h>
#include <lvgl.h>
#include "vars.h"
#include "screens.h"
#include "structs.h"
#include "ui.h"

/* variable declarations*/

bool performAnimation;

/* Get and set functions */ 

bool get_var_perform_animation(){
    return performAnimation;
}
void set_var_perform_animation(bool value){
    performAnimation = value;
}