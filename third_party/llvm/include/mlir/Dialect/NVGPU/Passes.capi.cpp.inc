/* Autogenerated by mlir-tblgen; don't manually edit. */
//===----------------------------------------------------------------------===//
// NVGPU Group Registration
//===----------------------------------------------------------------------===//

void mlirRegisterNVGPUPasses() {
  registerNVGPUPasses();
}

MlirPass mlirCreateNVGPUOptimizeSharedMemory() {
  return wrap(mlir::nvgpu::createOptimizeSharedMemoryPass().release());
}
void mlirRegisterNVGPUOptimizeSharedMemory() {
  registerOptimizeSharedMemory();
}
