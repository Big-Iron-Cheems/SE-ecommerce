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
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Postgres: ", PQerrorMessage(conn));
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
    if (doesDatabaseExist(conn, databaseName)) Utils::log(Utils::LogLevel::TRACE, std::cout, "Database `" + databaseName + "` already exists.");
    else {
        if (!execCommand(conn, "CREATE DATABASE " + databaseName)) return;
        Utils::log(Utils::LogLevel::TRACE, std::cout, "Database `" + databaseName + "` created.");
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
    if (doesUserExist(conn, username)) Utils::log(Utils::LogLevel::TRACE, std::cout, "User `" + username + "` already exists.");
    else {
        if (!execCommand(conn, "CREATE USER " + username + " WITH PASSWORD '" + password + "' " + options)) return;
        Utils::log(Utils::LogLevel::TRACE, std::cout, "User `" + username + "` created.");
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
    if (doesTableExist(conn, tableName)) Utils::log(Utils::LogLevel::TRACE, std::cout, "Table `" + tableName + "` already exists.");
    else {
        if (!execCommand(conn, "CREATE TABLE " + tableName + " (" + columns + ")")) return;
        Utils::log(Utils::LogLevel::TRACE, std::cout, "Table `" + tableName + "` created.");
    }
}

// FIXME: this does not work, it returns false even if the function exists
bool doesFunctionExist(PGconn *conn, const std::string &functionName, const std::vector<std::string> &argTypes) {
    // Build the query to get the function with the given name and argument types
    std::string query = "SELECT 1 FROM pg_proc, pg_type WHERE pg_proc.proname = '" + functionName + "' AND pg_proc.proargtypes = ARRAY[";

    // Add the argument types to the query
    for (size_t i = 0; i < argTypes.size(); ++i) {
        query += "(SELECT oid FROM pg_type WHERE typname = '" + argTypes[i] + "')";
        if (i < argTypes.size() - 1) query += ",";
    }
    query += "]::oidvector";

    // Execute the query
    PGresult *res = execCommand(conn, query);
    if (!res) return false;

    // Check if there is at least one row (function exists)
    bool functionExists = (PQntuples(res) > 0);
    PQclear(res);

    return functionExists;
}

// TODO: actually use this once doesFunctionExist is fixed
void createFunction(PGconn *conn,
                    const std::string &functionName,
                    const std::vector<std::pair<std::string, std::string>> &args,
                    const std::string &returnType,
                    const std::string &body) {
    // Get the argument types as a vector and as a string
    std::vector<std::string> argTypes;
    std::string argTypesStr;
    for (const std::pair<std::string, std::string> &arg: args) {
        argTypes.push_back(arg.second);
        argTypesStr += arg.second + ", ";
    }
    // Remove trailing comma and space
    if (!argTypesStr.empty()) {
        argTypesStr.pop_back();
        argTypesStr.pop_back();
    }

    // Check if the function already exists
    if (doesFunctionExist(conn, functionName, argTypes)) {
        Utils::log(Utils::LogLevel::TRACE, std::cout, "Function `" + functionName + "(" + argTypesStr + ")` already exists.");
        return;
    }

    Utils::log(Utils::LogLevel::TRACE, std::cout, "Creating function `" + functionName + "(" + argTypesStr + ")`...");
    // Start building the query
    std::string query = "CREATE OR REPLACE FUNCTION " + functionName + "(";

    // Add the arguments to the query
    for (size_t i = 0; i < args.size(); ++i) {
        query += args[i].first + " " + args[i].second;
        if (i < args.size() - 1) query += ", ";
    }

    // Finish building the query
    query += ") RETURNS " + returnType + " AS $$ BEGIN " + body + " END; $$ LANGUAGE plpgsql SECURITY DEFINER;";

    // Execute the query
    if (!execCommand(conn, query)) return;
    else Utils::log(Utils::LogLevel::TRACE, std::cout, "Function `" + functionName + "(" + argTypesStr + ")` created.");
}

PGresult *execCommand(PGconn *conn, const std::string &command) {
    Utils::log(Utils::LogLevel::DEBUG, std::cout, "Executing: " + command);

    if (PQstatus(conn) != CONNECTION_OK) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Connection to PostgreSQL failed: ", PQerrorMessage(conn));
        return nullptr;
    }

    PGresult *res = PQexec(conn, command.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to execute command: ", PQerrorMessage(conn));
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
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Postgres database 'postgres' as user 'postgres': ", PQerrorMessage(conn));
        PQfinish(conn);
        return nullptr;
    }

    // Create the 'ecommerce' user as `postgres` if it does not exist
    createUser(conn, "ecommerce", "ecommerce", "NOSUPERUSER CREATEDB NOCREATEROLE INHERIT LOGIN NOREPLICATION NOBYPASSRLS");

    // Create the 'customer', 'supplier', and 'transporter' users with fewer permissions
    createUser(conn, "customer", "customer", "NOSUPERUSER NOCREATEDB NOCREATEROLE INHERIT LOGIN NOREPLICATION NOBYPASSRLS");
    createUser(conn, "supplier", "supplier", "NOSUPERUSER NOCREATEDB NOCREATEROLE INHERIT LOGIN NOREPLICATION NOBYPASSRLS");
    createUser(conn, "transporter", "transporter", "NOSUPERUSER NOCREATEDB NOCREATEROLE INHERIT LOGIN NOREPLICATION NOBYPASSRLS");


    // Disconnect from the 'postgres' database as the 'postgres' user
    PQfinish(conn);

    // Connect to the default 'postgres' database as the 'ecommerce' user
    conn = conn2Postgres("postgres", "ecommerce", "ecommerce");
    if (PQstatus(conn) != CONNECTION_OK) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Postgres database 'postgres' as user 'ecommerce': ", PQerrorMessage(conn));
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
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Postgres database 'ecommerce' as user 'ecommerce': ", PQerrorMessage(conn));
        PQfinish(conn);
        return nullptr;
    }

    // Create the tables if they do not exist
    initTables(conn);

    // Define SQL functions
    initFunctions(conn);

    // Return the connection to the now ready database
    return conn;
}

