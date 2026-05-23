# RealSteel

**Sumobot autónomo basado en Arduino Nano Every**

RealSteel es un robot de combate tipo *mini-sumo* desarrollado como proyecto educativo. Su objetivo es detectar a un oponente dentro de un dohyo, atacarlo y empujarlo fuera del ring, evitando al mismo tiempo salirse del borde blanco que delimita el área de combate.

El robot está construido sobre un Arduino Nano Every y utiliza una máquina de estados sencilla para alternar entre búsqueda, ataque y evasión de borde. La lógica está inspirada en el proyecto *SumoBot Profesional RoboCup DGETI 2026*, adaptada a un hardware más accesible.

---

## Hardware

| Cantidad | Componente | Función |
|---|---|---|
| 1 | Arduino Nano Every | Microcontrolador principal |
| 1 | Driver TB6612FNG | Puente H para los dos motores |
| 2 | Motor TT 5V (doble eje tipo "I") | Tracción diferencial |
| 1 | HC-SR04 | Sensor ultrasónico frontal (detección de oponente) |
| 2 | QTR-1A | Sensores de línea analógicos (detección de borde) |

### Asignación de pines

**TB6612FNG (Puente H)**

| Señal | Pin Nano Every |
|---|---|
| AIN1 | 4 |
| AIN2 | 2 |
| PWMA | 3 |
| BIN1 | 7 |
| BIN2 | 8 |
| PWMB | 5 |
| STBY | 6 |

**Sensores**

| Sensor | Señal | Pin Nano Every |
|---|---|---|
| QTR-1A izquierdo | OUT | A0 |
| QTR-1A derecho | OUT | A1 |
| HC-SR04 | Trig | D9 |
| HC-SR04 | Echo | D10 |

Los QTR-1A están montados en las **esquinas delanteras** del chasis (uno izquierdo, uno derecho), separados al ancho máximo posible. Esto permite no solo detectar el borde, sino también identificar *de qué lado* se está acercando para evadirlo en diagonal hacia el centro del dohyo.

---

## Estrategia de funcionamiento

El firmware se organiza como una **máquina de estados** con cinco estados principales:

1. **INICIAL** – Configuración de pines y calibración automática de los QTR-1A. El robot debe colocarse sobre el dohyo (negro) al encender; el umbral de borde se calcula a partir de esa lectura.
2. **GIRO_INICIAL** – Al arrancar, el robot ejecuta un giro de 540° (1.5 vueltas) en sentido horario para desorientar al oponente y escanear el dohyo. Si durante el giro detecta al oponente con el HC-SR04, corta el giro y pasa directamente al ataque.
3. **BUSCANDO** – Gira lentamente escaneando con el ultrasónico. Si detecta un objeto a menos de 40 cm pasa a ataque. Si algún QTR ve borde, pasa a evasión.
4. **ATACANDO** – Avanza a máxima velocidad mientras siga viendo al oponente. Si lo pierde, regresa a búsqueda.
5. **EVADIENDO_BORDE** – Retrocede brevemente y gira hacia el lado contrario al sensor que detectó el borde. Si ambos sensores ven blanco simultáneamente, ejecuta un giro de aproximadamente 180°.

**Prioridad de eventos:** la detección de borde siempre tiene prioridad sobre la detección de oponente, para evitar autoexpulsiones.

### Dohyo asumido

- Superficie: **negra**
- Borde: **blanco**
- Formato: mini-sumo estándar

### Parámetros configurables (en el código)

- `UMBRAL_DETECCION_CM` – Distancia máxima de detección del HC-SR04 (40 cm por defecto)
- `MS_POR_GRADO` – Calibración del tiempo de giro de los motores
- `VEL_GIRO_INICIAL`, `VEL_BUSCANDO`, `VEL_ATACAR`, `VEL_EVADIR` – Velocidades PWM (0–255)
- `DURACION_RETROCESO_MS`, `DURACION_GIRO_EVASION_MS` – Tiempos de la maniobra de evasión

---

## Estado actual del desarrollo

🟢 **En desarrollo / fase de pruebas**

- [x] Lógica base de máquina de estados implementada
- [x] Control de motores con TB6612FNG
- [x] Lectura de HC-SR04 con timeout
- [x] Calibración automática de los QTR-1A al arrancar
- [x] Maniobra de evasión direccional según sensor disparado
- [x] Logs detallados por Serial (115200 baud) para depuración
- [ ] Calibración fina de `MS_POR_GRADO` con motores reales
- [ ] Pruebas físicas sobre dohyo
- [ ] Ajuste de velocidades y tiempos según comportamiento real
- [ ] Versión no bloqueante del HC-SR04 (opcional)
- [ ] Aleatorización del sentido de giro inicial (opcional)

⚠️ **Notas importantes:**
- Este proyecto es educativo, **no respeta la pausa reglamentaria de 5 segundos** de competencia oficial.
- El robot debe encenderse sobre la superficie negra del dohyo para que la calibración automática de los QTR sea válida.

---

## Estructura del repositorio

```
RealSteel/
├── src/
│   └── SumoBot_NanoEvery.ino    # Firmware principal
├── docs/
│   ├── diagrama_conexiones.md    # Diagrama de cableado
│   ├── maquina_estados.md        # Documentación de la lógica
│   └── calibracion.md            # Guía de calibración
├── hardware/
│   ├── lista_componentes.md      # BOM (Bill of Materials)
│   ├── esquematico/              # Esquemáticos del circuito
│   └── chasis/                   # Diseños 3D / planos del chasis
├── README.md
└── LICENSE
```

---

## Autor

**Demian Sánchez**

Proyecto desarrollado con fines educativos y de aprendizaje en robótica autónoma, control de motores y máquinas de estados embebidas.
