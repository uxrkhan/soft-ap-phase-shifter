#define A 27
#define B 26
#define C 2
#define D 4
#define E 5
#define F 25
#define G 33
#define DP 15

short ss[] = {A, B, C, D, E, F, G, DP};

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 8; i++) {
    pinMode(ss[i], OUTPUT);
    digitalWrite(ss[i], LOW);
  }
  delay(5000);
}

void segment_disp(short num) {
  if (num < 0 || num > 9) {
    return;
  }
  bool truth[][8] = {{0,0,0,0,0,0,1,1},
                     {1,0,0,1,1,1,1,1},
                     {0,0,1,0,0,1,0,1},
                     {0,0,0,0,1,1,0,1},
                     {1,0,0,1,1,0,0,1},
                     {0,1,0,0,1,0,0,1},
                     {0,1,0,0,0,0,0,1},
                     {0,0,0,1,1,1,1,1},
                     {0,0,0,0,0,0,0,1},
                     {0,0,0,0,1,0,0,1}};
                     
  for (int i = 0; i < 8; i++) {
    digitalWrite(ss[i], truth[num][i]);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i <= 9; i++) {
    segment_disp(i);
    delay(500);  
  }
}
