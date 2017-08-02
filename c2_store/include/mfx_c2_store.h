/********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2011-2017 Intel Corporation. All Rights Reserved.

*********************************************************************************

File: mfx_c2_store.h

Defined functions:

Defined help functions:

*********************************************************************************/
#pragma once

#include <C2Component.h>

#include <map>

using namespace android;

class MfxC2ComponentStore : public C2ComponentStore {
public:
    static MfxC2ComponentStore* Create(status_t* status);

private: // C2ComponentStore overrides
    status_t createComponent(C2String name, std::shared_ptr<C2Component>* const component) override;

    status_t createInterface(C2String name, std::shared_ptr<C2ComponentInterface>* const interface) override;

    std::vector<std::unique_ptr<const C2ComponentInfo>> getComponents() override;

    status_t copyBuffer(std::shared_ptr<C2GraphicBuffer> src, std::shared_ptr<C2GraphicBuffer> dst) override;

    status_t query_nb(
            const std::vector<C2Param * const> &stackParams,
            const std::vector<C2Param::Index> &heapParamIndices,
            std::vector<std::unique_ptr<C2Param>>*const heapParams) override;

    status_t config_nb(
            const std::vector<C2Param * const> &params,
            std::list<std::unique_ptr<C2SettingResult>>*const failures) override;

private: // implementation methods
    status_t readConfigFile();

    void* loadModule(const std::string& name);
private: // data
    struct ComponentDesc {
        std::string dso_name_;
        int flags_;
        ComponentDesc(const char* dso_name, int flags):
            dso_name_(dso_name), flags_(flags) {}
    };
    // this is a map between component names and component descriptions:
    //   (component's flags, dso name, etc.)
    // no mutexed access needed as written only once before any read access
    std::map<std::string, ComponentDesc> components_registry_;
};