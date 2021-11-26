#include <LiquidCrystal_I2C.h>
#include <Wire.h>

//오징어 게임 bgm
int buzzer = 6;
int pin[]={2};
int melody1[]={440,554,392,349,294,247};

//게임 뮤직
int melody[]={262,294,330,349,392,440,494,523};

#define TIME 120


#define PIN_BUTTON 2
#define PIN_AUTOPLAY 1
#define PIN_READWRITE 10
#define PIN_CONTRAST 12

#define SPRITE_RUN1 1
#define SPRITE_RUN2 2
#define SPRITE_JUMP 3
#define SPRITE_JUMP_UPPER '.'         // Use the '.' character for the head
#define SPRITE_JUMP_LOWER 4
#define SPRITE_TERRAIN_EMPTY ' '      // User the ' ' character
#define SPRITE_TERRAIN_SOLID 5
#define SPRITE_TERRAIN_SOLID_RIGHT 6
#define SPRITE_TERRAIN_SOLID_LEFT 7

#define HERO_HORIZONTAL_POSITION 1    // Horizontal position of hero on screen

#define TERRAIN_WIDTH 16
#define TERRAIN_EMPTY 0
#define TERRAIN_LOWER_BLOCK 1
#define TERRAIN_UPPER_BLOCK 2

#define HERO_POSITION_OFF 0          // Hero is invisible
#define HERO_POSITION_RUN_LOWER_1 1  // Hero is running on lower row (pose 1)
#define HERO_POSITION_RUN_LOWER_2 2  //                              (pose 2)

#define HERO_POSITION_JUMP_1 3       // Starting a jump
#define HERO_POSITION_JUMP_2 4       // Half-way up
#define HERO_POSITION_JUMP_3 5       // Jump is on upper row
#define HERO_POSITION_JUMP_4 6       // Jump is on upper row
#define HERO_POSITION_JUMP_5 7       // Jump is on upper row
#define HERO_POSITION_JUMP_6 8       // Jump is on upper row
#define HERO_POSITION_JUMP_7 9       // Half-way down
#define HERO_POSITION_JUMP_8 10      // About to land

#define HERO_POSITION_RUN_UPPER_1 11 // Hero is running on upper row (pose 1)
#define HERO_POSITION_RUN_UPPER_2 12 //     

LiquidCrystal_I2C lcd(0x27,16,2);

static bool playing = true;
static char terrainUpper[TERRAIN_WIDTH+1];
static char terrainLower[TERRAIN_WIDTH+1];
static bool buttonPushed = false;
static bool restarted = false;
static int score = 0;
static int preScore = -1;

void bgMusic(){
  for( int i=0;i<2;i++){
    for(int i=0;i<2;i++){
      tone(buzzer,melody1[0],280);
      delay(300);
    } 
    tone(buzzer,melody1[0],280);
    delay(500);  
   }
      //노래 중반 시작
   tone(buzzer,melody1[1],280);
   delay(300);
   for(int i=0;i<2;i++){
    tone(buzzer,melody1[0],280);
    delay(300);
   } 
   tone(buzzer,melody1[2],280);
   delay(300);
   tone(buzzer,melody1[3],280);
   delay(300);
   tone(buzzer,melody1[2],280);
   delay(300);
   tone(buzzer,melody1[0],280);
   delay(500);
  
}

void gameOverMusic(){
   tone(buzzer,melody1[0],280);
   delay(300);
   tone(buzzer,melody1[2],280);
   delay(300);
   tone(buzzer,melody1[3],280);
   delay(300);
   tone(buzzer,melody1[2],280);
   delay(300);
   tone(buzzer,melody1[3],280);
   delay(300);
   tone(buzzer,melody1[4],280);
   delay(300);
   tone(buzzer,melody1[4],280);
   delay(500);
}

