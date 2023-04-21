#include <doctest/doctest.h>

#include "utils.hpp"
#include "gpkg.hpp"

SCENARIO("User is accessing a a GeoPackage file")
{
    const std::string path = "data/nextgen_09.gpkg";
    gpkg              db;
    GIVEN("the path to the file")
    {
        REQUIRE_NOTHROW(db = gpkg(path));

        THEN("expected metadata is accessible")
        {
            REQUIRE(db.num_layers() == 8);
            REQUIRE(
              db.layers() == std::vector<std::string>{ "flowpaths",
                                                       "divides",
                                                       "nexus",
                                                       "flowpath_attributes",
                                                       "flowpath_edge_list",
                                                       "crosswalk",
                                                       "cfe_noahowp_attributes",
                                                       "forcing_metadata" }
            );

            REQUIRE(db.num_features("flowpaths") == 10709);
        }
    }
}