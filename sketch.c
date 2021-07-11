// Basic program skeleton for a Sketch File (.sk) Viewer
#include "displayfull.h"
#include "sketch.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


// Allocate memory for a drawing state and initialise it
state *newState() {
  //state *s = (state*)calloc(1, sizeof(state));
  state *s = (state*)malloc(sizeof(state));
  s->x = s->y = s->tx = s->ty = s->end = s->data = s->start = 0;
  s->tool = LINE;
  return s;
}

// Release all memory associated with the drawing state
void freeState(state *s) {
  free(s);
}

// Extract 2 bit opcode.
int getOpcode(byte b ){
  return b>>6;
}


// Extract an operand (-32..31) from the rightmost 6 bits of a byte.
int getOperand(byte b) {
  if ((b&63) > 31)
    return (b&63) - 64;
  return b&63;
}

//Helper funtions.
int min(int x,int y)
{
  if(x<y) return x;
  return y;
}


// Execute the next byte of the command sequence.
void obey(display *d, state *s, byte op) {
  //RULE: Keep state updated
  //step1: recognize opcode (tool, dx, dy)
  //step2: procees operand (integer value. can be tool number, delta of position, etc.)
  //step3: if opcode *was* dy, start the graphic processing step (Draw the stuff)

  switch (getOpcode(op))
  {
    case TOOL:
    
    switch(getOperand(op))
    {
      case NONE:
      case LINE:
      case BLOCK:
      s->tool = getOperand(op);
      break;

      case COLOUR:
      colour(d, s->data);
      break;

      case TARGETX:
      s->tx = s->data;
      break;

      case TARGETY:
      s->ty = s->data;
      break;

      case SHOW:
      show(d);
      break;

      case PAUSE:
      pause(d, s->data);
      break;

      case NEXTFRAME:
      s->end = 1;
      break;

      default:
      break;
    }
    s->data = 0;
    break;

    case DX:
    s->tx += getOperand(op);
    break;

    case DATA:
    s->data = s->data<<6;

    if (getOperand(op) < 0)
      s->data += (getOperand(op) + 64);
    else
      s->data += getOperand(op);

    break;

    case DY:
    s->ty += getOperand(op);
    switch(s->tool)
    {
      case NONE:
      break;

      case LINE:
      line(d,s->x,s->y,s->tx,s->ty);
      break;

      case BLOCK:
      block(d,min(s->x,s->tx),min(s->y,s->ty),abs(s->tx - s->x),abs(s->ty - s->y));
      break;

      default:
      break;
    }
    s->x = s->tx;
    s->y = s->ty;

    default:
    break;
  }
}

// Draw a frame of the sketch file. For basic and intermediate sketch files
// this means drawing the full sketch whenever this function is called.
// For advanced sketch files this means drawing the current frame whenever
// this function is called.

bool processSketch(display *d, void *data, const char pressedKey) {

    //TO DO: OPEN, PROCESS/DRAW A SKETCH FILE BYTE BY BYTE, THEN CLOSE IT
    //NOTE: CHECK DATA HAS BEEN INITIALISED... if (data == NULL) return (pressedKey == 27);
    //NOTE: TO GET ACCESS TO THE DRAWING STATE USE... state *s = (state*) data;
    //NOTE: TO GET THE FILENAME... char *filename = getName(d);
    //NOTE: DO NOT FORGET TO CALL show(d); AND TO RESET THE DRAWING STATE APART FROM
    //THE 'START' FIELD AFTER CLOSING THE FILE
    state *s = (state*)data;

    state original_state = *s;

    if (data == NULL)
      return (pressedKey == 27);

    FILE* file = fopen(getName(d), "rb");
    

    if (file == NULL)
      return (pressedKey == 27);

    byte op;
    
    int current_line = 0;


    if (s->start != 0)
    {
      for(int i = 0; i<=(s->start);i++)
      {
        current_line++;
        fgetc(file);
      }
    }  
    while(true)
    {
      if (s->end == 1)
        break;
      if (feof(file))
      {
        s->start = 0;
        break;
      }
      op = fgetc(file);
      obey(d,s,op);
      current_line++;
    }
    
    if (s->end == 1)
    {
      s->start = current_line - 1;
    }
    
    show(d);

    //show must not have been called in DY
    //show must not be called multiple times

    fclose(file);

    int current_start = s->start;   //take a copy of current start value
    *s = original_state;    //everything 0, even start
    s->start = current_start;   //return start value

  return (pressedKey==27);
}
//*/
// View a sketch file in a 200x200 pixel window given the filename
void view(char *filename) {
  display *d = newDisplay(filename, 200, 200);
  state *s = newState();
  run(d, s, processSketch);
  freeState(s);
  freeDisplay(d);
}

// Include a main function only if we are not testing (make sketch),
// otherwise use the main function of the test.c file (make test).
#ifndef TESTING
int main(int n, char *args[n]) {
  if (n != 2) { // return usage hint if not exactly one argument
    printf("Use ./sketch file\n");
    exit(1);
  } else view(args[1]); // otherwise view sketch file in argument
  return 0;
}
#endif


