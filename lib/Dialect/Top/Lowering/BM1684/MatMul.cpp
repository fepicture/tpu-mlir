//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// TPU-MLIR is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "../Lowering.h"
#include "tpu_mlir/Dialect/Top/IR/TopOps.h"
#include "tpu_mlir/Dialect/Tpu/IR/TpuOps.h"
#include "tpu_mlir/Support/MathUtils.h"
#include "tpu_mlir/Support/Helper/Quant.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"

using namespace mlir;
using namespace tpu_mlir;
using namespace tpu_mlir::helper;

Value top::MatMulOp::lowering_int8_bm1684() {
  // refer quantize_convlike_layer_int8
  auto op = getOperation();
  OpBuilder builder(op);
  std::vector<Value> operands;
  std::vector<NamedAttribute> attrs;
  int64_t batch, M, K, N;
  bool with_bias, relu;
  double relu_limit;
  parseParam(batch, M, K, N, with_bias, relu, relu_limit);
  assert(batch == 1); // only for fullyconnected now
  auto th_output = Quant::getThreshold(output());
  auto th_input = Quant::getThreshold(input());
  auto filterOp = cast<top::WeightOp>(right().getDefiningOp());
  auto filter_f32 = filterOp.read<float>();
  double filter_max = findMaxabs(filter_f32->data(), filter_f32->size());
  int rshift =
      calRightShiftNum(filter_max, th_input, th_output, Quant::BITS_INT8);
  rshift = rshift >= 0 ? rshift : 0;
  std::shared_ptr<std::vector<int16_t>> bias_int16;
  if (with_bias) {
    float bias_scale = 1.0 * (1 << rshift) * Quant::QMAX_INT8 / th_output;
    auto biasOp = cast<top::WeightOp>(bias().getDefiningOp());
    auto bias_f32 = biasOp.read<float>();
    bias_int16 = std::make_shared<std::vector<int16_t>>(bias_f32->size());
    float overflow_ratio = quantizeToInt16(bias_f32->data(), bias_int16->data(),
                                           bias_f32->size(), bias_scale);

    while (overflow_ratio > 0.03 && rshift > 0) {
      rshift--;
      bias_scale = 1.0 * (1 << rshift) * Quant::QMAX_INT8 / th_output;
      overflow_ratio = quantizeToInt16(bias_f32->data(), bias_int16->data(),
                                       bias_f32->size(), bias_scale);
    }
  }
  attrs.push_back(
      builder.getNamedAttr("rshift", builder.getI64IntegerAttr(rshift)));
  float scale = 1.0 * (1 << rshift) * th_input / th_output;
  auto filter_int8 = std::make_shared<std::vector<int8_t>>(filter_f32->size());
  quantizeToInt8(filter_f32->data(), filter_int8->data(), filter_f32->size(),
                 scale);
  auto filter_type = right().getType().cast<RankedTensorType>();
  auto new_type =
      RankedTensorType::get(filter_type.getShape(), builder.getI8Type());
  auto new_filter = WeightOp::create(op, "filter_int8", *filter_int8, new_type);
  operands.push_back(input());
  operands.push_back(new_filter);
  auto new_bias = bias();
  if (with_bias) {
    auto bias_type = bias().getType().cast<RankedTensorType>();
    auto new_type =
        RankedTensorType::get(bias_type.getShape(), builder.getIntegerType(16));
    new_bias = WeightOp::create(op, "bias_int16", *bias_int16, new_type);
  }
  operands.push_back(new_bias);
  for (auto &attr : op->getAttrs()) {
    attrs.push_back(attr);
  }
  auto newType = Quant::getQuantInt8Type(output());
  auto newOp =
      builder.create<tpu::MatMulOp>(op->getLoc(), newType, operands, attrs);
  return newOp.output();
}

Value top::MatMulOp::lowering_f32_bm1684() {
  return lowering_common_float<tpu::MatMulOp>(getOperation());
}
