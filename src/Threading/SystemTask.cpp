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
#include <osre/Threading/SystemTask.h>
#include <osre/Threading/TAsyncQueue.h>
#include <osre/Threading/TaskJob.h>
#include <osre/Platform/AtomicInt.h>
#include <osre/Common/AbstractEventHandler.h>
#include <osre/Common/Event.h>
#include <cassert>

#ifdef _WIN32
#   include <src/Platform/win32/Win32Thread.h>
#else
#   include <src/Platform/sdl2/SDL2Thread.h>
#endif

#include <sstream>

namespace OSRE {
namespace Threading {

using namespace ::OSRE::Common;

static const String Tag = "SystemTaskThread";

static bool DebugQueueSize = false;
//-------------------------------------------------------------------------------------------------
///	@class		::OSRE::Threading::TaskThread
///	@ingroup	Infrastructure
///
///	@brief
//-------------------------------------------------------------------------------------------------
#ifdef _WIN32
class SystemTaskThread : public Platform::Win32Thread {
#else
class SystemTaskThread : public Platform::SDL2Thread {
#endif

public:
    enum {
        StackSize = 4096
    };

public:
    //---------------------------------------------------------------------------------------------
#ifdef _WIN32
    SystemTaskThread( const String &threadName, TAsyncQueue<const TaskJob*> *jobQueue )
    : Win32Thread( threadName, StackSize )
#else
    SystemTaskThread( const String &threadName, TAsyncQueue<const TaskJob*> *jobQueue )
    : SDL2Thread( threadName, StackSize )
#endif
    , m_updateEvent( nullptr )
    , m_reqStop( nullptr )
    , m_activeJobQueue( jobQueue )
    , m_eventHandler( nullptr ) {
        assert( nullptr != jobQueue );

        Platform::AbstractThreadFactory *pThreadFactory( Platform::AbstractThreadFactory::getInstance() );
        if ( pThreadFactory ) {
            m_updateEvent = pThreadFactory->createThreadEvent();
            m_reqStop = pThreadFactory->createAtomic( 0 );
        } else {
            osre_error( Tag, "Invalid pointer to thread factory." );
        }
    }

    //---------------------------------------------------------------------------------------------
    ~SystemTaskThread()	{
        if ( m_eventHandler ) {
            m_eventHandler->detach( nullptr );
        }

        delete m_updateEvent;
        m_updateEvent = nullptr;
    }

    //---------------------------------------------------------------------------------------------
    void setEventHandler( Common::AbstractEventHandler *eventHandler ) {
        m_eventHandler = eventHandler;
        if ( m_eventHandler ) {
            m_eventHandler->attach( nullptr );
        }
    }
    //---------------------------------------------------------------------------------------------
    virtual bool stop() {
        if (nullptr == m_reqStop) {
#ifdef _WIN32
            return Win32Thread::stop();
#else
            return SDL2Thread::stop();
#endif
        }

        m_reqStop->incValue( 1 );
        m_updateEvent->waitForOne();
        AbstractThread::setState( AbstractThread::Terminated );
        return true;
    }

    //---------------------------------------------------------------------------------------------
    Common::AbstractEventHandler *getEventHandler() const {
        return m_eventHandler;
    }

    //---------------------------------------------------------------------------------------------
    void setActiveJobQueue( Threading::TAsyncQueue<const TaskJob*> *pJobQueue ) {
        m_activeJobQueue = pJobQueue;
    }

    //---------------------------------------------------------------------------------------------
    Threading::TAsyncQueue<const TaskJob*> *getActiveJobQueue() const {
        return m_activeJobQueue;
    }

