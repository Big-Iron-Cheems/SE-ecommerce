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

std::unique_ptr<pqxx::connection> conn2Postgres(const std::string &dbname, const std::string &user, const std::string &password) {
    std::string connInfo = "dbname=" + dbname + " user=" + user + " password=" + password;
    try {
        std::unique_ptr<pqxx::connection> conn = std::make_unique<pqxx::connection>(connInfo);
        Utils::log(Utils::LogLevel::DEBUG, std::cout, "Connected to Postgres database '" + dbname + "' as user '" + user + "'.");
        return conn;
    } catch (const pqxx::broken_connection &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to connect to Postgres database '" + dbname + "' as user '" + user + "': ", e.what());
        throw; // Rethrow the exception to propagate it to the caller
    }
}

bool doesDatabaseExist(std::unique_ptr<pqxx::connection> &conn, const std::string &databaseName) {
    std::string query = "SELECT 1 FROM pg_database WHERE datname = '" + databaseName + "'";
    try {
        pqxx::work W(*conn);
        pqxx::result R = W.exec(query);
        W.commit();
        return !R.empty();
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to check if database exists: ", e.what());
        return false;
    }
}

void createDatabase(std::unique_ptr<pqxx::connection> &conn, const std::string &databaseName) {
    if (doesDatabaseExist(conn, databaseName)) Utils::log(Utils::LogLevel::DEBUG, std::cout, "Database `" + databaseName + "` already exists.");
    else {
        pqxx::result R = execCommand(conn, "CREATE DATABASE " + databaseName);
        if (R.empty()) Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to create database: `" + databaseName + "`.");
        else Utils::log(Utils::LogLevel::DEBUG, std::cout, "Database `" + databaseName + "` created.");
    }
}

bool doesUserExist(std::unique_ptr<pqxx::connection> &conn, const std::string &username) {
    std::string query = "SELECT 1 FROM pg_user WHERE usename = '" + username + "'";
    try {
        pqxx::work W(*conn);
        pqxx::result R = W.exec(query);
        W.commit();
        return !R.empty();
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to check if user exists: ", e.what());
        return false;
    }
}

void createUser(std::unique_ptr<pqxx::connection> &conn, const std::string &username, const std::string &password, const std::string &options) {
    if (doesUserExist(conn, username)) Utils::log(Utils::LogLevel::DEBUG, std::cout, "User `" + username + "` already exists.");
    else {
        pqxx::result R = execCommand(conn, "CREATE USER " + username + " WITH PASSWORD '" + password + "' " + options);
        if (R.empty()) Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to create user: `" + username + "`.");
        else Utils::log(Utils::LogLevel::DEBUG, std::cout, "User `" + username + "` created.");
    }
}

bool doesTableExist(std::unique_ptr<pqxx::connection> &conn, const std::string &tableName) {
    std::string query = "SELECT 1 FROM pg_tables WHERE tablename = '" + tableName + "'";
    try {
        pqxx::work W(*conn);
        pqxx::result R = W.exec(query);
        W.commit();
        return !R.empty();
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to check if table exists: ", e.what());
        return false;
    }
}

void createTable(std::unique_ptr<pqxx::connection> &conn, const std::string &tableName, const std::string &columns) {
    if (doesTableExist(conn, tableName)) Utils::log(Utils::LogLevel::DEBUG, std::cout, "Table `" + tableName + "` already exists.");
    else {
        pqxx::result R = execCommand(conn, "CREATE TABLE " + tableName + " (" + columns + ")");
        if (R.empty()) Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to create table: `" + tableName + "`.");
        else Utils::log(Utils::LogLevel::DEBUG, std::cout, "Table `" + tableName + "` created.");
    }
}

// FIXME: this does not work, it returns false even if the function exists
bool doesFunctionExist(std::unique_ptr<pqxx::connection> &conn, const std::string &functionName, const std::vector<std::string> &argTypes) {
    std::string query = "SELECT 1 FROM pg_proc, pg_type WHERE pg_proc.proname = $1 AND pg_proc.proargtypes = ARRAY[";
    for (size_t i = 0; i < argTypes.size(); ++i) {
        query += "(SELECT oid FROM pg_type WHERE typname = $2)";
        if (i < argTypes.size() - 1) query += ",";
    }
    query += "]::oidvector";
    try {
        pqxx::work W(*conn);
        pqxx::result R = W.exec_params(query, functionName, argTypes);
        W.commit();
        return !R.empty();
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to check if function exists: ", e.what());
        return false;
    }
}

