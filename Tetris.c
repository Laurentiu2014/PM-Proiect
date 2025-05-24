#include <LCDWIKI_KBV.h>



LCDWIKI_KBV my_lcd(ILI9486, 40, 38, 39, 1, 41);

//Marimi Tetris
#define BLOCK_SIZE 12
#define GRID_WIDTH 12
#define GRID_HEIGHT 25
#define SCREEN_LEFT 50
#define SCREEN_TOP 10
//Joystick
const int pinJoystick = A0; 

// Butoane
#define BTN_LEFT 13
#define BTN_RIGHT 10
#define BTN_DOWN 11
#define BTN_ROTATE 12
#define BTN_SELECT 8
// Buzzer
#define BUZZER_PIN 9

// Culori
#define BLACK 0x0000
#define WHITE 0xFFFF
#define CYAN  0x07FF
#define YELLOW 0xFFE0
#define PURPLE 0x781F
#define GREEN 0x07E0
#define RED   0xF800
#define BLUE  0x001F
#define ORANGE 0xFD20
#define GRAY 0x8410

//Note Muzicale
#define NOTE_B3  247
#define NOTE_A3  220
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define REST     0

//========================================--STRUCTURA SI  FUNCTIE MELODIE--==============================================
struct Melody {
  const int* notes;
  const int* durations;
  int length;
};

int currentNote = 0;
unsigned long noteStartTime = 0;

// Funcție generală pentru redare melodie
void playMelodyNonBlocking(const Melody* melody) {
  unsigned long now = millis();

  if (currentNote < melody->length) {
    if (now - noteStartTime >= melody->durations[currentNote]) {
      noteStartTime = now;
      int note = melody->notes[currentNote];
      int duration = melody->durations[currentNote];

      if (note == REST) {
        noTone(BUZZER_PIN);
      } else {
        tone(BUZZER_PIN, note);
      }

      currentNote++;
    }
  } else {
    currentNote = 0; // reia melodia
  }
}

const int tetrisNotes[] = {
  NOTE_E4, NOTE_B3, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_B3, NOTE_A3,
  NOTE_A3, NOTE_C4, NOTE_E4, NOTE_D4, NOTE_C4, NOTE_B3, NOTE_C4,
  NOTE_D4, NOTE_E4, NOTE_C4, NOTE_A3, NOTE_A3, REST
};

const int tetrisDurations[] = {
  150, 150, 150, 150, 150, 150, 300,
  150, 150, 150, 150, 150, 150, 150,
  150, 150, 150, 150, 300, 150
};

Melody tetrisMelody = {
  tetrisNotes,
  tetrisDurations,
  sizeof(tetrisNotes) / sizeof(tetrisNotes[0])
};

const int gameOverNotes[] = {
  NOTE_E4, NOTE_C4, NOTE_A3, REST
};

const int gameOverDurations[] = {
  300, 300, 600, 300
};

Melody gameOverMelody = {
  gameOverNotes,
  gameOverDurations,
  sizeof(gameOverNotes) / sizeof(gameOverNotes[0])
};



// Grid joc: 0 = gol, altfel culoarea blocului
uint16_t grid[GRID_HEIGHT][GRID_WIDTH];

#define NUM_PIECES 7

// Piese 4x4, fiecare cu 4x4, 0=gol, 1=bloc
const uint8_t pieces[NUM_PIECES][4][4] = {
  // Piesa O
  {
    {0,0,0,0},
    {0,1,1,0},
    {0,1,1,0},
    {0,0,0,0}
  },
  // Piesa I
  {
    {0,0,0,0},
    {1,1,1,1},
    {0,0,0,0},
    {0,0,0,0}
  },
  // Piesa T
  {
    {0,0,0,0},
    {1,1,1,0},
    {0,1,0,0},
    {0,0,0,0}
  },
  // Piesa L
  {
    {0,0,0,0},
    {1,1,1,0},
    {1,0,0,0},
    {0,0,0,0}
  },
  // Piesa J
  {
    {0,0,0,0},
    {1,1,1,0},
    {0,0,1,0},
    {0,0,0,0}
  },
  // Piesa S
  {
    {0,0,0,0},
    {0,1,1,0},
    {1,1,0,0},
    {0,0,0,0}
  },
  // Piesa Z
  {
    {0,0,0,0},
    {1,1,0,0},
    {0,1,1,0},
    {0,0,0,0}
  }
};

