#pragma once

extern "C"
{
#include <sqlite3.h>
}

#include <iterator>
#include <string>
#include <stdexcept>
#include <vector>
#include <functional>
#include <memory>

struct sqlite_deleter
{
    void operator()(sqlite3* db) { sqlite3_close_v2(db); }
    void operator()(sqlite3_stmt* stmt) { sqlite3_finalize(stmt); }
};

using sqlite_t = std::unique_ptr<sqlite3, sqlite_deleter>;
using stmt_t   = std::shared_ptr<sqlite3_stmt>;

class sqlite_iter
{
  private:
    stmt_t        stmt     = nullptr;
    bool          finished = false;

    sqlite3_stmt* ptr() { return this->stmt.get(); }

  public:
    sqlite_iter(stmt_t stmt)
      : stmt(stmt){};

    ~sqlite_iter() { this->stmt.reset(); }

    // ITERATION
    bool         done() { return this->finished; }
    sqlite_iter& next()
    {
        if (!this->done()) {
            int returncode = sqlite3_step(this->ptr());
            if (returncode == SQLITE_DONE) {
                this->finished = true;
            }
        }

        return *this;
    }

    sqlite_iter& reset()
    {
        sqlite3_reset(this->ptr());
        return *this;
    }

    sqlite_iter& operator++() { return this->next(); }
    sqlite_iter& operator++(int) { return this->next(); }

    // ACCESS
    template<typename T, int col>
    T get();

    template<int col>
    std::vector<uint8_t> get()
    {
        return *reinterpret_cast<const std::vector<uint8_t>*>(sqlite3_column_blob(this->ptr(), col));
    }

    template<int col>
    double get()
    {
        return sqlite3_column_double(this->ptr(), col);
    };

    template<int col>
    int get()
    {
        return sqlite3_column_int(this->ptr(), col);
    };

    template<int col>
    std::string get()
    {
        // TODO: this won't work with non-ASCII text
        return std::string(reinterpret_cast<const char*>(sqlite3_column_text(this->ptr(), col)));
    };
};

class sqlite
{
  private:
    sqlite_t conn = nullptr;
    stmt_t   stmt = nullptr;

  public:
    sqlite(const std::string& path)
    {
        sqlite3* conn;
        sqlite3_open_v2(path.c_str(), &conn, SQLITE_OPEN_READONLY, NULL);
        this->conn = sqlite_t(conn);
    }

    ~sqlite()
    {
        this->stmt.reset();
        this->conn.reset();
    }

    sqlite3* connection() const noexcept { return this->conn.get(); }

    /**
     * Query the SQLite Database and get the result
     * @param statement String query
     * @return read-only SQLite row iterator (see: [sqlite_iter])
     */
    const sqlite_iter* query(const std::string& statement)
    {
        const auto    cstmt = statement.c_str();
        sqlite3_stmt* stmt;
        int           code = sqlite3_prepare_v2(this->connection(), cstmt, sizeof cstmt, &stmt, NULL);

        if (code != SQLITE_OK) {
            // something happened, can probably switch on result codes
            // https://www.sqlite.org/rescode.html
            auto errmsg = std::string("sqlite3_prepare_v2 returned code ") + std::to_string(code);
            throw std::runtime_error(errmsg);
        }

        this->stmt              = stmt_t(stmt, sqlite_deleter{});
        const sqlite_iter* iter = new sqlite_iter(this->stmt);
        return iter;
    }
};