# RealSteel

**Mini sumobot autónomo basado en Arduino Nano Every**

RealSteel es un robot de combate tipo *mini-sumo* desarrollado como proyecto educativo. Su objetivo es detectar a un oponente dentro de un dohyo, atacarlo y empujarlo fuera del ring, evitando al mismo tiempo salirse del borde blanco que delimita el área de combate.

El robot está construido sobre un Arduino Nano Every y utiliza una máquina de estados sencilla para alternar entre búsqueda, ataque y evasión de borde. La lógica está inspirada en el proyecto *Mini SumoBot Profesional RoboCup DGETI 2026*, adaptada a un hardware más accesible.

---

## Hardware

| Cantidad | Componente | Función |
|---|---|---|
| 1 | Arduino Nano Every | Microcontrolador principal |
| 1 | Driver TB6612FNG | Puente H para los dos motores |
| 2 | Motor TT 5V (doble eje tipo "I") | Tracción diferencial |
| 1 | HC-SR04 | Sensor ultrasónico frontal (detección de oponente) |
| 2 | QTR-1A | Sensores de línea analógicos (detección de borde) |

---

## Asignación de pines

### TB6612FNG (Puente H)

| Señal | Pin Nano Every |
|---|---|
| AIN1 | 4 |
| AIN2 | 2 |
| PWMA | 3 |
| BIN1 | 7 |
| BIN2 | 8 |
| PWMB | 5 |
| STBY | 6 |

### Sensores

| Sensor | Señal | Pin Nano Every |
|---|---|---|
| QTR-1A izquierdo | OUT | A0 |
| QTR-1A derecho | OUT | A1 |
| HC-SR04 | Trig | D9 |
| HC-SR04 | Echo | D10 |

Los QTR-1A están montados en las **esquinas delanteras** del chasis, separados al máximo posible para mejorar la detección de borde y permitir evasión direccional.

---

## Estrategia de funcionamiento

El firmware se organiza como una **máquina de estados** con cinco estados principales:

1. **INICIAL**  
   Configuración de pines y calibración automática de los QTR-1A. El robot debe encenderse sobre el dohyo (superficie negra) para obtener una calibración correcta.

2. **GIRO_INICIAL**  
   El robot realiza un giro de 540° (1.5 vueltas) para escanear el entorno.  
   Si detecta un oponente durante el giro, pasa directamente a ataque.

3. **BUSCANDO**  
   Gira lentamente mientras escanea con el HC-SR04.  
   Si detecta un objeto dentro del rango, cambia a ataque.  
   Si detecta borde, prioriza evasión.

4. **ATACANDO**  
   Avanza a máxima velocidad hacia el objetivo mientras lo detecte.  
   Si lo pierde, regresa a búsqueda.

5. **EVADIENDO_BORDE**  
   Retrocede brevemente y gira hacia el lado opuesto al sensor que detectó el borde.  
   Si ambos sensores detectan borde, ejecuta un giro de 180°.

---

### Prioridad de eventos

La detección de borde siempre tiene prioridad sobre la detección de oponente para evitar la expulsión del dohyo.

---

## Dohyo asumido

- Superficie: **negra**
- Borde: **blanco**
- Formato: **mini-sumo estándar**

---

## Parámetros configurables

- `UMBRAL_DETECCION_CM` → distancia de detección del HC-SR04 (default: 40 cm)
- `MS_POR_GRADO` → calibración de giro de motores
- `VEL_GIRO_INICIAL`
- `VEL_BUSCANDO`
- `VEL_ATACAR`
- `VEL_EVADIR`
- `DURACION_RETROCESO_MS`
- `DURACION_GIRO_EVASION_MS`

---

## Estado actual del desarrollo

🟢 En desarrollo / fase de pruebas

- [x] Máquina de estados implementada
- [x] Control de motores con TB6612FNG
- [x] Lectura HC-SR04 con timeout
- [x] Calibración automática de QTR-1A
- [x] Evasión direccional de borde
- [x] Logs por Serial (115200 baud)
- [ ] Calibración fina de `MS_POR_GRADO`
- [ ] Pruebas físicas en dohyo
- [ ] Ajuste de velocidades y tiempos
- [ ] Versión no bloqueante del HC-SR04 (opcional)
- [ ] Aleatorización de giro inicial (opcional)

---

## Notas importantes

- Proyecto educativo enfocado en **mini-sumo autónomo**
- No implementa regla de pausa de 5 segundos de competencia oficial
- Debe encenderse sobre el dohyo para calibración correcta

---

## Estructura del repositorio

```
SumoBot-RealSteel/
├── src/
│   └── SumoBot-RealSteel.ino    # Firmware principal
├── hardware/
│   └── stl/
│       └── SumoBot RealSteel.stl # Modelos 3D del chasis
├── LICENSE
└── README.md
```

---

## Autor

**Demian Sánchez**

Proyecto desarrollado con fines educativos y de aprendizaje en robótica autónoma, control de motores y máquinas de estados embebidas.
