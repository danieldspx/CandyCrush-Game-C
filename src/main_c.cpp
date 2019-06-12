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
#define ANIMATION_STAGE_MAX 50

#define INVERT_ANIMATION true

const int WIDTH_POINTS = 200;
//Properties
int DIM_TELA_X = 600;
int DIM_TELA_Y = 600;
int GRID_X = (int)ceil(DIM_TELA_X/GRID_SIZE);
int GRID_Y = (int)ceil(DIM_TELA_Y/GRID_SIZE);
clock_t G_begin_time;
//Flags
bool hasMatrixCandyChanged = true;//Start as a New state
bool hasAnimations = true;//Start as a New state
bool hasSelectedCandy = false;
int totalAnimation = 0;
int totalSelectedCandy = 0;
unsigned int G_totalPoints = 0;

typedef struct {
  int x;
  int y;
} Position;

typedef struct {
  Position from;
  Position to;
  bool isAnimating;
  bool shouldInvert;
  int stage;//Max stage wich means that the animation is finished
} Animation;

typedef struct {
  int type;
  Position position;
  Animation animation;
  bool willExplode;
  float red, green, blue;//Color
  int shapeSides;
} Candy;

typedef struct {
  int line;
  int column;
  Candy **ref;
} CandyRef;

//Variables
Candy **matrixCandy = (Candy **)malloc(CANDY_SIZE*CANDY_SIZE*sizeof(Candy *));
Position selectedCandies[2];

int getRandomType();
void updateWindowProps();
void setCandyProperties(Candy *candy);
Candy* getRandomCandy();
int getCenterGridX(int x);
int getCenterGridY(int y);
void drawBackGrid(int x, int y);
void markBackGridAsRed(Position pos);
void initMatrixCandyCrush(Candy **matrixCandyRef);
void drawCandy(Candy *candy);
void drawCandiesOnScreen();
void hightLightExplodableCandies(Candy **matrixCandyRef);
Candy* getCandy(CandyRef candyRef);
void removeCandyOfMatrix(CandyRef candyRef);
void explodeCandies(Candy **matrixCandyRef);
bool checkCrushX(Candy **matrixCandyRef, int line, bool shouldSetExplode);
bool checkCrushY(Candy **matrixCandyRef, int column, bool shouldSetExplode);
void checkCrush(Candy **matrixCandyRef);
void swapCandiesOnMatrix(Position from, Position to, Candy **matrix);
void initCandiesAnimation(Candy **matrixCandyRef);
void animateCandy(Candy *candy, Position center);
void setWillExplode(CandyRef candyRef);
void initFirstLineTop(Candy **matrixCandyRef);
bool isClickInsideArea(Position pointBottomLeft, Position pointTopRight, Position click);
Position mapCandyClicked(Position click);
void drawSelectionCandy();
bool isNeighbor(Position target, Position check);
void setAnimationProperties(Candy *candy1, Candy *candy2, bool invertAnimation);
void drawPointsOnScreen();
bool hasAnimationRunning(Candy **matrixCandyRef);
bool hasPossiblePlay(Candy **matrixCandyRef);
void resetMatrixCandies(Candy **matrixCandyRef);
void verifyMatrixCandyPlays(Candy **matrixCandyRef);
void updateAnimationCounter(Candy **matrixCandyRef);
bool isMatrixFullFilled(Candy **matrixCandyRef);
bool isClickOnResetBtn(Position click);
void drawTimeOnScreen();
void drawResetButton();
void resetTime();


//////////////////////////////////////////////////////

void render(){
    clear(0,0,0);
    updateWindowProps();
    updateAnimationCounter(matrixCandy);
    verifyMatrixCandyPlays(matrixCandy);
    initFirstLineTop(matrixCandy);
    initCandiesAnimation(matrixCandy);
    checkCrush(matrixCandy);
    drawSelectionCandy();
    drawCandiesOnScreen();
    drawPointsOnScreen();
    drawTimeOnScreen();
    drawResetButton();
    // rect(100, 100, 100+100, 100+100);
    //
    // circle(200, 200, 100, 4);
    //
    // text(20,500,"Programa Demo Canvas C.");
}

