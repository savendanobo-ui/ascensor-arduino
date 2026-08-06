int botones[4] = {2, 3, 4, 5};
int LEDS[2] = {10, 11};
int subirObajar[2] = {8, 9};
int IR[4] = {A0, A1, A2, A3};
int bcdPines[4] = {6, 7, 12, 13};

bool llamadasArriba[5] = {false, false, false, false, false};
bool llamadasAbajo[5]  = {false, false, false, false, false};

int pisoActual = 1;
int pisoDestino = 1;
bool enMovimiento = false;
bool direccionAscendente = true;

const unsigned long TIEMPO_POSICION = 1000;
const unsigned long TIEMPO_PAUSA = 1000;

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(botones[i], INPUT_PULLUP);
    pinMode(IR[i], INPUT_PULLUP);
    pinMode(bcdPines[i], OUTPUT);
  }
  for (int i = 0; i < 2; i++) {
    pinMode(subirObajar[i], OUTPUT);
    pinMode(LEDS[i], OUTPUT);
  }
  digitalWrite(bcdPines[0], HIGH);
  digitalWrite(bcdPines[1], LOW);
  digitalWrite(bcdPines[2], LOW);
  digitalWrite(bcdPines[3], LOW);
}

void loop() {
  actualizarPisoPorIR();
  guardar_llamadas();

  if (!enMovimiento) {
    procesarLlamadasSecuenciales();
  } else {
    verificarCambioDeRumbo();
    verificarLlegada();
  }

  delay(10);
}

void actualizarPisoPorIR() {
  if (digitalRead(IR[0]) == LOW && digitalRead(IR[1]) == HIGH && digitalRead(IR[2]) == HIGH && digitalRead(IR[3]) == HIGH) {
    digitalWrite(bcdPines[0], HIGH);
    digitalWrite(bcdPines[1], LOW);
    digitalWrite(bcdPines[2], LOW);
    digitalWrite(bcdPines[3], LOW);
    pisoActual = 1;
  }
  if (digitalRead(IR[0]) == HIGH && digitalRead(IR[1]) == LOW && digitalRead(IR[2]) == HIGH && digitalRead(IR[3]) == HIGH) {
    digitalWrite(bcdPines[0], LOW);
    digitalWrite(bcdPines[1], HIGH);
    digitalWrite(bcdPines[2], LOW);
    digitalWrite(bcdPines[3], LOW);
    pisoActual = 2;
  }
  if (digitalRead(IR[0]) == HIGH && digitalRead(IR[1]) == HIGH && digitalRead(IR[2]) == LOW && digitalRead(IR[3]) == HIGH) {
    digitalWrite(bcdPines[0], HIGH);
    digitalWrite(bcdPines[1], HIGH);
    digitalWrite(bcdPines[2], LOW);
    digitalWrite(bcdPines[3], LOW);
    pisoActual = 3;
  }
  if (digitalRead(IR[0]) == HIGH && digitalRead(IR[1]) == HIGH && digitalRead(IR[2]) == HIGH && digitalRead(IR[3]) == LOW) {
    digitalWrite(bcdPines[0], LOW);
    digitalWrite(bcdPines[1], LOW);
    digitalWrite(bcdPines[2], HIGH);
    digitalWrite(bcdPines[3], LOW);
    pisoActual = 4;
  }
}

// Indica si YA hay una llamada guardada en el arreglo dado.
bool hayLlamada(bool arreglo[]) {
  for (int p = 1; p <= 4; p++) {
    if (arreglo[p]) return true;
  }
  return false;
}

void guardar_llamadas() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(botones[i]) == LOW) {
      int piso = i + 1;

      // Se recalcula en cada botón detectado porque el estado de las colas
      // puede cambiar dentro de esta misma llamada si hay varios botones
      // presionados en el mismo barrido de lectura.
      bool enReposoTotal = (!enMovimiento &&
                             !hayLlamada(llamadasArriba) &&
                             !hayLlamada(llamadasAbajo));

      if (piso == pisoActual) {
        // ===================== AQUI SE CORRIGIO LA LOGICA DE BARRIDO =====================
        // ANTES: cualquier pulsación del piso donde el ascensor se encontraba en ese
        // instante se descartaba SIEMPRE ("llamadasArriba[piso]=false; llamadasAbajo[piso]=false;"),
        // sin importar si el ascensor estaba a punto de abandonar ese piso o si ya
        // tenía una dirección de barrido activa. Esto hacía que, en el caso de prueba
        // (ascensor en piso 4, se pulsa 3 y luego 4 justo cuando arranca hacia el 3),
        // la llamada al piso 4 se perdiera para siempre y el ascensor nunca regresara.
        if (enReposoTotal) {
          // El ascensor está realmente detenido y sin nada pendiente: pedir el
          // piso donde ya está no tiene sentido, se ignora legítimamente.
          llamadasArriba[piso] = false;
          llamadasAbajo[piso] = false;
        } else {
          // El ascensor ya está en movimiento o ya tiene una dirección de barrido
          // definida. La llamada a "mi propio piso" solo puede atenderse cuando
          // el ascensor VUELVA a pasar por aquí, es decir, en el barrido de
          // REGRESO (dirección contraria a la que se está usando ahora).
          if (direccionAscendente) {
            llamadasAbajo[piso] = true;   // AQUI SE CORRIGIO: se guarda para el regreso hacia abajo
          } else {
            llamadasArriba[piso] = true;  // AQUI SE CORRIGIO: se guarda para el regreso hacia arriba
          }
        }
        // =====================================================================================
        continue;
      }

      // La dirección de prioridad la define el PRIMER botón presionado
      // cuando el ascensor está en reposo total (sin llamadas pendientes).
      if (enReposoTotal) {
        direccionAscendente = (piso > pisoActual);
      }

      // Clasificación por posición relativa al piso actual.
      if (piso > pisoActual) {
        llamadasArriba[piso] = true;
      } else {
        llamadasAbajo[piso] = true;
      }
    }
  }
}

