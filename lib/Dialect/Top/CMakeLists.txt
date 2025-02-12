file(GLOB _sources
          IR/*.cpp
          Interfaces/*.cpp
          Canonicalize/*.cpp
          Transforms/*.cpp
          Lowering/*.cpp
          Lowering/BM1684/*.cpp
          Lowering/BM1684x/*.cpp
    )

add_mlir_library(TPUMLIRTop
  ${_sources}

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/tpu_mlir/Dialect/Top

  DEPENDS
  MLIRTopAttrGen
  MLIRTopOpsIncGen
  MLIRTopPassIncGen
  MLIRInferenceInterfaceIncGen
  MLIRLoweringInterfaceIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRPass
  MLIRSupport
  MLIRParser
  MLIRFuncDialect
  MLIRQuantDialect
  MLIRRewrite
  MLIRTransforms
  MLIRSideEffectInterfaces
  MLIRInferenceInterface
  MLIRLoweringInterface
  TPUMLIRSupport
  TPUMLIRTrait
)
