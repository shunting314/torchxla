#include "torch_xla/csrc/ops/arg_max.h"

#include "torch/csrc/lazy/core/ir.h"
#include "torch_xla/csrc/lowering_context.h"
#include "torch_xla/csrc/ops/infer_output_shape.h"
#include "torch_xla/csrc/reduction.h"

namespace torch_xla {
namespace ir {
namespace ops {
namespace {

xla::Shape NodeOutputShape(const Value& input, int64_t dim, bool keepdim) {
  auto lower_for_shape_fn =
      [&](absl::Span<const xla::XlaOp> operands) -> xla::XlaOp {
    return BuildArgMax(operands[0], dim, keepdim);
  };
  return InferOutputShape({input.shape()}, lower_for_shape_fn);
}

}  // namespace

ArgMax::ArgMax(const Value& input, int64_t dim, bool keepdim)
    : Node(torch::lazy::OpKind(at::aten::argmax), {input},
           [&]() { return NodeOutputShape(input, dim, keepdim); },
           /*num_outputs=*/1, torch::lazy::MHash(dim, keepdim)),
      dim_(dim),
      keepdim_(keepdim) {}

NodePtr ArgMax::Clone(OpList operands) const {
  return MakeNode<ArgMax>(operands.at(0), dim_, keepdim_);
}

XlaOpVector ArgMax::Lower(LoweringContext* loctx) const {
  xla::XlaOp input = loctx->GetOutputOp(operand(0));
  return ReturnOp(BuildArgMax(input, dim_, keepdim_), loctx);
}

std::string ArgMax::ToString() const {
  std::stringstream ss;
  ss << Node::ToString() << ", dim=" << dim_ << ", keepdim=" << keepdim_;
  return ss.str();
}

}  // namespace ops
}  // namespace ir
}  // namespace torch_xla
