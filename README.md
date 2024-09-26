# Ecommerce

## Description

This is a simple backend for an ecommerce application.  
It uses PostgreSQL for the database and Redis for caching.

## Installation

1. Clone the [repository](https://github.com/Big-Iron-Cheems/SE-ecommerce).
2. Download the required dependencies (_NOTE_: names are based on Arch Linux package names):
    * [postgresql-libs](https://www.postgresql.org/download/), PostgreSQL client library.
    * [libpqxx](https://github.com/jtv/libpqxx/), C++ client API for PostgreSQL.
    * [hiredis](https://github.com/redis/hiredis), C client for Redis
    * [redis-plus-plus](https://github.com/sewenew/redis-plus-plus), C++ client for Redis
3. Run `make` to compile the project, executable will be in the `bin/` directory, object files will be in the `obj/` directory.

## Usage

To ensure the program runs correctly you must:

- Have a PostgreSQL server running on `localhost:5432` (default port).
- Have a Redis server running on `localhost:6379` (default port).
- Have C++26 installed on your system.
- Have CMake 3.27 or higher installed on your system.

_NOTE_: to start the postgresql and redis services on Arch Linux, run the following commands:

```bash
sudo systemctl start postgresql.service redis.service
```

Running the program with the `-h` flag will display the help message.

```log
Usage: ./bin/ecommerce [options]
Options:
        -h, --help    Show this help message and exit
        --drop        Drop the database and exit
        -v            Enable verbose logging to console

```

By default, the program will manage 3 log files (`customer.log`,`supplier.log`,`transporter.log`) in the current directory.  
Using the `-v` flag will enable verbose logging to the console of all log messages.
