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
#include <pybind11/pybind11.h>
#include "oneflow/api/python/of_api_registry.h"
#include "oneflow/api/python/op/op_mgr_api.h"
#include "oneflow/core/framework/user_op_attr.cfg.h"

namespace py = pybind11;

ONEFLOW_API_PYBIND11_MODULE("", m) {
  m.def("IsOpTypeCaseCpuSupportOnly", &IsOpTypeCaseCpuSupportOnly);
  m.def("IsOpTypeNameCpuSupportOnly", &IsOpTypeNameCpuSupportOnly);
  m.def("GetUserOpAttrType", [](const std::string& op_type_name, const std::string& attr_name) {
    return static_cast<oneflow::cfg::AttrType>(GetUserOpAttrType(op_type_name, attr_name));
  });

  m.def("InferOpConf", &InferOpConf);
  m.def("GetSerializedOpAttributes", &GetSerializedOpAttributes);
  m.def("IsInterfaceOpTypeCase", &IsInterfaceOpTypeCase);

  m.def("GetOpParallelSymbolId", &GetOpParallelSymbolId);
  m.def("CheckAndCompleteUserOpConf", &CheckAndCompleteUserOpConf);
}
