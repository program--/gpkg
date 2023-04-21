#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <sqlite3.h>

/**
 * @brief Deleter used to provide smart pointer support for sqlite3 structs.
 */
struct sqlite_deleter
{
    void operator()(sqlite3* db) { sqlite3_close_v2(db); }
    void operator()(sqlite3_stmt* stmt) { sqlite3_finalize(stmt); }
};

/**
 * @brief Smart pointer (unique) type for sqlite3 database
 */
using sqlite_t = std::unique_ptr<sqlite3, sqlite_deleter>;

/**
 * @brief Smart pointer (shared) type for sqlite3 prepared statements
 */
using stmt_t = std::shared_ptr<sqlite3_stmt>;

/**
 * @brief Get a runtime error based on a function and code.
 *
 * @param f String denoting the function where the error originated
 * @param code sqlite3 result code
 * @return std::runtime_error
 */
static inline std::runtime_error sqlite_error(const std::string& f, int code)
{
    std::string errmsg = f + " returned code " + std::to_string(code);
    return std::runtime_error(errmsg);
}

/**
 * @brief SQLite3 row iterator
 *
 * Provides a simple iterator-like implementation
 * over rows of a SQLite3 query.
 */
class sqlite_iter
{
  private:
    stmt_t stmt;
    int    iteration_step     = -1;
    bool   iteration_finished = false;

    // column metadata
    int                      column_count;
    std::vector<std::string> column_names;

    // returns the raw pointer to the sqlite statement
    sqlite3_stmt* ptr() const noexcept;

  public:
    sqlite_iter(stmt_t stmt);
    ~sqlite_iter();
    bool                            done() const noexcept;
    sqlite_iter&                    next();
    sqlite_iter&                    reset();
    int                             current_row() const noexcept;
    int                             num_columns() const noexcept;
    int                             column_index(const std::string& name) const noexcept;
    const std::vector<std::string>& columns() const noexcept { return this->column_names; }

    template<typename T>
    T get(int col) const;

    template<typename T>
    T get(const std::string& name) const;
};

inline sqlite_iter::sqlite_iter(stmt_t stmt)
  : stmt(stmt)
{
    this->column_count = sqlite3_column_count(this->ptr());
    this->column_names = std::vector<std::string>();
    this->column_names.reserve(this->column_count);
    for (int i = 0; i < this->column_count; i++) {
        this->column_names.push_back(sqlite3_column_name(this->ptr(), i));
    }
};

inline sqlite_iter::~sqlite_iter()
{
    this->stmt.reset();
}

inline sqlite3_stmt* sqlite_iter::ptr() const noexcept
{
    return this->stmt.get();
}

inline bool sqlite_iter::done() const noexcept
{
    return this->iteration_finished;
}

inline sqlite_iter& sqlite_iter::next()
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

inline sqlite_iter& sqlite_iter::reset()
{
    sqlite3_reset(this->ptr());
    this->iteration_step = -1;
    return *this;
}

inline int sqlite_iter::current_row() const noexcept
{
    return this->iteration_step;
}

inline int sqlite_iter::num_columns() const noexcept
{
    return this->column_count;
}

inline int sqlite_iter::column_index(const std::string& name) const noexcept
{
    const ptrdiff_t pos =
      std::distance(this->column_names.begin(), std::find(this->column_names.begin(), this->column_names.end(), name));

    return pos >= this->column_names.size() ? -1 : pos;
}

template<>
inline std::vector<uint8_t> sqlite_iter::get<std::vector<uint8_t>>(int col) const
{
    return *reinterpret_cast<const std::vector<uint8_t>*>(sqlite3_column_blob(this->ptr(), col));
}

template<>
inline double sqlite_iter::get<double>(int col) const
{
    return sqlite3_column_double(this->ptr(), col);
}

template<>
inline int sqlite_iter::get<int>(int col) const
{
    return sqlite3_column_int(this->ptr(), col);
}

template<>
inline std::string sqlite_iter::get<std::string>(int col) const
{
    // TODO: this won't work with non-ASCII text
    return std::string(reinterpret_cast<const char*>(sqlite3_column_text(this->ptr(), col)));
}

template<typename T>
inline T sqlite_iter::get(const std::string& name) const
{
    const int index = this->column_index(name);
    return this->get<T>(index);
}