    //---------------------------------------------------------------------------------------------
    Platform::AbstractThreadEvent *getUpdateEvent() const {
        return m_updateEvent;
    }

protected:
    //---------------------------------------------------------------------------------------------
    i32 run() {
        osre_debug( Tag, "SystemThread::run" );
        bool running = true;
        while ( running ) {
            m_activeJobQueue->awaitEnqueuedItem();
            while ( !m_activeJobQueue->isEmpty() )	{
                // for debugging
				if (DebugQueueSize) {
					ui32 size = m_activeJobQueue->size();
					std::stringstream stream;
					stream << "queue size = " << size << std::endl;
					osre_debug(Tag, stream.str());
				}

                const TaskJob *job = m_activeJobQueue->dequeue();
                const Common::Event *ev = job->getEvent();
                if ( !ev ) {
                    running = false;
                    assert( nullptr != ev );
                }

                if ( m_eventHandler ) {
                    m_eventHandler->onEvent( *ev, job->getEventData() );
                }
            }

			if (m_updateEvent) {
				m_updateEvent->signal();
			}

            i32 v(m_reqStop->getValue());
            if ( v == 1 ) {
                running = false;
            }
        }

        if (m_updateEvent) {
            m_updateEvent->signal();
        }

        return 0;
    }

private:
    Platform::AbstractThreadEvent *m_updateEvent;
    Platform::AbstractAtomic *m_reqStop;
    Threading::TAsyncQueue<const TaskJob*> *m_activeJobQueue;
    Common::AbstractEventHandler *m_eventHandler;
};

//-------------------------------------------------------------------------------------------------
SystemTask::SystemTask( const String &taskName )
: AbstractTask( taskName )
, m_WorkingMode( Async )
, m_buffermode( SingleBuffer )
, m_pTaskThread( nullptr )
, m_asyncQueue( nullptr ) {
    // empty
}

//-------------------------------------------------------------------------------------------------
SystemTask::~SystemTask() {
    assert( !isRunning() );
}

//-------------------------------------------------------------------------------------------------
void SystemTask::setWorkingMode( AbstractTask::WorkingMode mode ) {
    if ( isRunning() ) {
        osre_error( Tag, "The working mode cannot be chanced in a running task." );
        return;
    }

    m_WorkingMode = mode;
}

//-------------------------------------------------------------------------------------------------
AbstractTask::WorkingMode SystemTask::getWorkingMode() const {
    return m_WorkingMode;
}

//-------------------------------------------------------------------------------------------------
void SystemTask::setBufferMode( BufferMode buffermode ) {
    m_buffermode = buffermode;
}

//-------------------------------------------------------------------------------------------------
SystemTask::BufferMode SystemTask::getBufferMode() const {
    return m_buffermode;
}

//-------------------------------------------------------------------------------------------------
bool SystemTask::start( Platform::AbstractThread *pThread ) {
    // ensure task is not running
    if( nullptr != m_pTaskThread ) {
        if ( Platform::AbstractThread::Running == m_pTaskThread->getCurrentState() ) {
            osre_debug( Tag, "Task " + Object::getName() + " is already running." );
            return false;
        }
    }

    // setup the thread context
    m_asyncQueue = new Threading::TAsyncQueue<const TaskJob*>( Platform::AbstractThreadFactory::getInstance() );
    if ( !pThread ) {
        m_pTaskThread = new SystemTaskThread( getName() + ".thread", m_asyncQueue );
    } else {
        m_pTaskThread = reinterpret_cast<SystemTaskThread*>( pThread );
    }

    // start the system task
    return ( m_pTaskThread->start( nullptr ) );
}

//-------------------------------------------------------------------------------------------------
bool SystemTask::stop() {
    if ( Platform::AbstractThread::Running != m_pTaskThread->getCurrentState() ) {
        osre_debug( Tag, "Task " + getName() + " is not running." );
        return false;
    }

    m_pTaskThread->stop();
    delete m_pTaskThread;
    m_pTaskThread = nullptr;

    return true;
}

//-------------------------------------------------------------------------------------------------
bool SystemTask::isRunning() const {
    if( nullptr != m_pTaskThread )
        return ( Platform::AbstractThread::Running == m_pTaskThread->getCurrentState() );

    return false;
}

//-------------------------------------------------------------------------------------------------
bool SystemTask::execute() {
    return true;
}

//-------------------------------------------------------------------------------------------------
void SystemTask::setThreadInstance( Platform::AbstractThread *pThreadInstance ) {
    if( Platform::AbstractThread::Running == m_pTaskThread->getCurrentState() ) {
        m_pTaskThread->stop();
    }

    m_pTaskThread = reinterpret_cast<SystemTaskThread*>( pThreadInstance );
}

//-------------------------------------------------------------------------------------------------
void SystemTask::attachEventHandler( Common::AbstractEventHandler *pEventHandler ) {
    assert( nullptr != m_pTaskThread );

    m_pTaskThread->setEventHandler( pEventHandler );
}

//-------------------------------------------------------------------------------------------------
void SystemTask::detachEventHandler() {
    assert( nullptr != m_pTaskThread );

    Common::AbstractEventHandler *pEH = m_pTaskThread->getEventHandler();
    if( pEH  ) {
        m_pTaskThread->setEventHandler( NULL );
    }
}

//-------------------------------------------------------------------------------------------------
bool SystemTask::sendEvent( const Common::Event *pEvent, const Common::EventData *pEventData ) {
    assert( nullptr != m_asyncQueue );
    assert( nullptr != pEvent );

    TaskJob *pTaskJob = new TaskJob( pEvent, pEventData );
    m_asyncQueue->enqueue( pTaskJob );

    return true;
}

//-------------------------------------------------------------------------------------------------
ui32 SystemTask::getEvetQueueSize() const {
    assert( NULL != m_asyncQueue );

    return m_asyncQueue->size();
}

//-------------------------------------------------------------------------------------------------
void SystemTask::onUpdate() {
    assert( NULL != m_pTaskThread );

    if ( !m_pTaskThread )
        return;

    if ( !m_asyncQueue ) {
        m_asyncQueue = m_pTaskThread->getActiveJobQueue();
    }
}

//-------------------------------------------------------------------------------------------------
void SystemTask::await() {
    if ( m_pTaskThread ) {
        Platform::AbstractThreadEvent *threadEvent = m_pTaskThread->getUpdateEvent();
        if ( threadEvent ) {
            threadEvent->waitForOne();
        }
    }
}

//-------------------------------------------------------------------------------------------------
SystemTask *SystemTask::create( const String &rTaskName ) {
    return new SystemTask( rTaskName );
}

//-------------------------------------------------------------------------------------------------

} // Namespace Threading
} // Namespace OSRE
