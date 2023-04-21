#pragma once

#define REQUIRE_NEAR(A, B) REQUIRE_EQ(A, doctest::Approx(B))