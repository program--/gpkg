#pragma once

#include "sqlite/sqlite_iter.hpp"

/**
 * @brief Wrapper around SQLite3 Databases
 */
class sqlite
{
  private:
    sqlite_t conn = nullptr;
    stmt_t   stmt = nullptr;

  public:
    sqlite() = default;

    /**
     * @brief Construct a new sqlite object from a path to database
     *
     * @param path File path to sqlite3 database
     */
    sqlite(const std::string& path);

    sqlite(sqlite& db);
    sqlite& operator=(sqlite& db);

    /**
     * @brief Take ownership of a sqlite3 database
     *
     * @param db sqlite3 database object
     */
    sqlite(sqlite&& db);

    /**
     * @brief Move assignment operator
     *
     * @param db sqlite3 database object
     * @return sqlite& reference to sqlite3 database
     */
    sqlite& operator=(sqlite&& db);

    /**
     * @brief Destroy the sqlite object
     */
    ~sqlite();

    /**
     * @brief Return the originating sqlite3 database pointer
     *
     * @return sqlite3*
     */
    sqlite3* connection() const noexcept;

    /**
     * @brief Check if SQLite database contains a given table
     *
     * @param table name of table
     * @return true if table does exist
     * @return false if table does not exist
     */
    bool has_table(const std::string& table) noexcept;

    /**
     * Query the SQLite Database and get the result
     * @param statement String query
     * @return read-only SQLite row iterator (see: [sqlite_iter])
     */
    sqlite_iter* query(const std::string& statement);

    /**
     * @brief TODO!!!
     *
     * @param statement
     * @param params
     * @return sqlite_iter*
     */
    template<typename... T>
    sqlite_iter* query(const std::string& statement, T const&... params);
};

inline sqlite::sqlite(const std::string& path)
{
    sqlite3* conn;
    int      code = sqlite3_open_v2(path.c_str(), &conn, SQLITE_OPEN_READONLY, NULL);
    if (code != SQLITE_OK) {
        throw sqlite_error("sqlite3_open_v2", code);
    }
    this->conn = sqlite_t(conn);
}

inline sqlite::sqlite(sqlite& db)
{
    this->conn = std::move(db.conn);
    this->stmt = db.stmt;
}

inline sqlite& sqlite::operator=(sqlite& db)
{
    this->conn = std::move(db.conn);
    this->stmt = db.stmt;
    return *this;
}

inline sqlite::sqlite(sqlite&& db)
{
    this->conn = std::move(db.conn);
    this->stmt = db.stmt;
}

inline sqlite& sqlite::operator=(sqlite&& db)
{
    this->conn = std::move(db.conn);
    this->stmt = db.stmt;
    return *this;
}

inline sqlite::~sqlite()
{
    this->stmt.reset();
    this->conn.reset();
}

inline sqlite3* sqlite::connection() const noexcept
{
    return this->conn.get();
}

inline bool sqlite::has_table(const std::string& table) noexcept
{
    const auto q = this->query("SELECT 1 from sqlite_master WHERE type='table' AND name = ?", table);
    q->next();
    return static_cast<bool>(q->get<int>(0));
};

inline sqlite_iter* sqlite::query(const std::string& statement)
{
    sqlite3_stmt* stmt;
    const auto    cstmt = statement.c_str();
    const int     code  = sqlite3_prepare_v2(this->connection(), cstmt, statement.length() + 1, &stmt, NULL);

    if (code != SQLITE_OK) {
        // something happened, can probably switch on result codes
        // https://www.sqlite.org/rescode.html
        throw sqlite_error("sqlite3_prepare_v2", code);
    }

    this->stmt        = stmt_t(stmt, sqlite_deleter{});
    sqlite_iter* iter = new sqlite_iter(this->stmt);
    return iter;
}

template<typename... T>
inline sqlite_iter* sqlite::query(const std::string& statement, T const&... params)
{
    sqlite3_stmt* stmt;
    const auto    cstmt = statement.c_str();
    const int     code  = sqlite3_prepare_v2(this->connection(), cstmt, statement.length() + 1, &stmt, NULL);

    if (code != SQLITE_OK) {
        throw sqlite_error("sqlite3_prepare_v2", code);
    }

    std::vector<std::string> binds{ { params... } };
    for (size_t i = 0; i < binds.size(); i++) {
        sqlite3_bind_text(stmt, i + 1, binds[i].c_str(), -1, SQLITE_STATIC);
    }

    this->stmt        = stmt_t(stmt, sqlite_deleter{});
    sqlite_iter* iter = new sqlite_iter(this->stmt);
    return iter;
}