#pragma once

#include "sead/basis/seadNew.hpp"
#include "sead/devenv/seadDebugFontMgrNvn.h"
#include "sead/gfx/seadViewport.h"
#include "sead/textwriter.h"

#include "agl/DrawContext.h"

#include "game/System/GameSystem.h"

#include "DeltaTime.hpp"
#include "al/util.hpp"
#include "logger.hpp"

extern sead::TextWriter* gTextWriter;

bool setupDebugMenu(agl::DrawContext* context, sead::Viewport* viewport);
void drawBackground(agl::DrawContext* context);
void setDebugMenuState(bool isInGame, bool isShow);