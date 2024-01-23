#include "dbutils.h"

/*
 * sudo systemctl start postgresql.service redis.service (to ensure the services are running)
 * sudo su postgres (to switch to the postgres user)
 * initdb -D /var/lib/postgres/data (to initialize the database)
 */

PGconn *conn2Postgres() {
    PGconn *conn = PQconnectdb("dbname=postgres user=postgres password=");
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "[ERROR] Failed to connect to Postgres:\n" << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return nullptr;
    }

    return conn;
}

bool doesUserExist(PGconn *conn, const std::string &username) {
    /*   const char *query = "SELECT 1 FROM pg_user WHERE usename = $1";
       const char *paramValues[1] = {username.c_str()};
       const int paramLengths[1] = {static_cast<int>(username.length())};
       const int paramFormats[1] = {1}; // Text format
       PGresult *res = PQexecParams(conn, query, 1, nullptr, paramValues, paramLengths, paramFormats, 0);

       if (PQresultStatus(res) != PGRES_TUPLES_OK) {
           fprintf(stderr, "Failed to execute query:\n%s", PQerrorMessage(conn));
           PQclear(res);
           return false;
       }

       // Check if there is at least one row (user exists)
       bool userExists = (PQntuples(res) > 0);
       PQclear(res);

       return userExists;
    */
    std::string query = "SELECT 1 FROM pg_user WHERE usename = '" + username + "'";
    PGresult *res = execCommand(conn, query);

    if (!res) return false;

    // Check if there is at least one row (user exists)
    bool userExists = (PQntuples(res) > 0);
    PQclear(res);

    return userExists;
}

void createUser(PGconn *conn, const std::string &username, const std::string &password) {
    if (doesUserExist(conn, username)) std::cout << "[DEBUG] User " + username + " already exists." << std::endl;
    else {
        if (!execCommand(conn, "CREATE USER ecommerce WITH PASSWORD 'ecommerce'")) return;
        std::cout << "[DEBUG] User " + username + " created." << std::endl;
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
    if (doesTableExist(conn, tableName)) std::cout << "[DEBUG] Table " + tableName + " already exists." << std::endl;
    else {
        if (!execCommand(conn, "CREATE TABLE " + tableName + " (" + columns + ")")) return;
        std::cout << "[DEBUG] Table " + tableName + " created." << std::endl;
    }
}

PGresult *execCommand(PGconn *conn, const std::string &command) {
    std::cout << "[DEBUG] Executing command: " << command.c_str() << std::endl;

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "[ERROR] Connection to PostgreSQL failed: " << PQerrorMessage(conn) << std::endl;
        return nullptr;
    }

    PGresult *res = PQexec(conn, command.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "[ERROR] Failed to execute command:\n" << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return nullptr;
    }

    return res;
}


// Sezione porcherie

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