void initTables(PGconn *conn) {

    // Seen only by the admins
    // TODO: possibly merge user types into a single table if that is better
    createTable(conn, "customers", "id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE NOT NULL, balance INT NOT NULL");
    createTable(conn, "suppliers", "id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE NOT NULL, balance INT NOT NULL");
    createTable(conn, "transporters", "id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE NOT NULL, balance INT NOT NULL");
    // createTable(conn, "users", "id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE NOT NULL, balance INT NOT NULL, user_type VARCHAR(255) NOT NULL");

    // Seen by customers and suppliers
    createTable(conn, "products", R"(
            id SERIAL PRIMARY KEY,
            name VARCHAR(255) NOT NULL,
            supplier_id INT NOT NULL,
            price INT NOT NULL,
            amount INT NOT NULL,
            description VARCHAR(255) NOT NULL,
            FOREIGN KEY (supplier_id) REFERENCES suppliers(id)
        )"); ///<

    createTable(conn, "orders", R"(
            id SERIAL PRIMARY KEY,
            customer_id INT NOT NULL,
            total_price INT NOT NULL,
            transporter_id INT NOT NULL,
            status VARCHAR(255) NOT NULL,
            address VARCHAR(255) NOT NULL,
            timestamp TIMESTAMP NOT NULL,
            FOREIGN KEY (customer_id) REFERENCES customers(id),
            FOREIGN KEY (transporter_id) REFERENCES transporters(id)
        )"); ///<

    createTable(conn, "order_items", R"(
            id SERIAL PRIMARY KEY,
            order_id INT NOT NULL,
            product_id INT NOT NULL,
            quantity INT NOT NULL,
            price INT NOT NULL,
            supplier_id INT NOT NULL,
            FOREIGN KEY (order_id) REFERENCES orders(id),
            FOREIGN KEY (product_id) REFERENCES products(id),
            FOREIGN KEY (supplier_id) REFERENCES suppliers(id)
        )"); ///<


    // Grant permissions
    execCommand(conn, "GRANT SELECT ON customers TO customer");
    // execCommand(conn, "GRANT SELECT ON suppliers TO customer, supplier, transporter");
    // execCommand(conn, "GRANT SELECT ON transporters TO customer, supplier, transporter");
    execCommand(conn, "GRANT SELECT ON products TO customer, supplier");
    // execCommand(conn, "GRANT SELECT ON orders TO customer, supplier, transporter");
    // execCommand(conn, "GRANT SELECT ON order_items TO customer, supplier, transporter");
}

