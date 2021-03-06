// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "platform.hpp"
#include "cache.hpp"
#include <string>
#include <optional>
#include <mutex>

using ImmCtx = D3D12TranslationLayer::ImmediateContext;

class Task;

using Submission = std::vector<::ref_ptr_int<Task>>;

class Device : public CLChildBase<Device, Platform, cl_device_id>
{
public:
    Device(Platform& parent, IDXCoreAdapter* pAdapter);
    ~Device();

    cl_bool IsAvailable() const noexcept;
    cl_ulong GetGlobalMemSize();
    DXCoreHardwareID const& GetHardwareIds() const noexcept;
    bool IsMCDM() const noexcept;
    bool IsUMA();
    bool SupportsInt16();
    ShaderCache& GetShaderCache();

    std::string GetDeviceName() const;

    void InitD3D();
    void ReleaseD3D();
    ID3D12Device* GetDevice() const { return m_spDevice.Get(); }
    ImmCtx& ImmCtx() { return m_ImmCtx.value(); }
    UINT64 GetTimestampFrequency() const { return m_TimestampFrequency; }
    INT64 GPUToQPCTimestampOffset() const { return m_GPUToQPCTimestampOffset; }

    void SubmitTask(Task*, TaskPoolLock const&);
    void ReadyTask(Task*, TaskPoolLock const&);
    void Flush(TaskPoolLock const&);

    std::unique_ptr<D3D12TranslationLayer::PipelineState> CreatePSO(D3D12TranslationLayer::COMPUTE_PIPELINE_STATE_DESC const& Desc);

protected:
    void ExecuteTasks(Submission& tasks);
    void CacheCaps(std::lock_guard<std::mutex> const&);

    ComPtr<IDXCoreAdapter> m_spAdapter;
    ComPtr<ID3D12Device> m_spDevice;
    DXCoreHardwareID m_HWIDs;

    // Lazy-initialized
    std::mutex m_InitLock;
    bool m_CapsValid = false;
    D3D12_FEATURE_DATA_D3D12_OPTIONS m_D3D12Options = {};
    D3D12_FEATURE_DATA_D3D12_OPTIONS4 m_D3D12Options4 = {};
    D3D12_FEATURE_DATA_ARCHITECTURE m_Architecture = {};
    D3D12TranslationLayer::TranslationLayerCallbacks m_Callbacks;
    std::optional<::ImmCtx> m_ImmCtx;
    unsigned m_ContextCount = 0;

    std::unique_ptr<Submission> m_RecordingSubmission;

    BackgroundTaskScheduler::Scheduler m_CompletionScheduler;
    std::optional<ShaderCache> m_ShaderCache;

    // All PSO creations need to be kicked off behind this lock,
    // which guards the root signature cache in the immediate context
    std::mutex m_PSOCreateLock;

    UINT64 m_TimestampFrequency = 0;
    INT64 m_GPUToQPCTimestampOffset = 0;
};
