# SistDistrib2C2017
Repositorio de Sistemas Distribuidos 1 - 2do Cuatrimestre 2017 FIUBA

# Build Status
[![Build Status](https://travis-ci.org/octaI/SistDistrib2C2017.svg?branch=master)](https://travis-ci.org/octaI/SistDistrib2C2017)

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