uint16_t pieceColors[NUM_PIECES] = {YELLOW, CYAN, PURPLE, ORANGE, BLUE, GREEN, RED};

uint8_t currentPiece[4][4];
uint16_t pieceColor;
int pieceSize = 4;

int pieceX = 4;
int pieceY = 0;

bool leaderboardNeedsUpdate = true;

// Scor
unsigned long score = 0;
unsigned long highScores[10] = {0,0,0,0,0,0,0,0,0,0};

// Control timpi
unsigned long lastFall = 0;
unsigned long fallInterval = 500; // ms pentru caderea piesei

// Meniu
bool inMenu = true;
int menuSelection = 0; // 0 = Play, 1 = Exit

#define MENU_ITEMS 2
const char* menuOptions[MENU_ITEMS] = {"Play", "Exit"};

// --- Functii desen ---

void drawBlock(int x, int y, uint16_t color) {
  int px = SCREEN_LEFT + x * BLOCK_SIZE;
  int py = SCREEN_TOP + y * BLOCK_SIZE;
  my_lcd.Set_Draw_color(color);
  my_lcd.Fill_Rectangle(px, py, px + BLOCK_SIZE - 1, py + BLOCK_SIZE - 1);
}

void drawGridBlocks() {
  for(int r=0; r<GRID_HEIGHT; r++) {
    for(int c=0; c<GRID_WIDTH; c++) {
      uint16_t col = (grid[r][c] != 0) ? grid[r][c] : BLACK;
      drawBlock(c, r, col);
    }
  }
}

void drawBorder() {
  int px1 = SCREEN_LEFT;
  int py1 = SCREEN_TOP;
  int px2 = SCREEN_LEFT + GRID_WIDTH * BLOCK_SIZE - 1;
  int py2 = SCREEN_TOP + GRID_HEIGHT * BLOCK_SIZE - 1;

  my_lcd.Set_Draw_color(WHITE);
  for (int i = 0; i < 2; i++) {  // contur de 2 pixeli grosime
    my_lcd.Draw_Rectangle(px1 - i, py1 - i, px2 + i, py2 + i);
  }
}


void drawScoreboardBackground() {
  int sbLeft = SCREEN_LEFT + GRID_WIDTH * BLOCK_SIZE + 20;
  int sbTop = SCREEN_TOP;
  my_lcd.Set_Draw_color(BLACK);
  my_lcd.Fill_Rectangle(sbLeft - 5, sbTop - 5, sbLeft + 140, sbTop + 240);
}

void drawScoreboardStatic() {
  int sbLeft = SCREEN_LEFT + GRID_WIDTH * BLOCK_SIZE + 20;
  int sbTop = SCREEN_TOP;

  my_lcd.Set_Text_Size(2);
  my_lcd.Set_Text_colour(WHITE);
  my_lcd.Set_Text_Back_colour(BLACK);

  my_lcd.Print_String("SCORE:", sbLeft, sbTop);
  my_lcd.Print_String("TOP 10:", sbLeft, sbTop + 60);
}

void updateScoreboard() {
  int sbLeft = SCREEN_LEFT + GRID_WIDTH * BLOCK_SIZE + 20;
  int sbTop = SCREEN_TOP;

  my_lcd.Set_Text_Size(2);
  my_lcd.Set_Text_colour(WHITE);
  my_lcd.Set_Text_Back_colour(BLACK);

  char buf[20];
  sprintf(buf, "%lu", score);
  // Sterge zona scorului vechi
  my_lcd.Set_Draw_color(BLACK);
  my_lcd.Fill_Rectangle(sbLeft, sbTop + 25, sbLeft + 100, sbTop + 40);
  my_lcd.Set_Text_colour(WHITE);
  my_lcd.Print_String(buf, sbLeft, sbTop + 25);

  if(leaderboardNeedsUpdate) {
    for(int i=0; i<10; i++) {
      sprintf(buf, "%d: %lu", i+1, highScores[i]);
      // sterge vechiul text
      my_lcd.Set_Draw_color(BLACK);
      my_lcd.Fill_Rectangle(sbLeft, sbTop + 85 + i*15, sbLeft + 140, sbTop + 85 + i*15 + 15);
      my_lcd.Set_Text_colour(WHITE);
      my_lcd.Print_String(buf, sbLeft, sbTop + 85 + i*15);
    }
    leaderboardNeedsUpdate = false;
  }
}

