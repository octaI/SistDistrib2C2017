# SistDistrib2C2017 [![Build Status](https://travis-ci.org/octaI/SistDistrib2C2017.svg?branch=master)](https://travis-ci.org/octaI/SistDistrib2C2017)
Repositorio de Sistemas Distribuidos 1 - 2do Cuatrimestre 2017 FIUBA

### Enunciado
Se quiere implementar un sistema de reserva de lugares para funciones de cine.
Los clientes ingresan al sistema, eligen una funcion y pueden ver los asientos reservados y libres.
Cada cliente puede seleccionar uno o mas asientos para reservar. Al hacerlo, los asientos deben aparecer como reservados para el resto de los clientes.
Mientras la reserva no esté confirmada, el cliente puede cambiar de opinion y elegir otros asientos.
Por ultimo el cliente realiza la transaccion para abonar las reservas. En ese momento la reserva se hace efectiva.

#### Objetivos:
- Analizar el problema.
- Elaborar un diagrama de secuencia.
- Implementar un prototipo del sistema que funcione concurrentemente en una computadora.

#### Notas:
- Utilizar IPC.
- Utilizar procesos, no threads.
- Existe un tiempo maximo de sesión del cliente, durante el cual el cliente puede reservar asientos.


## Requeriments
- ```sqlite3```
- ```cmake```

## Compile
```bash
    $ cmake .
    $ make
```

## Run
#### Server
```bash
    # Init DB data and initialize IPC's
    $ ./src/utils/database_data_init
    $ ./src/utils/initializer
    # Wake up cinema server
    $ ./src/cinema/cinema
    
    # After cinema close. Destroy IPC's
    $ ./src/utils/destructor
```
#### Client
```
    $ ./src/client/client
```

## Test
```bash
    $ ./test/commqueue_test
    $ ./test/db_test/db_test
```