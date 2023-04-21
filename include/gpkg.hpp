#pragma once

#include <unordered_map>
#include <boost/variant.hpp>

#include "sqlite.hpp"
#include "gpkg/feature.hpp"

class gpkg
{
  private:
    std::unique_ptr<sqlite>  db = nullptr;

    std::vector<std::string> layer_names;

  public:
    gpkg() = default;
    gpkg(const std::string& path)
    {
        sqlite s     = sqlite(path);
        this->db     = std::make_unique<sqlite>(s);
        const auto q = this->db->query("SELECT table_name FROM gpkg_contents ORDER BY data_type DESC;");

        q->next();
        while (!q->done()) {
            this->layer_names.push_back(q->get<std::string>(0));
            q->next();
        }
    };

    gpkg(gpkg&& g)
    {
        this->db          = std::move(g.db);
        this->layer_names = std::move(g.layer_names);
    }

    gpkg& operator=(gpkg&& g)
    {
        this->db          = std::move(g.db);
        this->layer_names = std::move(g.layer_names);
        return *this;
    }

    //
    int num_layers() const noexcept;
    int num_features(const std::string&) const;

    //
    const std::vector<std::string>& layers() const noexcept;
    const std::vector<feature>&     features(const std::string&) const;
};

inline int gpkg::num_layers() const noexcept
{
    return this->layer_names.size();
}

inline const std::vector<std::string>& gpkg::layers() const noexcept
{
    return this->layer_names;
}

inline int gpkg::num_features(const std::string& layer) const
{
    if (this->db->has_table(layer)) {
        const auto q = this->db->query("SELECT COUNT(*) FROM " + layer + ";");
        q->next();
        return q->get<int>(0);
    } else {
        return -1;
    }
}