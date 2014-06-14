#pragma once
#include "stdafx.h"
#include "forward.h"

namespace FramePrinter {

QString asciiInfo(const FramePtr& frame);
QString toPlainText(const FramePtr& frame);
QString toHtmlText(const FramePtr& frame);
QString toJson(const FramePtr& frame);

}
