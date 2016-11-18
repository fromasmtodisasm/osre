/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015 OSRE ( Open Source Render Engine ) by Kim Kulling

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-----------------------------------------------------------------------------------------------*/
#include <gtest/gtest.h>
#include <osre/IO/Uri.h>

namespace OSRE {
namespace UnitTest {

using namespace ::OSRE::IO;

class UriTest : public ::testing::Test {
  // empty
};

TEST_F( UriTest, createTest ) {
    bool ok( true );
    try {
        Uri uri1;
        Uri uri2( "file://c:/texture.jpg" );
        Uri uri3( uri1 );
    } catch ( ... ) {
        ok = false;
    }
    EXPECT_TRUE( ok );
}

TEST_F( UriTest, constructFromCompsTest ) {
    Uri uri1;
    const String Scheme   = "file";
    const String path     = "assets/Textures/Fonts/buildin_arial.bmp";
    const String resource = "buildin_arial.bmp";
    String result = uri1.constructFromComps( Scheme, path, resource );
    EXPECT_EQ( "file://assets/Textures/Fonts/buildin_arial.bmp", result );
}

} // Namespace UnitTest
} // Namespace OSRE