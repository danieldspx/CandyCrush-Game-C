/*********************************************************************
// Canvas para desenho - Versao C.
//  Autor: Cesar Tadeu Pozzer
//         10/2007
//
//  Pode ser utilizada para fazer desenhos ou animacoes, como jogos simples.
//  Tem tratamento de mosue
//  Estude o OpenGL antes de tentar compreender o arquivo gl_canvas.cpp
//
//  Instru��es:
//	  Para alterar a animacao, digite numeros entre 1 e 5
// *********************************************************************/

#include "GL/glut.h"
#include "GL/freeglut_ext.h" //callback da wheel do mouse.


#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "gl_canvas2d.h"

#define RGB(x) (float)x/255
#define PI 3.14159265
#define GRID_SIZE 9
#define CANDY_SIZE 30


int DIM_TELA_X = 600;
int DIM_TELA_Y = 600;
int GRID_X = (int)ceil(DIM_TELA_X/GRID_SIZE);
int GRID_Y = (int)ceil(DIM_TELA_Y/GRID_SIZE);
const int WIDTH_POINTS = 200;

typedef struct {
  int x;
  int y;
} Position;

typedef struct {
  int type;
  Position position;
  bool willExplode;
  float red, green, blue;//Color
  int shapeSides;
} Candy;

Candy **matrixCandy = (Candy **)malloc(CANDY_SIZE*CANDY_SIZE*sizeof(Candy *));

int getRandomType(){
  return ((rand() % 4)+1);
}

void updateWindowProps(){
  getWindowSize(&DIM_TELA_X, &DIM_TELA_Y);
  GRID_X = (int)ceil(DIM_TELA_X/GRID_SIZE) - ((int)WIDTH_POINTS/GRID_SIZE);
  GRID_Y = (int)ceil(DIM_TELA_Y/GRID_SIZE);
}

void setCandyProperties(Candy *candy){
  int sides, red, green, blue;
  switch (candy->type) {
    case 1://Bola Azul
      sides = 30;
      red = 66;
      green = 134;
      blue = 244;
      break;
    case 2://Bola Laranja
      sides = 30;
      red = 255;
      green = 108;
      blue = 40;
      break;
    case 3://Quadrado Verde
      sides = 4;
      red = 64;
      green = 249;
      blue = 114;
      break;
    case 4://Heptagono Roxo
      sides = 7;
      red = 179;
      green = 64;
      blue = 249;
      break;
  }
  candy->shapeSides = sides;
  candy->red = RGB(red);
  candy->green = RGB(green);
  candy->blue = RGB(blue);
  candy->willExplode = false;
}

Candy* getRandomCandy(){
  Candy *candy = (Candy *) malloc(sizeof(Candy));
  candy->type = getRandomType();
  setCandyProperties(candy);
  return candy;
}

int getCenterGridX(int x){
  return GRID_X*x + (int)GRID_X/2;
}

int getCenterGridY(int y){
  return GRID_Y*y + (int)GRID_Y/2;
}

void drawBackGrid(int x, int y){
  int x1 = GRID_X * x;
  int y1 = GRID_Y * y;

  int x2 =  x1 + GRID_X;
  int y2 =  y1 + GRID_Y;

  color(RGB(112), RGB(122), RGB(122));
  rect(x1, y1, x2, y2);
}

void markBackGridAsRed(Position pos){
  int x1 = GRID_X * pos.x;
  int y1 = GRID_Y * pos.y;

  int x2 =  x1 + GRID_X;
  int y2 =  y1 + GRID_Y;

  color(RGB(159), 0, 0);
  rectFill(x1, y1, x2, y2);
}

void initMatrixCandyCrush(){
  Candy *candy;
  for(int y = 0; y < GRID_SIZE; y++){//Line
    for(int x = 0; x < GRID_SIZE; x++){//Column
      candy = getRandomCandy();
      candy->position.x = x;
      candy->position.y = y;
      *((matrixCandy+y*GRID_SIZE) + x) = candy;
    }
  }
}

