/********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2017 Intel Corporation. All Rights Reserved.

*********************************************************************************/

#include "mfx_gralloc_allocator.h"

#include <ufo/graphics.h>

#include "mfx_debug.h"
#include "mfx_c2_debug.h"
#include "mfx_c2_utils.h"

using namespace android;

status_t MfxGrallocAllocator::Create(std::unique_ptr<MfxGrallocAllocator>* allocator)
{
    status_t res = OK;
    if (allocator) {
        std::unique_ptr<MfxGrallocAllocator> alloc(new (std::nothrow)MfxGrallocAllocator());
        if (alloc) {
            res = alloc->Init();
            if (res == OK) *allocator = std::move(alloc);
        } else {
            res = NO_MEMORY;
        }
    } else {
        res = UNEXPECTED_NULL;
    }
    return res;
}

status_t MfxGrallocAllocator::Init()
{
    status_t res = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &m_module);

    if (OK == res) {
        m_grallocModule = (gralloc_module_t *)m_module;
        res = gralloc_open(m_module, &m_allocDev);
    }
    return res;
}

MfxGrallocAllocator::~MfxGrallocAllocator()
{
    gralloc_close(m_allocDev);
}

status_t MfxGrallocAllocator::Alloc(const uint16_t width, const uint16_t height, buffer_handle_t* handle)
{
    MFX_DEBUG_TRACE_FUNC;
    status_t res = OK;

    MFX_DEBUG_TRACE_I32(width);
    MFX_DEBUG_TRACE_I32(height);

    int stride;
    res = m_allocDev->alloc(m_allocDev,
                        width, height,
                        HAL_PIXEL_FORMAT_NV12_TILED_INTEL,
                        GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_HW_TEXTURE,
                        (buffer_handle_t *)handle, &stride);

    MFX_DEBUG_TRACE_P(*handle);
    MFX_DEBUG_TRACE__android_status_t(res);
    return res;
}

status_t MfxGrallocAllocator::Free(const buffer_handle_t handle)
{
    MFX_DEBUG_TRACE_FUNC;
    status_t res = OK;

    MFX_DEBUG_TRACE_P(handle);

    if (handle) {
        res = m_allocDev->free(m_allocDev, handle);
    }

    MFX_DEBUG_TRACE__android_status_t(res);
    return res;
}

status_t MfxGrallocAllocator::LockFrame(buffer_handle_t handle, uint8_t** data, C2PlaneLayout *layout)
{
    MFX_DEBUG_TRACE_FUNC;
    MFX_DEBUG_TRACE_P(handle);

    status_t res = OK;
    intel_ufo_buffer_details_t info;
    MFX_ZERO_MEMORY(info);

    if (!layout) res = UNEXPECTED_NULL;

    if (OK == res) {
        *reinterpret_cast<uint32_t*>(&info) = sizeof(info);

        int err = m_grallocModule->perform(m_grallocModule, INTEL_UFO_GRALLOC_MODULE_PERFORM_GET_BO_INFO, handle, &info);
        if (0 != err) {
            MFX_DEBUG_TRACE_MSG("Failed to get BO_INFO");
            res = UNKNOWN_ERROR;
        }
    }

    mfxU8 *img = NULL;
    if (OK == res) {
        res = m_grallocModule->lock(m_grallocModule, handle, GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_SW_WRITE_MASK,
                                                        0, 0, info.width, info.height, (void**)&img);
    }

    if (OK == res) {
        InitNV12PlaneLayout(info.pitch, info.allocHeight, layout);
        *data = img;
    }

    MFX_DEBUG_TRACE__android_status_t(res);
    return res;
}

status_t MfxGrallocAllocator::UnlockFrame(buffer_handle_t handle)
{
    MFX_DEBUG_TRACE_FUNC;
    MFX_DEBUG_TRACE_P(handle);
    status_t res = OK;

    res = m_grallocModule->unlock(m_grallocModule, handle);

    MFX_DEBUG_TRACE__android_status_t(res);
    return res;
}