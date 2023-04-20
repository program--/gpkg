#pragma once

#include <unordered_map>
#include <boost/variant.hpp>

#include "sqlite.hpp"

using field = boost::variant<int, double, bool, std::string>;

class feature
{
  private:
    int                                    id;
    std::vector<uint8_t>                   geometry;
    std::unordered_map<std::string, field> properties;

  public:
    template<typename T>
    const T& get(const std::string& property) noexcept
    {
        return boost::get<T>(this->properties[property]);
    }

    template<typename T>
    void set(const std::string& property, T value) noexcept
    {
        this->properties[property] = value;
    }

    const std::vector<uint8_t>& wkb() const noexcept { return this->geometry; }
};

class gpkg
{
  private:
    const sqlite& db;

  public:
    gpkg(const std::string& path)
      : gpkg(sqlite(path)){};

    gpkg(const sqlite& db)
      : db(db){};

    gpkg(const sqlite&& db)
      : db(db){};

    //
    int num_layers() const noexcept;
    int num_features(const std::string&) const;

    //
    const std::vector<std::string>& layers() const noexcept;
    const std::vector<feature>&     features(const std::string&) const;
};