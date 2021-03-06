/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015-2018 OSRE ( Open Source Render Engine ) by Kim Kulling

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
#include "osre_testcommon.h"

#include <osre/Common/Ids.h>
#include <osre/Scene/Node.h>
#include <osre/Scene/Component.h>

namespace OSRE {
namespace UnitTest {
        
using namespace ::OSRE::Scene;

class ComponentTest : public ::testing::Test {
    // empty
};

class MockComponent : public Component {
public:
    MockComponent( Node *node, ui32 id ) 
    : Component( node, id ) {
        // empty
    }

    ~MockComponent() {
        // empty
    }

    void update( Time ) {
        // empty
    }

    void draw( RenderBackend::RenderBackendService* ) override {
        // empty
    }
};

TEST_F( ComponentTest, createTest ) {
    bool ok( true );
    try {
        MockComponent myComp( nullptr, 0 );
    } catch ( ... ) {
        ok = false;
    }
    EXPECT_TRUE( ok );
}

TEST_F(ComponentTest, accessNodeTest) {
    String name = "test";
    Common::Ids *ids = new Common::Ids;

    Node *n(new Node(name, *ids, Node::RenderCompRequest::RenderCompRequested, Node::TransformCompRequest::TransformCompRequested, nullptr));
    MockComponent myComp( n, 0);

    EXPECT_EQ(n, myComp.getOwnerNode());

}

TEST_F( ComponentTest, accessIdTest ) {
    String name = "test";
    Common::Ids *ids = new Common::Ids;
    Node *n(new Node(name, *ids, Node::RenderCompRequest::RenderCompRequested, Node::TransformCompRequest::TransformCompRequested, nullptr));

    Component *tc = n->getComponent(Node::ComponentType::TransformComponentType);
    EXPECT_NE(nullptr, tc);
    const ui32 id1 = tc->getId();

    Component *rc = n->getComponent(Node::ComponentType::RenderComponentType);
    EXPECT_NE(nullptr, rc);
    const ui32 id2 = rc->getId();

    EXPECT_NE(id1, id2);
}

} // Namespace UnitTest
} // Namespace OSRE
