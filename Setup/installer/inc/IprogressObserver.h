#pragma once

class IProgressObserver
{
public:
    virtual void OnInstallProgress(int) = 0; // 0 - 255:  255 == 100%
};