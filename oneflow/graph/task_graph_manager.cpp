#include "graph/task_graph_manager.h"

namespace oneflow {

void TaskGraphMgr::Init() {
  ordered_task_gphs_.clear();
  // data graph
  LOG(INFO) << "Build DataTaskGraph";
  auto data_task_gph = new DataTaskGraph(
        JobDesc::Singleton().train_dlnet_conf(),
        JobDesc::Singleton().strategy(),
        true);
  ordered_task_gphs_.emplace_back(data_task_gph);
  // construct data_chain2sorted_bp_comp_tasks
  HashMap<const ChainNode*, std::vector<CompTaskNode*>>
      data_chain2sorted_bp_comp_tasks;
  for (const auto& node : data_task_gph->nodes()) {
    auto bp_node = dynamic_cast<CompTaskNode*>(node.get());
    if (bp_node == nullptr || bp_node->IsFwNode()) { continue; }
    data_chain2sorted_bp_comp_tasks[bp_node->chain_node()].push_back(bp_node);
  }
  for (auto& pair : data_chain2sorted_bp_comp_tasks) {
    SortByParallelId(&(pair.second));
  }
  // model graph
  for (const auto& pair : data_chain2sorted_bp_comp_tasks) {
    const std::string dot_path_prefix = LogDir() + "/" + NewUniqueId() + "_";
    ParallelPolicy policy = pair.first->parallel_desc()->policy();
    // model update
    auto updt_gph = new MdUpdtTaskGraph(
        pair.first, pair.second, dot_path_prefix + "model_update_");
    ChainNode* updt_chain = updt_gph->chain_gph()->SoleSinkNode();
    auto sorted_updt_tasks = updt_gph->SortedCompTasksInChain(updt_chain);
    HashMap<uint64_t, CompTaskNode*> parallel_id2updt_task;
    for (CompTaskNode* update_task : sorted_updt_tasks) {
      CHECK(parallel_id2updt_task.emplace(
            update_task->parallel_id(), update_task).second);
    }
    // model load save
    auto load_gph = new MdLoadTaskGraph(
        updt_chain, parallel_id2updt_task, policy,
        dot_path_prefix + "model_load_");
    auto save_gph = new MdSaveTaskGraph(
        updt_chain, parallel_id2updt_task, policy,
        dot_path_prefix + "model_save_");
    ordered_task_gphs_.emplace_back(updt_gph);
    ordered_task_gphs_.emplace_back(load_gph);
    ordered_task_gphs_.emplace_back(save_gph);
  }
}

void TaskGraphMgr::InferShape4Regsts() {
  for (auto& task_gph : ordered_task_gphs_) {
    task_gph->InferShapeOfBlobsInProducedRegsts();
  }
}

} // namespace oneflow
