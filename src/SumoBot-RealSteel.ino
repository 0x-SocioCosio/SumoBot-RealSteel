/* =====================================================================
 *  SumoBot - Arduino Nano Every
 *  Hardware:
 *    - Driver TB6612FNG
 *    - 2x QTR-1A (sensores de linea analogicos, montados en esquinas
 *      delanteras izquierda y derecha)
 *    - 1x HC-SR04 (sensor ultrasonico al frente)
 *    - 2x Motor TT 5V (motorreductor doble eje tipo "I")
 *
 *  Logica basada en "SumoBot Profesional RoboCup DGETI 2026", adaptada
 *  a este hardware mas sencillo. Proyecto de aprendizaje (no competencia
 *  oficial), por lo que NO se respeta la pausa reglamentaria de 5 s.
 *
 *  Dohyo asumido: NEGRO con borde BLANCO (mini-sumo estandar).
 * ===================================================================== */

// =================== Pines: TB6612FNG (NO MODIFICAR) ===================
// Conservados tal cual el protoboard del usuario.
const int AIN1 = 4;
const int AIN2 = 2;
const int PWMA = 3;

const int BIN1 = 7;
const int BIN2 = 8;
const int PWMB = 5;

const int STBY = 6;

// =================== Pines: Sensores ===================
// QTR-1A salida analogica (0-Vcc segun reflejo)
const int QTR_IZQ = A0;   // QTR-1A esquina delantera izquierda
const int QTR_DER = A1;   // QTR-1A esquina delantera derecha

// HC-SR04 (5V, va directo al Nano Every sin divisor)
const int HC_TRIG = 9;
const int HC_ECHO = 10;

// =================== Parametros ajustables ===================
// --- Velocidades (0-255) ---
const int VEL_GIRO_INICIAL = 220;
const int VEL_BUSCANDO     = 150;
const int VEL_ATACAR       = 255;
const int VEL_EVADIR       = 220;

// --- Tiempos ---
// CALIBRAR ESTE VALOR: cuantos ms tarda el robot en girar 1 grado a VEL_GIRO_INICIAL.
// Se mide poniendo el robot a girar en seco una vuelta y dividiendo el tiempo entre 360.
const unsigned long MS_POR_GRADO = 5;
const int           GRADOS_GIRO_INICIAL = 540;
const unsigned long DURACION_GIRO_INICIAL_MS =
                       (unsigned long)MS_POR_GRADO * GRADOS_GIRO_INICIAL;

const unsigned long DURACION_RETROCESO_MS = 350;  // Retroceso antes de re-girar
const unsigned long DURACION_GIRO_EVASION_MS = 200; // Giro tras retroceder

// --- Umbrales ---
// HC-SR04: 40 cm maximo (2*400 us/cm aprox -> trabajamos en cm)
const int UMBRAL_DETECCION_CM = 40;

// QTR-1A: lectura analogica con analogRead (0-1023).
// Sobre NEGRO el sensor refleja poco -> valor BAJO.
// Sobre BLANCO refleja mucho -> valor ALTO.
// La calibracion ajusta este umbral en el setup.
int umbralBorde = 500;  // Valor inicial; se recalcula en calibrar()

// --- Tiempos de pulso HC-SR04 ---
// pulseIn timeout en us. 25000 us = ~4.3 m -> mas que suficiente.
const unsigned long ECHO_TIMEOUT_US = 25000;

// =================== Maquina de estados ===================
enum Estado {
  INICIAL,
  GIRO_INICIAL,
  BUSCANDO,
  ATACANDO,
  EVADIENDO_BORDE
};
Estado estado = INICIAL;

// Sub-fase de evasion
enum FaseEvasion { EV_RETROCEDER, EV_GIRAR };
FaseEvasion faseEvasion = EV_RETROCEDER;

// -1 = ninguno, 0 = izq, 1 = der, 2 = ambos
int qtrConBorde = -1;

unsigned long tInicio = 0;

// =================== SETUP ===================
void setup() {
  Serial.begin(115200);
  // En Nano Every no hace falta esperar al Serial; arrancamos directo.

  // --- Motores ---
  pinMode(STBY, OUTPUT);
  pinMode(PWMA, OUTPUT); pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT); pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  digitalWrite(STBY, HIGH);
  detenerMotores();

  // --- Sensores ---
  pinMode(HC_TRIG, OUTPUT);
  pinMode(HC_ECHO, INPUT);
  digitalWrite(HC_TRIG, LOW);
  // Los QTR-1A son analogicos, no requieren pinMode.

  Serial.println(F("=== SUMOBOT Nano Every ==="));
  Serial.println(F("Modo: NEGRO con borde BLANCO"));
  Serial.print(F("Umbral de deteccion HC-SR04: "));
  Serial.print(UMBRAL_DETECCION_CM); Serial.println(F(" cm"));

  // --- Calibracion rapida de los QTR-1A ---
  calibrarQTR();

  Serial.println(F(">>> COMBATE: giro inicial de 540 grados"));
  iniciarGiro(true);  // horario
  estado = GIRO_INICIAL;
  tInicio = millis();
}

