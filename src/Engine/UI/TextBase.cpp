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
#include <osre/UI/TextBase.h>
#include <osre/Scene/MaterialBuilder.h>
#include <osre/Scene/GeometryBuilder.h>
#include <osre/Common/Tokenizer.h>
//#include <osre/RenderBackend/FontBase.h>
#include <osre/RenderBackend/RenderCommon.h>

#include "UIRenderUtils.h"

namespace OSRE {
namespace UI {

using namespace ::OSRE::Common;
using namespace ::OSRE::RenderBackend;
using namespace ::OSRE::Scene;

TextBase::TextBase( const String &name, Widget *parent )
: Widget( name, parent )
, m_font( nullptr )
, m_text( "" ) {
    if ( nullptr != parent ) {
        setStackIndex( parent->getStackIndex() + 1 );
    }
}
    
TextBase::~TextBase() {
    // empty
}

void TextBase::setLabel( const String &text ) {
    if ( text == m_text ) {
        return;
    }
    
    m_text = text;
    Widget::requestRedraw();
}

const String &TextBase::getLabel() const {
    return m_text;
}

void TextBase::setFont( FontBase *font ) {
    if ( m_font == font ) {
        return;
    }
    
    m_font = font;
    Widget::requestRedraw();
}

RenderBackend::FontBase *TextBase::getFont() const {
    return m_font;
}

static const ui32 NumQuadVert = 4;

void TextBase::onRender( UiRenderCmdCache &renderCmdCache, RenderBackendService *rbSrv ) {
    if ( m_text.empty() ) {
        return;
    }
    f32 fontSize = 0.1f;
    if ( nullptr != m_font ) {
        fontSize = static_cast<f32>( m_font->getSize() );
    }

    const i32 stackId( getStackIndex() );
    const f32 z(-1.0f* static_cast<f32>( stackId ) );
    f32 x( static_cast<f32>( Widget::getRect().getX1() ) );
    f32 y( static_cast<f32>( Widget::getRect().getY1() ) );
    WidgetCoordMapping::mapPosToWorld( static_cast<ui32>(x), static_cast<ui32>(y), x, y );

    UiVertexCache vertexCache;
    UiIndexCache indexCache;
    
    Scene::MeshBuilder::allocUiTextBox(x, y, fontSize, m_text, BufferAccessType::ReadWrite, vertexCache, indexCache);

    UiRenderCmd *cmd( new UiRenderCmd );
    cmd->m_vc = vertexCache;
    cmd->m_ic = indexCache;
    cmd->m_mat = Scene::MaterialBuilder::createBuildinUiMaterial();;
    renderCmdCache.add( cmd );
}

} // Namespace UI
} // Namespace OSRE
