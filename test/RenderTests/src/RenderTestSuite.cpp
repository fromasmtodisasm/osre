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
#include "RenderTestSuite.h"
#include "AbstractRenderTest.h"

#include <osre/Properties/ConfigurationMap.h>
#include <osre/Threading/TaskScheduler.h>
#include <osre/RenderBackend/RenderBackendService.h>
#include <osre/Platform/PlatformInterface.h>
#include <osre/Platform/AbstractPlatformEventHandler.h>
#include <osre/Platform/AbstractTimer.h>

#include <iostream>
#include <cassert>

namespace OSRE {
namespace RenderTest {

using namespace OSRE::Common;
using namespace OSRE::RenderBackend;
using namespace OSRE::Platform;

using namespace CPPCore;

RenderTestSuite *RenderTestSuite::s_pInstance = nullptr;

static const ui32 AllTestsDone = 999999;

//-------------------------------------------------------------------------------------------------
class KeyboardEventListener : public Platform::OSEventListener {
public:
    KeyboardEventListener( RenderTestSuite *pRenderTestSuite )
    : OSEventListener( "rendertest/keyboardeventlistener" )
    , m_pTestSuite( pRenderTestSuite ) {
        assert( nullptr != pRenderTestSuite );
    }

    ~KeyboardEventListener() {
        // empty
    }

