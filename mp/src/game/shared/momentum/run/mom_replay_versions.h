#pragma once

#include "mom_replay_base.h"
#include "run_stats.h"

class CMomReplayV1 : public CMomReplayBase
{
public:
    CMomReplayV1();
    CMomReplayV1(CUtlBuffer &reader, bool bFull);
    virtual ~CMomReplayV1() OVERRIDE;

public:
    virtual uint8 GetVersion() OVERRIDE { return 1; }
    virtual CMomRunStats* GetRunStats() OVERRIDE;
    virtual int32 GetFrameCount() OVERRIDE;
    virtual CReplayFrame* GetFrame(int32 index) OVERRIDE;
    virtual void AddFrame(const CReplayFrame& frame) OVERRIDE;
    virtual bool SetFrame(int32 index, const CReplayFrame& frame) OVERRIDE;
    virtual CMomRunStats* CreateRunStats(uint8 stages) OVERRIDE;
    virtual void RemoveFrames(int num) OVERRIDE;

public:
    virtual void Serialize(CUtlBuffer &writer) OVERRIDE;

private:
    void Deserialize(CUtlBuffer &reader, bool bFull = true);

protected:
    CMomRunStats *m_pRunStats;
    CUtlVector<CReplayFrame> m_rgFrames;
};