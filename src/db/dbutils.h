#pragma once

#include "../Utils.h"
#include "PostgresConnectionPool.h"
#include <pqxx/pqxx>
#include <vector>

/**
 * Connect to a PostgreSQL database
 * @param dbname the name of the database to connect to
 * @param user the username to use
 * @param password the password to use
 * @return a pointer to the connection object
 */
std::shared_ptr<pqxx::connection> conn2Postgres(const std::string &dbname, const std::string &user, const std::string &password);

/**
 * Check if a database exists in PostgreSQL
 * @param conn a pointer to the connection object
 * @param databaseName the name of the database to check
 * @return true if the database exists, false otherwise
 */
bool doesDatabaseExist(std::shared_ptr<pqxx::connection> &conn, const std::string &databaseName);

/**
 * Create a new database in PostgreSQL
 * @param conn a pointer to the connection object
 * @param databaseName the name of the database to create
 */
void createDatabase(std::shared_ptr<pqxx::connection> &conn, const std::string &databaseName);

/**
 * Check if a user exists in PostgreSQL
 * @param conn a pointer to the connection object
 * @param username the username to check
 * @return true if the user exists, false otherwise
 */
bool doesUserExist(std::shared_ptr<pqxx::connection> &conn, const std::string &username);

/**
 * Create a new user in PostgreSQL
 * @param conn a pointer to the connection object
 * @param username the username to create
 * @param password the password to use
 */
void createUser(std::shared_ptr<pqxx::connection> &conn, const std::string &username, const std::string &password, const std::string &options);

/**
 * Check if a type exists in PostgreSQL
 * @param conn a pointer to the connection object
 * @param typeName the name of the type to check
 * @return true if the type exists, false otherwise
 */
bool doesTypeExist(std::shared_ptr<pqxx::connection> &conn, const std::string &typeName);

/**
 * Create a new type in PostgreSQL
 * @param conn a pointer to the connection object
 * @param typeName the name of the type to create
 * @param definition the definition of the type
 */
void createType(std::shared_ptr<pqxx::connection> &conn, const std::string &typeName, const std::string &definition);

/**
 * Check if a table exists in PostgreSQL
 * @param conn a pointer to the connection object
 * @param tableName the name of the table to check
 * @return true if the table exists, false otherwise
 */
bool doesTableExist(std::shared_ptr<pqxx::connection> &conn, const std::string &tableName);

/**
 * Create a new table in PostgreSQL with the given columns
 * @param conn a pointer to the connection object
 * @param tableName the name of the table to create
 * @param columns the columns to use
 */
void createTable(std::shared_ptr<pqxx::connection> &conn, const std::string &tableName, const std::string &columns);

/**
 * Check if a function exists in PostgreSQL
 * @param conn a pointer to the connection object
 * @param functionName the name of the function to check
 * @param argTypes the types of the arguments of the function
 * @return true if the function exists, false otherwise
 */
bool doesFunctionExist(std::shared_ptr<pqxx::connection> &conn, const std::string &functionName, const std::vector<std::string> &argTypes);

/**
 * Create a new function in PostgreSQL
 * @param conn a pointer to the connection object
 * @param functionName the name of the function to create
 * @param args the arguments of the function, as a vector of pairs of the argument name and type
 * @param returnType the return type of the function
 * @param body the body of the function
 */
void createFunction(std::shared_ptr<pqxx::connection> &conn,
                    const std::string &functionName,
                    const std::vector<std::pair<std::string, std::string>> &args,
                    const std::string &returnType,
                    const std::string &body);

/**
 * Execute a command in PostgreSQL
 * @param conn a pointer to the connection object
 * @param command the command to execute
 * @return the result of the command or nullptr if an error occurred
 */
pqxx::result execCommand(std::shared_ptr<pqxx::connection> &conn, const std::string &command);

/**
 * Pretty-print the rows of a result, aligning the columns
 * @param R the result to print
 */
void printRows(const pqxx::result &R);

/**
 * Initialize the ecommerce database
 * @return a pointer to the connection object
 * @throws std::runtime_error if the database connection fails
 */
void initDatabase();

/**
 * Initialize the types in PostgreSQL
 * @param conn a pointer to the connection object
 */
void initTypes(std::shared_ptr<pqxx::connection> &conn);

/**
 * Initialize the tables in PostgreSQL
 * @param conn a pointer to the connection object
 */
void initTables(std::shared_ptr<pqxx::connection> &conn);

/**
 * Initialize the functions in PostgreSQL
 * @param conn a pointer to the connection object
 */
void initFunctions(std::shared_ptr<pqxx::connection> &conn);

/**
 * Drop the ecommerce database and related objects
 * Used for testing purposes
 */
void dropDatabase();