void drawCandy(Candy *candy){
  if(candy->willExplode == true){
    markBackGridAsRed(candy->position);
  }
  const int sizeCandy = CANDY_SIZE;
  const int sides = candy->shapeSides;
  int centerX = getCenterGridX(candy->position.x);
  int centerY = getCenterGridY(candy->position.y);
  color(candy->red, candy->green, candy->blue);
  circleFill(centerX, centerY, sizeCandy, sides);
}

void drawCandiesOnScreen(){
  Candy *candy;
  for(int y = 0; y < GRID_SIZE; y++){
    for(int x = 0; x < GRID_SIZE; x++){
      candy = *((matrixCandy+x*GRID_SIZE) + y);
      drawBackGrid(x, y);
      if(candy != NULL){
        drawCandy(candy);
      }
    }
  }
}

Candy* getCandy(int line, int column, Candy **matrixCandyRef){
  return *(matrixCandyRef+line*GRID_SIZE + column);
}

void checkCrushX(Candy **matrixCandyRef, int line){//Check if there is candies to explode in a given line
  Candy *currentCandy;
  int keyType, contador = 0;
  for(int column = 0; column < GRID_SIZE; column++){
    currentCandy = getCandy(line, column, matrixCandyRef);
    if(column == 0){
      keyType = currentCandy->type;
      contador++;
    } else {
      if(keyType == currentCandy->type){
        contador++;
        if(contador >= 3){
          if(contador == 3){//Mark the 2 candies before the 3th
            (getCandy(line, column-1, matrixCandyRef))->willExplode = true;
            (getCandy(line, column-2, matrixCandyRef))->willExplode = true;
          }
          currentCandy->willExplode = true;
        }
      } else {
        contador = 1;
        keyType = currentCandy->type;
      }
    }
  }
}

void checkCrushY(Candy **matrixCandyRef, int column){//Check if there is candies to explode in a given line
  Candy *currentCandy;
  int keyType, contador = 0;
  for(int line = 0; line < GRID_SIZE; line++){
    currentCandy = getCandy(line, column, matrixCandyRef);
    if(line == 0){
      keyType = currentCandy->type;
      contador++;
    } else {
      if(keyType == currentCandy->type){
        contador++;
        if(contador >= 3){
          if(contador == 3){//Mark the 2 candies before the 3th
            (getCandy(line-1, column, matrixCandyRef))->willExplode = true;
            (getCandy(line-2, column, matrixCandyRef))->willExplode = true;
          }
          currentCandy->willExplode = true;
        }
      } else {
        contador = 1;
        keyType = currentCandy->type;
      }
    }
  }
}

void checkCrush(Candy **matrixCandyRef){//Check if there is candies to explode
  for(int i = 0; i < GRID_SIZE; i++){
    checkCrushX(matrixCandyRef, i);
    checkCrushY(matrixCandyRef, i);
  }
}


//////////////////////////////////////////////////////

void render(){

    clear(0,0,0);
    updateWindowProps();
    drawCandiesOnScreen();
    // rect(100, 100, 100+100, 100+100);
    //
    // circle(200, 200, 100, 4);
    //
    // text(20,500,"Programa Demo Canvas C.");
}

//funcao chamada toda vez que uma tecla for pressionada
void keyboard(int key){
   printf("\nTecla: %d" , key);

}
//funcao chamada toda vez que uma tecla for liberada
void keyboardUp(int key){
   printf("\nLiberou tecla: %d" , key);
}


//funcao para tratamento de mouse: cliques, movimentos e arrastos
void mouse(int button, int state, int wheel, int direction, int x, int y){
   y = DIM_TELA_Y - y;

   printf("\nmouse %d %d %d %d %d %d", button, state, wheel, direction,  x, y);

}

int main(void){
    srand(time(0));
    initMatrixCandyCrush();
    checkCrush(matrixCandy);
    initCanvas(DIM_TELA_X + WIDTH_POINTS, DIM_TELA_Y, "Candy Crush");
    runCanvas();
}
