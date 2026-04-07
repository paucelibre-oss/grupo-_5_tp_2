# Repositorio de entrega de Tp 2 del grupo 5

##Integrantes:
- Paulo Cesar Libreros Fajardo

# Trabajo práctico 2: Memoria Dinámica
## Problema

Se requiere implementar un sistema para el control de tres LEDs (rojo, verde y azul) mediante la activación de un botón.
El sistema debe responder de la siguiente forma:
1. Se debe detectar los siguientes estados del botón:.
-Pulso: Si el botón se presiona entre 200 ms y 1000 ms.
-Corto: Si el botón se presiona entre 1000 ms y 2000 ms.
-Largo: Si el botón se presiona más de 2000 ms.
-No presionado: para cualquier otro caso.
2. Si se detecta un Botón Pulso: el sistema debe pasar al Estado Rojo y mantener encendido el LED Rojo.
3. Si se detecta un Botón Corto: el sistema debe pasar al Estado Verde y mantener encendido el LED Verde.
4. Si se detecta un Botón Largo: el sistema debe pasar al Estado Azul y mantener encendido el LED Azul.
5. El sistema inicia en el Estado Standby con los LEDs apagados.
6. No se debe permitir que más de un LED esté encendido al mismo tiempo.