void initializeGraphics() {
  static byte graphics[] = {
    // Run position 1
    B01010,
    B11111,
    B10101,
    B10101,
    B11111,
    B11111,
    B01010,
    B01010,
    // Run position 2
    B01010,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B00100,
    B00100,  
    // Jump
    B01100,
    B01100,
    B00000,
    B11110,
    B01101,
    B11111,
    B10000,
    B00000,
    // Jump lower
    B11110,
    B01101,
    B11111,
    B10000,
    B00000,
    B00000,
    B00000,
    B00000,
    // Ground
    B00000,
    B00000,
    B01110,
    B11111,
    B01110,
    B00100,
    B00000,
    B00000, 
    // Ground right
    B00000, 
    B00000, 
    B00011, 
    B00111, 
    B00011, 
    B00001, 
    B00000, 
    B00000,
    // Ground left

    B00000,
    B00000,
    B11000,
    B11100,
    B11000,
    B10000,
    B00000,
    B00000,
  };
  int i;
  // Skip using character 0, this allows lcd.print() to be used to
  // quickly draw multiple characters
  for (i = 0; i < 7; ++i) {
    lcd.createChar(i + 1, &graphics[i * 8]);
  }
  for (i = 0; i < TERRAIN_WIDTH; ++i) {
    terrainUpper[i] = SPRITE_TERRAIN_EMPTY;
    terrainLower[i] = SPRITE_TERRAIN_EMPTY;
  }
}
void advanceTerrain(char* terrain, byte newTerrain) {
  for (int i = 0; i < TERRAIN_WIDTH; ++i) {
    char current = terrain[i];
    char next = (i == TERRAIN_WIDTH - 1) ? newTerrain : terrain[i + 1];
    terrain[i] = (next == SPRITE_TERRAIN_SOLID) ? SPRITE_TERRAIN_SOLID : SPRITE_TERRAIN_EMPTY;
  }
}

void buttonPush(){
  buttonPushed = true;
}

void restart(){
  restarted = true;
}

bool drawHero(byte position, char* terrainUpper, char* terrainLower, unsigned int distance){
  bool collide = false;
  char upperSave = terrainUpper[HERO_HORIZONTAL_POSITION];
  char lowerSave = terrainLower[HERO_HORIZONTAL_POSITION];
  byte upper, lower;
  switch (position) {
    case HERO_POSITION_OFF: //0
      upper = lower = SPRITE_TERRAIN_EMPTY;
      break;
    case HERO_POSITION_RUN_LOWER_1: //아래에서 뛸 때의 pose1 
      upper = SPRITE_TERRAIN_EMPTY;
      lower = SPRITE_RUN1;
      break;
    case HERO_POSITION_RUN_LOWER_2: //아래에서 뛸 때의 pose2
      upper = SPRITE_TERRAIN_EMPTY;
      lower = SPRITE_RUN2;
      break;
    case HERO_POSITION_RUN_UPPER_1: //위에서 뛸 때의 pose1
      upper = SPRITE_RUN1;
      lower = SPRITE_TERRAIN_EMPTY;
      break;
    case HERO_POSITION_RUN_UPPER_2: //위에서 뛸 때의 pose2
      upper = SPRITE_RUN2;
      lower = SPRITE_TERRAIN_EMPTY;
      break;
   }
  if (upper != ' ') {
  terrainUpper[HERO_HORIZONTAL_POSITION] = upper;
  collide = (upperSave == SPRITE_TERRAIN_EMPTY) ? false : true;
  }
  if (lower != ' ') {
    terrainLower[HERO_HORIZONTAL_POSITION] = lower;
    collide |= (lowerSave == SPRITE_TERRAIN_EMPTY) ? false : true;
  }

  byte digits_score = (score > 9999) ? 5 : (score > 999) ? 4 : (score > 99) ? 3 : (score > 9) ? 2 : 1;
  byte digits_d = (distance > 9999) ? 5 : (distance > 999) ? 4 : (distance > 99) ? 3 : (distance > 9) ? 2 : 1;

  if(collide){
    score++;
    for(int i=6;i<8;i++){
    tone(buzzer,melody[i],400);
    delay(30);
   }
    collide = false;
  }
  terrainUpper[TERRAIN_WIDTH] = '\0';
  terrainLower[TERRAIN_WIDTH] = '\0';
  lcd.setCursor(0,0);
  lcd.print(terrainUpper);
  lcd.setCursor(0,1);
  lcd.print(terrainLower);
  lcd.setCursor(16-digits_score,0);
  lcd.print(score);
  lcd.setCursor(16-digits_d,1);
  lcd.print(distance);
  return true;
}
void setup() {
  pinMode(PIN_READWRITE, OUTPUT);
  digitalWrite(PIN_READWRITE, LOW);
  pinMode(PIN_CONTRAST, OUTPUT);
  digitalWrite(PIN_CONTRAST, LOW);
  pinMode(PIN_BUTTON, INPUT);
  digitalWrite(PIN_BUTTON, HIGH);
  pinMode(PIN_AUTOPLAY, OUTPUT);
  digitalWrite(PIN_AUTOPLAY, HIGH);

  //음악
  pinMode(buzzer,OUTPUT);
  
  Serial.begin(9600);
  initializeGraphics();
  attachInterrupt(0, buttonPush, CHANGE);
  //CHANGE 택트 스위치를 클릭하거나, 땔 때도 함수를 한 번 출력 시킴, 그 외에는 출력 안됨.
  lcd.begin();
}