//funcao chamada toda vez que uma tecla for pressionada
void keyboard(int key){
   printf("\nTecla: %d" , key);
   printf("\nhasAnimations: %d" , hasAnimations);
   printf("\ntotalAnimation: %d" , totalAnimation);
   printf("\nhasMatrixCandyChanged: %d" , hasMatrixCandyChanged);
   printf("\nhasAnimationRunning: %d" , hasAnimationRunning(matrixCandy));
   printf("\nhasPossiblePlay: %d" , hasPossiblePlay(matrixCandy));
}
//funcao chamada toda vez que uma tecla for liberada
void keyboardUp(int key){
   printf("\nLiberou tecla: %d" , key);
}


//funcao para tratamento de mouse: cliques, movimentos e arrastos
void mouse(int button, int state, int wheel, int direction, int x, int y){
   y = DIM_TELA_Y - y;

   //printf("\nmouse %d %d %d %d %d %d", button, state, wheel, direction,  x, y);

   if(button == 0 && state == 0){
     Position pointBottomLeft = {0, 0};
     Position pointTopRight = {GRID_X*GRID_SIZE, GRID_Y*GRID_SIZE};
     Position click = {x, y};
     Position selected;
     CandyRef candyRef;
     Candy *candy1, *candy2;
     if(isClickInsideArea(pointBottomLeft, pointTopRight, click)){
       selected = mapCandyClicked(click);
       if(totalSelectedCandy == 1 && isNeighbor(selectedCandies[0], selected)){
         selectedCandies[1] = selected;
         candyRef.ref = matrixCandy;

         candyRef.line = selectedCandies[0].y;
         candyRef.column = selectedCandies[0].x;
         candy1 = getCandy(candyRef);

         candyRef.line = selectedCandies[1].y;
         candyRef.column = selectedCandies[1].x;
         candy2 = getCandy(candyRef);

         if(candy1 == NULL || candy2 == NULL){
           totalSelectedCandy = 0;
           hasSelectedCandy = false;
           return;
         }

         swapCandiesOnMatrix(selectedCandies[0], selectedCandies[1], matrixCandy);//Swap to simulate

         bool willMakeAPoint = checkCrushX(matrixCandy, selectedCandies[1].y, true) ||
                                checkCrushY(matrixCandy, selectedCandies[1].x, true) ||
                                checkCrushX(matrixCandy, selectedCandies[0].y, true) ||
                                checkCrushY(matrixCandy, selectedCandies[0].x, true);

         swapCandiesOnMatrix(selectedCandies[0], selectedCandies[1], matrixCandy);//Swap back after simulation

         if(willMakeAPoint){
           setAnimationProperties(candy1, candy2, false);
         } else {//If we should go back to original after animation
           setAnimationProperties(candy1, candy2, INVERT_ANIMATION);
         }

         swapCandiesOnMatrix(selectedCandies[0], selectedCandies[1], matrixCandy);

         totalSelectedCandy = 0;
         hasSelectedCandy = false;
       } else {
         selectedCandies[0] = mapCandyClicked(click);
         totalSelectedCandy = 1;
         hasSelectedCandy = true;
       }
     } else {
       totalSelectedCandy = 0;
       hasSelectedCandy = false;
       if(isClickOnResetBtn(click)){
         resetMatrixCandies(matrixCandy);
         resetTime();
       }
     }
   }

}

int main(void){
    resetTime();
    srand(time(0));
    initMatrixCandyCrush(matrixCandy);
    initCanvas(DIM_TELA_X + WIDTH_POINTS, DIM_TELA_Y, "Candy Crush");
    runCanvas();
}

void resetTime(){
  G_begin_time = clock();
}

void drawSelectionCandy(){
  if(hasSelectedCandy){
    for(int i = 0; i < totalSelectedCandy; i++){
      markBackGridAsRed(selectedCandies[i]);
    }
  }
}

bool isNeighbor(Position target, Position check){
  if((target.x == check.x || target.y == check.y) &&
    (abs(target.x - check.x) == 1 || abs(target.y - check.y) == 1 )){
    return true;
  }
  return false;
}

bool isClickInsideArea(Position pointBottomLeft, Position pointTopRight, Position click){
  int areaWidth = pointTopRight.x - pointBottomLeft.x;
  int areaHeight = pointTopRight.y - pointBottomLeft.y;
  int clickWidth = click.x - pointBottomLeft.x;
  int clickHeight = click.y - pointBottomLeft.y;
  if(clickHeight > 0 && clickWidth > 0){
    if(clickWidth <= areaWidth && clickHeight <= areaHeight){
      return true;
    }
  }
  return false;
}

Position mapCandyClicked(Position click){
  Position selected;
  selected.x = click.x / GRID_X;
  selected.y = click.y / GRID_Y;
  return selected;
}

