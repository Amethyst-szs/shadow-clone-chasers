#pragma once

#include "al/LiveActor/LiveActorGroup.h"
#include "container/seadPtrArray.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "game/StageScene/StageScene.h"
#include "math/seadVector.h"
#include "prim/seadSafeString.h"

class PuppetFrame {
public:
    // Transform
    sead::Vector3f mPosition;
    sead::Quatf mQuat;

    // Model states
    bool mIs2D;

    // Animation
    sead::FixedSafeString<0x30> mCurAnimStr;
    sead::FixedSafeString<0x30> mCurSubAnimStr;
    float mBlendWeights[6] = {};
};

class PuppetDirector {
public:
    // Puppet count info
    const int mWorldPuppetCountList[17] = {
        12, // Cap
        10, // Waterfall
        6, // Sand
        3, // Forest
        9, // Lake
        16, // Cloud
        7, // Clash
        5, // City
        7, // Sea
        6, // Snow
        6, // Lava
        16, // BossRaid
        8, // Sky
        10, // Moon
        10, // Peach
        15, // Special1
        12 // Special2
    };
    const int mSubAreaPuppetCount = 17;

    // State
    bool mIsEnabled = true;
    bool mIsRunning = false;
    bool mIsInstantKill = false;
    bool mIsPushPlayer = false;

    bool mRequestDamage = false;
    int mRequestDamageFrames = 0;
    bool mRequestKill = false;

    // Frame data
    sead::PtrArray<PuppetFrame> mFrames;
    int mPuppetCount = -1;
    int mPuppetSeperator = 60;
    int mPuppetStartOffset = 20;
    const int mPeakPuppetCount = 17;
    const int mPuppetAdditionalMemory = 300;
    int mPuppetFrames; // Calculated with count and seperator during initSequence
    int mPuppetQueueOffset = 0; // When player bound, add to value, decrease when not bound

    // Puppet Actors
    al::LiveActorGroup* mActorGroup;

    // Functions
    void initSequence();
    void initScene(const al::ActorInitInfo& initInfo, bool isMainStage, int worldID);
    void initFirstStep(sead::Vector3f* spawnLocation);
    void killScene();

    void updatePuppets(StageScene* scene, PlayerActorHakoniwa* p1, bool isBind);
    void calculateDamageData(StageScene* scene, PlayerActorHakoniwa* p1);
    void calculateBindData(bool isBind);
};

PuppetDirector& getPuppetDirector();