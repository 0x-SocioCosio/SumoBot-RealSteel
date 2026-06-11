// ===== HC-SR04 =====
#define TRIG_PIN 2
#define ECHO_PIN 15

// ===== QTR =====
#define QTR_IZQ 13
#define QTR_DER 12

// ===== L298N =====
#define ENA 32
#define IN1 33
#define IN2 25

#define IN3 26
#define IN4 27
#define ENB 14

const int DISTANCIA_ATAQUE = 40; // cm

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(QTR_IZQ, INPUT);
  pinMode(QTR_DER, INPUT);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  detener();
  delay(5000); // tiempo para colocar el robot
}

void loop() {

  int bordeIzq = digitalRead(QTR_IZQ);
  int bordeDer = digitalRead(QTR_DER);

  // ===== EVITAR BORDE =====
  if (bordeIzq == 0 && bordeDer == 0) {
    retroceder();
    delay(400);

    girarDerecha();
    delay(500);
    return;
  }

  if (bordeIzq == 0) {
    retroceder();
    delay(300);

    girarDerecha();
    delay(350);
    return;
  }

  if (bordeDer == 0) {
    retroceder();
    delay(300);

    girarIzquierda();
    delay(350);
    return;
  }

  // ===== BUSCAR OPONENTE =====
  float distancia = medirDistancia();

  if (distancia > 0 && distancia < DISTANCIA_ATAQUE) {
    avanzar();
  } else {
    girarDerecha();
  }

  delay(20);
}

float medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long tiempo = pulseIn(ECHO_PIN, HIGH, 30000);

  if (tiempo == 0) return -1;

  return tiempo * 0.0343 / 2.0;
}

void avanzar() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void retroceder() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void girarDerecha() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void girarIzquierda() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void detener() {
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}