void initFunctions(PGconn *conn) { // TODO: check beforehand if the functions already exist and skip them if they do
    // Define SQL functions
    std::string setBalanceProcedureCustomer = R"(
        CREATE OR REPLACE FUNCTION set_balance_customer(customer_id INT, increase_amount INT, add BOOLEAN)
        RETURNS VOID AS $$
        BEGIN
            IF add THEN
                UPDATE customers SET balance = balance + increase_amount WHERE id = customer_id;
            ELSE
                UPDATE customers SET balance = balance - increase_amount WHERE id = customer_id;
            END IF;
        END;
        $$ LANGUAGE plpgsql SECURITY DEFINER;
    )";
    execCommand(conn, setBalanceProcedureCustomer);

    // FIXME: if the types are int4, int4, bool, the function is found. Need to do mappings?
    /* createFunction(conn, "set_balance_supplier", {{"supplier_id", "INT"}, {"increase_amount", "INT"}, {"add", "BOOLEAN"}}, "VOID",
                   R"(
        IF add THEN
            UPDATE suppliers SET balance = balance + increase_amount WHERE id = supplier_id;
        ELSE
            UPDATE suppliers SET balance = balance - increase_amount WHERE id = supplier_id;
        END IF;
    )");*/

    std::string setBalanceProcedureSupplier = R"(
        CREATE OR REPLACE FUNCTION set_balance_supplier(supplier_id INT, increase_amount INT, add BOOLEAN)
        RETURNS VOID AS $$
        BEGIN
            IF add THEN
                UPDATE suppliers SET balance = balance + increase_amount WHERE id = supplier_id;
            ELSE
                UPDATE suppliers SET balance = balance - increase_amount WHERE id = supplier_id;
            END IF;
        END;
        $$ LANGUAGE plpgsql SECURITY DEFINER;
    )";
    execCommand(conn, setBalanceProcedureSupplier);

    std::string setBalanceProcedureTransporter = R"(
        CREATE OR REPLACE FUNCTION set_balance_transporter(transporter_id INT, increase_amount INT, add BOOLEAN)
        RETURNS VOID AS $$
        BEGIN
            IF add THEN
                UPDATE transporters SET balance = balance + increase_amount WHERE id = transporter_id;
            ELSE
                UPDATE transporters SET balance = balance - increase_amount WHERE id = transporter_id;
            END IF;
        END;
        $$ LANGUAGE plpgsql SECURITY DEFINER;
    )";
    execCommand(conn, setBalanceProcedureTransporter);

    // Grant the EXECUTE permission to the respective users
    execCommand(conn, "GRANT EXECUTE ON FUNCTION set_balance_customer(INT, INT, BOOL) TO customer;");
    execCommand(conn, "GRANT EXECUTE ON FUNCTION set_balance_supplier(INT, INT, BOOL) TO supplier;");
    execCommand(conn, "GRANT EXECUTE ON FUNCTION set_balance_transporter(INT, INT, BOOL) TO transporter;");
}
