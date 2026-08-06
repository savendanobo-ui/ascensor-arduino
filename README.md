### Sistema de Control para Ascensor de Cuatro Pisos con Arduino UNO

##Resumen del Proyecto

Este proyecto implementa un sistema de control para un ascensor de cuatro pisos utilizando un **Arduino UNO** como unidad central de procesamiento. El sistema gestiona solicitudes de llamada desde cada piso y coordina el movimiento de la cabina mediante un motor DC.

### Características Principales

- Control de 4 pisos con sensores infrarrojos FC-51
- Lógica de barrido con prioridad ascendente/descendente
- Visualización del piso actual en display de 7 segmentos
- Indicación del sentido de movimiento con LEDs
- Control de motor DC mediante puente H L293D

## Componentes Utilizados

| Componente | Modelo | Cantidad |
|------------|--------|----------|
| Microcontrolador | Arduino UNO R3 | 1 |
| Puente H | L293D | 1 |
| Motoreductor | Motor DC | 1 |
| Sensor IR | FC-51 | 4 |
| Display 7 segmentos | Ánodo común | 1 |
| Decodificador BCD | SN74LS47N | 1 |
| Pulsadores | Push button | 4 |
| Regulador de voltaje | LM7805CV | 1 |

## Código

El código principal se encuentra en `ascensor_con_microcontroladores.ino`.

### Funciones Principales
- `setup()`: Configuración inicial del sistema
- `loop()`: Bucle principal de ejecución
- `actualizarPisoPorIR()`: Lectura de sensores de posición
- `guardar_llamadas()`: Captura de solicitudes de piso
- `procesarLlamadasSecuenciales()`: Algoritmo de barrido

## Pruebas

El sistema fue probado con los siguientes escenarios:

- Llamada desde cualquier piso individual
- Múltiples llamadas secuenciales
- Prioridad ascendente/descendente
- Detección de posición por sensores IR
- Visualización correcta del display

## Posibles mejoras futuras

- Agregar antirrebote por hardware en botones
- Instalar sensores de fin de carrera
- Implementar máquina de estados con temporizadores
- Agregar panel de control en cabina

## Autores

- **Samir Steven Avendaño Bolaños**
- **Leonardo Emilio López Pinzón**
- **Laura Valentina Garcia Casas**

## Licencia

Este proyecto está bajo la licencia MIT - ver el archivo [LICENSE](LICENSE) para más detalles.
