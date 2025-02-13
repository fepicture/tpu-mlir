//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// TPU-MLIR is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "../Lowering.h"

using namespace tpu_mlir;
using namespace tpu_mlir::helper;
using namespace mlir;

Value top::CastOp::lowering_int8_bm1684x(bool asymmetric) {
  return lowering_common<tpu::CastOp>(getOperation(), output().getType());
}

Value top::CastOp::lowering_f32_bm1684x() { return lowering_quant_bm1684x(); }

Value top::CastOp::lowering_bf16_bm1684x() { return lowering_quant_bm1684x(); }

Value top::CastOp::lowering_f16_bm1684x() { return lowering_quant_bm1684x(); }

Value top::CastOp::lowering_quant_bm1684x() {
  return lowering_int8_bm1684x(true);
}