int getRandomType(){
  return ((rand() % 4)+1);
}

void updateWindowProps(){
  getWindowSize(&DIM_TELA_X, &DIM_TELA_Y);
  GRID_X = (int)ceil(DIM_TELA_X/GRID_SIZE) - ((int)WIDTH_POINTS/GRID_SIZE);
  GRID_Y = (int)ceil(DIM_TELA_Y/GRID_SIZE);
}

void setAnimationProperties(Candy *candy1, Candy *candy2, bool invertAnimation){//candy1 <-> candy2
  candy1->animation.isAnimating = true;
  candy1->animation.stage = 0;
  candy1->animation.from = candy1->position;
  candy1->animation.to = candy2->position;

  candy2->animation.isAnimating = true;
  candy2->animation.stage = 0;
  candy2->animation.from = candy2->position;
  candy2->animation.to = candy1->position;

  if(invertAnimation == INVERT_ANIMATION){
    totalAnimation++;
    candy1->animation.shouldInvert = true;
    //We do not need to say that candy2 should invert
    //Because when candy1 inverts, it will be with candy2
  } else {
    candy1->animation.shouldInvert = false;
    candy2->animation.shouldInvert = false;
    totalAnimation += 2;
  }
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
  candy->animation.stage = 0;
  candy->animation.isAnimating = false;
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

void initMatrixCandyCrush(Candy **matrixCandyRef){
  Candy *candy;
  for(int y = 0; y < GRID_SIZE; y++){//Line
    for(int x = 0; x < GRID_SIZE; x++){//Column
      candy = getRandomCandy();
      candy->position.x = x;
      candy->position.y = y;
      *((matrixCandyRef+y*GRID_SIZE) + x) = candy;
    }
  }
}

void initFirstLineTop(Candy **matrixCandyRef){
  Candy *candy;
  CandyRef candyRef;
  candyRef.ref = matrixCandyRef;
  candyRef.line = GRID_SIZE-1;
  for(int column = 0; column < GRID_SIZE; column++){//Column
    candyRef.column = column;
    candy = getCandy(candyRef);
    if(candy == NULL){
      candy = getRandomCandy();
      totalAnimation++;
      candy->animation.isAnimating = true;
      candy->animation.shouldInvert = false;
      candy->animation.from.x = column;
      candy->animation.from.y = GRID_SIZE;
      candy->animation.to.x = column;
      candy->animation.to.y = GRID_SIZE-1;
      candy->position.x = column;
      candy->position.y = candyRef.line;
      *((matrixCandyRef+candyRef.line*GRID_SIZE) + column) = candy;
      hasMatrixCandyChanged = true;
    }
  }
}

void drawCandy(Candy *candy){
  const int sizeCandy = CANDY_SIZE;
  const int sides = candy->shapeSides;
  Position center;
  center.x = getCenterGridX(candy->position.x);
  center.y = getCenterGridY(candy->position.y);
  color(candy->red, candy->green, candy->blue);//Color wont change anyway
  if(candy->animation.isAnimating){
    animateCandy(candy, center);
  } else {
    circleFill(center.x, center.y, sizeCandy, sides);
  }
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

void hightLightExplodableCandies(Candy **matrixCandyRef){
  Candy *currentCandy;
  CandyRef candyRef;
  candyRef.ref = matrixCandyRef;
  for(int line = 0; line < GRID_SIZE; line++){
    for(int column = 0; column < GRID_SIZE; column++){
      candyRef.line = line;
      candyRef.column = column;
      currentCandy = getCandy(candyRef);
      if(currentCandy != NULL){
        if(currentCandy->willExplode){
          Position positionRed;
          positionRed.x = column;
          positionRed.y = line;
          markBackGridAsRed(positionRed);
        }
      }
    }
  }
}

Candy* getCandy(CandyRef candyRef){
  return *(candyRef.ref+candyRef.line*GRID_SIZE + candyRef.column);
}

void removeCandyOfMatrix(CandyRef candyRef){
  *(candyRef.ref+candyRef.line*GRID_SIZE + candyRef.column) = NULL;
}

void explodeCandies(Candy **matrixCandyRef){
  Candy *currentCandy;
  CandyRef candyRef;
  candyRef.ref = matrixCandyRef;
  for(int line = 0; line < GRID_SIZE; line++){
    for(int column = 0; column < GRID_SIZE; column++){
      candyRef.line = line;
      candyRef.column = column;
      currentCandy = getCandy(candyRef);
      if(currentCandy == NULL){
        continue;
      }
      if(currentCandy != NULL && currentCandy->willExplode){
        free(currentCandy);
        removeCandyOfMatrix(candyRef);
        G_totalPoints++;
      }
    }
  }
}

bool checkCrushX(Candy **matrixCandyRef, int line, bool shouldSetExplode){//Check if there is candies to explode in a given line
  Candy *currentCandy = NULL;
  CandyRef candyRef;
  candyRef.ref = matrixCandyRef;
  int keyType, contador = 0;
  bool hasExplodable = false;
  for(int column = 0; column < GRID_SIZE; column++){
    candyRef.line = line;
    candyRef.column = column;
    currentCandy = getCandy(candyRef);
    if(currentCandy == NULL){
      continue;
    }
    if(column == 0){
      keyType = currentCandy->type;
      contador++;
    } else {
      if(keyType == currentCandy->type){
        contador++;
        if(contador >= 3){
          if(shouldSetExplode){
            if(contador == 3){//Mark the 2 candies before the 3th
              candyRef.column--;//Back one column
              setWillExplode(candyRef);
              candyRef.column--;//Back one column again
              setWillExplode(candyRef);
            }
            currentCandy->willExplode = true;
          }
          hasExplodable = true;
        }
      } else {
        contador = 1;
        keyType = currentCandy->type;
      }
    }
  }
  return hasExplodable;
}

void setWillExplode(CandyRef candyRef){
  Candy *candy = getCandy(candyRef);
  if(candy != NULL){
    candy->willExplode = true;
  }
}

bool checkCrushY(Candy **matrixCandyRef, int column, bool shouldSetExplode){//Check if there is candies to explode in a given line
  Candy *currentCandy = NULL;
  CandyRef candyRef;
  candyRef.ref = matrixCandyRef;
  int keyType, contador = 0;
  bool hasExplodable = false;
  for(int line = 0; line < GRID_SIZE; line++){
    candyRef.line = line;
    candyRef.column = column;
    currentCandy = getCandy(candyRef);
    if(currentCandy == NULL){
      continue;
    }
    if(line == 0){
      keyType = currentCandy->type;
      contador++;
    } else {
      if(keyType == currentCandy->type){
        contador++;
        if(contador >= 3){
          if(shouldSetExplode){
            if(contador == 3){//Mark the 2 candies before the 3th
              candyRef.line--;//Back one column
              setWillExplode(candyRef);
              candyRef.line--;//Back one column again
              setWillExplode(candyRef);
            }
            currentCandy->willExplode = true;
          }
          hasExplodable = true;
        }
      } else {
        contador = 1;
        keyType = currentCandy->type;
      }
    }
  }
  return hasExplodable;
}

void checkCrush(Candy **matrixCandyRef){//Check if there is candies to explode
  if(totalAnimation < 0){
    totalAnimation = 0;
  }
  if(totalAnimation == 0){
    if(hasMatrixCandyChanged && !hasAnimations){
      hasMatrixCandyChanged = false;
      for(int i = 0; i < GRID_SIZE; i++){
        checkCrushX(matrixCandyRef, i, true);
        checkCrushY(matrixCandyRef, i, true);
      }
      hightLightExplodableCandies(matrixCandyRef);
    }
    explodeCandies(matrixCandyRef);
  }
}

void swapCandiesOnMatrix(Position from, Position to, Candy **matrix){
  Candy *tempCandy1 = *(matrix+from.y*GRID_SIZE + from.x);
  Candy *tempCandy2 = *(matrix+to.y*GRID_SIZE + to.x);
  if(tempCandy1 != NULL){
    tempCandy1->position.x = to.x;
    tempCandy1->position.y = to.y;
  }
  if(tempCandy2 != NULL){
    tempCandy2->position.x = from.x;
    tempCandy2->position.y = from.y;
  }
  *(matrix+to.y*GRID_SIZE + to.x) = tempCandy1;
  *(matrix+from.y*GRID_SIZE + from.x) = tempCandy2;
  hasMatrixCandyChanged = true;
}

void initCandiesAnimation(Candy **matrixCandyRef){
  Candy *upCandy, *belowCandy;
  CandyRef candyRefUp, candyRefBelow;
  hasAnimations = false;
  candyRefUp.ref = matrixCandyRef;
  candyRefBelow.ref = matrixCandyRef;
  for(int column = 0; column < GRID_SIZE; column++){
    for(int line = 0; line < (GRID_SIZE-1) ; line++){//Iterates bottom to top on the Matrix
      candyRefUp.line = line+1;
      candyRefBelow.line = line;
      candyRefBelow.column = candyRefUp.column = column;

      upCandy = getCandy(candyRefUp);
      belowCandy = getCandy(candyRefBelow);

      if(belowCandy == NULL && upCandy != NULL){
        if(upCandy->animation.isAnimating == false){
          upCandy->animation.isAnimating = true;
          upCandy->animation.shouldInvert = false;
          upCandy->animation.stage = 0;
          upCandy->animation.from.y = line+1;//Line up
          upCandy->animation.from.x = column;//Line up
          upCandy->animation.to.y = line;//Line below
          upCandy->animation.to.x = column;//Line below
          swapCandiesOnMatrix(upCandy->animation.from, upCandy->animation.to, matrixCandyRef);
          totalAnimation++;//Add one animation to the counter
          hasAnimations = true;
        }
      }
    }
  }
}

void animateCandy(Candy *candy, Position center){
  const int sizeCandy = CANDY_SIZE;
  const int stage = candy->animation.stage;
  float animatioPosY = center.y;
  float animatioPosX = center.x;
  if(stage < ANIMATION_STAGE_MAX){
    //Calc the position of the animation stage
    animatioPosY += (candy->animation.from.y - candy->animation.to.y)*(CANDY_SIZE*2.2)*(1-((float)stage/ANIMATION_STAGE_MAX));
    animatioPosX += (candy->animation.from.x - candy->animation.to.x)*(CANDY_SIZE*2.2)*(1-((float)stage/ANIMATION_STAGE_MAX));
    circleFill(animatioPosX, animatioPosY, sizeCandy, candy->shapeSides);
  } else {//Animation has finished
    totalAnimation--;//Remove one animation from the counter
    circleFill(center.x, center.y, sizeCandy, candy->shapeSides);
    if(candy->animation.shouldInvert){//Should invert position back to original before swap
      CandyRef candyRef;
      candyRef.ref = matrixCandy;
      candyRef.line = candy->animation.from.y;//`animation.from` holds the other candy that was in position `animation.to`
      candyRef.column = candy->animation.from.x;

      candy->animation.shouldInvert = false;

      setAnimationProperties(candy, getCandy(candyRef), false);
      swapCandiesOnMatrix(candy->animation.from, candy->animation.to, matrixCandy);//Swap back
    } else {
      candy->animation.isAnimating = false;
    }
  }
  candy->animation.stage++;
}

void drawPointsOnScreen(){
  int margin = 20;
  int posX = DIM_TELA_X - WIDTH_POINTS + margin;
  int posY = DIM_TELA_Y - 20;
  char points[50];
  sprintf(points, "Pontos: %d", G_totalPoints);
  color(RGB(255), RGB(255), RGB(255));
  text(posX, posY, points);
}

bool hasPossiblePlay(Candy **matrixCandyRef){
  Position from, to;
  bool hasPossiblePlay = false;
  CandyRef candyRef;
  bool stateMatrixChanged;
  candyRef.ref = matrixCandyRef;
  for(int line = 0; line < GRID_SIZE; line++){
    for(int column = 0; column < GRID_SIZE; column++){
      candyRef.line = line;
      candyRef.column = column;
      if(getCandy(candyRef) != NULL){
        from.y = line;
        from.x = column;

        to.y = line;
        to.x = column+1;
        if(column == (GRID_SIZE-1)){//Avoid overflow
          to.x = column;
        }
        // printf("(%d, %d) to (%d, %d) Right\n", from.x, from.y, to.x, to.y);
        stateMatrixChanged = hasMatrixCandyChanged;//Keep the state of the variable before the changes
        swapCandiesOnMatrix(from, to, matrixCandyRef);//Move Candy (right)
        if(column == (GRID_SIZE-1)){
          hasPossiblePlay = checkCrushX(matrixCandyRef, line, false) ||
                            checkCrushY(matrixCandyRef, column, false);
        } else {
          hasPossiblePlay = checkCrushX(matrixCandyRef, line, false) ||
                            checkCrushY(matrixCandyRef, column, false) ||
                            checkCrushY(matrixCandyRef, column+1, false);
        }
        swapCandiesOnMatrix(to, from, matrixCandyRef);//Goes back to original (left)
        hasMatrixCandyChanged = stateMatrixChanged;
        if(hasPossiblePlay){
          return true;
        }

        to.y = line+1;
        to.x = column;
        if(line == (GRID_SIZE-1)){//Avoid overflow
          to.y = line;
        }
        stateMatrixChanged = hasMatrixCandyChanged;
        swapCandiesOnMatrix(from, to, matrixCandyRef);//Move Candy (down)
        if(line == (GRID_SIZE-1)){
          hasPossiblePlay = checkCrushX(matrixCandyRef, line, false) ||
                            checkCrushY(matrixCandyRef, column, false);
        } else {
          hasPossiblePlay = checkCrushX(matrixCandyRef, line, false) ||
                            checkCrushX(matrixCandyRef, line+1, false) ||
                            checkCrushY(matrixCandyRef, column, false);
        }
        swapCandiesOnMatrix(to, from, matrixCandyRef);//Goes back to original (up)
        hasMatrixCandyChanged = stateMatrixChanged;
        if(hasPossiblePlay){
          return true;
        }
      }
    }
  }
  return false;
}

void resetMatrixCandies(Candy **matrixCandyRef){
  for(int line = 0; line < GRID_SIZE; line++){
    for(int column = 0; column < GRID_SIZE; column++){
      free(*((matrixCandyRef+line*GRID_SIZE) + column));
      *((matrixCandyRef+line*GRID_SIZE) + column) = NULL;
    }
  }
}

bool hasAnimationRunning(Candy **matrixCandyRef){
  Candy *candy;
  for(int line = 0; line < GRID_SIZE; line++){
    for(int column = 0; column < GRID_SIZE; column++){
      candy = *((matrixCandyRef+line*GRID_SIZE) + column);
      if(candy != NULL){
        if(candy->animation.isAnimating){
          return true;
        }
      }
    }
  }
  return false;
}

bool isMatrixFullFilled(Candy **matrixCandyRef){
  Candy *candy;
  for(int line = 0; line < GRID_SIZE; line++){
    for(int column = 0; column < GRID_SIZE; column++){
      candy = *((matrixCandyRef+line*GRID_SIZE) + column);
      if(candy == NULL){
        return false;
      }
    }
  }
  return true;
}

void verifyMatrixCandyPlays(Candy **matrixCandyRef){
  if(isMatrixFullFilled(matrixCandyRef) && !hasPossiblePlay(matrixCandyRef) && !hasAnimationRunning(matrixCandyRef)){
    resetMatrixCandies(matrixCandyRef);
  }
}

void updateAnimationCounter(Candy **matrixCandyRef){
  if(hasAnimationRunning(matrixCandyRef) == false && totalAnimation != 0){
    totalAnimation = 0;
  }
}

void drawTimeOnScreen(){
  float time = float( clock () - G_begin_time ) /  CLOCKS_PER_SEC;
  int margin = 20;
  int posX = DIM_TELA_X - WIDTH_POINTS + margin;
  int posY = DIM_TELA_Y - 50;
  char timeScreen[50];
  sprintf(timeScreen, "Tempo: %.2f", time);
  color(RGB(255), RGB(255), RGB(255));
  text(posX, posY, timeScreen);
}

bool isClickOnResetBtn(Position click){
  int margin = 20, marginTop = 100;
  int height = 40, width = 100;

  Position bottomLeft, topRight;

  bottomLeft.x = DIM_TELA_X - WIDTH_POINTS + margin;
  bottomLeft.y = DIM_TELA_Y - (height + marginTop);

  topRight.x = bottomLeft.x + width;
  topRight.y = bottomLeft.y + height;

  return isClickInsideArea(bottomLeft, topRight, click);
}

void drawResetButton(){
  int margin = 20, marginTop = 100;
  int height = 40, width = 100;

  Position bottomLeft, topRight;

  bottomLeft.x = DIM_TELA_X - WIDTH_POINTS + margin;
  bottomLeft.y = DIM_TELA_Y - (height + marginTop);

  topRight.x = bottomLeft.x + width;
  topRight.y = bottomLeft.y + height;

  color(RGB(255), RGB(0), RGB(0));
  rectFill(bottomLeft.x, bottomLeft.y, topRight.x, topRight.y);

  color(RGB(255), RGB(255), RGB(255));
  text(bottomLeft.x + 5, topRight.y-((topRight.y - bottomLeft.y)/2 + 5), "Resetar");
}
