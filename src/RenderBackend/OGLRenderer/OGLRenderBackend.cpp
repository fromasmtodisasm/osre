/* ZFX Community Engine 2  (ZFXCE2)
---------------------------------------------------------------------------------------------------
Copyright (c) 2011-2015, ZFXCE2 Development Team
All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions
and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this list of conditions
and the following disclaimer in the documentation and/or other materials provided with the
distribution.

* Neither the name of the ZFXCE2 team, nor the names of its contributors may be used to endorse or
promote products derived from this software without specific prior written permission of the
ZFXCE2 Development Team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-------------------------------------------------------------------------------------------------*/
#include "OGLRenderBackend.h"
#include "OGLShader.h"
#include "OGLCommon.h"
#include "OGLEnum.h"

#include <osre/Platform/AbstractRenderContext.h>
#include <osre/Common/Logger.h>

#include <cppcore/CPPCoreCommon.h>

#include "SOIL.h"

#include <iostream>
#include <cassert>

namespace OSRE {
namespace RenderBackend {

using namespace ::CPPCore;

static const String VertexAttribName      = "vVertex";
static const String NormalAttribName      = "vNormal";
static const String DiffuseColorAttribute = "vDiffuseColor";
static const String TexCoord0Attribute    = "TexCoord0Attribute";

//-------------------------------------------------------------------------------------------------
static bool setParameterInShader( OGLParameter *param, OGLShader *shader ) {
    assert( nullptr != param );
    assert( nullptr != shader );

    if( NoneLocation == param->m_loc ) {
        param->m_loc = ( *shader )( param->m_name );
        if( NoneLocation == param->m_loc ) {
            osre_debug( "Cannot location for parameter " + param->m_name + " in shader " + shader->getName() + "." );
            return false;
        }
    }

    bool success( true );
    switch( param->m_type ) {
        case PT_Int: {
                GLint data;
                ::memcpy( &data, param->m_data->getData(), sizeof( GLint ) );
                glUniform1i( param->m_loc, data );
            }
            break;
        case PT_Float: {
                GLfloat value;
                ::memcpy( &value, param->m_data->getData(), sizeof( GLfloat ) );
                glUniform1f( param->m_loc, value );
            }
            break;
        case PT_Float2: {
                GLfloat value[ 2 ];
                ::memcpy( &value[ 0 ], param->m_data->getData(), sizeof( GLfloat ) * 2 );
                glUniform2f( param->m_loc, value[ 0 ], value[ 1 ] );
            }
            break;
        case PT_Float3: {
                GLfloat value[ 3 ];
                ::memcpy( &value[ 0 ], param->m_data->getData(), sizeof( GLfloat ) * 3 );
                glUniform3f( param->m_loc, value[ 0 ], value[ 1 ], value[ 2 ] );
            }
            break;
        case PT_Mat4: {
                glm::mat4 mat;
                ::memcpy( &mat, param->m_data->getData(), sizeof( glm::mat4 ) );
                glUniformMatrix4fv( param->m_loc, 1, GL_FALSE, glm::value_ptr( mat ) );
            }
            break;
        case PT_Mat4Array: {
                glUniformMatrix4fv( param->m_loc, param->m_numItems, GL_FALSE, ( f32* ) param->m_data->getData() );
            }
            break;

        default:
            success = false;
            break;
    }
    
    return success;
}

//-------------------------------------------------------------------------------------------------
GLenum getGLTextureStage( TextureStageType texType ) {
    switch( texType ) {
        case TextureStage0:
            return GL_TEXTURE0;
        case TextureStage1:
            return GL_TEXTURE1;
        case TextureStage2:
            return GL_TEXTURE2;
        case TextureStage3:
            return GL_TEXTURE3;
        default:
            break;
    }
    return GL_TEXTURE0;
}

//-------------------------------------------------------------------------------------------------
OGLRenderBackend::OGLRenderBackend( )
: m_pRenderCtx( nullptr )
, m_buffers()
, m_vertexarrays()
, m_shaders()
, m_textures()
, m_freeTexSlots()
, m_texLookupMap()
, m_parameters()
, m_pShaderInUse( nullptr )
, m_freeBufferSlots()
, m_primitives() {
    // empty
}

//-------------------------------------------------------------------------------------------------
OGLRenderBackend::~OGLRenderBackend( ) {
    releaseAllShaders();
    releaseAllTextures();
    releaseAllVertexArrays( );
    releaseAllBuffers();
    releaseAllParameters();
    releaseAllPrimitiveGroups();
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::setRenderContext( Platform::AbstractRenderContext *renderCtx ) {
    if ( m_pRenderCtx != renderCtx ) {
        m_pRenderCtx = renderCtx;
        if ( m_pRenderCtx ) {
            m_pRenderCtx->activate();
        }
    }
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::clearRenderTarget( ui32 targets ) {
    GLbitfield glTarget( 0 );

    if( targets & ColorBit ) {
        glTarget |= GL_COLOR_BUFFER_BIT;
    }
    if( targets & DepthBit ) {
        glTarget |= GL_DEPTH_BUFFER_BIT;
    }
    if( targets & StencilBit ) {
        glTarget |= GL_STENCIL_BUFFER_BIT;
    }

    glClear( glTarget );
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::setViewport( i32 x, i32 y, i32 w, i32 h ) {
    glViewport( x, y, w, h );
}

//-------------------------------------------------------------------------------------------------
OGLBuffer *OGLRenderBackend::createBuffer( BufferType type ) {
    ui32 handle( OGLNotSetId );
    GLuint bufferId( OGLNotSetId );
    glGenBuffers( 1, &bufferId );
    OGLBuffer *pBuf( nullptr );
    if ( m_freeBufferSlots.isEmpty() ) {
        pBuf   = new OGLBuffer;
        handle = m_buffers.size();
        m_buffers.add( pBuf );
    } else {
        handle = m_freeBufferSlots.back();
        pBuf   = m_buffers[ handle ];
        m_freeBufferSlots.removeBack();
    }
    pBuf->m_handle = handle;
    pBuf->m_type = type;
    pBuf->m_id   = bufferId;
    pBuf->m_size = 0;

    return pBuf;
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::bindBuffer( OGLBuffer *buffer ) {
    assert( nullptr != buffer );

    GLenum target = OGLEnum::getGLBufferType( buffer->m_type );
    glBindBuffer( target, buffer->m_id );
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::bindBuffer( ui32 handle ) {
    assert( handle <= m_buffers.size() );

    OGLBuffer *buf( m_buffers[ handle ] );
    if( nullptr != buf ) {
        bindBuffer( buf );
    }
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::unbindBuffer( OGLBuffer *buffer ) {
    assert( nullptr != buffer );

    GLenum target = OGLEnum::getGLBufferType( buffer->m_type );
    glBindBuffer( target, 0 );
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::bufferData( OGLBuffer *pBuffer, void *pData, ui32 size, BufferAccessType usage ) {
    assert( nullptr != pData );

    GLenum target = OGLEnum::getGLBufferType( pBuffer->m_type );
    glBufferData( target, size, pData, OGLEnum::getGLBufferAccessType( usage ) );
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::releaseBuffer( OGLBuffer *pBuffer ) {
    assert( pBuffer->m_handle < m_buffers.size() );

    const ui32 slot = pBuffer->m_handle;
    glDeleteBuffers( 1, &pBuffer->m_id );
    pBuffer->m_handle = OGLNotSetId;
    pBuffer->m_type = EmptyBuffer;
    pBuffer->m_id   = OGLNotSetId;
    m_freeBufferSlots.add( slot );
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::releaseAllBuffers() {
    for ( ui32 i=0; i<m_buffers.size(); ++i ) {
        OGLBuffer *pBuffer = m_buffers[ i ];
        if ( pBuffer ) {
            if ( pBuffer->m_type != EmptyBuffer ) {
                releaseBuffer( pBuffer );
            }
        }
    }
    m_buffers.clear();
    m_freeBufferSlots.clear();
}

//-------------------------------------------------------------------------------------------------
bool OGLRenderBackend::createVertexAttribArray( VertexType type, OGLShader *pShader, 
                                                TArray<OGLVertexAttribute*> &attributes ) {
    if( !pShader ) {
        return false;
    }

    OGLVertexAttribute *pAttibute( nullptr );
    switch( type ) {
        case ColorVertex:
            pAttibute = new OGLVertexAttribute;
            pAttibute->m_pAttributeName = VertexAttribName.c_str();
            pAttibute->m_index = ( ( *pShader )[ VertexAttribName ] );
            pAttibute->m_size = 3;
            pAttibute->m_type = GL_FLOAT;
            pAttibute->m_ptr = 0;
            attributes.add( pAttibute );

            pAttibute = new OGLVertexAttribute;
            pAttibute->m_pAttributeName = DiffuseColorAttribute.c_str( );
            pAttibute->m_index = ( *pShader )[ DiffuseColorAttribute ];
            pAttibute->m_size = 3;
            pAttibute->m_type = GL_FLOAT;
            pAttibute->m_ptr = ( const GLvoid* ) offsetof( ColorVert, color );
            attributes.add( pAttibute );
            break;

        case RenderVertex:
            pAttibute = new OGLVertexAttribute;
            pAttibute->m_pAttributeName = VertexAttribName.c_str();
            pAttibute->m_index = ( ( *pShader )[ VertexAttribName ] );
            pAttibute->m_size = 3;
            pAttibute->m_type = GL_FLOAT;
            pAttibute->m_ptr = 0;
            attributes.add( pAttibute );

            pAttibute = new OGLVertexAttribute;
            pAttibute->m_pAttributeName = TexCoord0Attribute.c_str( );
            pAttibute->m_index = ( *pShader )[ TexCoord0Attribute ];
            pAttibute->m_size = 2;
            pAttibute->m_type = GL_FLOAT;
            pAttibute->m_ptr = ( const GLvoid* ) offsetof( RenderVert, tex0 );
            attributes.add( pAttibute );
            break;

        default:
            break;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::releasevertexAttribArray( TArray<OGLVertexAttribute*> &attributes ) {
    if( attributes.isEmpty() ) {
        return;
    }

    for( ui32 i = 0; i < attributes.size(); ++i ) {
        if( attributes[ i ] ) {
            delete attributes[ i ];
        }
    }
    attributes.clear();
}

//-------------------------------------------------------------------------------------------------
OGLVertexArray *OGLRenderBackend::createVertexArray() {
    OGLVertexArray *pVertexArray = new OGLVertexArray;
    glGenVertexArrays( 1, &pVertexArray->m_id );
    ui32 id = m_vertexarrays.size();
    m_vertexarrays.add( pVertexArray );
    pVertexArray->m_slot = id;

    return pVertexArray;
}

//-------------------------------------------------------------------------------------------------
bool OGLRenderBackend::bindVertexAttribute( OGLVertexArray *va, OGLShader *shader, ui32 stride, GLint loc, OGLVertexAttribute* attrib ) {
    if( nullptr == va || nullptr == shader || nullptr == attrib ) {
        return false;
    }

    glEnableVertexAttribArray( loc );
    glVertexAttribPointer( loc, attrib->m_size,
        attrib->m_type,
        GL_FALSE,
        stride,
        attrib->m_ptr );

    return true;
}

//-------------------------------------------------------------------------------------------------
bool OGLRenderBackend::bindVertexAttributes( OGLVertexArray *va, OGLShader *shader, ui32 stride, 
                                             const TArray<OGLVertexAttribute*> &attributes ) {
    if( nullptr == va || nullptr == shader ) {
        return false;
    }

    for( ui32 i = 0; i < attributes.size(); ++i ) {
        const c8 *pAttribName = attributes[ i ]->m_pAttributeName;
        assert( nullptr != pAttribName );
        const GLint loc = ( *shader )[ pAttribName ];
        if( -1 != loc ) {
            glEnableVertexAttribArray( loc );
            glVertexAttribPointer( loc, attributes[ i ]->m_size,
                                   attributes[ i ]->m_type,
                                   GL_FALSE,
                                   stride,
                                   attributes[ i ]->m_ptr );
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::destroyVertexArray( OGLVertexArray *pVertexArray ) {
    glDeleteVertexArrays( 1, &pVertexArray->m_id );
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::bindVertexArray( OGLVertexArray *pVertexArray ) {
    glBindVertexArray( pVertexArray->m_id );
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::unbindVertexArray( OGLVertexArray *pVertexArray ) {
    glBindVertexArray( 0 );
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::releaseAllVertexArrays( ) {
    for ( ui32 i=0; i<m_vertexarrays.size(); ++i ) {
        destroyVertexArray( m_vertexarrays[ i ] );
    }
    m_vertexarrays.clear();
}

//-------------------------------------------------------------------------------------------------
OGLShader *OGLRenderBackend::createShader( const String &name, Shader *pShaderInfo ) {
    if( name.empty() ) {
        return nullptr;
    }

    OGLShader *pOGLShader = new OGLShader( name );
    m_shaders.add( pOGLShader );

    if( pShaderInfo ) {
        bool result( false );
        if( !pShaderInfo->m_src[ SH_VertexShaderType ].empty() ) {
            result = pOGLShader->loadFromSource( SH_VertexShaderType, pShaderInfo->m_src[ SH_VertexShaderType ] );
            if( !result ) {
                osre_error( "Error while compiling VertexShader." );
            }
        }

        if( !pShaderInfo->m_src[ SH_FragmentShaderType ].empty( ) ) {
            result = pOGLShader->loadFromSource( SH_FragmentShaderType, pShaderInfo->m_src[ SH_FragmentShaderType ] );
            if( !result ) {
                osre_error( "Error while compiling FragmentShader." );
            }
        }

        if( !pShaderInfo->m_src[ SH_GeometryShaderType ].empty( ) ) {
            result = pOGLShader->loadFromSource( SH_GeometryShaderType, pShaderInfo->m_src[ SH_GeometryShaderType ] );
            if( !result ) {
                osre_error( "Error while compiling GeometryShader." );
            }
        }

        result = pOGLShader->createAndLink();
        if( !result ) {
            osre_error( "Error while linking shader" );
        }
    }

    return pOGLShader;
}

//-------------------------------------------------------------------------------------------------
OGLShader *OGLRenderBackend::getShader( const String &name ) {
    if( name.empty() ) {
        return nullptr;
    }

    for( ui32 i = 0; i < m_shaders.size(); ++i ) {
        if( m_shaders[ i ]->getName() == name ) {
            return m_shaders[ i ];
        }
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------
bool OGLRenderBackend::useShader( OGLShader *pShader ) {
    if( !pShader ) {
        if( m_pShaderInUse ) {
            m_pShaderInUse->unuse();
            m_pShaderInUse = nullptr;
            return true;
        }
        return false;
    }

    if( m_pShaderInUse == pShader ) {
        return true;
    }
    if( m_pShaderInUse ) {
        m_pShaderInUse->unuse();
    }
    m_pShaderInUse = pShader;
    m_pShaderInUse->use();

    return true;
}

//-------------------------------------------------------------------------------------------------
OGLShader *OGLRenderBackend::getActiveShader() const {
    return m_pShaderInUse;
}

//-------------------------------------------------------------------------------------------------
bool OGLRenderBackend::releaseShader( OGLShader *pShader ) {
    if( !pShader ) {
        return false;
    }

    // look for the shader
    ui32 idx( 0 );
    bool found( false  );
    for( ui32 i = 0; i < m_shaders.size(); ++i ) {
        if( m_shaders[ i ] == pShader ) {
            found = true;
            idx = i;
            break;
        }
    }
    
    // remove shader from list
    if( found ) {
        delete m_shaders[ idx ];
        m_shaders.remove( idx );
    }

    return found;
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::releaseAllShaders( ) {
    for( ui32 i = 0; i < m_shaders.size(); ++i ) {
        if( m_shaders[ i ] ) {
            delete m_shaders[ i ];
        }
    }
    m_shaders.clear();
}

//-------------------------------------------------------------------------------------------------
OGLTexture *OGLRenderBackend::createEmptyTexture( const String &name, TextureTargetType target,
                                                  ui32 width, ui32 height, ui32 channels ) {
    if( name.empty() ) {
        osre_debug( "Texture anme is empty." );
        return nullptr;
    }

    // lookup for texture
    OGLTexture *pTexture( findTexture( name ) );
    if( pTexture ) {
        return pTexture;
    }

    // get texture slot
    ui32 slot( 0 );
    if( m_freeTexSlots.isEmpty() ) {
        slot = m_textures.size();
        pTexture = new OGLTexture;
        m_textures.add( pTexture );
    } else {
        slot = m_freeTexSlots.back();
        m_freeTexSlots.removeBack();
        pTexture = m_textures[ slot ];
    }
    pTexture->m_slot = slot;
    m_texLookupMap[ name ] = slot;

    GLuint textureId;
    glGenTextures( 1, &textureId );
    pTexture->m_textureId = textureId;
    pTexture->m_name      = name;
    pTexture->m_width     = static_cast< ui32 >( width );
    pTexture->m_height    = static_cast< ui32 >( height );
    pTexture->m_channels  = static_cast< ui32 >( channels );
    pTexture->m_format    = GL_RGB;

    glActiveTexture( GL_TEXTURE0 );
    pTexture->m_target = OGLEnum::getGLTextureTarget( Texture2D );
    glBindTexture( pTexture->m_target, textureId );
    
    glTexParameteri( pTexture->m_target, OGLEnum::getGLTextureParameterName( TextureParamMinFilter ), GL_LINEAR );
    glTexParameteri( pTexture->m_target, OGLEnum::getGLTextureParameterName( TextureParamMagFilter ), GL_LINEAR );
    glTexParameteri( pTexture->m_target, OGLEnum::getGLTextureParameterName( TextureParamWrapS ), GL_CLAMP );
    glTexParameteri( pTexture->m_target, OGLEnum::getGLTextureParameterName( TextureParamWrapT ), GL_CLAMP );

    return pTexture;
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::updateTexture( OGLTexture *pOGLTextue, ui32 offsetX, ui32 offsetY, c8 *data,
                                      ui32 size ) {
    if( !pOGLTextue ) {
        osre_error( "Pointer to texture is a nullptr." );
        return;
    }

    const ui32 diffX( pOGLTextue->m_width - offsetX );
    const ui32 diffY( pOGLTextue->m_height - offsetY );
    const ui32 subSize( diffX * diffY * pOGLTextue->m_channels );
    assert( size < subSize );
    glTexSubImage2D( pOGLTextue->m_target, 0, offsetX, offsetY, pOGLTextue->m_width,
                     pOGLTextue->m_height, pOGLTextue->m_format, GL_UNSIGNED_BYTE, data );
}

//-------------------------------------------------------------------------------------------------
OGLTexture *OGLRenderBackend::createTextureFromFile( const String &name, const String &filename ) {
    OGLTexture *pTexture( findTexture( name ) );
    if( pTexture ) {
        return pTexture;
    }

    // import the texture
    i32 width( 0 ), height( 0 ), channels( 0 );
    GLubyte *pData = SOIL_load_image( filename.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO );
    if( !pData ) {
        osre_debug( "Cannot load texture " + filename );
        return nullptr;
    }

    // swap the texture data
    // todo: must be done by my codec
    for( i32 j = 0; j * 2 < height; ++j ) {
        i32 index1 = j * width * channels;
        i32 index2 = ( height - 1 - j ) * width * channels;
        for( i32 i = width*channels; i > 0; --i ) {
            GLubyte temp = pData[ index1 ];
            pData[index1] = pData[ index2 ];
            pData[ index2 ] = temp;
            ++index1;
            ++index2;
        }
    }

    // create texture and fill it
    pTexture = createEmptyTexture( name, Texture2D, width, height, channels );    
    glTexImage2D( pTexture->m_target, 0, GL_RGB, width, height, 0, pTexture->m_format, GL_UNSIGNED_BYTE, pData );

    SOIL_free_image_data( pData );

    return pTexture;
}

//-------------------------------------------------------------------------------------------------
OGLTexture *OGLRenderBackend::findTexture( const String &name ) const {
    if( name.empty() ) {
        return nullptr;
    }

    std::map<String, ui32>::const_iterator it( m_texLookupMap.find( name ) );
    if( it == m_texLookupMap.end() ) {
        return nullptr;
    }

    return m_textures[ it->second ];
}

//-------------------------------------------------------------------------------------------------
bool OGLRenderBackend::bindTexture( OGLTexture *pOGLTexture, TextureStageType stageType ) {
    if( !pOGLTexture ) {
        return false;
    }

    GLenum glStageType = getGLTextureStage( stageType );
    glActiveTexture( glStageType );
    glBindTexture( pOGLTexture->m_target, pOGLTexture->m_textureId );

    return true;
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::releaseTexture( OGLTexture *pTexture ) {
    if( m_textures[ pTexture->m_slot ] ) {
        glDeleteTextures( 1, &pTexture->m_textureId );
        pTexture->m_textureId = OGLNotSetId;
        pTexture->m_width = 0;
        pTexture->m_height = 0;
        pTexture->m_channels = 0;

        m_freeTexSlots.add( pTexture->m_slot );

        std::map<String, ui32>::iterator it( m_texLookupMap.find( pTexture->m_name ) );
        if( m_texLookupMap.end() != it ) {
            it = m_texLookupMap.erase( it );
        }
        pTexture->m_slot = 0;
    }
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::releaseAllTextures( ) {
    for( ui32 i = 0; i < m_textures.size(); ++i ) {
        if( m_textures[ i ]->m_textureId != OGLNotSetId ) {
            releaseTexture( m_textures[ i ] );
        }
    }
    m_textures.clear();
    m_texLookupMap.clear();
}

//-------------------------------------------------------------------------------------------------
OGLParameter *OGLRenderBackend::createParameter( const String &name, ParameterType type, 
                                                 ParamDataBlob *blob, ui32 numItems ) {
    OGLParameter *param = new OGLParameter;
    param->m_name       = name;
    param->m_type       = type;
    param->m_loc        = NoneLocation;
    param->m_numItems   = numItems;
    param->m_data       = ParamDataBlob::create( type, param->m_numItems );
    if( blob ) {
        ::memcpy( param->m_data->getData(), blob->getData(), blob->m_size );
    }
    m_parameters.add( param );

    return param;
}

//-------------------------------------------------------------------------------------------------
OGLParameter *OGLRenderBackend::getParameter( const String &name ) const {
    if( name.empty() ) {
        return nullptr;
    }

    for( ui32 i = 0; i < m_parameters.size(); ++i ) {
        if( m_parameters[ i ]->m_name == name ) {
            return m_parameters[ i ];
        }
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::setParameter( OGLParameter *param ) {
    if( !m_pShaderInUse ) {
        return;
    }

    setParameterInShader( param, m_pShaderInUse );
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::releaseAllParameters() {
    ContainerClear( m_parameters );
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::setParameter( OGLParameter **param, ui32 numParam ) {
    if( !m_pShaderInUse ) {
        return;
    }

    for( ui32 i = 0; i < numParam; ++i ) {
        OGLParameter *currentParam = param[ i ];
        if( currentParam ) {
            const bool success = setParameterInShader( currentParam ,m_pShaderInUse );
//            osre_validate( success, "Error setting parameter " + currentParam->m_name );
        }
    }
}

//-------------------------------------------------------------------------------------------------
ui32 OGLRenderBackend::addPrimitiveGroup( PrimitiveGroup *grp ) {
    OGLPrimGroup *oglGrp    = new OGLPrimGroup;
    oglGrp->m_primitive     = OGLEnum::getGLPrimitiveType( grp->m_primitive );
    oglGrp->m_indexType     = OGLEnum::getGLIndexType( grp->m_indexType );
    oglGrp->m_startIndex    = grp->m_startIndex;
    oglGrp->m_numPrimitives = grp->m_numPrimitives;
    
    const ui32 idx( m_primitives.size() );
    m_primitives.add( oglGrp );

    return idx;
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::releaseAllPrimitiveGroups() {
    ContainerClear( m_primitives );
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::render( ui32 primpGrpIdx ) {
    OGLPrimGroup *grp( m_primitives[ primpGrpIdx ] );
    if( grp ) {
        glDrawElements( grp->m_primitive, 
                        grp->m_numPrimitives, 
                        grp->m_indexType, 
                        ( const GLvoid* ) grp->m_startIndex );
    }
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::render( ui32 primpGrpIdx, ui32 numInstances ) {
    OGLPrimGroup *grp( m_primitives[ primpGrpIdx ] );
    if( grp ) {
        glDrawArraysInstanced( grp->m_primitive, grp->m_startIndex, grp->m_numPrimitives, numInstances );
    }
}

//-------------------------------------------------------------------------------------------------
void OGLRenderBackend::renderFrame() {
    assert( nullptr != m_pRenderCtx );    
    
    m_pRenderCtx->update();
}

//-------------------------------------------------------------------------------------------------
ui32 OGLRenderBackend::getVertexSize( VertexType vertextype ) {
    ui32 vertexSize( 0 );
    switch( vertextype ) {
        case ColorVertex:
            vertexSize = sizeof( ColorVert );
            break;

        case RenderVertex:
            vertexSize = sizeof( RenderVert );
            break;

        default:
            break;
    }

    return vertexSize;
}

//-------------------------------------------------------------------------------------------------

} // Namespace RenderBackend
} // Namespace OSRE