void drawPiece() {
  for(int r=0; r<4; r++) {
    for(int c=0; c<4; c++) {
      if(currentPiece[r][c] != 0) {
        drawBlock(pieceX + c, pieceY + r, pieceColor);
      }
    }
  }
}
bool gameOver = false;

void drawGameOver() {
  my_lcd.Fill_Screen(BLACK);
  my_lcd.Set_Text_Size(4);
  my_lcd.Set_Text_colour(RED);
  my_lcd.Set_Text_Back_colour(BLACK);
  my_lcd.Print_String("GAME OVER", 60, 120);
  my_lcd.Set_Text_Size(2);
  my_lcd.Print_String("Press SELECT", 80, 180);
  
  while (digitalRead(BTN_SELECT) == HIGH) {
  playMelodyNonBlocking(&gameOverMelody);
  delay(10);  // așteaptă până când SELECT e apăsat (pin LOW)
}
}

void drawMenu() {
  my_lcd.Set_Text_Size(3);
  my_lcd.Set_Text_Back_colour(BLACK);
  my_lcd.Fill_Screen(BLACK);

  int yBase = 60;
  for(int i=0; i<MENU_ITEMS; i++) {
    if(i == menuSelection) {
      my_lcd.Set_Text_colour(ORANGE);
    } else {
      my_lcd.Set_Text_colour(WHITE);
    }
    my_lcd.Print_String(menuOptions[i], 60, yBase + i*50);
  }
}

// --- Functii logica joc ---

void spawnNewPiece() {
  int idx = random(NUM_PIECES);
  for(int r=0; r<4; r++) {
    for(int c=0; c<4; c++) {
      currentPiece[r][c] = pieces[idx][r][c];
    }
  }
  pieceColor = pieceColors[idx];
  pieceX = (GRID_WIDTH / 2) - 2;
  pieceY = 0;
}

bool checkCollision(int newX, int newY) {
  for(int r=0; r<4; r++) {
    for(int c=0; c<4; c++) {
      if(currentPiece[r][c] != 0) {
        int x = newX + c;
        int y = newY + r;
        if(x < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT) return true;
        if(y >= 0 && grid[y][x] != 0) return true;
      }
    }
  }
  return false;
}

void placePiece() {
  for(int r=0; r<4; r++) {
    for(int c=0; c<4; c++) {
      if(currentPiece[r][c] != 0) {
        int x = pieceX + c;
        int y = pieceY + r;
        if(y >= 0 && y < GRID_HEIGHT && x >= 0 && x < GRID_WIDTH) {
          grid[y][x] = pieceColor;
        }
      }
    }
  }
}

void clearLines() {
  int linesCleared = 0;
  for(int r = GRID_HEIGHT - 1; r >= 0; r--) {
    bool full = true;
    for(int c=0; c<GRID_WIDTH; c++) {
      if(grid[r][c] == 0) {
        full = false;
        break;
      }
    }
    if(full) {
      linesCleared++;
      for(int rr=r; rr>0; rr--) {
        for(int c=0; c<GRID_WIDTH; c++) {
          grid[rr][c] = grid[rr-1][c];
        }
      }
      for(int c=0; c<GRID_WIDTH; c++) {
        grid[0][c] = 0;
      }
      r++; // verificam iar linia actualizata
    }
  }
  if(linesCleared > 0) {
    score += linesCleared * 100;
    leaderboardNeedsUpdate = true;
  }
}

void rotatePiece() {
  uint8_t temp[4][4];
  for(int r=0; r<4; r++)
    for(int c=0; c<4; c++)
      temp[c][3 - r] = currentPiece[r][c];

  // verificam coliziuni dupa rotire
  for(int r=0; r<4; r++) {
    for(int c=0; c<4; c++) {
      if(temp[r][c] != 0) {
        int x = pieceX + c;
        int y = pieceY + r;
        if(x < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT) return; // invalida
        if(y >= 0 && grid[y][x] != 0) return;
      }
    }
  }
  // daca trece verificarea, aplicam rotirea
  for(int r=0; r<4; r++)
    for(int c=0; c<4; c++)
      currentPiece[r][c] = temp[r][c];
}

