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
#include "AbstractRenderTest.h"
#include "RenderTestUtils.h"

#include <osre/RenderBackend/RenderBackendService.h>
#include <osre/Scene/GeometryBuilder.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace OSRE {
namespace RenderTest {

using namespace ::OSRE::RenderBackend;

class DbgFontRenderTest : public AbstractRenderTest {
    TransformMatrixBlock m_transformMatrix;

public:
    DbgFontRenderTest()
    : AbstractRenderTest( "rendertest/dbgfontrendertest" ) {
        // empty
    }

    ~DbgFontRenderTest() {

    }

    virtual bool onCreate( RenderBackendService *rbSrv ) {
		rbSrv->sendEvent( &OnAttachViewEvent, nullptr );
		Scene::GeometryBuilder builder;
		RenderTextEventData *data = new RenderTextEventData;
        data->m_geo = builder.createQuad();

        //data->m_geo = builder.createTextBox( -1, -1, 0.1f, "Hello, World!" );

        m_transformMatrix.m_model = glm::rotate( m_transformMatrix.m_model, 0.0f, glm::vec3( 1, 1, 0 ) );

        Parameter *parameter = Parameter::create( "MVP", PT_Mat4 );
        ::memcpy( parameter->m_data.m_data, glm::value_ptr( m_transformMatrix.m_projection*m_transformMatrix.m_view*m_transformMatrix.m_model ), sizeof( glm::mat4 ) );

        data->m_geo->m_parameter = parameter;
        data->m_geo->m_numParameter++;

		rbSrv->sendEvent( &OnRenderTextEvent, data );
		
		return true;
    }

    virtual bool onDestroy( RenderBackendService *rbSrv ) {
        return true;
    }

    virtual bool onRender( d32 timediff, RenderBackendService *rbSrv ) {
        return true;
    }

};

ATTACH_RENDERTEST( DbgFontRenderTest )

}
}