    void onOSEvent( const Event &osEvent, const EventData *pData ) {
         if( osEvent == KeyboardButtonDownEvent ) {
            if( pData ) {
                bool result( false );
                ui32 next( 0 );
                Key key = reinterpret_cast< const KeyboardButtonEventData* >( pData )->m_Key;
                if( key == KEY_SPACE ) {
                    result = m_pTestSuite->requestNextTest( next );
                    if ( !result ) {
                        osre_log( "All tests done." );
                    }
                }
            }
        }
    }

private:
    RenderTestSuite *m_pTestSuite;
};

//-------------------------------------------------------------------------------------------------
RenderTestSuite *RenderTestSuite::create(const String &suiteName, const String &renderAPI ) {
    if (!s_pInstance) {
        s_pInstance = new RenderTestSuite(suiteName, renderAPI);
        s_pInstance->setup();
    }

    return s_pInstance;
}

//-------------------------------------------------------------------------------------------------
RenderTestSuite *RenderTestSuite::getInstance() {
    if (!s_pInstance) {
        static_cast<void>( RenderTestSuite::create("tests", "opengl") );
    }

    return s_pInstance;
}

//-------------------------------------------------------------------------------------------------
bool RenderTestSuite::setup() {
    // get configuration parameter
    Properties::ConfigurationMap *pConfig = new Properties::ConfigurationMap;
    pConfig->setString(Properties::ConfigurationMap::RenderAPI, "opengl");
    pConfig->setBool( Properties::ConfigurationMap::PollingMode, true );

#ifdef CE_WINDOWS
    //pConfig->setInt( Properties::ConfigurationMap::PlatformPlugin, static_cast<i32>( Platform::SDL2Plugin) );
    pConfig->setInt( Properties::ConfigurationMap::PlatformPlugin, static_cast< i32 >( Platform::WindowsPlugin ) );
#else
    pConfig->setInt( Properties::ConfigurationMap::PlatformPlugin, static_cast<i32>( Platform::SDL2Plugin) );
#endif

    // create the platform abstraction
    m_pPlatformInterface = Platform::PlatformInterface::create( pConfig );
    if( m_pPlatformInterface ) {
        if( !m_pPlatformInterface->open() ) {
            return false;
        }
    }

    // create the task manager
    Threading::TaskScheduler::setInstance(Threading::TaskScheduler::create());

    m_pRenderBackendServer = new RenderBackendService();
    if ( !m_pRenderBackendServer->open()) {
        m_pRenderBackendServer->release();
        m_pRenderBackendServer = nullptr;
    }

    if( m_pPlatformInterface ) {
        CreateRendererEventData *pData = new CreateRendererEventData( m_pPlatformInterface->getRootSurface() );
        m_pRenderBackendServer->sendEvent( &OnCreateRendererEvent, pData );
    }

    m_pTimer = PlatformInterface::getInstance()->getTimer();
    
    return true;
}

//-------------------------------------------------------------------------------------------------
bool RenderTestSuite::teardown() {
    if( m_pRenderBackendServer ) {
        m_pRenderBackendServer->close();
        m_pRenderBackendServer->release();
        m_pRenderBackendServer = nullptr;
    }

    if ( m_pTimer ) {
        m_pTimer->release();
        m_pTimer = nullptr;
    }
    
    if( m_pPlatformInterface ) {
        PlatformInterface::destroy();
        m_pPlatformInterface = nullptr;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
void RenderTestSuite::kill() {
    assert(NULL != s_pInstance);
    if ( s_pInstance ) {
        s_pInstance->teardown();
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

//-------------------------------------------------------------------------------------------------
void RenderTestSuite::attachRenderTest( AbstractRenderTest *pRenderTest ) {
    assert( nullptr != pRenderTest );

    m_attachedRenderTests.add( pRenderTest );
}

//-------------------------------------------------------------------------------------------------
ui32 RenderTestSuite::getNumRenderTests() const {
    return m_attachedRenderTests.size();
}

//-------------------------------------------------------------------------------------------------
void RenderTestSuite::startTests() {
    if (m_attachedRenderTests.isEmpty()) {
        return;
    }
}

//-------------------------------------------------------------------------------------------------
void RenderTestSuite::showTestReport() {
    if (m_FailureLog.isEmpty()) {
        return;
    }

    std::cout << "Errors:" << std::endl << "=======" << std::endl;
    for ( TArray<String>::Iterator it = m_FailureLog.begin(); it != m_FailureLog.end(); ++it) {
        String msg(*it);
        std::cout << msg << std::endl;
    }
}

//-------------------------------------------------------------------------------------------------
void RenderTestSuite::setRenderAPI(const String &renderAPI) {
    m_renderAPI = renderAPI;
}

//-------------------------------------------------------------------------------------------------
const String &RenderTestSuite::getRenderAPI() const {
    return m_renderAPI;
}

//-------------------------------------------------------------------------------------------------
void RenderTestSuite::setMediaPath( const String &mediaPath ) {
    m_mediaPath = mediaPath;
}

//-------------------------------------------------------------------------------------------------
const String &RenderTestSuite::getMediaPath() const {
    return m_mediaPath;
}

//-------------------------------------------------------------------------------------------------
AbstractTimer *RenderTestSuite::getTimer() const {
    return m_pTimer;
}

//-------------------------------------------------------------------------------------------------
bool RenderTestSuite::update( d32 timediff ) {
    if( !m_pPlatformInterface ) {
        return false;
    }

    TArray<const Common::Event*> eventArray;
    eventArray.add( &KeyboardButtonDownEvent );    
    m_pListener = new KeyboardEventListener( this );
    AbstractPlatformEventHandler *pEvHandler = m_pPlatformInterface->getPlatformEventHandler( );
    pEvHandler->registerEventListener( eventArray, m_pListener );

    while( m_pPlatformInterface->update( timediff ) ) {
        if( m_activeTestIdx == AllTestsDone ) {
            break;
        }

        if( m_pActiveRenderTest != m_attachedRenderTests[ m_activeTestIdx ] ) {
            if( m_pActiveRenderTest ) {
                m_pActiveRenderTest->destroy( m_pRenderBackendServer );
                clearTestEnv();
            }
            
            m_pActiveRenderTest = m_attachedRenderTests[ m_activeTestIdx ];
            if( m_pActiveRenderTest ) {
                m_pActiveRenderTest->create( m_pRenderBackendServer );
            }
        }
        
        if( m_pActiveRenderTest ) {
            m_pActiveRenderTest->setup( m_pRenderBackendServer );
            if( !m_pActiveRenderTest->render( timediff, m_pRenderBackendServer ) ) {
                addFailureLog( "Error : Cannot render test " + m_pActiveRenderTest->getTestName( ) + "\n" );
            }
            m_pRenderBackendServer->update( timediff );

            m_pActiveRenderTest->teardown( m_pRenderBackendServer );
        }
    }
    if( m_pActiveRenderTest ) {
        m_pActiveRenderTest->destroy( m_pRenderBackendServer );
        clearTestEnv();
    }

    pEvHandler->unregisterEventListener( eventArray, m_pListener );

    return false;
}

//-------------------------------------------------------------------------------------------------
bool RenderTestSuite::clearTestEnv() {
    m_pRenderBackendServer->sendEvent( &OnClearSceneEvent, nullptr );

    return true;
}

//-------------------------------------------------------------------------------------------------
bool RenderTestSuite::requestNextTest( ui32 &next ) {
    ++m_activeTestIdx;
    if( m_activeTestIdx >= m_attachedRenderTests.size() ) {
        m_activeTestIdx = AllTestsDone;
        next = AllTestsDone;
        return false;
    }
    next = m_activeTestIdx;

    return true;
}

//-------------------------------------------------------------------------------------------------
void RenderTestSuite::addFailureLog( const String &logEntry ) {
    if (!logEntry.empty()) {
        m_FailureLog.add(logEntry);
    }
}

//-------------------------------------------------------------------------------------------------
RenderTestSuite::RenderTestSuite( const String &suiteName, const String &renderAPI )
: AbstractTestFixture( suiteName )
, m_pActiveRenderTest( nullptr )
, m_activeTestIdx( 0 )
, m_attachedRenderTests()
, m_FailureLog()
, m_pPlatformInterface( nullptr )
, m_pListener( nullptr )
, m_pTimer( nullptr )
, m_pRenderBackendServer( nullptr )
, m_renderAPI(renderAPI)
, m_mediaPath() {
    assert( !suiteName.empty() );
}

//-------------------------------------------------------------------------------------------------
RenderTestSuite::~RenderTestSuite() {
    delete m_pListener;
    m_pListener = nullptr;

    for( ui32 i = 0; i < m_attachedRenderTests.size(); i++ ) {
        delete m_attachedRenderTests[ i ];
    }
}

//-------------------------------------------------------------------------------------------------

} // Namespace RenderTest
} // Namespace OSRE
 