// =================== LOOP ===================
void loop() {

  switch (estado) {

    case GIRO_INICIAL: {
      // Durante el giro tambien escaneamos: si detectamos oponente al frente,
      // cortamos el giro y atacamos directamente.
      int dCm = medirDistanciaCm();
      if (dCm > 0 && dCm < UMBRAL_DETECCION_CM) {
        Serial.print(F(">>> Oponente detectado durante giro a "));
        Serial.print(dCm); Serial.println(F(" cm. ATACAR."));
        estado = ATACANDO;
        break;
      }
      if (millis() - tInicio >= DURACION_GIRO_INICIAL_MS) {
        Serial.println(F(">>> Giro inicial completado. BUSCANDO."));
        estado = BUSCANDO;
      }
      break;
    }

    case BUSCANDO: {
      // Prioridad 1: borde
      if (detectarBorde()) { entrarEvasion(); break; }

      // Prioridad 2: oponente
      int dCm = medirDistanciaCm();
      if (dCm > 0 && dCm < UMBRAL_DETECCION_CM) {
        Serial.print(F(">>> Oponente a "));
        Serial.print(dCm); Serial.println(F(" cm. ATACAR."));
        estado = ATACANDO;
        break;
      }

      // Si no hay nada: girar lento (horario) para escanear
      motorIzq(+1, VEL_BUSCANDO);
      motorDer(-1, VEL_BUSCANDO);
      break;
    }

    case ATACANDO: {
      // Prioridad 1: borde (siempre)
      if (detectarBorde()) { entrarEvasion(); break; }

      // Prioridad 2: seguir viendo al oponente
      int dCm = medirDistanciaCm();
      if (dCm > 0 && dCm < UMBRAL_DETECCION_CM) {
        // Con un solo HC-SR04 al frente, atacar = avanzar a maxima velocidad
        avanzar(VEL_ATACAR);
      } else {
        Serial.println(F(">>> Oponente perdido. Volver a BUSCAR."));
        estado = BUSCANDO;
      }
      break;
    }

    case EVADIENDO_BORDE: {
      // Sub-fase 1: retroceder o avanzar segun de que lado se detecto el borde
      if (faseEvasion == EV_RETROCEDER) {
        // Como ambos QTR estan al frente, SIEMPRE retrocedemos para alejarnos
        // del borde, y luego giramos hacia el lado contrario al sensor que
        // disparo (si solo disparo uno).
        retroceder(VEL_EVADIR);

        if (millis() - tInicio >= DURACION_RETROCESO_MS) {
          // Pasar a sub-fase de giro
          faseEvasion = EV_GIRAR;
          tInicio = millis();

          // Decidir hacia donde girar
          if (qtrConBorde == 0) {
            // Borde detectado a la IZQUIERDA -> girar a la DERECHA
            Serial.println(F(">>> Evasion: giro DERECHA"));
            motorIzq(+1, VEL_EVADIR);
            motorDer(-1, VEL_EVADIR);
          } else if (qtrConBorde == 1) {
            // Borde detectado a la DERECHA -> girar a la IZQUIERDA
            Serial.println(F(">>> Evasion: giro IZQUIERDA"));
            motorIzq(-1, VEL_EVADIR);
            motorDer(+1, VEL_EVADIR);
          } else {
            // Ambos sensores detectaron borde -> giro 180 a la derecha
            Serial.println(F(">>> Evasion: giro 180"));
            motorIzq(+1, VEL_EVADIR);
            motorDer(-1, VEL_EVADIR);
          }
        }
      } else { // EV_GIRAR
        unsigned long duracionGiro = (qtrConBorde == 2)
                                       ? DURACION_GIRO_EVASION_MS * 2  // 180
                                       : DURACION_GIRO_EVASION_MS;
        if (millis() - tInicio >= duracionGiro) {
          detenerMotores();
          Serial.println(F(">>> Evasion terminada. BUSCANDO."));
          estado = BUSCANDO;
        }
      }
      break;
    }

    default: break;
  }
}