void loop() {
  static byte heroPos = HERO_POSITION_RUN_LOWER_1;
  static byte newTerrainType = TERRAIN_EMPTY;
  static byte newTerrainDuration = 1;
  static bool playing = false;
  static bool blink = false;
  static unsigned int distance = 0;
  static int count = TIME;
  static int t = 50;
  static int idx = 0;
  if(idx == 0){bgMusic(); idx = 1;}
  if(idx == 2){gameOverMusic(); idx = 1;}
  t++;
  if (!playing) { 
    lcd.clear();
    if(t < 50){
      lcd.setCursor(0,1);
      lcd.print("Ur score : " + String(score));
      buttonPushed = false;
    }else{
      if(preScore >= 0){
        lcd.setCursor(0,1);
        lcd.print("Pre score :" + String(preScore));
      }
      if (blink) {
        lcd.setCursor(0, 0);
        lcd.print("Press Start");
      }
      delay(250);
      blink = !blink;

      if (buttonPushed) {
        initializeGraphics();
        heroPos = HERO_POSITION_RUN_LOWER_1;
        playing = true;
        buttonPushed = false;
        score = 0;
        count = TIME;
        t = 0;
      }
    }
  }else{
    advanceTerrain(terrainLower, newTerrainType == TERRAIN_LOWER_BLOCK ? SPRITE_TERRAIN_SOLID : SPRITE_TERRAIN_EMPTY);
    advanceTerrain(terrainUpper, newTerrainType == TERRAIN_UPPER_BLOCK ? SPRITE_TERRAIN_SOLID : SPRITE_TERRAIN_EMPTY);
    if (--newTerrainDuration == 0) {
      if (newTerrainType == TERRAIN_EMPTY) {
        newTerrainType = (random(2) == 0) ? TERRAIN_UPPER_BLOCK : TERRAIN_LOWER_BLOCK; // 2/3은 아래 블록
        newTerrainDuration = 1 + random(4); //최소 블럭 2~11개까지 연속으로 나올 수 있음.
        
      } else {
        newTerrainType = TERRAIN_EMPTY; //연속 블럭이 다 출력된 후, 공백의 시간. 이게 없다면 hero가 쉽게 죽음.
        newTerrainDuration = 1 + random(4);  //공백 범위
      }
    }
  
    //점프
    if(buttonPushed){
      if(heroPos <= HERO_POSITION_RUN_LOWER_2){heroPos = HERO_POSITION_RUN_UPPER_1;}
      else{heroPos = HERO_POSITION_RUN_LOWER_1;}
      buttonPushed = false;
      tone(buzzer,melody[0],400);
    }
  
    //HERO 다리, 눈 움직임 heroPos에 대입
    if(heroPos == HERO_POSITION_RUN_LOWER_1){
      heroPos = HERO_POSITION_RUN_LOWER_2;
    }else if(heroPos == HERO_POSITION_RUN_LOWER_2){
      heroPos = HERO_POSITION_RUN_LOWER_1;
    }
    if(heroPos == HERO_POSITION_RUN_UPPER_1){
      heroPos = HERO_POSITION_RUN_UPPER_2;
    }else if(heroPos == HERO_POSITION_RUN_UPPER_2){
      heroPos = HERO_POSITION_RUN_UPPER_1;
    }
    count -= 2;
    distance = count/10;
    if(--distance == 0){
      playing = false;
      t = 0;
      idx = 2;
    }
    drawHero(heroPos, terrainUpper, terrainLower, distance);
    preScore = score;
   }
  
  digitalWrite(PIN_AUTOPLAY, terrainLower[HERO_HORIZONTAL_POSITION + 2] == SPRITE_TERRAIN_EMPTY ? HIGH : LOW);
  delay(100);

}
