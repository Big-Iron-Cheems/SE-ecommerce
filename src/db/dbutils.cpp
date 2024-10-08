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

std::shared_ptr<pqxx::connection> conn2Postgres(const std::string &dbname, const std::string &user, const std::string &password) {
    return PostgresConnectionPool::getInstance().getConnection(dbname, user, password);
}

bool doesDatabaseExist(std::shared_ptr<pqxx::connection> &conn, const std::string &databaseName) {
    std::string query = std::format("SELECT 1 FROM pg_database WHERE datname = '{}'", databaseName);
    try {
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();
        return !R.empty();
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to check if database exists: {}", e.what()));
        return false;
    }
}

void createDatabase(std::shared_ptr<pqxx::connection> &conn, const std::string &databaseName) {
    if (doesDatabaseExist(conn, databaseName)) Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("Database `{}` already exists.", databaseName));
    else {
        try {
            pqxx::nontransaction ntx(*conn);
            ntx.exec(std::format("CREATE DATABASE {}", databaseName));
            Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("Database `{}` created.", databaseName));
        } catch (const std::exception &e) {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to create database: `{}`. Error: {}", databaseName, e.what()));
        }
    }
}

bool doesUserExist(std::shared_ptr<pqxx::connection> &conn, const std::string &username) {
    std::string query = std::format("SELECT 1 FROM pg_user WHERE usename = '{}'", username);
    try {
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();
        return !R.empty();
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to check if user exists: {}", e.what()));
        return false;
    }
}

void createUser(std::shared_ptr<pqxx::connection> &conn, const std::string &username, const std::string &password, const std::string &options) {
    if (doesUserExist(conn, username)) Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("User `{}` already exists.", username));
    else {
        try {
            execCommand(conn, std::format("CREATE USER {} WITH PASSWORD {} {}", username, conn->quote(password), options));
            Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("User `{}` created.", username));
        } catch (const std::exception &e) {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to create user: `{}`. Error: {}", username, e.what()));
        }
    }
}

bool doesTypeExist(std::shared_ptr<pqxx::connection> &conn, const std::string &typeName) {
    std::string query = std::format("SELECT 1 FROM pg_type WHERE typname = {}", conn->quote(typeName));
    try {
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();
        return !R.empty();
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to check if type exists: {}", e.what()));
        return false;
    }
}

void createType(std::shared_ptr<pqxx::connection> &conn, const std::string &typeName, const std::string &typeDef) {
    // Check if the type already exists
    if (doesTypeExist(conn, typeName)) {
        Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("Type `{}` already exists.", typeName));
        return;
    }

    // Build the query
    std::string query = std::format("CREATE TYPE {} AS {};", typeName, typeDef);

    // Execute the query
    try {
        pqxx::work tx(*conn);
        tx.exec(query);
        tx.commit();
        Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("Type `{}` created.", typeName));
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to create type: {}", e.what()));
    }
}

bool doesTableExist(std::shared_ptr<pqxx::connection> &conn, const std::string &tableName) {
    std::string query = std::format("SELECT 1 FROM pg_tables WHERE tablename = {}", conn->quote(tableName));
    try {
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(query);
        tx.commit();
        return !R.empty();
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to check if table exists: {}", e.what()));
        return false;
    }
}

void createTable(std::shared_ptr<pqxx::connection> &conn, const std::string &tableName, const std::string &columns) {
    if (doesTableExist(conn, tableName)) Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("Table `{}` already exists.", tableName));
    else {
        try {
            pqxx::work tx(*conn);
            tx.exec(std::format("CREATE TABLE {} ({})", tableName, columns));
            tx.commit();
            Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("Table `{}` created", tableName));
        } catch (const std::exception &e) {
            Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to create table: `{}`. Error: {}", tableName, e.what()));
        }
    }
}

bool doesFunctionExist(std::shared_ptr<pqxx::connection> &conn, const std::string &functionName, const std::vector<std::string> &argTypes) {
    std::string query = "SELECT to_regprocedure($1)";
    std::string functionSignature = functionName + "(";

    for (size_t i = 0; i < argTypes.size(); ++i) {
        functionSignature += argTypes[i];
        if (i < argTypes.size() - 1) functionSignature += ", ";
    }
    functionSignature += ")";

    try {
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec_params(query, functionSignature);
        tx.commit();
        return !R[0][0].is_null();
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to check if function exists: {}", e.what()));
        return false;
    }
}

