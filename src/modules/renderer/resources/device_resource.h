//
// Created by lepag on 6/10/2025.
//

#pragma once


class RenderingDevice;

class IDeviceResource {
public:
    explicit IDeviceResource(RenderingDevice& device): m_device(device) {}
    virtual ~IDeviceResource() = default;


    virtual bool init() = 0;
    virtual void cleanup() = 0;
protected:
    RenderingDevice& m_device;
};
