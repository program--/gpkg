#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include <sqlite3.h>

template<typename T, typename U>
using enabled_t = std::enable_if_t<std::is_same<T, U>::value, U>;

struct sqlite_deleter
{
    void operator()(sqlite3* db) { sqlite3_close_v2(db); }
    void operator()(sqlite3_stmt* stmt) { sqlite3_finalize(stmt); }
};

using sqlite_t = std::unique_ptr<sqlite3, sqlite_deleter>;
using stmt_t   = std::shared_ptr<sqlite3_stmt>;

inline std::runtime_error sqlite_error(std::string f, int code)
{
    std::string errmsg = f + " returned code " + std::to_string(code);
    return std::runtime_error(errmsg);
}

class sqlite_iter
{
  private:
    stmt_t stmt;
    int    iteration_step     = -1;
    bool   iteration_finished = false;

    // column metadata
    int                      column_count = 0;
    std::vector<std::string> column_names;

    // returns the raw pointer to the sqlite statement
    sqlite3_stmt* ptr() const noexcept { return this->stmt.get(); }

  public:
    sqlite_iter(stmt_t stmt)
      : stmt(stmt)
    {
        this->column_count = sqlite3_column_count(this->ptr());
        this->column_names = std::vector<std::string>();
        this->column_names.reserve(this->column_count);
        for (int i = 0; i < this->column_count; i++) {
            this->column_names.push_back(sqlite3_column_name(this->ptr(), i));
        }
    };

    ~sqlite_iter() { this->stmt.reset(); }

    // ITERATION
    bool         done() { return this->iteration_finished; }

    sqlite_iter& next()
    {
        if (!this->done()) {
            const int returncode = sqlite3_step(this->ptr());
            if (returncode == SQLITE_DONE) {
                this->iteration_finished = true;
            }
            this->iteration_step++;
        }

        return *this;
    }

    sqlite_iter& reset()
    {
        sqlite3_reset(this->ptr());
        this->iteration_step = -1;
        return *this;
    }

    int current_row() const noexcept { return this->iteration_step; }

    int num_columns() const noexcept { return this->column_count; }

    int column_index(const std::string& name) const noexcept
    {
        const ptrdiff_t pos = std::distance(
          this->column_names.begin(), std::find(this->column_names.begin(), this->column_names.end(), name)
        );

        return pos >= this->column_names.size() ? -1 : pos;
    }

    //
    const std::vector<std::string>& columns() const noexcept { return this->column_names; }

    // ACCESS

    template<typename T>
    inline enabled_t<T, std::vector<uint8_t>> get(int col) const
    {
        return *reinterpret_cast<const std::vector<uint8_t>*>(sqlite3_column_blob(this->ptr(), col));
    }

    template<typename T>
    inline enabled_t<T, double> get(int col) const
    {
        return sqlite3_column_double(this->ptr(), col);
    };

    template<typename T>
    inline enabled_t<T, int> get(int col) const
    {
        return sqlite3_column_int(this->ptr(), col);
    };

    template<typename T>
    inline enabled_t<T, std::string> get(int col) const
    {
        // TODO: this won't work with non-ASCII text
        return std::string(reinterpret_cast<const char*>(sqlite3_column_text(this->ptr(), col)));
    };

    template<typename T>
    inline T get(const std::string& name) const
    {
        const int index = this->column_index(name);
        return this->get<T>(index);
    }
};

class sqlite
{
  private:
    sqlite_t conn = nullptr;
    stmt_t   stmt = nullptr;

  public:
    sqlite() = default;

    sqlite(const std::string& path)
    {
        sqlite3* conn;
        int      code = sqlite3_open_v2(path.c_str(), &conn, SQLITE_OPEN_READONLY, NULL);
        if (code != SQLITE_OK) {
            throw sqlite_error("sqlite3_open_v2", code);
        }
        this->conn = sqlite_t(conn);
    }

    // move constructor
    sqlite(sqlite&& db)
    {
        this->conn = std::move(db.conn);
        this->stmt = db.stmt;
    }

    // move assignment operator
    sqlite& operator=(sqlite&& db)
    {
        this->conn = std::move(db.conn);
        this->stmt = db.stmt;
        return *this;
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
    sqlite_iter* query(const std::string& statement)
    {
        const auto    cstmt = statement.c_str();
        sqlite3_stmt* stmt;
        const int     code = sqlite3_prepare_v2(this->connection(), cstmt, statement.length() + 1, &stmt, NULL);

        if (code != SQLITE_OK) {
            // something happened, can probably switch on result codes
            // https://www.sqlite.org/rescode.html
            throw sqlite_error("sqlite3_prepare_v2", code);
        }

        this->stmt        = stmt_t(stmt, sqlite_deleter{});
        sqlite_iter* iter = new sqlite_iter(this->stmt);
        return iter;
    }
};