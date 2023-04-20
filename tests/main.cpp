#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "sqlite.hpp"

#define REQUIRE_NEAR(A, B) REQUIRE_EQ(A, doctest::Approx(B))

SCENARIO("User provides a SQLite Database")
{
    const std::string path = "data/nextgen_09.gpkg";
    sqlite            db;

    GIVEN("a path to the database")
    {
        REQUIRE_NOTHROW(db = sqlite(path));

        THEN("user provides a query")
        {
            const std::string query = "SELECT * FROM gpkg_contents LIMIT 1;";
            sqlite_iter*      iter  = nullptr;
            REQUIRE_NOTHROW(iter = db.query(query));

            THEN("user wants metadata about the table")
            {
                REQUIRE_EQ(iter->num_columns(), 10);
                REQUIRE_EQ(
                  iter->columns(),
                  std::vector<std::string>({ "table_name",
                                             "data_type",
                                             "identifier",
                                             "description",
                                             "last_change",
                                             "min_x",
                                             "min_y",
                                             "max_x",
                                             "max_y",
                                             "srs_id" })
                );
            }

            THEN("user iterates over single row")
            {
                REQUIRE_NOTHROW(iter->next());

                GIVEN("column indices")
                {
                    REQUIRE_EQ(iter->get<std::string>(0), "flowpaths");
                    REQUIRE_EQ(iter->get<std::string>(1), "features");
                    REQUIRE_EQ(iter->get<std::string>(2), "flowpaths");
                    REQUIRE_EQ(iter->get<std::string>(3), "");
                    REQUIRE_EQ(iter->get<std::string>(4), "2022-09-24T07:29:14.150Z");
                    REQUIRE_NEAR((iter->get<double>(5)), -563916.270060378);
                    REQUIRE_NEAR((iter->get<double>(6)), 2503998.31199251);
                    REQUIRE_NEAR((iter->get<double>(7)), 409052.081110541);
                    REQUIRE_NEAR((iter->get<double>(8)), 2929839.25614086);
                    REQUIRE_EQ((iter->get<int>(9)), 5070);
                }

                GIVEN("column names")
                {
                    REQUIRE_EQ(iter->get<std::string>("table_name"), "flowpaths");
                    REQUIRE_EQ(iter->get<std::string>("data_type"), "features");
                    REQUIRE_EQ(iter->get<std::string>("identifier"), "flowpaths");
                    REQUIRE_EQ(iter->get<std::string>("description"), "");
                    REQUIRE_EQ(iter->get<std::string>("last_change"), "2022-09-24T07:29:14.150Z");
                    REQUIRE_NEAR((iter->get<double>("min_x")), -563916.270060378);
                    REQUIRE_NEAR((iter->get<double>("min_y")), 2503998.31199251);
                    REQUIRE_NEAR((iter->get<double>("max_x")), 409052.081110541);
                    REQUIRE_NEAR((iter->get<double>("max_y")), 2929839.25614086);
                    REQUIRE_EQ((iter->get<int>("srs_id")), 5070);
                }

                WHEN("user wants to reiterate row")
                {
                    iter->reset();
                    REQUIRE_EQ(iter->current_row(), -1);
                    REQUIRE_THROWS(iter->get<std::string>(0));

                    iter->next();
                    REQUIRE_EQ(iter->current_row(), 0);
                }

                WHEN("user is done iterating")
                {
                    iter->next();
                    REQUIRE(iter->done());
                    REQUIRE_EQ(iter->current_row(), 1);
                    REQUIRE_THROWS(iter->get<std::string>(0));

                    // if the iteration is done, next()
                    // should behave idempotently
                    REQUIRE_NOTHROW(iter->next());
                    REQUIRE(iter->done());
                    REQUIRE_EQ(iter->current_row(), 1);

                    // checking destructor
                    REQUIRE_NOTHROW(iter->~sqlite_iter());
                }
            }
        }
    }
}