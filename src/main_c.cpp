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

void initMatrixCandyCrush(){
  Candy *candy;
  for(int i = 0; i < GRID_SIZE; i++){//Line
    for(int j = 0; j < GRID_SIZE; j++){//Column
      candy = getRandomCandy();
      candy->position.x = i;
      candy->position.y = j;
      *((matrixCandy+i*GRID_SIZE) + j) = candy;
    }
  }
}

void drawCandy(Candy *candy){
  const int sizeCandy = CANDY_SIZE;
  const int sides = candy->shapeSides;
  int centerX = getCenterGridX(candy->position.x);
  int centerY = getCenterGridY(candy->position.y);
  color(candy->red, candy->green, candy->blue);
  circleFill(centerX, centerY, sizeCandy, sides);
}

void drawCandiesOnScreen(){
  Candy *candy;
  for(int x = 0; x < GRID_SIZE; x++){
    for(int y = 0; y < GRID_SIZE; y++){
      candy = *(matrixCandy+x*GRID_SIZE + y);
      drawBackGrid(x, y);
      if(*candy != NULL){
        drawCandy(candy);
      }
    }
  }
}

void checkCrushX(Candy **matrixCandyX, int line){//Check if there is candies to explode in a given line
  Candy **currentLine = matrixCandyX+line*GRID_SIZE;
  Candy *currentCandy;
  int keyType, contador = 0;
  for(int x = 0; x < GRID_SIZE; x++){
    currentCandy = currentLine[x];
    if(x == 0){
      keyType = currentCandy->type;
    } else {
      if(keyType == currentCandy->type){
        contador++;
        if(contador >= 3){
          if(contador == 3){//Mark the 2 candies before the 3th
            (currentLine[x-1])->willExplode = true;
            (currentLine[x-2])->willExplode = true;
          }
          currentCandy->willExplode = true;
          printf("(%d, %d)\n", x, line);
        }
      } else {
        contador = 0;
        keyType = currentCandy->type;
      }
    }
  }
}

void checkCrush(){//Check if there is candies to explode

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
    // checkCrushX(matrixCandy, 0);
    initCanvas(DIM_TELA_X + WIDTH_POINTS, DIM_TELA_Y, "Candy Crush");
    runCanvas();
}
