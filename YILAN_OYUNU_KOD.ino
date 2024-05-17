#include <LedControl.h>

// Yılanı bir yapı olarak tanımla
typedef struct Snake Snake;
struct Snake {
  int head[2];     // yılanın başının (satır, sütun) koordinatları
  int body[40][2]; // (satır, sütun) koordinatlarını içeren bir dizi
  int len;         // yılanın uzunluğu
  int dir[2];      // yılanı hareket ettirecek yön

// Elmayı bir yapı olarak tanımla
typedef struct Apple Apple;
struct Apple {
  int rPos; // elmanın satır indeksi
  int cPos; // elmanın sütun indeksi

// MAX72XX led Matrix
const int DIN = 12;
const int CS = 11;
const int CLK = 10;
LedControl lc = LedControl(DIN, CLK, CS, 1);

const int varXPin = A0; // Joystick'ten X Değeri
const int varYPin = A1; // Joystick'ten Y Değeri
const int buzzerPin = 8; // Buzzer pin

byte pic[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // LED Matrisinin 8 satırı

Snake snake = {{1, 5}, {{0, 5}, {1, 5}}, 2, {1, 0}}; // Bir yılan nesnesi başlat
Apple apple = {(int)random(0, 8), (int)random(0, 8)}; // Bir elma nesnesi başlat

// Oyun Zamanını İşlemek için Değişkenler
float oldTime = 0;
float timer = 0;
float updateRate = 3;

int i, j; // Sayıcılar
void setup() {
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
  */
  lc.shutdown(0, false);
  // Set the brightness to a medium values
  lc.setIntensity(0, 8);
  // and clear the display
  lc.clearDisplay(0);

  // Joystick Pinlerini GİRİŞ olarak ayarla
  pinMode(varXPin, INPUT);
  pinMode(varYPin, INPUT);
  // Set Buzzer Pin as OUTPUT
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
   // Delta zamanını hesapla
  float deltaTime = calculateDeltaTime();
  timer += deltaTime;

  // Girişleri Kontrol Et
  int xVal = analogRead(varXPin);
  int yVal = analogRead(varYPin);

  if (xVal < 100 && snake.dir[1] == 0) {
    snake.dir[0] = 0;
    snake.dir[1] = -1;
  } else if (xVal > 920 && snake.dir[1] == 0) {
    snake.dir[0] = 0;
    snake.dir[1] = 1;
  } else if (yVal < 100 && snake.dir[0] == 0) {
    snake.dir[0] = -1;
    snake.dir[1] = 0;
  } else if (yVal > 920 && snake.dir[0] == 0) {
    snake.dir[0] = 1;
    snake.dir[1] = 0;
  }

// Update
  if (timer > 1000 / updateRate) {
    timer = 0;
    Update();
  }

  // Render
  Render();
}

float calculateDeltaTime() {
  float currentTime = millis();
  float dt = currentTime - oldTime;
  oldTime = currentTime;
  return dt;
}

void reset() {
  for (int j = 0; j < 8; j++) {
    pic[j] = 0;
  }
}

void Update() {
  reset(); // Reset (Clear) the 8x8 LED matrix

  int newHead[2] = {snake.head[0] + snake.dir[0], snake.head[1] + snake.dir[1]};

  // Handle Borders
  if (newHead[0] == 8) {
    newHead[0] = 0;
  } else if (newHead[0] == -1) {
    newHead[0] = 7;
  } else if (newHead[1] == 8) {
    newHead[1] = 0;
  } else if (newHead[1] == -1) {
    newHead[1] = 7;
  }

  // Yılanın Kendine Çarpıp Çarpmadığını Kontrol Et
  for (j = 0; j < snake.len; j++) {
    if (snake.body[j][0] == newHead[0] && snake.body[j][1] == newHead[1]) {
      // Pause the game for 1 sec then Reset it
      buzzerSound(); // Play sound
      delay(1000);
      snake = {{1, 5}, {{0, 5}, {1, 5}}, 2, {1, 0}}; // Yılan nesnesini yeniden başlat
      apple = {(int)random(0, 8), (int)random(0, 8)}; // Reinitialize an apple object
      return;
    }
  }

   // Yılanın elmayı yediğini kontrol et
  if (newHead[0] == apple.rPos && newHead[1] == apple.cPos) {
    snake.len = snake.len + 1;
    apple.rPos = (int)random(0, 8);
    apple.cPos = (int)random(0, 8);
  } else {
    removeFirst(); // Shifting the array to the left
  }

  snake.body[snake.len - 1][0] = newHead[0];
  snake.body[snake.len - 1][1] = newHead[1];

  snake.head[0] = newHead[0];
  snake.head[1] = newHead[1];

  // Update the pic Array to Display (snake and apple)
  for (j = 0; j < snake.len; j++) {
    pic[snake.body[j][0]] |= 128 >> snake.body[j][1];
  }
  pic[apple.rPos] |= 128 >> apple.cPos;
}

void Render() {
  for (i = 0; i < 8; i++) {
    lc.setRow(0, i, pic[i]);
  }
}

void removeFirst() {
  for (j = 1; j < snake.len; j++) {
    snake.body[j - 1][0] = snake.body[j][0];
    snake.body[j - 1][1] = snake.body[j][1];
  }
}

void buzzerSound() {
  tone(buzzerPin, 1000); // Buzzer'da 1000Hz frekansında bir ses çal
  delay(500);           // 1 saniye bekle
  noTone(buzzerPin);     // Buzzer'dan sesi kapat
}
