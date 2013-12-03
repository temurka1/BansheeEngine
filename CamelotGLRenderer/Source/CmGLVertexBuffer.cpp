/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "CmGLHardwareBufferManager.h"
#include "CmGLVertexBuffer.h"
#include "CmException.h"

namespace CamelotFramework 
{
    GLVertexBuffer::GLVertexBuffer(HardwareBufferManager* mgr, UINT32 vertexSize, 
        UINT32 numVertices, GpuBufferUsage usage)
        : VertexBuffer(mgr, vertexSize, numVertices, usage, false)
    {
    }

    GLVertexBuffer::~GLVertexBuffer()
    {
    }

    void* GLVertexBuffer::lockImpl(UINT32 offset, 
        UINT32 length, GpuLockOptions options)
    {
        GLenum access = 0;

        if(mIsLocked)
        {
			CM_EXCEPT(InternalErrorException,
                "Invalid attempt to lock a vertex buffer that has already been locked");
        }

		glBindBuffer(GL_ARRAY_BUFFER, mBufferId);

		if ((options == GBL_WRITE_ONLY) || (options == GBL_WRITE_ONLY_NO_OVERWRITE) || (options == GBL_WRITE_ONLY_DISCARD))
		{
			access = GL_MAP_WRITE_BIT;

			if(options == GBL_WRITE_ONLY_DISCARD)
				access |= GL_MAP_INVALIDATE_BUFFER_BIT;
			else if(options == GBL_WRITE_ONLY_NO_OVERWRITE)
				access |= GL_MAP_UNSYNCHRONIZED_BIT;
		}
		else if (options == GBL_READ_ONLY)
			access = GL_MAP_READ_BIT;
		else
			access = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;

		void* pBuffer = glMapBufferRange(GL_ARRAY_BUFFER, offset, length, access);

		if(pBuffer == 0)
		{
			CM_EXCEPT(InternalErrorException, "Vertex Buffer: Out of memory");
		}

		void* retPtr = static_cast<void*>(static_cast<unsigned char*>(pBuffer));

		mIsLocked = true;
		return retPtr;
    }

	void GLVertexBuffer::unlockImpl(void)
    {
		glBindBuffer(GL_ARRAY_BUFFER, mBufferId);

		if(!glUnmapBuffer(GL_ARRAY_BUFFER))
		{
			CM_EXCEPT(InternalErrorException, "Buffer data corrupted, please reload");
		}

        mIsLocked = false;
    }

    void GLVertexBuffer::readData(UINT32 offset, UINT32 length, void* pDest)
    {
		void* bufferData = lock(offset, length, GBL_READ_ONLY);
		memcpy(pDest, bufferData, length);
		unlock();
    }

    void GLVertexBuffer::writeData(UINT32 offset, UINT32 length, 
		const void* pSource, BufferWriteType writeFlags)
    {
		GpuLockOptions lockOption = GBL_WRITE_ONLY;
		if(writeFlags == BufferWriteType::Discard)
			lockOption = GBL_WRITE_ONLY_DISCARD;
		else if(writeFlags == BufferWriteType::NoOverwrite)
			lockOption = GBL_WRITE_ONLY_NO_OVERWRITE;

		void* bufferData = lock(offset, length, lockOption);
		memcpy(bufferData, pSource, length);
		unlock();
    }

	void GLVertexBuffer::initialize_internal()
	{
		glGenBuffers(1, &mBufferId);

		if (!mBufferId)
		{
			CM_EXCEPT(InternalErrorException, 
				"Cannot create GL vertex buffer");
		}

		glBindBuffer(GL_ARRAY_BUFFER, mBufferId);

		// Initialise mapped buffer and set usage
		glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, NULL, 
			GLHardwareBufferManager::getGLUsage(mUsage));

		VertexBuffer::initialize_internal();
	}

	void GLVertexBuffer::destroy_internal()
	{
		glDeleteBuffers(1, &mBufferId);

		VertexBuffer::destroy_internal();
	}
}
