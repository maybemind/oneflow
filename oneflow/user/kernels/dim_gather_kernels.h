/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "oneflow/core/framework/framework.h"
#include "oneflow/core/common/balanced_splitter.h"
#include "oneflow/user/kernels/dim_gather_kernel_util.h"
#include <stdio.h>

namespace oneflow {

namespace user_op {
template<DeviceType device_type, typename IN_T, typename IDX_T>
struct GatherDimFunctor final {
  void operator()(NdIndexArg<IDX_T> inputArg, NdIndexArg<IDX_T> indexArg, int64_t elem_cnt,
                  int64_t dim, const IDX_T* index, const IN_T* input, IN_T* output, DeviceCtx* ctx);
};

template<DeviceType device_type, typename IN_T, typename IDX_T>
class GatherDimKernel final : public user_op::OpKernel {
 public:
  GatherDimKernel() = default;
  ~GatherDimKernel() override = default;

 private:
  void Compute(KernelComputeContext* ctx) const override {
    const Tensor* input_tensor = ctx->Tensor4ArgNameAndIndex("input", 0);
    const Tensor* index_tensor = ctx->Tensor4ArgNameAndIndex("index", 0);
    Tensor* out_tensor = ctx->Tensor4ArgNameAndIndex("output", 0);
    const int64_t dim = ctx->Attr<int64_t>("dim");

    if (index_tensor->shape().elem_cnt() == 0) { return; }

    const IN_T* input = input_tensor->dptr<IN_T>();
    const IDX_T* index = index_tensor->dptr<IDX_T>();
    IN_T* output = out_tensor->mut_dptr<IN_T>();

    NdIndexArg<IDX_T> inputArg(input_tensor->shape());
    NdIndexArg<IDX_T> indexArg(index_tensor->shape());
    GatherDimFunctor<device_type, IN_T, IDX_T>()(inputArg, indexArg,
                                                 index_tensor->shape().elem_cnt(), dim, index,
                                                 input, output, ctx->device_ctx());
  }
  bool AlwaysComputeWhenAllOutputsEmpty() const override { return false; }
};

template<DeviceType device_type, typename IN_T, typename IDX_T>
struct ScatterDimAddFunctor final {
  void operator()(NdIndexArg<IDX_T> srcArg, NdIndexArg<IDX_T> outputArg, int64_t elem_cnt,
                  int64_t dim, const IDX_T* index, const IN_T* src, IN_T* output, DeviceCtx* ctx);
};

template<DeviceType device_type, typename IN_T, typename IDX_T>
class ScatterDimKernel final : public user_op::OpKernel {
 public:
  ScatterDimKernel() = default;
  ~ScatterDimKernel() override = default;

 private:
  void Compute(KernelComputeContext* ctx) const override {
    const Tensor* src_tensor = ctx->Tensor4ArgNameAndIndex("src", 0);
    const Tensor* index_tensor = ctx->Tensor4ArgNameAndIndex("index", 0);
    Tensor* out_tensor = ctx->Tensor4ArgNameAndIndex("output", 0);
    const int64_t dim = ctx->Attr<int64_t>("dim");

    if (index_tensor->shape().elem_cnt() == 0) { return; }

    const IN_T* src = src_tensor->dptr<IN_T>();
    const IDX_T* index = index_tensor->dptr<IDX_T>();
    IN_T* output = out_tensor->mut_dptr<IN_T>();
    size_t out_bytes_size =
        out_tensor->shape().elem_cnt() * GetSizeOfDataType(out_tensor->data_type());
    Memset<device_type>(ctx->device_ctx(), output, 0, out_bytes_size);

    NdIndexArg<IDX_T> srcArg(src_tensor->shape());
    NdIndexArg<IDX_T> outputArg(out_tensor->shape());

    ScatterDimAddFunctor<device_type, IN_T, IDX_T>()(srcArg, outputArg,
                                                     src_tensor->shape().elem_cnt(), dim, index,
                                                     src, output, ctx->device_ctx());
  }
  bool AlwaysComputeWhenAllOutputsEmpty() const override { return false; }
};

}  // namespace user_op
}  // namespace oneflow