// TODO: actually use this once doesFunctionExist is fixed
void createFunction(std::unique_ptr<pqxx::connection> &conn,
                    const std::string &functionName,
                    const std::vector<std::pair<std::string, std::string>> &args,
                    const std::string &returnType,
                    const std::string &body) {
    // Get the argument types as a vector
    std::vector<std::string> argTypes;
    for (const std::pair<std::string, std::string> &arg: args) {
        argTypes.push_back(arg.second);
    }

    // Check if the function already exists
    if (doesFunctionExist(conn, functionName, argTypes)) {
        Utils::log(Utils::LogLevel::DEBUG, std::cout, "Function `" + functionName + "` already exists.");
        return;
    }

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
    try {
        pqxx::work W(*conn);
        W.exec(query);
        W.commit();
        Utils::log(Utils::LogLevel::DEBUG, std::cout, "Function `" + functionName + "` created.");
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to create function: ", e.what());
    }
}

pqxx::result execCommand(std::unique_ptr<pqxx::connection> &conn, const std::string &command) {
    Utils::log(Utils::LogLevel::DEBUG, std::cout, "Executing command: ", command);
    try {
        pqxx::work W(*conn);
        pqxx::result R = W.exec(command);
        W.commit();
        return R;
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, "Failed to execute command: ", e.what());
        return {};
    }
}

void printRows(const pqxx::result &R) {
    if (R.empty()) {
        Utils::log(Utils::LogLevel::TRACE, std::cout, "No results found.");
        return;
    }

    std::vector<std::vector<std::string>> rows;
    std::vector<size_t> maxLengths(R.columns(), 0);

    // Store column names and find maximum length of content in each column
    std::vector<std::string> columnNames;
    for (int i = 0; i < R.columns(); ++i) {
        std::string columnName = R.column_name(i);
        maxLengths[i] = std::max(maxLengths[i], columnName.size());
        columnNames.push_back(std::move(columnName));
    }
    rows.push_back(std::move(columnNames));

    // Store rows and find maximum length of content in each column
    for (const auto &row: R) {
        std::vector<std::string> rowData;
        for (const auto &element: row) {
            std::string value = element.c_str();
            maxLengths[rowData.size()] = std::max(maxLengths[rowData.size()], value.size());
            rowData.emplace_back(std::move(value));
        }
        rows.push_back(std::move(rowData));
    }

    // Calculate total width
    size_t totalWidth = std::accumulate(maxLengths.begin(), maxLengths.end(), maxLengths.size() * 3) - 1; // -1 to align the last column with the +

    // Print top border
    Utils::log(Utils::LogLevel::TRACE, std::cout, "+", std::string(totalWidth, '-'), "+");

    // Print each row with adjusted column widths
    std::ostringstream rowStream;
    for (const auto &rowData: rows) {
        rowStream.str("");
        for (size_t i = 0; i < rowData.size(); ++i) {
            rowStream << "| " << std::setw(maxLengths[i]) << std::left << rowData[i] << " ";
        }
        rowStream << "|";
        Utils::log(Utils::LogLevel::TRACE, std::cout, rowStream.str());

        // Print separator line after column names
        if (&rowData == &rows.front()) Utils::log(Utils::LogLevel::TRACE, std::cout, "+", std::string(totalWidth, '-'), "+");
    }

    // Print bottom border
    Utils::log(Utils::LogLevel::TRACE, std::cout, "+", std::string(totalWidth, '-'), "+");
}


// Sezione porcherie