void createFunction(std::shared_ptr<pqxx::connection> &conn,
                    const std::string &functionName,
                    const std::vector<std::pair<std::string, std::string>> &args,
                    const std::string &returnType,
                    const std::string &body) {
    // Get the argument types as a vector
    std::vector<std::string> argTypes(args.size());
    std::transform(args.begin(), args.end(), argTypes.begin(), [](const auto &arg) { return arg.second; });

    // Check if the function already exists
    if (doesFunctionExist(conn, functionName, argTypes)) {
        Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("Function `{}` already exists.", functionName));
        return;
    }

    // Build the query
    std::string query = std::format("CREATE OR REPLACE FUNCTION {}(", functionName);
    for (size_t i = 0; i < args.size(); ++i) {
        query += std::format("{} {}", args[i].first, args[i].second);
        if (i < args.size() - 1) query += ", ";
    }
    query += std::format(") RETURNS {} AS $${}\n$$ LANGUAGE plpgsql SECURITY DEFINER;", returnType, body);

    // Execute the query
    try {
        pqxx::work tx(*conn);
        tx.exec(query);
        tx.commit();
        Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("Function `{}` created.", functionName));
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to create function: {}", e.what()));
    }
}

pqxx::result execCommand(std::shared_ptr<pqxx::connection> &conn, const std::string &command) {
    Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("Executing: {}", command));
    try {
        pqxx::work tx(*conn);
        pqxx::result R = tx.exec(command);
        tx.commit();
        return R;
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to execute command: {}", e.what()));
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
            auto value = element.as<std::string>();
            maxLengths[rowData.size()] = std::max(maxLengths[rowData.size()], value.size());
            rowData.emplace_back(std::move(value));
        }
        rows.push_back(std::move(rowData));
    }

    // Calculate total width
    size_t totalWidth = std::accumulate(maxLengths.begin(), maxLengths.end(), maxLengths.size() * 3) - 1; // -1 to align the last column with the +

    // Insert top border, rows and bottom border
    std::ostringstream oss;
    oss << "\n+" << std::string(totalWidth, '-') << "+\n";
    for (const auto &rowData: rows) {
        for (size_t i = 0; i < rowData.size(); ++i) {
            oss << "| " << std::setw(static_cast<int>(maxLengths[i])) << std::left << rowData[i] << " ";
        }
        oss << "|\n";
        if (&rowData == &rows.front()) oss << "+" << std::string(totalWidth, '-') << "+\n";
    }
    oss << "+" << std::string(totalWidth, '-') << "+";

    Utils::log(Utils::LogLevel::TRACE, std::cout, oss.str());
}

// Init functions, required to set up the database

void initDatabase() {
    // Connect to the default 'postgres' database as the 'postgres' user
    auto conn = conn2Postgres("postgres", "postgres", "");

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

    // Define types
    initTypes(conn);

    // Create the tables if they do not exist
    initTables(conn);

    // Define SQL functions
    initFunctions(conn);
}

void initTypes(std::shared_ptr<pqxx::connection> &conn) {
    createType(conn, "user_role", "ENUM ('customer', 'supplier', 'transporter')");
    createType(conn, "order_status", "ENUM ('shipped', 'delivered', 'cancelled')");
}

