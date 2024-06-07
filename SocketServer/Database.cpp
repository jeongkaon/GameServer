#include "stdafx.h"
#include "Database.h"

Database::Database()
{
    // Allocate environment handle
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    // Set the ODBC version environment attribute
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    // Allocate connection handle
    SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
}

void Database::disconnect()
{
    if (hdbc) {
        SQLDisconnect(hdbc);
    }

}

bool Database::connect()
{
    // Set login timeout to 5 seconds
    SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
    // Connect to the DSN (Data Source Name)
    ret = SQLConnect(hdbc, (SQLWCHAR*)L"2024_TF_GS", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        std::cout << "Connected to the database.\n";
        return true;
    }
    else {
        std::cout << "Failed to connect to the database.\n";
        return false;
    }
}

bool Database::CheckIdInDatabase(const char* id)
{
    SQLHSTMT hStmt = NULL;
    SQLRETURN ret;
    bool exists = false;

    // Allocate statement handle
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hStmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        // Create SQL query to check if the ID exists
        std::string tableName = "user_id";
        std::string query = "SELECT 1 FROM " + tableName + " WHERE id = '" + id + "'";

        // Execute SQL query
        ret = SQLExecDirect(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            // Check if any rows are returned
            if (SQLFetch(hStmt) == SQL_SUCCESS) {
                exists = true;
            }
        }
        else {
            std::cout << "Failed to execute SQL query.\n";
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }

    return exists;
}



Database::~Database()
{
    disconnect();
    if (hdbc) SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    if (henv) SQLFreeHandle(SQL_HANDLE_ENV, henv);
}