std::unique_ptr<pqxx::connection> initDatabase() {
    // Connect to the default 'postgres' database as the 'postgres' user
    std::unique_ptr<pqxx::connection> conn = conn2Postgres("postgres", "postgres", "");

    // Create the 'ecommerce' user as `postgres` if it does not exist
    createUser(conn, "ecommerce", "ecommerce", "NOSUPERUSER CREATEDB NOCREATEROLE INHERIT LOGIN NOREPLICATION NOBYPASSRLS");

    // Create the 'customer', 'supplier', and 'transporter' users with fewer permissions
    createUser(conn, "customer", "customer", "NOSUPERUSER NOCREATEDB NOCREATEROLE INHERIT LOGIN NOREPLICATION NOBYPASSRLS");
    createUser(conn, "supplier", "supplier", "NOSUPERUSER NOCREATEDB NOCREATEROLE INHERIT LOGIN NOREPLICATION NOBYPASSRLS");
    createUser(conn, "transporter", "transporter", "NOSUPERUSER NOCREATEDB NOCREATEROLE INHERIT LOGIN NOREPLICATION NOBYPASSRLS");


    // Disconnect from the 'postgres' database as the 'postgres' user
    // Connect to the default 'postgres' database as the 'ecommerce' user
    conn = conn2Postgres("postgres", "ecommerce", "ecommerce");

    // Create the 'ecommerce' database as `ecommerce` if it does not exist
    createDatabase(conn, "ecommerce");

    // Disconnect from the 'postgres' database as the 'ecommerce' user
    // Connect to the 'ecommerce' database as the 'ecommerce' user
    conn = conn2Postgres("ecommerce", "ecommerce", "ecommerce");

    // Create the tables if they do not exist
    initTables(conn);

    // Define SQL functions
    initFunctions(conn);

    // Return the connection to the now ready database
    return conn;
}

void initTables(std::unique_ptr<pqxx::connection> &conn) {

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


    // Grant permissions // TODO: set all perms accordingly
    execCommand(conn, "GRANT SELECT ON customers TO customer");
    execCommand(conn, "GRANT SELECT ON suppliers TO supplier");
    // execCommand(conn, "GRANT SELECT ON suppliers TO customer, supplier, transporter");
    // execCommand(conn, "GRANT SELECT ON transporters TO customer, supplier, transporter");
    execCommand(conn, "GRANT SELECT ON products TO customer, supplier");
    // execCommand(conn, "GRANT SELECT ON orders TO customer, supplier, transporter");
    // execCommand(conn, "GRANT SELECT ON order_items TO customer, supplier, transporter");
}

void initFunctions(std::unique_ptr<pqxx::connection> &conn) { // TODO: check beforehand if the functions already exist and skip them if they do
    // Define SQL functions
    std::string setBalanceProcedureCustomer = R"(
        CREATE OR REPLACE FUNCTION set_balance_customer(customer_id INT, increase_amount INT, add BOOLEAN)
        RETURNS INT AS $$
        DECLARE
            new_balance INT;
        BEGIN
            IF add THEN
                UPDATE customers SET balance = balance + increase_amount WHERE id = customer_id;
            ELSE
                UPDATE customers SET balance = balance - increase_amount WHERE id = customer_id;
            END IF;

            SELECT balance INTO new_balance FROM customers WHERE id = customer_id;
            RETURN new_balance;
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
        RETURNS INT AS $$
        DECLARE
            new_balance INT;
        BEGIN
            IF add THEN
                UPDATE suppliers SET balance = balance + increase_amount WHERE id = supplier_id;
            ELSE
                UPDATE suppliers SET balance = balance - increase_amount WHERE id = supplier_id;
            END IF;

            SELECT balance INTO new_balance FROM suppliers WHERE id = supplier_id;
            RETURN new_balance;
        END;
        $$ LANGUAGE plpgsql SECURITY DEFINER;
    )";
    execCommand(conn, setBalanceProcedureSupplier);

    std::string setBalanceProcedureTransporter = R"(
        CREATE OR REPLACE FUNCTION set_balance_transporter(transporter_id INT, increase_amount INT, add BOOLEAN)
        RETURNS INT AS $$
        DECLARE
            new_balance INT;
        BEGIN
            IF add THEN
                UPDATE transporters SET balance = balance + increase_amount WHERE id = transporter_id;
            ELSE
                UPDATE transporters SET balance = balance - increase_amount WHERE id = transporter_id;
            END IF;

            SELECT balance INTO new_balance FROM transporters WHERE id = transporter_id;
            RETURN new_balance;
        END;
        $$ LANGUAGE plpgsql SECURITY DEFINER;
    )";
    execCommand(conn, setBalanceProcedureTransporter);

    // Grant the EXECUTE permission to the respective users
    execCommand(conn, "GRANT EXECUTE ON FUNCTION set_balance_customer(INT, INT, BOOL) TO customer;");
    execCommand(conn, "GRANT EXECUTE ON FUNCTION set_balance_supplier(INT, INT, BOOL) TO supplier;");
    execCommand(conn, "GRANT EXECUTE ON FUNCTION set_balance_transporter(INT, INT, BOOL) TO transporter;");
}
