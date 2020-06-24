/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min_unittest.h"     // required unit test header
#include "ht.between.cpp"    // need the source of our object so that we can access it

// Unit tests are written using the Catch framework as described at
// https://github.com/philsquared/Catch/blob/master/docs/tutorial.md

SCENARIO("object produces correct output") {
    ext_main(nullptr);    // every unit test must call ext_main() once to configure the class

    GIVEN("An instance of our object") {

        test_wrapper<ht_between> an_instance;
        ht_between&              my_object = an_instance;

        // check that default attr values are correct
        REQUIRE((my_object.min == Approx(0.0)));
        REQUIRE((my_object.max == Approx(1.0)));

        // now proceed to testing various sequences of events
        WHEN("a 'number' is received") {
            my_object.number(0.5);
            THEN("our greeting is produced at the outlet") {
                auto& output = *c74::max::object_getoutput(my_object, 0);
                REQUIRE((output.size() == 1));
                REQUIRE((output[0].size() == 1));
                REQUIRE((output[0][0] == 0));
            }
        }
        
        WHEN("a 'list' is received") {
            std::array<number, 3> mes = {5, 10, 20};
            my_object.list(mes);
            THEN("our greeting is produced at the outlet") {
                auto& output = *c74::max::object_getoutput(my_object, 0);
                REQUIRE((output.size() == 1));
                REQUIRE((output[0].size() == 1));
                REQUIRE((output[0][0] == -1));
            }
        }

        WHEN("a 'list' is received again") {
            std::array<number, 3> mes = {30.4, 10.5, 20.1};
            my_object.list(mes);
            THEN("our greeting is produced at the outlet") {
                auto& output = *c74::max::object_getoutput(my_object, 0);
                REQUIRE((output.size() == 1));
                REQUIRE((output[0].size() == 1));
                REQUIRE((output[0][0] == 1));
            }
        }
    }
}
