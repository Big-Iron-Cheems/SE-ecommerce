#include "dbutils.h"

/*
 * // PostgreSQL setup
 * sudo systemctl start postgresql.service redis.service (to ensure the services are running)
 * sudo su postgres (to switch to the postgres user)
 * initdb -D /var/lib/postgres/data (to initialize the database)
 *
 * // Reset user
 * REASSIGN OWNED BY ecommerce TO postgres;
 * DROP OWNED BY ecommerce;
 * DROP USER ecommerce;
 */

PGconn *conn2Postgres(const std::string &dbname, const std::string &user, const std::string &password) {
    std::string connInfo = "dbname=" + dbname + " user=" + user + " password=" + password;
    PGconn *conn = PQconnectdb(connInfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        errorPrint("Failed to connect to Postgres: " + std::string(PQerrorMessage(conn)));
        PQfinish(conn);
        return nullptr;
    }

    return conn;
}

bool doesDatabaseExist(PGconn *conn, const std::string &databaseName) {
    std::string query = "SELECT 1 FROM pg_database WHERE datname = '" + databaseName + "'";
    PGresult *res = execCommand(conn, query);

    if (!res) return false;

    // Check if there is at least one row (database exists)
    bool databaseExists = (PQntuples(res) > 0);
    PQclear(res);

    return databaseExists;
}

void createDatabase(PGconn *conn, const std::string &databaseName) {
    if (doesDatabaseExist(conn, databaseName)) debugPrint("Database `" + databaseName + "` already exists.");
    else {
        if (!execCommand(conn, "CREATE DATABASE " + databaseName)) return;
        debugPrint("Database `" + databaseName + "` created.");
    }
}

bool doesUserExist(PGconn *conn, const std::string &username) {
    std::string query = "SELECT 1 FROM pg_user WHERE usename = '" + username + "'";
    PGresult *res = execCommand(conn, query);

    if (!res) return false;

    // Check if there is at least one row (user exists)
    bool userExists = (PQntuples(res) > 0);
    PQclear(res);

    return userExists;
}

void createUser(PGconn *conn, const std::string &username, const std::string &password, const std::string &options) {
    if (doesUserExist(conn, username)) debugPrint("User `" + username + "` already exists.");
    else {
        if (!execCommand(conn, "CREATE USER " + username + " WITH PASSWORD '" + password + "' " + options)) return;
        debugPrint("User `" + username + "` created.");
    }
}

bool doesTableExist(PGconn *conn, const std::string &tableName) {
    std::string query = "SELECT 1 FROM pg_tables WHERE tablename = '" + tableName + "'";
    PGresult *res = execCommand(conn, query);

    if (!res) return false;

    // Check if there is at least one row (table exists)
    bool tableExists = (PQntuples(res) > 0);
    PQclear(res);

    return tableExists;
}

void createTable(PGconn *conn, const std::string &tableName, const std::string &columns) {
    if (doesTableExist(conn, tableName)) debugPrint("Table `" + tableName + "` already exists.");
    else {
        if (!execCommand(conn, "CREATE TABLE " + tableName + " (" + columns + ")")) return;
        debugPrint("Table `" + tableName + "` created.");
    }
}

PGresult *execCommand(PGconn *conn, const std::string &command) {
    debugPrint("Executing command: " + command);

    if (PQstatus(conn) != CONNECTION_OK) {
        errorPrint("Connection to PostgreSQL failed: " + std::string(PQerrorMessage(conn)));
        return nullptr;
    }

    PGresult *res = PQexec(conn, command.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK) {
        errorPrint("Failed to execute command: " + std::string(PQerrorMessage(conn)));
        PQclear(res);
        return nullptr;
    }

    return res;
}


// Sezione porcherie