// =================== Helpers de estado ===================
void entrarEvasion() {
  Serial.print(F(">>> BORDE detectado en: "));
  if (qtrConBorde == 0)      Serial.println(F("IZQUIERDA"));
  else if (qtrConBorde == 1) Serial.println(F("DERECHA"));
  else                       Serial.println(F("AMBOS"));
  estado = EVADIENDO_BORDE;
  faseEvasion = EV_RETROCEDER;
  tInicio = millis();
}

// =================== Calibracion de QTR-1A ===================
// Toma una lectura inicial sobre el dohyo (negro) y calcula un umbral
// razonable. El usuario debe colocar el robot SOBRE el dohyo (sobre negro)
// al encender. El umbral se fija a la mitad entre la lectura inicial
// (negro) y el maximo teorico (blanco ~ 1023).
void calibrarQTR() {
  Serial.println(F(">>> Calibrando QTR (mantener el robot sobre NEGRO)..."));
  delay(500);  // Pequeno respiro

  long sumaIzq = 0, sumaDer = 0;
  const int N = 50;
  for (int i = 0; i < N; i++) {
    sumaIzq += analogRead(QTR_IZQ);
    sumaDer += analogRead(QTR_DER);
    delay(5);
  }
  int promIzq = sumaIzq / N;
  int promDer = sumaDer / N;
  int promNegro = (promIzq + promDer) / 2;

  // Umbral = punto medio entre el negro leido y el blanco teorico (1023).
  // Si negro = 200 y blanco ~ 900, umbral ~ 550.
  umbralBorde = (promNegro + 1023) / 2;

  Serial.print(F("   QTR IZQ negro = ")); Serial.println(promIzq);
  Serial.print(F("   QTR DER negro = ")); Serial.println(promDer);
  Serial.print(F("   Umbral borde  = ")); Serial.println(umbralBorde);
}

// =================== HC-SR04 ===================
// Devuelve distancia en cm. Si no hay eco (timeout) devuelve -1.
int medirDistanciaCm() {
  digitalWrite(HC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(HC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(HC_TRIG, LOW);

  unsigned long duracion = pulseIn(HC_ECHO, HIGH, ECHO_TIMEOUT_US);
  if (duracion == 0) return -1;  // timeout

  // Velocidad del sonido ~ 343 m/s -> 58.2 us por cm (ida y vuelta)
  int cm = (int)(duracion / 58);
  return cm;
}

// =================== QTR-1A: deteccion de borde ===================
// Sobre dohyo NEGRO los QTR leen valores BAJOS.
// Sobre borde BLANCO los QTR leen valores ALTOS.
// Borde = lectura > umbralBorde.
bool detectarBorde() {
  int vIzq = analogRead(QTR_IZQ);
  int vDer = analogRead(QTR_DER);

  bool bordeIzq = (vIzq > umbralBorde);
  bool bordeDer = (vDer > umbralBorde);

  if (bordeIzq && bordeDer) { qtrConBorde = 2; return true; }
  if (bordeIzq)             { qtrConBorde = 0; return true; }
  if (bordeDer)             { qtrConBorde = 1; return true; }

  qtrConBorde = -1;
  return false;
}

// =================== Motores ===================
// motorIzq: motor A. dir = +1 adelante, -1 atras, 0 parado.
void motorIzq(int dir, int v) {
  if (dir > 0)      { digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); }
  else if (dir < 0) { digitalWrite(AIN1, LOW);  digitalWrite(AIN2, HIGH); }
  else              { digitalWrite(AIN1, LOW);  digitalWrite(AIN2, LOW);  v = 0; }
  analogWrite(PWMA, v);
}

void motorDer(int dir, int v) {
  if (dir > 0)      { digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); }
  else if (dir < 0) { digitalWrite(BIN1, LOW);  digitalWrite(BIN2, HIGH); }
  else              { digitalWrite(BIN1, LOW);  digitalWrite(BIN2, LOW);  v = 0; }
  analogWrite(PWMB, v);
}

void avanzar(int v)    { motorIzq(+1, v); motorDer(+1, v); }
void retroceder(int v) { motorIzq(-1, v); motorDer(-1, v); }

void iniciarGiro(bool horario) {
  if (horario) { motorIzq(+1, VEL_GIRO_INICIAL); motorDer(-1, VEL_GIRO_INICIAL); }
  else         { motorIzq(-1, VEL_GIRO_INICIAL); motorDer(+1, VEL_GIRO_INICIAL); }
}

void detenerMotores() {
  // "Short brake" del TB6612FNG: ambos IN en HIGH
  digitalWrite(AIN1, HIGH); digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, HIGH); digitalWrite(BIN2, HIGH);
  analogWrite(PWMA, 0); analogWrite(PWMB, 0);
}
