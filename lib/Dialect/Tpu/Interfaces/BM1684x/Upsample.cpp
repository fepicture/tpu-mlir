//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// TPU-MLIR is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "tpu_mlir/Dialect/Tpu/IR/TpuOps.h"
#include "tpu_mlir/Backend/BM168x/BM1684x.h"
#include "tpu_mlir/Support/Helper/Quant.h"
#include "tpu_mlir/Support/Helper/Module.h"

using namespace mlir;
using namespace tpu_mlir;
using namespace tpu_mlir::helper;
using namespace tpu_mlir::backend;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned long long input_addr;
  unsigned long long output_addr;
  int input_n;
  int input_c;
  int input_h;
  int input_w;
  int size;
  int if_relu;
  DATA_TYPE_T dtype;
} upsample_spec_t;

#ifdef __cplusplus
}
#endif

// =========================================
// GlobalGenInterface
// =========================================
void tpu::UpsampleOp::codegen_global_bm1684x() {
  assert(scale_h() == scale_w());
  auto op = getOperation();
  int64_t n, c, h, w;
  Module::getNCHW(input(), n, c, h, w);

  upsample_spec_t spec = {0};
  spec.input_addr = Module::getAddress(input());
  spec.output_addr = Module::getAddress(output());
  spec.input_n = n;
  spec.input_c = c;
  spec.input_h = h;
  spec.input_w = w;
  spec.size = scale_h();
  spec.if_relu = do_relu();
  spec.dtype = BM168x::getDataType(output());
  BM1684x::instance().call_global_func("backend_api_upsample_global", &spec,
                                       sizeof(spec));
}

// =========================================
// LocalGenInterface
// =========================================

int64_t tpu::UpsampleOp::getBufferSize_bm1684x(
    int64_t in_lmem_bytes, int64_t out_lmem_bytes, int64_t in_nslice,
    int64_t in_hslice, int64_t out_nslice, int64_t out_hslice) {
  return 0;
}

void tpu::UpsampleOp::codegen_local_bm1684x(int64_t n_step, int64_t h_step) {
  assert(scale_h() == scale_w());
  auto op = getOperation();
  int64_t n, c, h, w;
  Module::getNCHW(input(), n, c, h, w);
  auto in_gi = LocalGenInterface::getGroupInfo(input(), n_step, h_step);
  auto gi = getGroupInfo(n_step, h_step);
  upsample_spec_t spec = {0};
  spec.input_addr = in_gi.out_addr;
  spec.output_addr = gi.out_addr;
  spec.size = scale_h();
  spec.if_relu = do_relu();
  spec.dtype = BM168x::getDataType(input());
  spec.input_n = in_gi.n_slice;
  spec.input_c = c;
  spec.input_h = in_gi.h_slice;
  spec.input_w = w;
  BM1684x::instance().call_local_func("backend_api_upsample_local", &spec,
                                      sizeof(spec));
}