void initTables(std::shared_ptr<pqxx::connection> &conn) {
    // Seen only by the admins
    createTable(conn, "customers", "id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE NOT NULL, balance INT NOT NULL, logged_in BOOL NOT NULL DEFAULT FALSE");
    createTable(conn, "suppliers", "id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE NOT NULL, balance INT NOT NULL, logged_in BOOL NOT NULL DEFAULT FALSE");
    createTable(conn, "transporters", "id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE NOT NULL, balance INT NOT NULL, logged_in BOOL NOT NULL DEFAULT FALSE");
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
    )"); ///< Products offered by suppliers
    createTable(conn, "orders", R"(
            id SERIAL PRIMARY KEY,
            customer_id INT NOT NULL,
            total_price INT NOT NULL,
            transporter_id INT NOT NULL,
            status order_status NOT NULL,
            address VARCHAR(255) NOT NULL,
            timestamp TIMESTAMP NOT NULL,
            FOREIGN KEY (customer_id) REFERENCES customers(id),
            FOREIGN KEY (transporter_id) REFERENCES transporters(id)
    )"); ///< Orders placed by customers
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
    )"); ///< Products listed in an order

    // Grant permissions
    execCommand(conn, "GRANT SELECT ON products TO customer, supplier");
    execCommand(conn, "GRANT SELECT ON orders TO customer, supplier, transporter");
    execCommand(conn, "GRANT SELECT ON order_items TO supplier, transporter");
}

