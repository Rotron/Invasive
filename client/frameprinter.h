#pragma once
#include "stdafx.h"
#include "forward.h"

namespace FramePrinter {

QString toPlainText(const FramePtr& frame);
QString toHtmlText(const FramePtr& frame);

}
