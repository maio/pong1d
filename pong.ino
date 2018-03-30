#include <Adafruit_NeoPixel.h>
#include "pitches.h"
#include "melodies.h"

#define PIXELS 15
#define LED_PIN 14
#define BUTTON1_PIN 15
#define BUTTON2_PIN 12
#define SPEAKER_PIN 4

Adafruit_NeoPixel strip = Adafruit_NeoPixel(
  PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

typedef enum {LEFT, RIGHT} side_type;

struct Button {
  int pin;
  bool down;    // button has been released just now
  bool up;      // button has been pressed just now
  bool isPressed; // button is being pressed
};

struct Player {
  uint32_t color;
  uint16_t lives;
  side_type side;
};

struct Ball {
  uint32_t color;
  float position;
  side_type direction;
  float speed; // pixels/s
};

struct Button button1;
struct Button button2;
struct Player p1;
struct Player p2;
struct Ball ball;

void setup() {
  button1 = (Button) {
    .pin = BUTTON1_PIN
  };
  
  button2 = (Button) {
    .pin = BUTTON2_PIN
  };
  
  playMelody1(SPEAKER_PIN);
  pinMode(button1.pin, INPUT);

  strip.begin();
  strip.setBrightness(5);
  strip.show();

  ball = (Ball) {
    .color = strip.Color(0, 255, 0),
    .position = 7,
    .direction = LEFT,
    .speed = 10
  };

  p1 = (Player) {
    .color = strip.Color(255, 0, 0),
    .lives = 3,
    .side = LEFT,
  };

  p2 = (Player) {
    .color = strip.Color(0, 0, 255),
    .lives = 3,
    .side = RIGHT
  };
}

// Fill the dots one after the other with a color
void setAllTo(uint32_t color) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
}

void renderPlayer(Player *p) {
  int maxRight = PIXELS - 1;
  
  if (p->side == RIGHT) {
    for (int i = 0; i < p->lives; i++) {
      strip.setPixelColor(maxRight - i, p->color);
    }
    return;
  }

  if (p->side == LEFT) {
    for (int i = 0; i < p->lives; i++) {
      strip.setPixelColor(i, p->color);
    }
    return;
  }
}

void renderBall(Ball *ball) {
  strip.setPixelColor(ball->position, ball->color);
}

// Refresh interval at which to set our game loop 
// To avoid having the game run at different speeds depending on hardware
const int refreshInterval = 16; // 60 FPS

// Used to calculate the delta between loops for a steady frame-rate
unsigned long lastRefreshTime = 0;

void processButtonInput(Button *button) {
  bool prevPressed = button->isPressed;

  int state = digitalRead(button->pin);
  bool newPressed = state == HIGH;
  
  if (prevPressed && !newPressed) { // just released
    button->up = true;
    button->down = false;
  } else if (!prevPressed && newPressed) { // just pressed
    button->up = false;
    button->down = true;    
  } else {
    button->up = false;
    button->down = false; 
  }

  button->isPressed = newPressed;
}

void processInput() {
  processButtonInput(&button1);
  processButtonInput(&button2);
}

void draw() {
  setAllTo(strip.Color(10, 10, 10));
  renderPlayer(&p1);
  renderPlayer(&p2);
  renderBall(&ball);
  strip.show();  
}

void updateBall(Ball *ball, unsigned int td) {
  float moveBy = ball->speed * (td / (float) 1000);

  if (button1.down && ball->direction == LEFT && (ball->position <= 3)) {
    ball->direction = RIGHT;
  }
  if (button2.down && ball->direction == RIGHT && (ball->position >= (PIXELS - 1 - 3))) {
    ball->direction = LEFT;
  }

  switch (ball->direction) {
    case LEFT:
      ball->position = ball->position - moveBy;
      if (ball->position < 0) {
        p1.lives = p1.lives - 1;
        ball->direction = RIGHT;
        ball->position = ball->position * -1;
      }
      break;
    case RIGHT:
      ball->position = ball->position + moveBy;
      if (ball->position > PIXELS - 1) {
        p2.lives = p2.lives - 1;
        ball->direction = LEFT;
        ball->position = (float) (PIXELS - 1) - ((float) (PIXELS - 1) - ball->position);
      }
      break;
  }
}

void update(unsigned int td) {
  updateBall(&ball, td);
}

void loop() {
  unsigned long now = millis();

  if (lastRefreshTime == 0) {
    lastRefreshTime = now;
    return;
  }

  unsigned int td = now - lastRefreshTime;

  if (td > refreshInterval) {    
    lastRefreshTime = now;

    processInput();
    update(td);
    draw();
  }
}