void initFunctions(std::shared_ptr<pqxx::connection> &conn) {
    createFunction(conn, "get_target_table", {{"user_type", "user_role"}}, "VARCHAR(255)", R"(
    DECLARE
        target_table VARCHAR(255);
    BEGIN
        -- Determine the target table based on user type
        CASE user_type
            WHEN 'customer' THEN
                target_table := 'customers';
            WHEN 'supplier' THEN
                target_table := 'suppliers';
            WHEN 'transporter' THEN
                target_table := 'transporters';
            ELSE
                RAISE EXCEPTION 'Invalid user type';
        END CASE;

        RETURN target_table;
    END;)"); ///< Determine the target table based on user type
    createFunction(conn, "check_user", {{"user_type", "user_role"}, {"username", "VARCHAR"}}, "TABLE(id INT, balance INT, logged_in BOOL)", R"(
    BEGIN
        -- Check if the user exists
        RETURN QUERY EXECUTE format('SELECT id, balance, logged_in FROM %I WHERE username = $1', get_target_table(user_type))
        USING username;
    END;)"); ///< Check if the user exists
    createFunction(conn, "insert_user", {{"user_type", "user_role"}, {"username", "VARCHAR"}}, "INT", R"(
    DECLARE
        new_id INT;
    BEGIN
        -- Insert a new user into the appropriate table
        EXECUTE format('INSERT INTO %I (username, balance, logged_in) VALUES ($1, 0, true) RETURNING id', get_target_table(user_type))
        INTO new_id
        USING username;

        RETURN new_id;
    END;)"); ///< Insert a new user into the appropriate table
    createFunction(conn, "set_logged_in", {{"user_type", "user_role"}, {"user_id", "INT"}, {"is_logged_in", "BOOL"}}, "VOID", R"(
    BEGIN
        -- Update the logged_in field in the appropriate table
        EXECUTE format('UPDATE %I SET logged_in = $1 WHERE id = $2', get_target_table(user_type))
        USING is_logged_in, user_id;
    END;)"); ///< Update the logged_in field in the appropriate table
    createFunction(conn, "get_balance", {{"user_type", "user_role"}, {"user_id", "INT"}}, "INT", R"(
    DECLARE
        balance INT;
    BEGIN
        -- Retrieve the balance from the appropriate table
        EXECUTE format('SELECT balance FROM %I WHERE id = $1', get_target_table(user_type))
        INTO balance
        USING user_id;

        RETURN balance;
    END;)"); ///< Retrieve the balance from the appropriate table
    createFunction(conn, "set_balance", {{"user_type", "user_role"}, {"user_id", "INT"}, {"amount", "INT"}}, "INT", R"(
    DECLARE
        new_balance INT;
        operation CHAR;
    BEGIN
        -- Determine the operation based on the sign of the amount
        IF amount > 0 THEN
            operation := '+';
        ELSIF amount < 0 THEN
            operation := '-';
        ELSE
            -- If amount is zero, return the current balance
            EXECUTE format('SELECT balance FROM %I WHERE id = $2', get_target_table(user_type))
            INTO new_balance
            USING user_id;
            RETURN new_balance;
        END IF;

        -- Calculate the new balance
        EXECUTE format('SELECT balance %s $1 FROM %I WHERE id = $2', operation, get_target_table(user_type))
        INTO new_balance
        USING ABS(amount), user_id;  -- Use the absolute value of amount

        -- Check if the new balance would be negative
        IF new_balance < 0 THEN
            RAISE EXCEPTION 'New balance would be negative';
        END IF;

        -- Update the balance in the appropriate table and retrieve the new balance
        EXECUTE format('UPDATE %I SET balance = balance %s $1 WHERE id = $2 RETURNING balance', get_target_table(user_type), operation)
        INTO new_balance
        USING ABS(amount), user_id;  -- Use the absolute value of amount

        RETURN new_balance;
    END;)"); ///< Update the balance in the appropriate table and retrieve the new balance

    // Customers
    createFunction(conn, "make_order", {{"customer_id", "INT"}, {"total_price", "INT"}, {"address", "VARCHAR(255)"}}, "INT", R"(
    DECLARE
        new_order_id INT;
    BEGIN
        -- Insert a new order into the orders table and return its id
        INSERT INTO orders (customer_id, total_price, transporter_id, status, address, timestamp)
        VALUES ($1, $2, (SELECT id FROM transporters ORDER BY RANDOM() LIMIT 1), 'shipped', $3, NOW())
        RETURNING id INTO new_order_id;

        RETURN new_order_id;
    END;)"); ///< Make an order from the products in the cart
    createFunction(conn, "add_order_item", {{"order_id", "INT"}, {"product_id", "INT"}, {"quantity", "INT"}, {"price", "INT"}, {"supplier_id", "INT"}}, "VOID", R"(
    BEGIN
        -- Insert a new product into the order_items table
        INSERT INTO order_items (order_id, product_id, quantity, price, supplier_id)
        VALUES ($1, $2, $3, $4, $5);

        -- Update the products table to decrement the stock
        UPDATE products
        SET amount = amount - $3
        WHERE id = $2;
    END;)"); ///< Add a product to the order_items table


    // Suppliers
    createFunction(conn, "add_product", {{"name", "VARCHAR(255)"}, {"supplier_id", "INT"}, {"price", "INT"}, {"amount", "INT"}, {"description", "VARCHAR(255)"}}, "INT", R"(
    DECLARE
        new_id INT;
    BEGIN
        -- Insert a new product into the products table and return its id
        INSERT INTO products (name, supplier_id, price, amount, description)
        VALUES ($1, $2, $3, $4, $5)
        RETURNING id INTO new_id;
        RETURN new_id;
    END;)"); ///< Add a product to the supplier's catalog
    createFunction(conn, "remove_product", {{"product_id", "INT"}}, "INT", R"(
    DECLARE
        removed_id INT;
    BEGIN
        -- Check if the product exists
        SELECT id INTO removed_id FROM products WHERE id = $1;

        -- If the product does not exist, return 0
        IF removed_id IS NULL THEN
            RETURN 0;
        END IF;

        -- Set the amount of the product to -1 to mark it as removed
        UPDATE products
        SET amount = -1
        WHERE id = $1;

        RETURN removed_id;
    END;)"); ///< Remove a product from the supplier's catalog
    createFunction(conn, "edit_product", {{"product_id", "INT"}, {"new_name", "VARCHAR(255)"}, {"new_price", "INT"}, {"new_amount", "INT"}, {"new_description", "VARCHAR(255)"}},
                   "INT", R"(
    DECLARE
        edited_id INT;
    BEGIN
        -- Check if the product exists
        SELECT id INTO edited_id FROM products WHERE id = $1;

        -- If the product does not exist, return 0
        IF edited_id IS NULL THEN
            RETURN 0;
        END IF;

        -- Update the product in the products table and return its id
        UPDATE products
        SET name = COALESCE(new_name, products.name),
            price = COALESCE(new_price, products.price),
            amount = COALESCE(new_amount, products.amount),
            description = COALESCE(new_description, products.description)
        WHERE id = product_id
        RETURNING id INTO edited_id;

        RETURN edited_id;
    END;)"); ///< Edit a product from the supplier's catalog

    // Transporters
    createFunction(conn, "get_ongoing_orders", {{"transporter_id", "INT"}}, "TABLE(order_id INT, customer_username VARCHAR(255), address VARCHAR(255))", R"(
    BEGIN
        -- Retrieve the ongoing orders
        RETURN QUERY SELECT o.id, c.username, o.address
        FROM orders o
        JOIN customers c ON o.customer_id = c.id
        WHERE o.transporter_id = $1 AND o.status = 'shipped';
    END;)"); ///< Get the ongoing orders
    createFunction(conn, "set_order_status", {{"user_type", "user_role"}, {"user_id", "INT"}, {"order_id", "INT"}, {"new_status", "order_status"}}, "VOID", R"(
    DECLARE
        current_status order_status;
    BEGIN
        -- Check if the order exists and is handled by the given transporter
        IF user_type = 'transporter' THEN
            SELECT o.status INTO current_status FROM orders o WHERE o.id = order_id AND o.transporter_id = user_id;
        ELSE
            SELECT o.status INTO current_status FROM orders o WHERE o.id = order_id AND o.customer_id = user_id;
        END IF;

        IF NOT FOUND THEN
            RAISE EXCEPTION 'Order does not exist or is not handled by this transporter.';
        END IF;

        -- Status validation
        IF current_status = 'delivered' OR current_status = 'cancelled' THEN
            RAISE EXCEPTION 'Cannot change status from %', current_status;
        END IF;

        -- Update the order status
        UPDATE orders SET status = new_status WHERE id = order_id;
    END;)"); ///< Set the status of an order

    // Grant the EXECUTE permission to the respective users
    execCommand(conn, "GRANT EXECUTE ON FUNCTION check_user(user_role, VARCHAR) TO customer, supplier, transporter;");
    execCommand(conn, "GRANT EXECUTE ON FUNCTION insert_user(user_role, VARCHAR) TO customer, supplier, transporter;");
    execCommand(conn, "GRANT EXECUTE ON FUNCTION set_logged_in(user_role, INT, BOOL) TO customer, supplier, transporter;");
    execCommand(conn, "GRANT EXECUTE ON FUNCTION get_balance(user_role, INT) TO customer, supplier, transporter;");
    execCommand(conn, "GRANT EXECUTE ON FUNCTION set_balance(user_role, INT, INT) TO customer, supplier, transporter;");

    execCommand(conn, "GRANT EXECUTE ON FUNCTION make_order(INT, INT, VARCHAR(255)) TO customer;");
    execCommand(conn, "GRANT EXECUTE ON FUNCTION add_order_item(INT, INT, INT, INT, INT) TO customer;");

    execCommand(conn, "GRANT EXECUTE ON FUNCTION add_product(VARCHAR(255), INT, INT, INT, VARCHAR(255)) TO supplier;");
    execCommand(conn, "GRANT EXECUTE ON FUNCTION remove_product(INT) TO supplier;");
    execCommand(conn, "GRANT EXECUTE ON FUNCTION edit_product(INT, VARCHAR(255), INT, INT, VARCHAR(255)) TO supplier;");

    execCommand(conn, "GRANT EXECUTE ON FUNCTION get_ongoing_orders(INT) TO transporter;");
    execCommand(conn, "GRANT EXECUTE ON FUNCTION set_order_status(user_role, INT, INT, order_status) TO customer, transporter;");
}

void dropDatabase() {
    // Connect to the default 'postgres' database as the 'postgres' user
    auto conn = conn2Postgres("postgres", "postgres", "");

    // Drop the 'ecommerce' database if it exists
    try {
        pqxx::nontransaction ntx(*conn);
        ntx.exec("DROP DATABASE IF EXISTS ecommerce");
        Utils::log(Utils::LogLevel::DEBUG, std::cout, std::format("Database `ecommerce` dropped."));
    } catch (const std::exception &e) {
        Utils::log(Utils::LogLevel::ERROR, std::cerr, std::format("Failed to drop database: `{}`. Error: {}", "ecommerce", e.what()));
    }

    // Drop the 'ecommerce' related users
    execCommand(conn, "DROP USER IF EXISTS ecommerce");
    execCommand(conn, "DROP USER IF EXISTS customer");
    execCommand(conn, "DROP USER IF EXISTS supplier");
    execCommand(conn, "DROP USER IF EXISTS transporter");
}