int buscarPisoMasCercano(bool arreglo[]) {
  int mejorPiso = -1;
  int mejorDistancia = 999;
  for (int p = 1; p <= 4; p++) {
    if (arreglo[p]) {
      int distancia = abs(p - pisoActual);
      if (distancia < mejorDistancia) {
        mejorDistancia = distancia;
        mejorPiso = p;
      }
    }
  }
  return mejorPiso;
}

void procesarLlamadasSecuenciales() {
  bool hayArriba = hayLlamada(llamadasArriba);
  bool hayAbajo  = hayLlamada(llamadasAbajo);

  if (!hayArriba && !hayAbajo) {
    return; // no hay nada que atender
  }

  // AQUI SE CORRIGIO: si en la dirección actual ya no quedan llamadas, se
  // invierte el sentido de barrido Y, en la MISMA pasada, se evalúa de una
  // vez la nueva dirección, para no desperdiciar un ciclo del loop() antes
  // de reaccionar (comportamiento SCAN sin pausas innecesarias).
  if (direccionAscendente && !hayArriba) {
    direccionAscendente = false;
  } else if (!direccionAscendente && !hayAbajo) {
    direccionAscendente = true;
  }

  if (direccionAscendente && hayArriba) {
    int siguientePiso = buscarPisoMasCercano(llamadasArriba);
    llamadasArriba[siguientePiso] = false;
    iniciarMovimiento(siguientePiso);
  } else if (!direccionAscendente && hayAbajo) {
    int siguientePiso = buscarPisoMasCercano(llamadasAbajo);
    llamadasAbajo[siguientePiso] = false;
    iniciarMovimiento(siguientePiso);
  }
}

void verificarCambioDeRumbo() {
  if (direccionAscendente && hayLlamada(llamadasArriba)) {
    int pisoMasCercano = buscarPisoMasCercano(llamadasArriba);
    if (abs(pisoMasCercano - pisoActual) < abs(pisoDestino - pisoActual)) {
      pisoDestino = pisoMasCercano;
    }
  } else if (!direccionAscendente && hayLlamada(llamadasAbajo)) {
    int pisoMasCercano = buscarPisoMasCercano(llamadasAbajo);
    if (abs(pisoMasCercano - pisoActual) < abs(pisoDestino - pisoActual)) {
      pisoDestino = pisoMasCercano;
    }
  }
}

void iniciarMovimiento(int destino) {
  pisoDestino = destino;

  if (destino == pisoActual) {
    enMovimiento = false;
    return;
  }

  enMovimiento = true;
  if (destino > pisoActual) {
    digitalWrite(subirObajar[0], HIGH);
    digitalWrite(subirObajar[1], LOW);
    digitalWrite(LEDS[0], HIGH);
    digitalWrite(LEDS[1], LOW);
  } else {
    digitalWrite(subirObajar[0], LOW);
    digitalWrite(subirObajar[1], HIGH);
    digitalWrite(LEDS[0], LOW);
    digitalWrite(LEDS[1], HIGH);
  }
}

void verificarLlegada() {
  int pisoAnterior = pisoActual;

  actualizarPisoPorIR();

  if (pisoActual == pisoDestino) {
    delay(TIEMPO_POSICION);

    digitalWrite(subirObajar[0], LOW);
    digitalWrite(subirObajar[1], LOW);
    digitalWrite(LEDS[0], LOW);
    digitalWrite(LEDS[1], LOW);

    enMovimiento = false;

    unsigned long inicio = millis();
    while (millis() - inicio < TIEMPO_PAUSA) {
      guardar_llamadas();
      delay(10);
    }
  } else {
    if (pisoActual != pisoAnterior) {
      actualizarDisplay();
    }
  }
}

void actualizarDisplay() {
  if (pisoActual == 1) {
    digitalWrite(bcdPines[0], HIGH);
    digitalWrite(bcdPines[1], LOW);
    digitalWrite(bcdPines[2], LOW);
    digitalWrite(bcdPines[3], LOW);
  } else if (pisoActual == 2) {
    digitalWrite(bcdPines[0], LOW);
    digitalWrite(bcdPines[1], HIGH);
    digitalWrite(bcdPines[2], LOW);
    digitalWrite(bcdPines[3], LOW);
  } else if (pisoActual == 3) {
    digitalWrite(bcdPines[0], HIGH);
    digitalWrite(bcdPines[1], HIGH);
    digitalWrite(bcdPines[2], LOW);
    digitalWrite(bcdPines[3], LOW);
  } else if (pisoActual == 4) {
    digitalWrite(bcdPines[0], LOW);
    digitalWrite(bcdPines[1], LOW);
    digitalWrite(bcdPines[2], HIGH);
    digitalWrite(bcdPines[3], LOW);
  }
}