PGconn *initDatabase() {
    // Connect to the default 'postgres' database as the 'postgres' user
    PGconn *conn = PQconnectdb("dbname=postgres user=postgres password=");
    if (PQstatus(conn) != CONNECTION_OK) {
        errorPrint("Failed to connect to Postgres database 'postgres' as user 'postgres': " + std::string(PQerrorMessage(conn)));
        PQfinish(conn);
        return nullptr;
    }

    // Create the 'ecommerce' user as `postgres` if it does not exist
    createUser(conn, "ecommerce", "ecommerce", "NOSUPERUSER CREATEDB NOCREATEROLE INHERIT LOGIN NOREPLICATION NOBYPASSRLS");

    // Disconnect from the 'postgres' database as the 'postgres' user
    PQfinish(conn);

    // Connect to the default 'postgres' database as the 'ecommerce' user
    conn = conn2Postgres("postgres", "ecommerce", "ecommerce");
    if (PQstatus(conn) != CONNECTION_OK) {
        errorPrint("Failed to connect to Postgres database 'postgres' as user 'ecommerce': " + std::string(PQerrorMessage(conn)));
        PQfinish(conn);
        return nullptr;
    }

    // Create the 'ecommerce' database as `ecommerce` if it does not exist
    createDatabase(conn, "ecommerce");

    // Disconnect from the 'postgres' database as the 'ecommerce' user
    PQfinish(conn);

    // Connect to the 'ecommerce' database as the 'ecommerce' user
    conn = conn2Postgres("ecommerce", "ecommerce", "ecommerce");
    if (PQstatus(conn) != CONNECTION_OK) {
        errorPrint("Failed to connect to Postgres database 'ecommerce' as user 'ecommerce': " + std::string(PQerrorMessage(conn)));
        PQfinish(conn);
        return nullptr;
    }

    // Create the tables if they do not exist
    initTables(conn);

    // Return the connection to the now ready database
    return conn;
}

void initTables(PGconn *conn) {

    // Seen only by the admins
    // TODO: possibly merge user types into a single table if that is better
    createTable(conn, "customers", "id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE NOT NULL, balance INT NOT NULL");
    createTable(conn, "suppliers", "id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE NOT NULL, balance INT NOT NULL");
    createTable(conn, "transporters", "id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE NOT NULL, balance INT NOT NULL");
//    createTable(conn, "users", "id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE NOT NULL, balance INT NOT NULL, user_type VARCHAR(255) NOT NULL");

    // Seen by customers and suppliers
    createTable(conn, "products", "id SERIAL PRIMARY KEY, "
                                  "name VARCHAR(255) NOT NULL, "
                                  "supplier_id INT NOT NULL,"
                                  "price INT NOT NULL, "
                                  "amount INT NOT NULL, "
                                  "description VARCHAR(255) NOT NULL, "
                                  "FOREIGN KEY (supplier_id) REFERENCES suppliers(id)"
    ); ///<

    createTable(conn, "orders", "id SERIAL PRIMARY KEY, "
                                "customer_id INT NOT NULL, "
                                "total_price INT NOT NULL, "
                                "transporter_id INT NOT NULL, "
                                "status VARCHAR(255) NOT NULL, "
                                "address VARCHAR(255) NOT NULL, "
                                "timestamp TIMESTAMP NOT NULL,"
                                "FOREIGN KEY (customer_id) REFERENCES customers(id), "
                                "FOREIGN KEY (transporter_id) REFERENCES transporters(id)"
    ); ///<

    createTable(conn, "order_items", "id SERIAL PRIMARY KEY, "
                                     "order_id INT NOT NULL, "
                                     "product_id INT NOT NULL, "
                                     "quantity INT NOT NULL, "
                                     "price INT NOT NULL, "
                                     "supplier_id INT NOT NULL, "
                                     "FOREIGN KEY (order_id) REFERENCES orders(id), "
                                     "FOREIGN KEY (product_id) REFERENCES products(id), "
                                     "FOREIGN KEY (supplier_id) REFERENCES suppliers(id)"
    ); ///<


}
