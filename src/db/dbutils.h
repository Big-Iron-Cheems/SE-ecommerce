#ifndef ECOMMERCE_DBUTILS_H
#define ECOMMERCE_DBUTILS_H

#include <libpq-fe.h>
#include <cstring>
#include <string>
#include <iostream>

/**
 * Connect to PostgreSQL
 * @return a pointer to the connection object
 */
PGconn *conn2Postgres();

/**
 * Check if a user exists in PostgreSQL
 * @param conn a pointer to the connection object
 * @param username the username to check
 * @return
 */
bool doesUserExist(PGconn *conn, const std::string &username);

/**
 * Create a new user in PostgreSQL
 * @param conn a pointer to the connection object
 * @param username the username to create
 * @param password the password to use
 */
void createUser(PGconn *conn, const std::string& username, const std::string &password);

/**
 * Check if a table exists in PostgreSQL
 * @param conn a pointer to the connection object
 * @param tableName the name of the table to check
 * @return true if the table exists, false otherwise
 */
bool doesTableExist(PGconn *conn, const std::string &tableName);

/**
 * Create a new table in PostgreSQL with the given columns
 * @param conn a pointer to the connection object
 * @param tableName the name of the table to create
 * @param columns the columns to use
 */
void createTable(PGconn *conn, const std::string &tableName, const std::string &columns);


/**
 * Execute a command in PostgreSQL
 * @param conn a pointer to the connection object
 * @param command the command to execute
 * @return the result of the command or nullptr if an error occurred
 */
PGresult * execCommand(PGconn *conn, const std::string &command);

/**
 * Initialize the tables in PostgreSQL
 * @param conn a pointer to the connection object
 */
void initTables(PGconn *conn);

#endif //ECOMMERCE_DBUTILS_H
