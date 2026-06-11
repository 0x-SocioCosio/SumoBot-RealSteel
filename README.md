# RealSteel

**Mini sumobot autónomo basado en ESP32 DevKit**

RealSteel es un robot de combate tipo *mini-sumo* desarrollado como proyecto educativo. Su objetivo es detectar a un oponente dentro de un dohyo, atacarlo y empujarlo fuera del ring, evitando al mismo tiempo salirse del borde blanco que delimita el área de combate.

El robot está construido sobre un **ESP32 DevKit** y utiliza una estrategia reactiva basada en sensores para buscar oponentes, atacar y evitar abandonar el área de combate.

---

## Hardware

| Cantidad | Componente                       | Función                                                |
| -------- | -------------------------------- | ------------------------------------------------------ |
| 1        | ESP32 DevKit                     | Microcontrolador principal                             |
| 1        | L298N                            | Driver de motores (puente H dual)                      |
| 2        | Motor TT 5V (doble eje tipo "I") | Tracción diferencial                                   |
| 1        | HC-SR04                          | Sensor ultrasónico frontal para detección de oponentes |
| 2        | QTR                              | Sensores de línea para detección del borde del dohyo   |

---

## Asignación de pines

### HC-SR04

| Señal | Pin ESP32 |
| ----- | --------- |
| Trig  | GPIO 2    |
| Echo  | GPIO 15   |

### Sensores QTR

| Sensor        | Pin ESP32 |
| ------------- | --------- |
| QTR izquierdo | GPIO 13   |
| QTR derecho   | GPIO 12   |

### L298N

| Señal | Pin ESP32 |
| ----- | --------- |
| ENA   | GPIO 32   |
| IN1   | GPIO 33   |
| IN2   | GPIO 25   |
| IN3   | GPIO 26   |
| IN4   | GPIO 27   |
| ENB   | GPIO 14   |

Los sensores QTR están montados en las esquinas delanteras del robot para detectar el borde del dohyo y permitir maniobras de evasión direccionales.

---

## Estrategia de funcionamiento

### 1. Buscar oponente

Cuando el sensor HC-SR04 no detecta ningún objeto dentro de la distancia configurada, el robot gira sobre su propio eje para localizar un adversario.

### 2. Atacar

Si el HC-SR04 detecta un objeto dentro de la distancia de ataque, el robot avanza a máxima velocidad para intentar expulsarlo del dohyo.

### 3. Evitar salir del dohyo

Los sensores QTR monitorean continuamente el borde blanco del área de combate.

* Si el sensor izquierdo detecta la línea blanca, el robot retrocede y gira hacia la derecha.
* Si el sensor derecho detecta la línea blanca, el robot retrocede y gira hacia la izquierda.
* Si ambos sensores detectan la línea blanca, el robot retrocede y realiza un giro más amplio para regresar al centro del dohyo.

La detección de borde tiene prioridad sobre cualquier otra acción.

---

## Prioridad de acciones

1. Detección de línea blanca (QTR)
2. Detección de oponente (HC-SR04)
3. Búsqueda de oponente

Este orden evita que el robot abandone el dohyo mientras persigue un objetivo.

---

## Dohyo asumido

* Superficie: **negra**
* Borde: **blanco**
* Formato: **mini-sumo estándar**

---

## Parámetros configurables

* `DISTANCIA_ATAQUE` → distancia máxima para iniciar el ataque (40 cm por defecto)

Los tiempos de evasión y búsqueda pueden modificarse directamente dentro del código fuente según las necesidades de calibración del robot.

---

## Estado actual del desarrollo

🟢 En desarrollo / fase de pruebas

* [x] Migración a ESP32 DevKit
* [x] Control de motores mediante L298N
* [x] Lectura del sensor HC-SR04
* [x] Detección de borde mediante sensores QTR
* [x] Búsqueda automática de oponentes
* [x] Ataque automático al detectar objetivo
* [x] Evasión de borde mediante sensores QTR
* [x] Comunicación Serial para depuración
* [ ] Ajuste fino de tiempos de evasión
* [ ] Optimización de estrategia de combate
* [ ] Pruebas físicas extensivas en dohyo

---

## Notas importantes

* Proyecto educativo enfocado en robótica autónoma tipo mini-sumo.
* El HC-SR04 se utiliza como sensor principal para localizar oponentes.
* Los sensores QTR funcionan en modo digital:

  * `1` = superficie negra.
  * `0` = línea blanca.
* El robot espera 5 segundos después del encendido antes de iniciar el combate.
* La evasión de borde tiene prioridad sobre cualquier acción de ataque.

---

## Estructura del repositorio

```text
RealSteel/
├── src/
│   └── RealSteel.ino
├── hardware/
│   └── stl/
│       └── RealSteel.stl
├── LICENSE
└── README.md
```

---

## Autor

**Demian Sánchez**

Proyecto desarrollado con fines educativos y de aprendizaje en robótica autónoma, sensores, control de motores y sistemas embebidos.