void updateHighScores() {
  // Inserare scor nou în highScores sortat descrescator
  for(int i=0; i<10; i++) {
    if(score > highScores[i]) {
      for(int j=9; j>i; j--) {
        highScores[j] = highScores[j-1];
      }
      highScores[i] = score;
      leaderboardNeedsUpdate = true;
      break;
    }
  }
}
void handleMenu() {
  if(digitalRead(BTN_DOWN) == LOW) {
    menuSelection = (menuSelection + 1) % MENU_ITEMS;
    drawMenu();
    delay(200);
  }
  if(digitalRead(BTN_ROTATE) == LOW) {
    menuSelection = (menuSelection - 1 + MENU_ITEMS) % MENU_ITEMS;
    drawMenu();
    delay(200);
  }
  if(digitalRead(BTN_SELECT) == LOW) {
    if(menuSelection == 0) {
      inMenu = false;
      my_lcd.Fill_Screen(BLACK);
      // Initialize game
      for(int r=0; r<GRID_HEIGHT; r++)
        for(int c=0; c<GRID_WIDTH; c++)
          grid[r][c] = 0;
      score = 0;
      spawnNewPiece();
      drawBorder();
      drawGridBlocks();
      drawScoreboardBackground();
      drawScoreboardStatic();
      updateScoreboard();
    } else if(menuSelection == 1) {
      my_lcd.Fill_Screen(BLACK);
      my_lcd.Set_Text_Size(3);
      my_lcd.Set_Text_colour(WHITE);
      my_lcd.Print_String("Goodbye!", 60, 120);
      while(1);
    }
    delay(200);
  }
}
// --- Setup și loop ---

void setup() {
  randomSeed(analogRead(0));
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_ROTATE, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(pinJoystick, INPUT);

  my_lcd.Init_LCD();
  my_lcd.Set_Rotation(3);
  my_lcd.Fill_Screen(BLACK);

  drawMenu();
}



void loop() {
  static unsigned long lastButtonPress = 0;
  static unsigned long now = 0;
  now = millis();

  if(inMenu) {
    handleMenu();
    return;
  }

  bool needRedraw = false;

  // Citire butoane cu debounce simplu
  if(now - lastButtonPress > 150) {  // delay minim intre apasari butoane
    if(digitalRead(BTN_LEFT) == LOW || analogRead(pinJoystick)>600) {
      if(!checkCollision(pieceX - 1, pieceY)) {
        pieceX--;
        needRedraw = true;
      }
      lastButtonPress = now;
    } 
    else if(digitalRead(BTN_RIGHT) == LOW || analogRead(pinJoystick)<400) {
      if(!checkCollision(pieceX + 1, pieceY)) {
        pieceX++;
        needRedraw = true;
      }
      lastButtonPress = now;
    } 
    else if(digitalRead(BTN_ROTATE) == LOW) {
      rotatePiece();
      needRedraw = true;
      lastButtonPress = now;
    } 
    else if(digitalRead(BTN_DOWN) == LOW) {
      // accelereaza caderea piesei cat timp e apasat butonul jos
      if(!checkCollision(pieceX, pieceY + 3)) {
        pieceY=pieceY+3;
        needRedraw = true;
      }
      lastButtonPress = now;
    }
  }

  // Caderea automata a piesei la interval regulat
  if(now - lastFall > fallInterval) {
    lastFall = now;
    if(!checkCollision(pieceX, pieceY + 1)) {
      pieceY++;
      needRedraw = true;
    } else {
      placePiece();
      clearLines();
      spawnNewPiece();
      needRedraw = true;

      // verifica coliziune la spawn - game over
if(checkCollision(pieceX, pieceY)) {
  updateHighScores();
  gameOver = true;
  drawGameOver();
  delay(20);
  inMenu = true;
  gameOver = false;
  drawMenu();
  return;
}

    }
  }
  playMelodyNonBlocking(&tetrisMelody); 
  // Redesenare doar daca e nevoie
  if(needRedraw) {
    drawGridBlocks();
    drawPiece();
    updateScoreboard();
  }
}

